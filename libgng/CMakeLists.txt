project(libgng)
cmake_minimum_required(VERSION 2.8)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(libgng_sources
	point.cpp
	imagegenerator.cpp
	node.cpp
	edge.cpp
	gng.cpp
	)

add_library(gng SHARED ${libgng_sources})
target_link_libraries(gng ${QT_LIBRARIES})
