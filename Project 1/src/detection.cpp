#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>

using namespace cv;
using namespace std;

struct Region {
    int label;
    int pixelCount;
    vector<Point> pixels;
};

// Function to calculate histogram
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

// 4-connected component labeling algorithm
vector<Region> connectedComponentLabeling(Mat binaryImage) {
    Mat labeledImage = Mat::zeros(binaryImage.size(), CV_32S);
    vector<Region> regions;
    int currentLabel = 1;
    
    for (int y = 0; y < binaryImage.rows; y++) {
        for (int x = 0; x < binaryImage.cols; x++) {
            if (binaryImage.at<uchar>(y, x) == 255) { // Foreground pixel
                vector<int> neighborLabels;
                
                if (y > 0 && labeledImage.at<int>(y-1, x) > 0) {
                    neighborLabels.push_back(labeledImage.at<int>(y-1, x));
                }
                if (x > 0 && labeledImage.at<int>(y, x-1) > 0) {
                    neighborLabels.push_back(labeledImage.at<int>(y, x-1));
                }
                
                if (neighborLabels.empty()) {
                    labeledImage.at<int>(y, x) = currentLabel++;
                } else {
                    int minLabel = *min_element(neighborLabels.begin(), neighborLabels.end());
                    labeledImage.at<int>(y, x) = minLabel;
                }
            }
        }
    }
    
    map<int, int> labelMap;
    map<int, Region> regionMap;
    
    for (int y = 0; y < labeledImage.rows; y++) {
        for (int x = 0; x < labeledImage.cols; x++) {
            int label = labeledImage.at<int>(y, x);
            if (label > 0) {
                int rootLabel = label;
                while (labelMap.find(rootLabel) != labelMap.end()) {
                    rootLabel = labelMap[rootLabel];
                }
                
                labeledImage.at<int>(y, x) = rootLabel;
                
                if (regionMap.find(rootLabel) == regionMap.end()) {
                    regionMap[rootLabel] = Region();
                    regionMap[rootLabel].label = rootLabel;
                    regionMap[rootLabel].pixelCount = 0;
                }
                regionMap[rootLabel].pixelCount++;
                regionMap[rootLabel].pixels.push_back(Point(x, y));
            }
        }
    }
    
    for (auto& pair : regionMap) {
        regions.push_back(pair.second);
    }
    
    return regions;
}

// Function to create colored output image
Mat createColoredImage(Mat binaryImage, vector<Region> regions) {
    Mat coloredImage = Mat::zeros(binaryImage.size(), CV_8UC1);
    
    sort(regions.begin(), regions.end(), [](const Region& a, const Region& b) {
        return a.pixelCount > b.pixelCount;
    });
    
    for (size_t i = 0; i < regions.size(); i++) {
        uchar color;
        if (i == 0) {
            color = 200; // RED - largest region
        } else if (i == 1) {
            color = 120; // GREEN - medium region
        } else {
            color = 60;  // BLUE - smallest region
        }
        
        // Color all pixels in this region
        for (const Point& pixel : regions[i].pixels) {
            coloredImage.at<uchar>(pixel.y, pixel.x) = color;
        }
    }
    
    return coloredImage;
}

// Function to process a single image
void processImage(const string& imageName) {
    cout << "\n" << string(50, '=') << endl;
    cout << "Processing image: " << imageName << endl;
    cout << string(50, '=') << endl;
    
    Mat image = imread(imageName, IMREAD_GRAYSCALE);
    if (image.empty()) {
        cout << "Could not open or find the image: " << imageName << endl;
        return;
    }
    
    cout << "Image size: " << image.cols << "x" << image.rows << endl;
    
    int histSize = 256;
    float range[] = {0, 256};
    const float* histRange = {range};
    Mat hist;
    calcHist(&image, 1, 0, Mat(), hist, 1, &histSize, &histRange, true, false);
    
    // Apply histogram equalization first to improve contrast
    Mat equalizedImage;
    equalizeHist(image, equalizedImage);
    
    Mat binaryImage;
    double thresholdValue = threshold(equalizedImage, binaryImage, 0, 255, THRESH_BINARY | THRESH_OTSU);
    
    cout << "Using histogram equalization + Otsu thresholding" << endl;
    cout << "Optimal threshold T: " << thresholdValue << endl;
    
    Mat f1 = binaryImage.clone();
    
    vector<Region> regions = connectedComponentLabeling(f1);
    
    cout << "Number of detected regions: " << regions.size() << endl;
    
    // Print region information
    for (size_t i = 0; i < regions.size(); i++) {
        cout << "Region " << (i+1) << ": " << regions[i].pixelCount << " pixels" << endl;
    }
    
    // Step 4: Create colored image f2 with ranked regions
    Mat f2 = createColoredImage(f1, regions);
    
    // Step 5: Display results
    Mat histImage = drawHistogram(hist);
    
    string baseName = imageName.substr(0, imageName.find_last_of('.'));
    imshow("Original Image - " + baseName, image);
    imshow("Histogram - " + baseName, histImage);
    imshow("Binary Image f1 - " + baseName, f1);
    imshow("Labeled Image f2 - " + baseName, f2);
    
    string f1Name = baseName + "_f1_binary.bmp";
    string f2Name = baseName + "_f2_labeled.bmp";
    string histName = baseName + "_histogram.bmp";
    
    imwrite(f1Name, f1);
    imwrite(f2Name, f2);
    imwrite(histName, histImage);
    
    cout << "\nImages saved:" << endl;
    cout << "- " << f1Name << " (binary image)" << endl;
    cout << "- " << f2Name << " (labeled image with colors)" << endl;
    cout << "- " << histName << " (image histogram)" << endl;
    
    cout << "\nColor coding:" << endl;
    cout << "- Gray level 200 (RED): Largest region" << endl;
    cout << "- Gray level 120 (GREEN): Medium region" << endl;
    cout << "- Gray level 60 (BLUE): Smallest region" << endl;
}

int main() {
    cout << "CS455 Project 1 - Connected Component Detection" << endl;
    cout << "===============================================" << endl;
    
    // List of images to process
    vector<string> imageFiles = {"guide_8bits.bmp", "shapes2.1.bmp"};
    
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
