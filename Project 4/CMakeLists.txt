cmake_minimum_required(VERSION 3.10)
project(DCTTransform)

set(OpenCV_DIR "D:/opencv/build/x64/vc16/lib")
find_package(OpenCV REQUIRED)

add_executable(DCTTransform solution.cpp)
add_executable(Compression compression.cpp)
add_executable(ImageSegmentation image_segmentation.cpp)

target_link_libraries(DCTTransform ${OpenCV_LIBS})
target_link_libraries(Compression ${OpenCV_LIBS})
target_link_libraries(ImageSegmentation ${OpenCV_LIBS})