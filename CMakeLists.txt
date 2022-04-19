cmake_minimum_required(VERSION 3.18)
project(Signalverarbeitung C)

#set(CMAKE_C_STANDARD 17)
set(CMAKE_C_FLAGS "-lpthread -lrt -lm")

add_executable(Signalverarbeitung main.c FileOut.c Filter.c Generator.c Timer.c pigpio.c)
target_include_directories(Signalverarbeitung PRIVATE include)
target_link_libraries(Signalverarbeitung pthread rt m)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE DEBUG)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
