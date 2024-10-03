cmake_minimum_required(VERSION 3.5)
project(soft_robot_control)

# Find necessary packages
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

# Include directories
include_directories(include)

# Add the executable for your C++ node
add_executable(force_sensor_data_reader soft_robot_control/force_sensor_data_reader.cpp)

# Link against necessary libraries
ament_target_dependencies(force_sensor_data_reader rclcpp std_msgs)

# Install the node
install(TARGETS
  force_sensor_data_reader
  DESTINATION lib/${PROJECT_NAME}
)

# Finalize the package
ament_package()

