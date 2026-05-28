// image_segmentation.cpp
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

// Function to convert RGB to HSI
void RGB2HSI(Mat& src, Mat& dst) {
    dst.create(src.size(), CV_32FC3);

    for (int i = 0; i < src.rows; i++) {
        for (int j = 0; j < src.cols; j++) {
            Vec3b rgb = src.at<Vec3b>(i, j);
            float B = rgb[0] / 255.0f;
            float G = rgb[1] / 255.0f;
            float R = rgb[2] / 255.0f;

            float intensity = (R + G + B) / 3.0f;

            float min_val = min(R, min(G, B));
            float saturation = 0.0f;
            if ((R + G + B) > 0)
                saturation = 1.0f - (3.0f * min_val / (R + G + B));
            else
                saturation = 0.0f;

            float hue = 0.0f;
            float numerator = 0.5f * ((R - G) + (R - B));
            float denominator = sqrtf((R - G) * (R - G) + (R - B) * (G - B));

            if (denominator != 0.0f) {
                float theta = acosf(std::max(-1.0f, std::min(1.0f, numerator / denominator))) * 180.0f / CV_PI;
                if (B <= G)
                    hue = theta;
                else
                    hue = 360.0f - theta;
            }

            dst.at<Vec3f>(i, j) = Vec3f(hue, saturation, intensity);
        }
    }
}

// Function to segment the image based on HSI values
void segmentImage(Mat& src, Mat& dst) {
    dst.create(src.size(), CV_8U);

    // Compute intensity channel (scaled to 0-255)
    Mat intensityChannel(src.size(), CV_8U);
    for (int r = 0; r < src.rows; r++) {
        for (int c = 0; c < src.cols; c++) {
            float intensity = src.at<Vec3f>(r, c)[2];
            intensityChannel.at<uchar>(r, c) = static_cast<uchar>(intensity * 255);
        }
    }

    // Compute Otsu threshold 
    Mat dummyOutput;
    double otsu_thresh = threshold(intensityChannel, dummyOutput, 0, 255, THRESH_BINARY | THRESH_OTSU);
    float intensityThreshold = static_cast<float>(otsu_thresh / 255.0f);

    // Define thresholds
    float hueThresholdLow = 0.0f;
    float hueThresholdHigh = 360.0f;
    float saturationThreshold = 0.05f;

    // Segment image
    for (int i = 0; i < src.rows; i++) {
        for (int j = 0; j < src.cols; j++) {
            Vec3f hsi = src.at<Vec3f>(i, j);
            float hue = hsi[0];
            float saturation = hsi[1];
            float intensity = hsi[2];

            if ((hue >= hueThresholdLow && hue <= hueThresholdHigh) &&
                saturation >= saturationThreshold &&
                intensity >= intensityThreshold) {
                dst.at<uchar>(i, j) = 255;
            } else {
                dst.at<uchar>(i, j) = 0;
            }
        }
    }

    cout << "Otsu Intensity Threshold: " << intensityThreshold << endl;
}

// Function to detect and draw region boundaries (contours)
void detectBoundaries(Mat& src, Mat& dst) {
    // Ensure binary image input
    Mat binary;
    if (src.channels() == 3)
        cvtColor(src, binary, COLOR_BGR2GRAY);
    else
        binary = src.clone();

    // Smoothing to clean up edges
    GaussianBlur(binary, binary, Size(3, 3), 0);

    // Find external contours of white regions
    vector<vector<Point>> contours;
    findContours(binary, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    // Draw contours on a color version of the binary image
    cvtColor(binary, dst, COLOR_GRAY2BGR);
    drawContours(dst, contours, -1, Scalar(0, 0, 255), 2);

    cout << "Contours found: " << contours.size() << endl;
}

// Main
int main() {
    try {
        // Load first image
        Mat image = imread("Building1.bmp");
        if (image.empty()) {
            cout << "Could not open or find Building1.bmp" << endl;
            return -1;
        }

        Mat hsiImage, segmentedImage, boundaryImage;
        RGB2HSI(image, hsiImage);
        segmentImage(hsiImage, segmentedImage);
        detectBoundaries(segmentedImage, boundaryImage);

        imshow("Original Image 1", image);
        imshow("Segmented Image 1", segmentedImage);
        imshow("Boundary Image 1", boundaryImage);

        // Load second image
        Mat image2 = imread("Disk.bmp");
        if (image2.empty()) {
            cout << "Could not open or find Disk.bmp" << endl;
            return -1;
        }

        Mat hsiImage2, segmentedImage2, boundaryImage2;
        RGB2HSI(image2, hsiImage2);
        segmentImage(hsiImage2, segmentedImage2);
        detectBoundaries(segmentedImage2, boundaryImage2);

        imshow("Original Image 2", image2);
        imshow("Segmented Image 2", segmentedImage2);
        imshow("Boundary Image 2", boundaryImage2);

        waitKey(0);
        return 0;

    } catch (const std::exception& e) {
        cerr << "Exception: " << e.what() << endl;
        return -1;
    }
}
