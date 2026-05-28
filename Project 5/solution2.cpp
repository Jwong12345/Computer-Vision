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
            file.seekg(54 + 1024, ios::beg); // Skip palette
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

    uint8_t getPixel(int x, int y) const {
        // Clamp coordinates to image boundaries
        x = max(0, min(x, width - 1));
        y = max(0, min(y, height - 1));
        return data[y * width + x];
    }
};

// Motion vector structure
struct MotionVector {
    int dx, dy;  // Displacement in x and y directions
    int blockX, blockY;  // Position of the block in frame 1
    double sad;  // Sum of Absolute Differences
};

// Calculate Sum of Absolute Differences for an 8x8 block
double calculateSAD(const Image& img1, int x1, int y1, 
                    const Image& img2, int x2, int y2, int blockSize = 8) {
    double sad = 0.0;
    for (int dy = 0; dy < blockSize; dy++) {
        for (int dx = 0; dx < blockSize; dx++) {
            int px1 = x1 + dx;
            int py1 = y1 + dy;
            int px2 = x2 + dx;
            int py2 = y2 + dy;
            
            if (px1 >= 0 && px1 < img1.width && py1 >= 0 && py1 < img1.height &&
                px2 >= 0 && px2 < img2.width && py2 >= 0 && py2 < img2.height) {
                int diff = static_cast<int>(img1.at(px1, py1)) - static_cast<int>(img2.at(px2, py2));
                sad += abs(diff);
            }
        }
    }
    return sad;
}

// Block-based motion estimation using full search
vector<MotionVector> estimateMotionVectors(const Image& frame1, const Image& frame2, 
    int blockSize = 8, int searchRange = 16) {
    vector<MotionVector> motionVectors;
    
    // Process frame in 8x8 blocks
    for (int by = 0; by < frame1.height; by += blockSize) {
        for (int bx = 0; bx < frame1.width; bx += blockSize) {
            double minSAD = numeric_limits<double>::max();
            int bestDx = 0, bestDy = 0;
            
            // Search in the search window
            for (int dy = -searchRange; dy <= searchRange; dy++) {
                for (int dx = -searchRange; dx <= searchRange; dx++) {
                    int searchX = bx + dx;
                    int searchY = by + dy;
                    
                    // Check if search position is valid
                    if (searchX >= 0 && searchX + blockSize <= frame2.width &&
                        searchY >= 0 && searchY + blockSize <= frame2.height) {
                        
                        double sad = calculateSAD(frame1, bx, by, frame2, searchX, searchY, blockSize);
                        
                        if (sad < minSAD) {
                            minSAD = sad;
                            bestDx = dx;
                            bestDy = dy;
                        }
                    }
                }
            }
            
            MotionVector mv;
            mv.dx = bestDx;
            mv.dy = bestDy;
            mv.blockX = bx;
            mv.blockY = by;
            mv.sad = minSAD;
            motionVectors.push_back(mv);
        }
    }
    
    return motionVectors;
}

// Visualize motion vectors on an image
Image visualizeMotionVectors(const Image& frame, const vector<MotionVector>& motionVectors, 
    int blockSize = 8, int scale = 2) {
    Image result = frame;
    
    // Draw motion vectors as lines
    for (const auto& mv : motionVectors) {
        int centerX = mv.blockX + blockSize / 2;
        int centerY = mv.blockY + blockSize / 2;
        int endX = centerX + mv.dx * scale;
        int endY = centerY + mv.dy * scale;
        
        // Draw line using Bresenham's algorithm (simplified)
        int steps = max(abs(endX - centerX), abs(endY - centerY));
        if (steps > 0) {
            for (int i = 0; i <= steps; i++) {
                int x = centerX + (endX - centerX) * i / steps;
                int y = centerY + (endY - centerY) * i / steps;
                
                if (x >= 0 && x < result.width && y >= 0 && y < result.height) {
                    result.at(x, y) = 255;  // White line
                }
            }
        }
        
        // Draw arrow head (simple dot at the end)
        if (endX >= 0 && endX < result.width && endY >= 0 && endY < result.height) {
            result.at(endX, endY) = 255;
            // Draw a small cross at the end
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    int ax = endX + dx;
                    int ay = endY + dy;
                    if (ax >= 0 && ax < result.width && ay >= 0 && ay < result.height) {
                        result.at(ax, ay) = 255;
                    }
                }
            }
        }
        
        // Draw block boundary
        for (int dx = 0; dx < blockSize; dx++) {
            if (mv.blockX + dx < result.width) {
                if (mv.blockY >= 0 && mv.blockY < result.height) {
                    result.at(mv.blockX + dx, mv.blockY) = 128;
                }
                if (mv.blockY + blockSize - 1 >= 0 && mv.blockY + blockSize - 1 < result.height) {
                    result.at(mv.blockX + dx, mv.blockY + blockSize - 1) = 128;
                }
            }
        }
        for (int dy = 0; dy < blockSize; dy++) {
            if (mv.blockY + dy < result.height) {
                if (mv.blockX >= 0 && mv.blockX < result.width) {
                    result.at(mv.blockX, mv.blockY + dy) = 128;
                }
                if (mv.blockX + blockSize - 1 >= 0 && mv.blockX + blockSize - 1 < result.width) {
                    result.at(mv.blockX + blockSize - 1, mv.blockY + dy) = 128;
                }
            }
        }
    }
    
    return result;
}

