cmake_minimum_required(VERSION 3.10)

project(PatternClassification VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add executables
add_executable(solution solution.cpp)
add_executable(solution2 solution2.cpp)

# Compiler-specific options
if(MSVC)
    # MSVC compiler flags
    target_compile_options(solution PRIVATE /W4)
    target_compile_options(solution2 PRIVATE /W4)
else()
    # GCC/Clang compiler flags
    target_compile_options(solution PRIVATE -Wall -Wextra -O2)
    target_compile_options(solution2 PRIVATE -Wall -Wextra -O2)
endif()

# Print configuration info
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

