cmake_minimum_required(VERSION 3.2)

project(BMFont VERSION 0.0.1)

add_executable(BMFont stdafx.cpp main.cpp)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS}) 
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(BMFont ${OpenCV_LIBS})

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS program_options filesystem REQUIRED)
if(NOT Boost_FOUND)
    message("Not found Boost")
endif()
 
include_directories(${Boost_INCLUDE_DIRS})
message("${Boost_INCLUDE_DIRS}")
message("${Boost_LIBRARIES}")
target_link_libraries(BMFont ${Boost_LIBRARIES})

