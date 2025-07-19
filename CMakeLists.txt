cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(simulated_annealing VERSION 0.1)

# Set compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-parameter -g")

include_directories(include)
include_directories(test)

add_library(annealer_lib src/annealing.cpp src/data.cpp src/placer.cpp src/ui.cpp src/panic.cpp)

add_executable(annealer src/main.cpp)
target_link_libraries(annealer annealer_lib)

add_executable(test test/test.cpp test/data_test.cpp test/annealing_test.cpp test/placer_test.cpp test/ui_test.cpp)
target_link_libraries(test annealer_lib)
target_link_libraries(test annealer_lib)
