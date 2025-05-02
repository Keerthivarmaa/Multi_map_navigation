# ROS 2 Multi-Map Navigation System Documentation

# Project Overview

- This project implements a multi-map navigation system for a four-wheeled robot using ROS 2, Gazebo, and RViz2. The robot can:

- Navigate autonomously within different static maps (room1, room2, room3)

- Transition between maps via virtual wormholes

- Plan navigation goals via a custom action NavigateWithWormhole.action

- Retrieve wormhole positions using a SQLite3 database

# System Architecture

- Robot Base: custom four-wheeled robot

- Map Handling: nav2_map_server for loading static maps

- Navigation: nav2_bt_navigator, nav2_controller, nav2_planner

- Custom Node: wormhole_navigator (handles multi-map navigation)

- Database: SQLite3 stores wormhole coordinates per map

# Workflow

Input: Start pose, goal pose, current map
Output: Navigation through maps with wormhole transitions

1. If start and goal are in the same map:
   
     Send goal to nav2 action server

3. If in different maps:
   
    a. Get list of wormholes from SQLite DB
   
    b. For each wormhole:
   
        i. Navigate to wormhole exit in current map
   
        ii. Call load_map service to switch map
   
        iii. Update robot's initial pose
   
    c. Once in destination map, send final goal




# AMR Funtionality

- Create a custom robot along with neccesary files (or) Use a existing one available in open sources

# Clone package

        mkdir -p ~/ros_ws/src

        cd ros_ws/src

        git clone <URL>


# Build

        cd ~/ros_ws
        
        colcon build
        
        source install/setup.bash

# Run navigation stack
        ros2 launch <your_robot_package> navigation_launch.py

# Run action server node

        ros2 run multi_map_nav wormhole_navigator

# Send goal with wormhole(Client)

        ros2 action send_goal /navigate_with_wormhole multi_map_nav/action/NavigateWithWormhole "{x: -4.0, y: -4.0, target_map: 'room1'}"

*edit the traget_coordinates and target_map

