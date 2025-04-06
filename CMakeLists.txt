cmake_minimum_required(VERSION 3.8)
project(overlay_value_display)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(rviz_default_plugins REQUIRED)
find_package(OGRE REQUIRED)
find_package(rviz_common REQUIRED)

# Message generation dependencies
find_package(rosidl_default_generators REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Generate the custom message
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/OverlayValueInRange.msg"
  DEPENDENCIES builtin_interfaces std_msgs
)

# Export dependencies for runtime
ament_export_dependencies(rosidl_default_runtime)

# Include directories
include_directories(
  include
  ${rclcpp_INCLUDE_DIRS}
  ${rviz_common_INCLUDE_DIRS}
  ${OGRE_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR} # Include generated message headers
)

# Source files
add_library(overlay_value_display_lib SHARED
  src/overlay_value_display.cpp
)

# Link the library
ament_target_dependencies(overlay_value_display_lib
  rclcpp
  rviz_common
  rviz_default_plugins
  OGRE
  std_msgs
)

# Ensure the generated interfaces are exported
rosidl_target_interfaces(overlay_value_display_lib
  ${PROJECT_NAME} "rosidl_typesupport_cpp"
)

target_link_libraries(overlay_value_display_lib ${OGRE_LIBRARIES})

# Install the library and headers
install(TARGETS overlay_value_display_lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
install(DIRECTORY include/
  DESTINATION include
)

# Install the generated headers
install(
  DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/
  DESTINATION include
  FILES_MATCHING PATTERN "*.h"
  PATTERN ".svn" EXCLUDE
)

# Install the plugin description file
install(FILES overlay_value_display_description.xml
  DESTINATION share/${PROJECT_NAME}
)

# Export the library and include directories
ament_export_libraries(overlay_value_display_lib)
ament_export_include_directories(include)
ament_export_dependencies(rclcpp rviz_common rosidl_default_runtime)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_lint_auto_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
