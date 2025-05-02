#include "multi_map_nav/wormhole_navigator.hpp"
#include <ament_index_cpp/get_package_share_directory.hpp>
#include <sqlite3.h>
#include <filesystem>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;
using NavigateWithWormhole = multi_map_nav::action::NavigateWithWormhole;
using GoalHandleNavigate = rclcpp_action::ServerGoalHandle<NavigateWithWormhole>;

WormholeNavigator::WormholeNavigator() : Node("wormhole_navigator")
{
  this->declare_parameter("current_map", "room2");
  this->declare_parameter("sleep_duration_ms", 2000);  // for navigation client wait
  this->get_parameter("current_map", current_map_);

  action_server_ = rclcpp_action::create_server<NavigateWithWormhole>(
    this, "navigate_with_wormhole",
    std::bind(&WormholeNavigator::handle_goal, this, std::placeholders::_1, std::placeholders::_2),
    std::bind(&WormholeNavigator::handle_cancel, this, std::placeholders::_1),
    std::bind(&WormholeNavigator::handle_accepted, this, std::placeholders::_1));

  pose_pub_ = this->create_publisher<geometry_msgs::msg::PoseWithCovarianceStamped>("/initialpose", 10);
  marker_pub_ = this->create_publisher<visualization_msgs::msg::Marker>("wormhole_marker", 10);
  map_client_ = this->create_client<nav2_msgs::srv::LoadMap>("/map_server/load_map");
  nav_client_ = rclcpp_action::create_client<nav2_msgs::action::NavigateToPose>(this, "navigate_to_pose");

  RCLCPP_INFO(this->get_logger(), "Wormhole Navigator started. Current map: %s", current_map_.c_str());
}

rclcpp_action::GoalResponse WormholeNavigator::handle_goal(
  const rclcpp_action::GoalUUID &, std::shared_ptr<const NavigateWithWormhole::Goal> goal)
{
  RCLCPP_INFO(this->get_logger(), "Received goal: x=%.2f y=%.2f map=%s", goal->x, goal->y, goal->target_map.c_str());
  return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse WormholeNavigator::handle_cancel(
  const std::shared_ptr<GoalHandleNavigate>)
{
  RCLCPP_INFO(this->get_logger(), "Cancel request received.");
  return rclcpp_action::CancelResponse::ACCEPT;
}

void WormholeNavigator::handle_accepted(const std::shared_ptr<GoalHandleNavigate> goal_handle)
{
  std::thread{std::bind(&WormholeNavigator::execute, this, goal_handle)}.detach();
}

void WormholeNavigator::execute(const std::shared_ptr<GoalHandleNavigate> goal_handle)
{
  auto goal = goal_handle->get_goal();
  auto result = std::make_shared<NavigateWithWormhole::Result>();

  if (goal->target_map != current_map_) {
    // Build path to database
    std::string db_path = ament_index_cpp::get_package_share_directory("multi_map_nav") + "/config/wormholes.db";

    sqlite3 *db;
    if (sqlite3_open(db_path.c_str(), &db)) {
      RCLCPP_ERROR(this->get_logger(), "Can't open database: %s", sqlite3_errmsg(db));
      result->success = false;
      goal_handle->abort(result);
      return;
    }

    std::string query = "SELECT x, y FROM wormholes WHERE source_map = ? AND target_map = ?";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
      RCLCPP_ERROR(this->get_logger(), "Failed to prepare statement");
      sqlite3_close(db);
      result->success = false;
      goal_handle->abort(result);
      return;
    }

    sqlite3_bind_text(stmt, 1, current_map_.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, goal->target_map.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
      double wormhole_x = sqlite3_column_double(stmt, 0);
      double wormhole_y = sqlite3_column_double(stmt, 1);
      RCLCPP_INFO(this->get_logger(), "Wormhole at x=%.2f y=%.2f", wormhole_x, wormhole_y);

      nav2_msgs::action::NavigateToPose::Goal nav_goal;
      nav_goal.pose.header.frame_id = "map";
      nav_goal.pose.header.stamp = this->now();
      nav_goal.pose.pose.position.x = wormhole_x;
      nav_goal.pose.pose.position.y = wormhole_y;
      nav_goal.pose.pose.orientation.w = 1.0;

      if (!nav_client_->wait_for_action_server(5s)) {
        RCLCPP_ERROR(this->get_logger(), "NavigateToPose action server not available.");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        result->success = false;
        goal_handle->abort(result);
        return;
      }

      auto future = nav_client_->async_send_goal(nav_goal);
      if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), future) !=
          rclcpp::FutureReturnCode::SUCCESS) {
        RCLCPP_ERROR(this->get_logger(), "Failed to send NavigateToPose goal");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        result->success = false;
        goal_handle->abort(result);
        return;
      }

      auto goal_handle_nav = future.get();
      if (!goal_handle_nav) {
        RCLCPP_ERROR(this->get_logger(), "NavigateToPose goal was rejected");
        result->success = false;
        goal_handle->abort(result);
        return;
      }

      auto result_future = nav_client_->async_get_result(goal_handle_nav);
      if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), result_future) !=
          rclcpp::FutureReturnCode::SUCCESS) {
        RCLCPP_ERROR(this->get_logger(), "NavigateToPose goal failed");
        result->success = false;
        goal_handle->abort(result);
        return;
      }

      // Load new map after navigation
      auto request = std::make_shared<nav2_msgs::srv::LoadMap::Request>();
      request->map_url = "package://multi_map_nav/maps/" + goal->target_map + ".yaml";

      if (!map_client_->wait_for_service(5s)) {
        RCLCPP_ERROR(this->get_logger(), "LoadMap service not available.");
        result->success = false;
        goal_handle->abort(result);
        return;
      }

      auto response = map_client_->async_send_request(request);
      if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), response) !=
          rclcpp::FutureReturnCode::SUCCESS) {
        RCLCPP_ERROR(this->get_logger(), "Failed to load map.");
        result->success = false;
        goal_handle->abort(result);
        return;
      }

      current_map_ = goal->target_map;
      teleport_to(goal->x, goal->y);

    } else {
      RCLCPP_ERROR(this->get_logger(), "No wormhole from %s to %s", current_map_.c_str(), goal->target_map.c_str());
      result->success = false;
      goal_handle->abort(result);
      sqlite3_finalize(stmt);
      sqlite3_close(db);
      return;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
  } else {
    teleport_to(goal->x, goal->y);
  }

  result->success = true;
  goal_handle->succeed(result);
  RCLCPP_INFO(this->get_logger(), "Navigation complete.");
}

void WormholeNavigator::teleport_to(double x, double y)
{
  geometry_msgs::msg::PoseWithCovarianceStamped pose;
  pose.header.stamp = this->now();
  pose.header.frame_id = "map";
  pose.pose.pose.position.x = x;
  pose.pose.pose.position.y = y;
  pose.pose.pose.orientation.w = 1.0;

  pose_pub_->publish(pose);
  


}

