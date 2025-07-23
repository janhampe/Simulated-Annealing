cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(simulated_annealing VERSION 0.1)

add_subdirectory(include)

# Set compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -g -lfmt")

# Enable debug logs. Remove to make the code run faster
add_compile_definitions(DEBUG_LOG)
add_compile_definitions(ERROR_LOG)
add_compile_definitions(INFO_LOG)

include_directories(include)
include_directories(test)
# include_directories(lorina)

# add_library(lorina INTERFACE)
# target_include_directories(lorina INTERFACE ${PROJECT_SOURCE_DIR}/include)
# target_link_libraries(lorina INTERFACE fmt infint)
# if (ENABLE_COVERAGE)
# target_link_libraries(lorina INTERFACE gcov)
# endif()
find_package(fmt REQUIRED)

add_library(annealer_lib src/annealing.cpp src/data.cpp src/placer.cpp src/ui.cpp src/panic.cpp src/xoshiro256pp.cpp src/input.cpp)

add_executable(annealer src/main.cpp)
# target_link_libraries(annealer PRIVATE fmt::fmt)
target_link_libraries(annealer annealer_lib)

add_executable(test test/test.cpp test/data_test.cpp test/annealing_test.cpp test/placer_test.cpp test/ui_test.cpp)
target_link_libraries(test annealer_lib)
