// c:\Users\wongj\OneDrive\Desktop\github\CS455\Project 4\compression.cpp
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

// Function to perform 2D DCT on an 8x8 block
void dct2D(Mat& block) {
    Mat floatBlock;
    block.convertTo(floatBlock, CV_32F);
    dct(floatBlock, block);
}

// Function to perform 2D IDCT on an 8x8 block
void idct2D(Mat& block) {
    idct(block, block);
    block.convertTo(block, CV_8U);
}

int main() {
    // 1. Obtain image f1 (“basel3.bmp”) and display it
    Mat f1 = imread("basel3.bmp", IMREAD_COLOR);
    if (f1.empty()) {
        cout << "Could not open or find the image" << endl;
        return -1;
    }
    imshow("Original Image", f1);
    waitKey(0);

    // 2. Convert image f1 from RGB to HSI, obtain intensity image I
    Mat hsi;
    cvtColor(f1, hsi, COLOR_BGR2HSV);
    vector<Mat> hsi_channels;
    split(hsi, hsi_channels);
    Mat I = hsi_channels[2]; // Intensity channel

    imshow("Intensity Image", I);
    waitKey(0);

    // 3. Apply 8*8 DCT transform to image I, obtain frequency domain image F
    Mat F = Mat::zeros(I.size(), CV_32F);
    for (int i = 0; i < I.rows - 7; i += 8) {
        for (int j = 0; j < I.cols - 7; j += 8) {
            Mat block = I(Rect(j, i, 8, 8)).clone();
            dct2D(block);
            block.convertTo(block, CV_32F);
            block.copyTo(F(Rect(j, i, 8, 8)));
        }
    }
    
    // 4. In each 8*8 frequency block, keep DC component and remove all other frequency components from F, obtain frequency domain image D1
    Mat D1 = Mat::zeros(F.size(), CV_32F);
    for (int i = 0; i < F.rows - 7; i += 8) {
        for (int j = 0; j < F.cols - 7; j += 8) {
            Mat block = F(Rect(j, i, 8, 8)).clone();
            block.setTo(0);
            block.at<float>(0, 0) = F.at<float>(i, j); // Keep DC component
            block.copyTo(D1(Rect(j, i, 8, 8)));
        }
    }

    // 5. Similar to 4, in each 8*8 frequency block, keep first 9 low frequency components and remove all other high frequency components from F, obtain frequency domain image D2
   Mat D2 = Mat::zeros(F.size(), CV_32F);
    for (int i = 0; i < F.rows - 7; i += 8) {
        for (int j = 0; j < F.cols - 7; j += 8) {
            Mat block = F(Rect(j, i, 8, 8)).clone();
            Mat new_block = Mat::zeros(8,8, CV_32F);
            
            //copy the top left 9 components
            for (int x = 0; x < 3; x++){
                for (int y = 0; y < 3; y++){
                   new_block.at<float>(x,y) = block.at<float>(x,y);
                }
            }

            new_block.copyTo(D2(Rect(j, i, 8, 8)));
        }
    }


    // 6. Apply IDCT on D1 and D2, obtain image R1 and R2
    Mat R1 = Mat::zeros(I.size(), CV_8U);
    for (int i = 0; i < D1.rows - 7; i += 8) {
        for (int j = 0; j < D1.cols - 7; j += 8) {
            Mat block = D1(Rect(j, i, 8, 8)).clone();
            idct2D(block);
            block.copyTo(R1(Rect(j, i, 8, 8)));
        }
    }

    Mat R2 = Mat::zeros(I.size(), CV_8U);
    for (int i = 0; i < D2.rows - 7; i += 8) {
        for (int j = 0; j < D2.cols - 7; j += 8) {
            Mat block = D2(Rect(j, i, 8, 8)).clone();
            idct2D(block);
            block.copyTo(R2(Rect(j, i, 8, 8)));
        }
    }


    imshow("Reconstructed Image R1 (DC Component Only)", R1);
    waitKey(0);
    imshow("Reconstructed Image R2 (9 Low-Frequency Components)", R2);
    waitKey(0);

    return 0;
}