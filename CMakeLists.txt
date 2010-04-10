project(gng)
cmake_minimum_required(VERSION 2.8)
SET(CMAKE_BUILD_TYPE RelWithDebInfo)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(libgng)

set(gngtest_sources
	gngtest.cpp
	gngviewer.cpp
	)

add_executable(gngtest ${gngtest_sources})
target_link_libraries(gngtest ${QT_LIBRARIES} gng)

