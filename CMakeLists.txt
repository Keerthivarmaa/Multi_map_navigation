cmake_minimum_required(VERSION 3.8)
project(multi_map_nav)

# Compiler warnings
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(nav2_map_server REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(nav2_common REQUIRED)
find_package(action_msgs REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(std_srvs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(SQLite3 REQUIRED)

# Action and service interface generation
rosidl_generate_interfaces(${PROJECT_NAME}
  "action/NavigateWithWormhole.action"
  "srv/LoadMap.srv"
  DEPENDENCIES builtin_interfaces action_msgs geometry_msgs std_msgs
)

# Include directories
include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_cpp
)

# Library target for wormhole_navigator
add_library(wormhole_navigator_lib SHARED
  src/wormhole_navigator.cpp
)

ament_target_dependencies(wormhole_navigator_lib
  rclcpp
  rclcpp_action
  nav2_msgs
  geometry_msgs
  std_srvs
  nav2_common
  visualization_msgs
  SQLite3
)

# Main executable
add_executable(wormhole_navigator src/main.cpp)
target_link_libraries(wormhole_navigator
  wormhole_navigator_lib
  ${cpp_typesupport_target}
)
set_target_properties(wormhole_navigator PROPERTIES
  INSTALL_RPATH "$ORIGIN"
)
# Link ROS 2 interfaces to executable
rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} rosidl_typesupport_cpp)
target_link_libraries(wormhole_navigator ${cpp_typesupport_target})

# Install targets
install(TARGETS
  wormhole_navigator
  wormhole_navigator_lib
  DESTINATION lib/${PROJECT_NAME}
)

# Install headers
install(DIRECTORY include/
  DESTINATION include/
)

# Required for ros2 run
ament_export_include_directories(include)
ament_export_dependencies(
  rclcpp
  rclcpp_action
  nav2_msgs
  geometry_msgs
  std_srvs
  nav2_common
  visualization_msgs
  SQLite3
  rosidl_default_runtime
)

ament_package()

