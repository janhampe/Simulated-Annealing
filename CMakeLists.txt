cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(simulated_annealing VERSION 0.1)

# Set compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-parameter")

include_directories(include)

add_library(annealer_lib src/annealing.cpp src/data.cpp src/placer.cpp src/ui.cpp src/panic.cpp)

add_executable(annealer src/main.cpp)
target_link_libraries(annealer annealer_lib)
