#ifndef WORMHOLE_NAVIGATOR_HPP_
#define WORMHOLE_NAVIGATOR_HPP_

#include <string>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <geometry_msgs/msg/pose_with_covariance_stamped.hpp>
#include <visualization_msgs/msg/marker.hpp>
#include <nav2_msgs/srv/load_map.hpp>
#include <nav2_msgs/action/navigate_to_pose.hpp>
#include "multi_map_nav/action/navigate_with_wormhole.hpp"

class WormholeNavigator : public rclcpp::Node
{
public:
  WormholeNavigator();

private:
  std::string current_map_;

  // ROS 2 interfaces
  rclcpp_action::Server<multi_map_nav::action::NavigateWithWormhole>::SharedPtr action_server_;
  rclcpp::Publisher<geometry_msgs::msg::PoseWithCovarianceStamped>::SharedPtr pose_pub_;
  rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr marker_pub_;
  rclcpp::Client<nav2_msgs::srv::LoadMap>::SharedPtr map_client_;
  rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::SharedPtr nav_client_;

  // Action server callbacks
  rclcpp_action::GoalResponse handle_goal(
    const rclcpp_action::GoalUUID &,
    std::shared_ptr<const multi_map_nav::action::NavigateWithWormhole::Goal> goal);
  
  rclcpp_action::CancelResponse handle_cancel(
    const std::shared_ptr<rclcpp_action::ServerGoalHandle<multi_map_nav::action::NavigateWithWormhole>> goal_handle);
  
  void handle_accepted(
    const std::shared_ptr<rclcpp_action::ServerGoalHandle<multi_map_nav::action::NavigateWithWormhole>> goal_handle);

  // Core logic
  void execute(
    const std::shared_ptr<rclcpp_action::ServerGoalHandle<multi_map_nav::action::NavigateWithWormhole>> goal_handle);
  
  void teleport_to(double x, double y);
};

#endif  // WORMHOLE_NAVIGATOR_HPP_

