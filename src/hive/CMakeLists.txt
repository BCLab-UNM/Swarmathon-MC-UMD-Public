cmake_minimum_required(VERSION 2.8.3)
project(hive)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  hive_srv
  
)

catkin_package(
CATKIN_DEPENDS roscpp std_msgs hive_srv
)

include_directories(
  include ${catkin_INCLUDE_DIRS}
)


add_executable(hive
    src/Hive.cpp
)

add_dependencies(hive hive_srv_gencpp)

target_link_libraries(hive
   ${catkin_LIBRARIES}
)
