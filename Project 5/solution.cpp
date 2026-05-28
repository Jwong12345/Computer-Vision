#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <climits>
#include <cstring>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::ifstream;
using std::ofstream;
using std::ios;
using std::max;
using std::min;
using std::abs;
using std::sqrt;
using std::round;
using std::numeric_limits;

#pragma pack(push, 1)
struct BMPHeader {
    char signature[2]; 
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t dataOffset;
};

struct BMPInfoHeader {
    uint32_t headerSize;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    int32_t xPixelsPerM;
    int32_t yPixelsPerM;
    uint32_t colorsUsed;
    uint32_t colorsImportant;
};
#pragma pack(pop)

class Image {
public:
    int width, height;
    vector<uint8_t> data;

    Image() : width(0), height(0) {}
    
    Image(int w, int h) : width(w), height(h), data(w * h, 0) {}

    bool loadBMP(const string& filename) {
        ifstream file(filename, ios::binary);
        if (!file.is_open()) {
            cerr << "Error: Cannot open file " << filename << endl;
            return false;
        }

        BMPHeader header;
        BMPInfoHeader infoHeader;

        file.read(reinterpret_cast<char*>(&header), sizeof(header));
        file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

        if (header.signature[0] != 'B' || header.signature[1] != 'M') {
            cerr << "Error: Not a valid BMP file" << endl;
            return false;
        }

        if (infoHeader.bitsPerPixel != 8 && infoHeader.bitsPerPixel != 24) {
            cerr << "Error: Unsupported bits per pixel: " << infoHeader.bitsPerPixel << endl;
            return false;
        }

        width = infoHeader.width;
        height = abs(infoHeader.height);
        bool topDown = infoHeader.height < 0;

        // Skip color palette if 8-bit
        if (infoHeader.bitsPerPixel == 8) {
            file.seekg(54 + 1024, ios::beg);
        } else {
            file.seekg(header.dataOffset, ios::beg);
        }

        int rowSize = ((width * infoHeader.bitsPerPixel + 31) / 32) * 4;
        int padding = rowSize - (width * infoHeader.bitsPerPixel / 8);

        data.resize(width * height);

        if (infoHeader.bitsPerPixel == 8) {
            for (int y = 0; y < height; y++) {
                int row = topDown ? y : height - 1 - y;
                for (int x = 0; x < width; x++) {
                    uint8_t pixel;
                    file.read(reinterpret_cast<char*>(&pixel), 1);
                    data[row * width + x] = pixel;
                }
                file.seekg(padding, ios::cur);
            }
        } else if (infoHeader.bitsPerPixel == 24) {
            for (int y = 0; y < height; y++) {
                int row = topDown ? y : height - 1 - y;
                for (int x = 0; x < width; x++) {
                    uint8_t b, g, r;
                    file.read(reinterpret_cast<char*>(&b), 1);
                    file.read(reinterpret_cast<char*>(&g), 1);
                    file.read(reinterpret_cast<char*>(&r), 1);
                    // Convert to grayscale
                    data[row * width + x] = static_cast<uint8_t>(0.299 * r + 0.587 * g + 0.114 * b);
                }
                file.seekg(padding, ios::cur);
            }
        }

        file.close();
        return true;
    }

    bool saveBMP(const string& filename) {
        ofstream file(filename, ios::binary);
        if (!file.is_open()) {
            cerr << "Error: Cannot create file " << filename << endl;
            return false;
        }

        BMPHeader header;
        BMPInfoHeader infoHeader;

        header.signature[0] = 'B';
        header.signature[1] = 'M';
        int rowSize = ((width + 3) / 4) * 4;
        int imageSize = rowSize * height;
        header.fileSize = 54 + 256 * 4 + imageSize;
        header.reserved1 = 0;
        header.reserved2 = 0;
        header.dataOffset = 54 + 256 * 4;

        infoHeader.headerSize = 40;
        infoHeader.width = width;
        infoHeader.height = height;
        infoHeader.planes = 1;
        infoHeader.bitsPerPixel = 8;
        infoHeader.compression = 0;
        infoHeader.imageSize = imageSize;
        infoHeader.xPixelsPerM = 0;
        infoHeader.yPixelsPerM = 0;
        infoHeader.colorsUsed = 256;
        infoHeader.colorsImportant = 256;

        file.write(reinterpret_cast<char*>(&header), sizeof(header));
        file.write(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

        // Write grayscale palette
        for (int i = 0; i < 256; i++) {
            file.write(reinterpret_cast<const char*>(&i), 1); // B
            file.write(reinterpret_cast<const char*>(&i), 1); // G
            file.write(reinterpret_cast<const char*>(&i), 1); // R
            file.write("\0", 1); // Reserved
        }

        // Write image data
        int padding = rowSize - width;
        for (int y = height - 1; y >= 0; y--) {
            for (int x = 0; x < width; x++) {
                file.write(reinterpret_cast<const char*>(&data[y * width + x]), 1);
            }
            for (int p = 0; p < padding; p++) {
                file.write("\0", 1);
            }
        }

        file.close();
        return true;
    }

    uint8_t& at(int x, int y) {
        return data[y * width + x];
    }

    const uint8_t& at(int x, int y) const {
        return data[y * width + x];
    }
};

// Pattern vector: 16-dimensional (4x4 block)
struct Pattern {
    vector<uint8_t> features; // 16 values
    int classLabel; // 0, 128, or 255
    int originalX, originalY; 

