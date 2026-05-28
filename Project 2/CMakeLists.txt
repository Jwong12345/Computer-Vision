cmake_minimum_required(VERSION 3.10)
project(ImageProcessing)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set compiler flags
if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3 /O2")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2")
endif()

# Add executable
add_executable(solution solution.c)

# Link math library (only needed for GCC)
if(NOT MSVC)
    target_link_libraries(solution m)
endif()

# Custom target to clean only generated files
add_custom_target(clean_output
    COMMAND ${CMAKE_COMMAND} -E remove -f
        basel_enhanced.bmp
        ant_enhanced.bmp
        basel_sobel.bmp
        ant_sobel.bmp
        basel_log_7x7.bmp
        basel_log_11x11.bmp
        ant_log_7x7.bmp
        ant_log_11x11.bmp
    COMMENT "Cleaning generated output files"
)

# Custom target to run the program
add_custom_target(run
    COMMAND solution
    DEPENDS solution
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running image processing program"
)
