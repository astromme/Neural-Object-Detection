project(libaibo)
cmake_minimum_required(VERSION 2.6)

find_package(Qt4 REQUIRED)
find_package(OpenCV REQUIRED)

include(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(libaibo_sources
    aibo.cpp
    aibocontrol.cpp
        )

set(libaibo_headers
    aibo.h
    aibocontrol.h
    )

qt4_wrap_cpp(libaibo_mocs ${libaibo_headers})

add_library(aibo SHARED ${libaibo_sources} ${libaibo_mocs})
target_link_libraries(aibo ${QT_LIBRARIES} ${QT_QTNETWORK_LIBRARY} ${OpenCV_LIBS})
