# Multi_map_navigation
Multi-map navigation system enables the robot to navigate in different maps using wormhole mechanism

📘 ROS 2 Multi-Map Navigation System Documentation

🧠 Project Overview

This project implements a multi-map navigation system for a four-wheeled robot using ROS 2, Gazebo, and RViz2. The robot can:

Navigate autonomously within different static maps (room1, room2, room3)

Transition between maps via virtual wormholes

Dynamically load maps using the /map_server/load_map service

Plan navigation goals via a custom action NavigateWithWormhole.action

Retrieve wormhole positions using a SQLite3 database

⚙️ System Architecture

Robot Base: Differential-drive, four-wheeled robot

Map Handling: nav2_map_server for loading static maps

Navigation: nav2_bt_navigator, nav2_controller, nav2_planner

Custom Node: wormhole_navigator (handles multi-map navigation)

Database: SQLite3 stores wormhole coordinates per map

🧩 Algorithm (Pseudocode)

Input: Start pose, goal pose, current map
Output: Navigation through maps with wormhole transitions

1. If start and goal are in the same map:
    a. Send goal to nav2 action server
    b. Wait for success or failure

2. If in different maps:
    a. Get list of intermediate wormholes from SQLite DB
    b. For each wormhole:
        i. Navigate to wormhole exit in current map
        ii. Call load_map service to switch map
        iii. Update robot's initial pose
    c. Once in destination map, send final goal

📁 File Structure

multi_map_nav/
├── action/
│   └── NavigateWithWormhole.action
├── srv/
│   └── LoadMap.srv
├── include/
│   └── multi_map_nav/
│       └── wormhole_navigator.hpp
├── src/
│   ├── main.cpp
│   └── wormhole_navigator.cpp
├── maps/
│   ├── room1.yaml
│   ├── room2.yaml
│   └── room3.yaml
├── database/
│   └── wormholes.db
├── launch/
│   └── navigation_launch.py
├── CMakeLists.txt
└── package.xml

🧱 Key Components

NavigateWithWormhole.action

# Goal
geometry_msgs/PoseStamped start
geometry_msgs/PoseStamped goal
string start_map
string goal_map

---
# Result
bool success
string message

---
# Feedback
string current_status

LoadMap.srv

string map_name
---
bool success

wormhole_navigator.cpp

Handles:

Action server logic

Wormhole DB lookup

Map switching

Path planning and navigation

wormhole_navigator.hpp

Defines the WormholeNavigator class, interfaces to:

rclcpp_action::Server

SQLite3

/map_server/load_map

navigation_launch.py

Launches:

robot_state_publisher

nav2 stack

map_server

wormhole_navigator

🛠️ Build & Run

# Build
colcon build
. install/setup.bash

# Run navigation stack
ros2 launch multi_map_nav navigation_launch.py

# Send goal with wormhole
ros2 action send_goal /navigate_with_wormhole multi_map_nav/NavigateWithWormhole ...

🧪 Testing Procedure

Launch simulation in Gazebo and RViz2

Start the navigation_launch.py launch file

Send a goal in a different map via the custom action

Verify:

The robot transitions maps correctly

Navigates to wormhole exits and re-initializes pose

Reaches the final goal

✅ Achievements

Multi-map navigation with smooth transitions

Modular architecture using actions and services

SQLite-powered dynamic wormhole handling

Fully ROS 2 compliant
