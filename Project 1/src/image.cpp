#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;

int main() {
    Mat image = imread("House_width_times4.bmp", IMREAD_GRAYSCALE);
    if (image.empty()) {
        std::cout << "Could not open or find the image!" << std::endl;
        return -1;
    }

    Mat negative = Mat::zeros(image.size(), image.type());

    for (int y = 0; y < image.rows; y++) {
        for (int x = 0; x < image.cols; x++) {
            negative.at<uchar>(y, x) = 255 - image.at<uchar>(y, x);
        }
    }

    imshow("Original Image", image);
    imshow("Negative Image", negative);

    waitKey(0);
    return 0;
}