// Calculate frame difference with improved visualization
Image calculateFrameDifference(const Image& frame1, const Image& frame2, bool overlay = true) {
    Image diff(frame1.width, frame1.height);
    
    // First pass: find maximum difference for normalization
    int maxDiff = 0;
    for (int y = 0; y < frame1.height; y++) {
        for (int x = 0; x < frame1.width; x++) {
            int pixel1 = static_cast<int>(frame1.at(x, y));
            int pixel2 = static_cast<int>(frame2.at(x, y));
            int difference = abs(pixel1 - pixel2);
            maxDiff = max(maxDiff, difference);
        }
    }
    
    // Normalize and enhance differences
    double scale = (maxDiff > 0) ? (255.0 / maxDiff) : 1.0;
    // Apply additional enhancement (multiply by 3 for better visibility)
    scale = min(scale * 3.0, 10.0);
    
    for (int y = 0; y < frame1.height; y++) {
        for (int x = 0; x < frame1.width; x++) {
            int pixel1 = static_cast<int>(frame1.at(x, y));
            int pixel2 = static_cast<int>(frame2.at(x, y));
            int difference = abs(pixel1 - pixel2);
            
            if (overlay) {
                // Overlay difference on original frame (blend with frame2)
                int diffValue = static_cast<int>(difference * scale);
                diffValue = min(255, diffValue);
                // Blend: 70% original frame, 30% difference (brightened)
                int blended = static_cast<int>(0.7 * pixel2 + 0.3 * diffValue);
                diff.at(x, y) = static_cast<uint8_t>(min(255, blended));
            } else {
                // Pure difference image (brightened)
                int diffValue = static_cast<int>(difference * scale);
                diff.at(x, y) = static_cast<uint8_t>(min(255, diffValue));
            }
        }
    }
    
    return diff;
}

// Create motion-compensated prediction frame
Image createMotionCompensatedFrame(const Image& frame1, const vector<MotionVector>& motionVectors, 
    int blockSize = 8) {
    Image predicted(frame1.width, frame1.height);
    
    // Initialize with frame1
    for (int y = 0; y < frame1.height; y++) {
        for (int x = 0; x < frame1.width; x++) {
            predicted.at(x, y) = frame1.at(x, y);
        }
    }
    
    // Apply motion vectors to create prediction
    for (const auto& mv : motionVectors) {
        for (int dy = 0; dy < blockSize; dy++) {
            for (int dx = 0; dx < blockSize; dx++) {
                int srcX = mv.blockX + dx;
                int srcY = mv.blockY + dy;
                int dstX = srcX + mv.dx;
                int dstY = srcY + mv.dy;
                
                if (srcX >= 0 && srcX < frame1.width && srcY >= 0 && srcY < frame1.height &&
                    dstX >= 0 && dstX < predicted.width && dstY >= 0 && dstY < predicted.height) {
                    predicted.at(dstX, dstY) = frame1.at(srcX, srcY);
                }
            }
        }
    }
    
    return predicted;
}

// Calculate motion-compensated difference with improved visualization
Image calculateMotionCompensatedDifference(const Image& frame1, const Image& frame2, 
    const vector<MotionVector>& motionVectors, 
    int blockSize = 8, bool overlay = true) {
    // Create motion-compensated prediction
    Image predicted = createMotionCompensatedFrame(frame1, motionVectors, blockSize);
    
    // First pass: find maximum difference for normalization
    int maxDiff = 0;
    for (int y = 0; y < frame2.height; y++) {
        for (int x = 0; x < frame2.width; x++) {
            int pixelPred = static_cast<int>(predicted.at(x, y));
            int pixelActual = static_cast<int>(frame2.at(x, y));
            int difference = abs(pixelPred - pixelActual);
            maxDiff = max(maxDiff, difference);
        }
    }
    
    // Normalize and enhance differences
    double scale = (maxDiff > 0) ? (255.0 / maxDiff) : 1.0;
    // Apply additional enhancement (multiply by 3 for better visibility)
    scale = min(scale * 3.0, 10.0);
    
    // Calculate difference between predicted and actual frame2
    Image diff(frame2.width, frame2.height);
    
    for (int y = 0; y < frame2.height; y++) {
        for (int x = 0; x < frame2.width; x++) {
            int pixelPred = static_cast<int>(predicted.at(x, y));
            int pixelActual = static_cast<int>(frame2.at(x, y));
            int difference = abs(pixelPred - pixelActual);
            
            if (overlay) {
                // Overlay difference on original frame (blend with frame2)
                int diffValue = static_cast<int>(difference * scale);
                diffValue = min(255, diffValue);
                // Blend: 70% original frame, 30% difference (brightened)
                int blended = static_cast<int>(0.7 * pixelActual + 0.3 * diffValue);
                diff.at(x, y) = static_cast<uint8_t>(min(255, blended));
            } else {
                // Pure difference image (brightened)
                int diffValue = static_cast<int>(difference * scale);
                diff.at(x, y) = static_cast<uint8_t>(min(255, diffValue));
            }
        }
    }
    
    return diff;
}

