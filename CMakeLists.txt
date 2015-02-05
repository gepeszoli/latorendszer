cmake_minimum_required(VERSION 2.8)
project( latorendszer )
find_package( OpenCV REQUIRED )
set(PROJECT_SRC
 main.cpp
 globals.cpp 
 display.cpp
 locator.cpp 
 calibration.cpp
 broadcaster.cpp
 inputcommand.cpp
 project.h
)
add_executable( server ${PROJECT_SRC})
target_link_libraries( server ${OpenCV_LIBS} rt pthread)
