#include "rclcpp/rclcpp.hpp"
#include "multi_map_nav/wormhole_navigator.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<WormholeNavigator>());
  rclcpp::shutdown();
  return 0;
}

