cmake_minimum_required(VERSION 3.10)

project(Project_1)

set(OpenCV_DIR "D:/opencv/build")
find_package(OpenCV REQUIRED)

add_executable(${PROJECT_NAME} src/equalization.cpp)

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