// Process a pair of frames
void processFramePair(const string& frame1Name, const string& frame2Name,
    const string& prefix) {
    cout << "\nProcessing " << prefix << " frames..." << endl;
    
    // Load frames
    Image frame1, frame2;
    if (!frame1.loadBMP(frame1Name)) {
        cerr << "Failed to load " << frame1Name << endl;
        return;
    }
    if (!frame2.loadBMP(frame2Name)) {
        cerr << "Failed to load " << frame2Name << endl;
        return;
    }
    
    if (frame1.width != frame2.width || frame1.height != frame2.height) {
        cerr << "Error: Frame dimensions don't match!" << endl;
        return;
    }
    
    cout << "Frame size: " << frame1.width << "x" << frame1.height << endl;
    
    // Estimate motion vectors
    cout << "Estimating motion vectors..." << endl;
    vector<MotionVector> motionVectors = estimateMotionVectors(frame1, frame2, 8, 16);
    cout << "Found " << motionVectors.size() << " motion vectors" << endl;
    
    // Calculate statistics
    double totalMagnitude = 0;
    double maxMagnitude = 0;
    for (const auto& mv : motionVectors) {
        double magnitude = sqrt(mv.dx * mv.dx + mv.dy * mv.dy);
        totalMagnitude += magnitude;
        maxMagnitude = max(maxMagnitude, magnitude);
    }
    cout << "Average motion magnitude: " << (totalMagnitude / motionVectors.size()) << endl;
    cout << "Maximum motion magnitude: " << maxMagnitude << endl;
    
    // Visualize motion vectors
    Image motionViz = visualizeMotionVectors(frame1, motionVectors, 8, 2);
    string mvFilename = prefix + "_motion_vectors.bmp";
    motionViz.saveBMP(mvFilename);
    cout << "Saved motion vectors visualization: " << mvFilename << endl;
    
    // Calculate frame difference (with overlay for better visibility)
    Image frameDiff = calculateFrameDifference(frame1, frame2, true);
    string diffFilename = prefix + "_frame_difference.bmp";
    frameDiff.saveBMP(diffFilename);
    cout << "Saved frame difference (overlaid on original): " << diffFilename << endl;
    
    // Also create pure difference image (brightened)
    Image frameDiffPure = calculateFrameDifference(frame1, frame2, false);
    string diffPureFilename = prefix + "_frame_difference_pure.bmp";
    frameDiffPure.saveBMP(diffPureFilename);
    cout << "Saved pure frame difference (brightened): " << diffPureFilename << endl;
    
    // Calculate motion-compensated difference (with overlay for better visibility)
    Image compensatedDiff = calculateMotionCompensatedDifference(frame1, frame2, motionVectors, 8, true);
    string compDiffFilename = prefix + "_compensated_difference.bmp";
    compensatedDiff.saveBMP(compDiffFilename);
    cout << "Saved motion-compensated difference (overlaid on original): " << compDiffFilename << endl;
    
    // Also create pure compensated difference image (brightened)
    Image compensatedDiffPure = calculateMotionCompensatedDifference(frame1, frame2, motionVectors, 8, false);
    string compDiffPureFilename = prefix + "_compensated_difference_pure.bmp";
    compensatedDiffPure.saveBMP(compDiffPureFilename);
    cout << "Saved pure motion-compensated difference (brightened): " << compDiffPureFilename << endl;
    
    // Calculate and display error metrics (using pure difference images)
    double totalDiff = 0, totalCompDiff = 0;
    for (int y = 0; y < frame2.height; y++) {
        for (int x = 0; x < frame2.width; x++) {
            totalDiff += static_cast<double>(frameDiffPure.at(x, y));
            totalCompDiff += static_cast<double>(compensatedDiffPure.at(x, y));
        }
    }
    double meanDiff = totalDiff / (frame2.width * frame2.height);
    double meanCompDiff = totalCompDiff / (frame2.width * frame2.height);
    
    cout << "Mean frame difference: " << meanDiff << endl;
    cout << "Mean compensated difference: " << meanCompDiff << endl;
    cout << "Improvement: " << ((meanDiff - meanCompDiff) / meanDiff * 100) << "%" << endl;
}

int main() {
    cout << "Motion Estimation Algorithm" << endl;
    cout << "===========================" << endl;
    cout << "\nAlgorithm: Block-based motion estimation with 8x8 blocks" << endl;
    cout << "Search method: Full search within ±16 pixel range" << endl;
    cout << "Matching criterion: Sum of Absolute Differences (SAD)" << endl;
    
    // Process Tennis frames
    processFramePair("TennisFrame1_320x200.bmp", "TennisFrame2_320x200.bmp", "Tennis");
    
    // Process Play frames
    processFramePair("PlayFrame1_320x200.bmp", "PlayFrame2_320x200.bmp", "Play");
    
    cout << "\nAll processing complete!" << endl;
    
    return 0;
}

