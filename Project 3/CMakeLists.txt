cmake_minimum_required(VERSION 3.10)
project(MorphologicalOperations)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(solution solution.cpp)

# Set output directory
set_target_properties(solution PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Release"
)

# Copy test images to output directory
file(COPY ${CMAKE_SOURCE_DIR}/TestImage-even-width.bmp 
     DESTINATION ${CMAKE_BINARY_DIR}/Release)
file(COPY ${CMAKE_SOURCE_DIR}/TestImage-odd-width.bmp 
     DESTINATION ${CMAKE_BINARY_DIR}/Release)
