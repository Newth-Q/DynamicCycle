cmake_minimum_required(VERSION 3.7)
project(DynamicCycle)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories("/root/CycleEnum/DynamicCycle/tclap-1.2.5/include")

set(SOURCE_FILES main.cpp graph.h graph.cpp Timer.h)

# set(CMAKE_BUILD_TYPE Debug) 
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0") 

add_executable(DynamicCycle ${SOURCE_FILES})

if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE -Ofast)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O3  -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3  -Wall")