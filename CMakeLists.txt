project(gng)
cmake_minimum_required(VERSION 2.8)
SET(CMAKE_BUILD_TYPE RelWithDebInfo)

find_package(Qt4 REQUIRED)
find_package(OpenCV REQUIRED) # Hack because I need <cv.h> in the header of libgng
include(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

add_subdirectory(libgng)

set(gngtest_sources
	gngtest.cpp
	gngapp.cpp
	gngviewer.cpp
	)


set(gngtest_headers
    gngapp.h
    )

qt4_wrap_cpp(gngtest_mocs ${gngtest_headers})

add_executable(gngtest ${gngtest_sources} ${gngtest_mocs})
target_link_libraries(gngtest ${QT_LIBRARIES} gng)
target_link_libraries(gngtest ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable(webcamtest webcamtest.cpp gngviewer.cpp)
target_link_libraries(webcamtest ${QT_LIBRARIES} gng ${Boost_PROGRAM_OPTIONS_LIBRARY})