    Pattern() : classLabel(-1), originalX(0), originalY(0), features(16, 0) {}

    double averageGrayLevel() const {
        double sum = 0;
        for (uint8_t val : features) {
            sum += val;
        }
        return sum / 16.0;
    }

    double distance(const Pattern& other) const {
        double dist = 0;
        for (size_t i = 0; i < features.size(); i++) {
            double diff = static_cast<double>(features[i]) - static_cast<double>(other.features[i]);
            dist += diff * diff;
        }
        return sqrt(dist);
    }
};

// Extract 4x4 blocks from image
vector<Pattern> extractPatterns(const Image& img, int startY, int endY) {
    vector<Pattern> patterns;
    
    for (int y = startY; y < endY; y += 4) {
        for (int x = 0; x < img.width; x += 4) {
            if (x + 4 <= img.width && y + 4 <= img.height) {
                Pattern p;
                p.originalX = x;
                p.originalY = y;
                
                for (int dy = 0; dy < 4; dy++) {
                    for (int dx = 0; dx < 4; dx++) {
                        p.features[dy * 4 + dx] = img.at(x + dx, y + dy);
                    }
                }
                patterns.push_back(p);
            }
        }
    }
    
    return patterns;
}

// Manually classify training patterns
void classifyTrainingPatterns(vector<Pattern>& patterns) {
    for (auto& p : patterns) {
        double avg = p.averageGrayLevel();
        if (avg < 125) {
            p.classLabel = 0;
        } else if (avg < 175) {
            p.classLabel = 128;
        } else {
            p.classLabel = 255;
        }
    }
}

// Nearest Neighbor classification
int nearestNeighbor(const Pattern& test, const vector<Pattern>& training) {
    double minDist = numeric_limits<double>::max();
    int nearestIdx = -1;

    for (size_t i = 0; i < training.size(); i++) {
        double dist = test.distance(training[i]);
        if (dist < minDist) {
            minDist = dist;
            nearestIdx = i;
        }
    }

    return nearestIdx;
}

// Create image from patterns with class labels
Image createLabeledImage(const vector<Pattern>& patterns, int imgWidth, int imgHeight, int startY) {
    Image result(imgWidth, imgHeight);
    
    for (const auto& p : patterns) {
        for (int dy = 0; dy < 4; dy++) {
            for (int dx = 0; dx < 4; dx++) {
                if (p.originalX + dx < imgWidth && p.originalY + dy < imgHeight) {
                    result.at(p.originalX + dx, p.originalY + dy) = static_cast<uint8_t>(p.classLabel);
                }
            }
        }
    }
    
    return result;
}

// Combine two images
Image combineImages(const Image& upperImg, const Image& lowerImg, int midY) {
    Image result(upperImg.width, upperImg.height);
    
    // Copy upper half from upperImg
    for (int y = 0; y < midY; y++) {
        for (int x = 0; x < upperImg.width; x++) {
            result.at(x, y) = upperImg.at(x, y);
        }
    }
    
    // Copy lower half from lowerImg
    for (int y = midY; y < result.height; y++) {
        for (int x = 0; x < lowerImg.width; x++) {
            result.at(x, y) = lowerImg.at(x, y);
        }
    }
    
    return result;
}

// Create image N2: Replace testing patterns with nearest training vector
Image createN2Image(const vector<Pattern>& testPatterns, const vector<Pattern>& training, 
                    int imgWidth, int imgHeight, int startY) {
    Image result(imgWidth, imgHeight);
    
    for (const auto& test : testPatterns) {
        int nearestIdx = nearestNeighbor(test, training);
        if (nearestIdx >= 0) {
            const Pattern& nearest = training[nearestIdx];
            for (int dy = 0; dy < 4; dy++) {
                for (int dx = 0; dx < 4; dx++) {
                    if (test.originalX + dx < imgWidth && test.originalY + dy < imgHeight) {
                        result.at(test.originalX + dx, test.originalY + dy) = nearest.features[dy * 4 + dx];
                    }
                }
            }
        }
    }
    
    return result;
}

// Create image N3: Replace with average of nearest training vector
Image createN3Image(const vector<Pattern>& testPatterns, const vector<Pattern>& training,
                    int imgWidth, int imgHeight, int startY) {
    Image result(imgWidth, imgHeight);
    
    for (const auto& test : testPatterns) {
        int nearestIdx = nearestNeighbor(test, training);
        if (nearestIdx >= 0) {
            const Pattern& nearest = training[nearestIdx];
            double avg = nearest.averageGrayLevel();
            uint8_t avgVal = static_cast<uint8_t>(avg);
            
            for (int dy = 0; dy < 4; dy++) {
                for (int dx = 0; dx < 4; dx++) {
                    if (test.originalX + dx < imgWidth && test.originalY + dy < imgHeight) {
                        result.at(test.originalX + dx, test.originalY + dy) = avgVal;
                    }
                }
            }
        }
    }
    
    return result;
}

// Create image N4: Replace with average of corresponding class
Image createN4Image(const vector<Pattern>& testPatterns, const vector<Pattern>& training,
                    int imgWidth, int imgHeight, int startY) {
    Image result(imgWidth, imgHeight);
    
    // Calculate class averages
    vector<double> classSums(3, 0);
    vector<int> classCounts(3, 0);
    
    for (const auto& train : training) {
        int classIdx = (train.classLabel == 0) ? 0 : (train.classLabel == 128) ? 1 : 2;
        classSums[classIdx] += train.averageGrayLevel();
        classCounts[classIdx]++;
    }
    
    vector<double> classAverages(3);
    for (int i = 0; i < 3; i++) {
        classAverages[i] = (classCounts[i] > 0) ? classSums[i] / classCounts[i] : 0;
    }
    
    for (const auto& test : testPatterns) {
        int nearestIdx = nearestNeighbor(test, training);
        if (nearestIdx >= 0) {
            int classLabel = training[nearestIdx].classLabel;
            int classIdx = (classLabel == 0) ? 0 : (classLabel == 128) ? 1 : 2;
            uint8_t avgVal = static_cast<uint8_t>(classAverages[classIdx]);
            
            for (int dy = 0; dy < 4; dy++) {
                for (int dx = 0; dx < 4; dx++) {
                    if (test.originalX + dx < imgWidth && test.originalY + dy < imgHeight) {
                        result.at(test.originalX + dx, test.originalY + dy) = avgVal;
                    }
                }
            }
        }
    }
    
    return result;
}

// K-means clustering
vector<Pattern> kMeansClustering(vector<Pattern>& patterns, int k, int maxIterations = 100) {
    if (patterns.empty()) return patterns;
    
    // Initialize centroids with evenly distributed patterns
    vector<Pattern> centroids(k);
    for (int i = 0; i < k; i++) {
        int idx = (i * patterns.size()) / k;
        if (idx >= static_cast<int>(patterns.size())) idx = patterns.size() - 1;
        centroids[i] = patterns[idx];
        centroids[i].classLabel = (i == 0) ? 0 : (i == 1) ? 128 : 255;
    }
    
    for (int iter = 0; iter < maxIterations; iter++) {
        // Assign patterns to nearest centroid
        for (auto& p : patterns) {
            double minDist = numeric_limits<double>::max();
            int nearestCentroid = -1;
            
            for (size_t i = 0; i < centroids.size(); i++) {
                double dist = p.distance(centroids[i]);
                if (dist < minDist) {
                    minDist = dist;
                    nearestCentroid = i;
                }
            }
            
            if (nearestCentroid >= 0) {
                p.classLabel = centroids[nearestCentroid].classLabel;
            }
        }
        
        // Update centroids - use double for calculations
        vector<vector<double>> newCentroidFeatures(k, vector<double>(16, 0.0));
        vector<int> counts(k, 0);
        
        for (const auto& p : patterns) {
            int classIdx = (p.classLabel == 0) ? 0 : (p.classLabel == 128) ? 1 : 2;
            if (classIdx >= 0 && classIdx < k) {
                for (size_t j = 0; j < p.features.size() && j < 16; j++) {
                    newCentroidFeatures[classIdx][j] += static_cast<double>(p.features[j]);
                }
                counts[classIdx]++;
            }
        }
        
        // Check for convergence and update centroids
        bool converged = true;
        for (int i = 0; i < k; i++) {
            if (counts[i] > 0) {
                Pattern newCentroid;
                newCentroid.features.resize(16);
                newCentroid.classLabel = centroids[i].classLabel;
                
                for (size_t j = 0; j < 16; j++) {
                    double avg = newCentroidFeatures[i][j] / counts[i];
                    newCentroid.features[j] = static_cast<uint8_t>(round(avg));
                }
                
                // Check if centroid changed significantly
                double dist = centroids[i].distance(newCentroid);
                if (dist > 0.1) {
                    converged = false;
                }
                centroids[i] = newCentroid;
            }
        }
        
        if (converged) break;
    }
    
    return patterns;
}

// Compute error rate between two labeled images
double computeErrorRate(const vector<Pattern>& patterns1, const vector<Pattern>& patterns2) {
    if (patterns1.size() != patterns2.size()) {
        cerr << "Error: Pattern count mismatch" << endl;
        return -1;
    }
    
    int errors = 0;
    for (size_t i = 0; i < patterns1.size(); i++) {
        if (patterns1[i].classLabel != patterns2[i].classLabel) {
            errors++;
        }
    }
    
    return static_cast<double>(errors) / patterns1.size();
}

int main() {
    // Load the test image
    Image img;
    if (!img.loadBMP("TestImage-even-width.bmp")) {
        cerr << "Failed to load TestImage-even-width.bmp" << endl;
        return 1;
    }
    
    cout << "Image loaded: " << img.width << "x" << img.height << endl;
    
    // Divide image: upper half for training, lower half for testing
    int midY = img.height / 2;
    
    // Extract training patterns
    vector<Pattern> trainingPatterns = extractPatterns(img, 0, midY);
    cout << "Training patterns: " << trainingPatterns.size() << endl;
    
    // Extract testing patterns
    vector<Pattern> testPatterns = extractPatterns(img, midY, img.height);
    cout << "Testing patterns: " << testPatterns.size() << endl;
    
    // Manually classify training patterns
    classifyTrainingPatterns(trainingPatterns);
    
    // Create M1: Training patterns with class labels
    Image M1 = createLabeledImage(trainingPatterns, img.width, img.height, 0);
    M1.saveBMP("M1.bmp");
    cout << "Created M1.bmp (training patterns with labels)" << endl;
    
    // Perform NN classification on testing patterns
    for (auto& test : testPatterns) {
        int nearestIdx = nearestNeighbor(test, trainingPatterns);
        if (nearestIdx >= 0) {
            test.classLabel = trainingPatterns[nearestIdx].classLabel;
        }
    }
    
    // Create N1: Testing patterns with NN class labels
    Image N1_lower = createLabeledImage(testPatterns, img.width, img.height, midY);
    Image N1 = combineImages(M1, N1_lower, midY);
    N1.saveBMP("N1.bmp");
    cout << "Created N1.bmp (full image: training labels + testing patterns with NN labels)" << endl;
    
    // Create N2: Replace testing patterns with nearest training vector
    Image N2_lower = createN2Image(testPatterns, trainingPatterns, img.width, img.height, midY);
    Image N2 = combineImages(M1, N2_lower, midY);
    N2.saveBMP("N2.bmp");
    cout << "Created N2.bmp (full image: training labels + testing patterns replaced with nearest training vector)" << endl;
    
    // Create N3: Replace with average of nearest training vector
    Image N3_lower = createN3Image(testPatterns, trainingPatterns, img.width, img.height, midY);
    Image N3 = combineImages(M1, N3_lower, midY);
    N3.saveBMP("N3.bmp");
    cout << "Created N3.bmp (full image: training labels + testing patterns replaced with average of nearest training vector)" << endl;
    
    // Create N4: Replace with average of corresponding class
    Image N4_lower = createN4Image(testPatterns, trainingPatterns, img.width, img.height, midY);
    Image N4 = combineImages(M1, N4_lower, midY);
    N4.saveBMP("N4.bmp");
    cout << "Created N4.bmp (full image: training labels + testing patterns replaced with average of corresponding class)" << endl;
    
    // Create T1: Manually classify testing patterns
    vector<Pattern> testPatternsForT1 = extractPatterns(img, midY, img.height);
    classifyTrainingPatterns(testPatternsForT1);
    Image T1_lower = createLabeledImage(testPatternsForT1, img.width, img.height, midY);
    Image T1 = combineImages(M1, T1_lower, midY);
    T1.saveBMP("T1.bmp");
    cout << "Created T1.bmp (full image: training labels + testing patterns with manual labels)" << endl;
    
    // Compute error rate E
    double errorRate = computeErrorRate(testPatterns, testPatternsForT1);
    cout << "Error rate E: " << (errorRate * 100) << "%" << endl;
    
    // Perform k-means clustering on upper half
    vector<Pattern> trainingForKMeans = extractPatterns(img, 0, midY);
    kMeansClustering(trainingForKMeans, 3);
    Image K1 = createLabeledImage(trainingForKMeans, img.width, img.height, 0);
    K1.saveBMP("K1.bmp");
    cout << "Created K1.bmp (k-means clustering on training patterns)" << endl;
    
    cout << "\nAll images created successfully!" << endl;
    
    return 0;
}

