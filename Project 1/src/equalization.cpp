#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

Mat drawHistogram(Mat hist) {
    int histSize = 256;        
    int hist_w = 512, hist_h = 400;
    int bin_w = cvRound((double) hist_w / histSize);

    Mat histImage(hist_h, hist_w, CV_8UC1, Scalar(255));

    normalize(hist, hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

    for (int i = 1; i < histSize; i++) {
        line(histImage,
             Point(bin_w * (i - 1), hist_h - cvRound(hist.at<float>(i - 1))),
             Point(bin_w * (i), hist_h - cvRound(hist.at<float>(i))),
             Scalar(0), 2, 8, 0);
    }
    return histImage;
}

// Function to process a single image
void processImage(const string& imageName) {
    cout << "\n" << string(50, '=') << endl;
    cout << "Processing image: " << imageName << endl;
    cout << string(50, '=') << endl;
    
    // Load the image
    Mat image = imread(imageName, IMREAD_GRAYSCALE);
    if (image.empty()) {
        cout << "Could not open or find the image: " << imageName << endl;
        return;
    }
    
    cout << "Image size: " << image.cols << "x" << image.rows << endl;

    // Create negative image
    Mat negative = Mat::zeros(image.size(), image.type());
    for (int y = 0; y < image.rows; y++) {
        for (int x = 0; x < image.cols; x++) {
            negative.at<uchar>(y, x) = 255 - image.at<uchar>(y, x);
        }
    }

    // Equalize both original and negative images
    Mat equalized, equalizedNegative;
    equalizeHist(image, equalized);
    equalizeHist(negative, equalizedNegative);

    int histSize = 256;
    float range[] = {0, 256};
    const float* histRange = {range};

    // Calculate histograms for all images
    Mat histOriginal, histNegative, histEqualized, histEqualizedNegative;
    calcHist(&image, 1, 0, Mat(), histOriginal, 1, &histSize, &histRange, true, false);
    calcHist(&negative, 1, 0, Mat(), histNegative, 1, &histSize, &histRange, true, false);
    calcHist(&equalized, 1, 0, Mat(), histEqualized, 1, &histSize, &histRange, true, false);
    calcHist(&equalizedNegative, 1, 0, Mat(), histEqualizedNegative, 1, &histSize, &histRange, true, false);

    // Draw histograms
    Mat histImageOriginal = drawHistogram(histOriginal);
    Mat histImageNegative = drawHistogram(histNegative);
    Mat histImageEqualized = drawHistogram(histEqualized);
    Mat histImageEqualizedNegative = drawHistogram(histEqualizedNegative);

    // Create unique window names for each image
    string baseName = imageName.substr(0, imageName.find_last_of('.'));
    
    // Display all images and histograms
    imshow("Original Image - " + baseName, image);
    imshow("Negative Image - " + baseName, negative);
    imshow("Equalized Original - " + baseName, equalized);
    imshow("Equalized Negative - " + baseName, equalizedNegative);

    imshow("Histogram - Original " + baseName, histImageOriginal);
    imshow("Histogram - Negative " + baseName, histImageNegative);
    imshow("Histogram - Equalized Original " + baseName, histImageEqualized);
    imshow("Histogram - Equalized Negative " + baseName, histImageEqualizedNegative);

    // Save images with unique names
    imwrite(baseName + "_original.bmp", image);
    imwrite(baseName + "_negative.bmp", negative);
    imwrite(baseName + "_equalized_original.bmp", equalized);
    imwrite(baseName + "_equalized_negative.bmp", equalizedNegative);
    imwrite(baseName + "_hist_original.bmp", histImageOriginal);
    imwrite(baseName + "_hist_negative.bmp", histImageNegative);
    imwrite(baseName + "_hist_equalized_original.bmp", histImageEqualized);
    imwrite(baseName + "_hist_equalized_negative.bmp", histImageEqualizedNegative);

    cout << "Images saved:" << endl;
    cout << "- " << baseName << "_original.bmp" << endl;
    cout << "- " << baseName << "_negative.bmp" << endl;
    cout << "- " << baseName << "_equalized_original.bmp" << endl;
    cout << "- " << baseName << "_equalized_negative.bmp" << endl;
    cout << "- " << baseName << "_hist_original.bmp" << endl;
    cout << "- " << baseName << "_hist_negative.bmp" << endl;
    cout << "- " << baseName << "_hist_equalized_original.bmp" << endl;
    cout << "- " << baseName << "_hist_equalized_negative.bmp" << endl;
}

int main() {
    cout << "CS455 Project 1 - Histogram Equalization" << endl;
    cout << "=========================================" << endl;
    
    // List of images to process
    vector<string> imageFiles = {"House_width_times4.bmp", "NYC_width_4times.bmp"};
    
    // Check which images exist
    vector<string> existingImages;
    for (const string& imageFile : imageFiles) {
        Mat testImage = imread(imageFile, IMREAD_GRAYSCALE);
        if (!testImage.empty()) {
            existingImages.push_back(imageFile);
        } else {
            cout << "Warning: Could not find image: " << imageFile << endl;
        }
    }
    
    if (existingImages.empty()) {
        cout << "Error: No valid images found!" << endl;
        cout << "Please ensure at least one of the following images exists:" << endl;
        for (const string& imageFile : imageFiles) {
            cout << "- " << imageFile << endl;
        }
        return -1;
    }
    
    cout << "\nFound " << existingImages.size() << " image(s) to process:" << endl;
    for (const string& imageFile : existingImages) {
        cout << "- " << imageFile << endl;
    }
    
    // Process each image
    for (const string& imageFile : existingImages) {
        processImage(imageFile);
    }
    
    cout << "\n" << string(50, '=') << endl;
    cout << "Processing complete! Press any key in the image windows to close them." << endl;
    cout << string(50, '=') << endl;
    
    waitKey(0);
    return 0;
}
