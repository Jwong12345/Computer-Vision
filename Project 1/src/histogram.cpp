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

int main() {
    Mat image = imread("House_width_times4.bmp", IMREAD_GRAYSCALE);
    if (image.empty()) {
        cout << "Could not open or find the image!" << endl;
        return -1;
    }
    Mat negative;
    bitwise_not(image, negative); 

    int histSize = 256;  
    float range[] = {0, 256};
    const float* histRange = {range};

    Mat histOriginal, histNegative;
    calcHist(&image, 1, 0, Mat(), histOriginal, 1, &histSize, &histRange, true, false);
    calcHist(&negative, 1, 0, Mat(), histNegative, 1, &histSize, &histRange, true, false);

    Mat histImageOriginal = drawHistogram(histOriginal);
    Mat histImageNegative = drawHistogram(histNegative);

    imshow("Original Image", image);
    imshow("Negative Image", negative);
    imshow("Histogram - Original", histImageOriginal);
    imshow("Histogram - Negative", histImageNegative);

    waitKey(0);
    return 0;
}
