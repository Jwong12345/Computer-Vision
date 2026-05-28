#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <cmath>

using namespace std;

// BMP file header structure
#pragma pack(push, 1)
struct BMPHeader {
    char signature[2];        // "BM"
    uint32_t fileSize;        // File size in bytes
    uint32_t reserved;        // Reserved
    uint32_t dataOffset;      // Offset to image data
    uint32_t headerSize;      // Header size (40 for BITMAPINFOHEADER)
    int32_t width;           // Image width
    int32_t height;          // Image height
    uint16_t planes;         // Number of color planes
    uint16_t bitsPerPixel;   // Bits per pixel
    uint32_t compression;    // Compression type
    uint32_t imageSize;      // Image size in bytes
    int32_t xPixelsPerMeter; // Horizontal resolution
    int32_t yPixelsPerMeter; // Vertical resolution
    uint32_t colorsUsed;     // Number of colors used
    uint32_t colorsImportant; // Number of important colors
};
#pragma pack(pop)

class ImageProcessor {
private:
    vector<vector<unsigned char>> image;
    int width, height;
    int inputBitDepth;
    
public:
    ImageProcessor() : width(0), height(0), inputBitDepth(0) {}
    
    // Read BMP file
    bool readBMP(const string& filename) {
        ifstream file(filename, ios::binary);
        if (!file.is_open()) {
            cout << "Error: Cannot open file " << filename << endl;
            return false;
        }
        
        BMPHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(header));
        
        if (header.signature[0] != 'B' || header.signature[1] != 'M') {
            cout << "Error: Not a valid BMP file" << endl;
            return false;
        }
        
        if (header.bitsPerPixel != 8 && header.bitsPerPixel != 24) {
            cout << "Error: Only 8-bit and 24-bit BMP files are supported" << endl;
            return false;
        }
        
        width = header.width;
        height = header.height;
        inputBitDepth = header.bitsPerPixel;
        
        // Skip to image data
        file.seekg(header.dataOffset);
        
        // Read image data (BMP is stored bottom-up)
        image.resize(height, vector<unsigned char>(width));
        
        if (header.bitsPerPixel == 8) {
            // 8-bit grayscale: read directly
            for (int y = height - 1; y >= 0; y--) {
                file.read(reinterpret_cast<char*>(image[y].data()), width);
                // Skip padding
                int padding = (4 - (width % 4)) % 4;
                file.seekg(padding, ios::cur);
            }
        } else if (header.bitsPerPixel == 24) {
            // 24-bit RGB: convert to grayscale
            for (int y = height - 1; y >= 0; y--) {
                for (int x = 0; x < width; x++) {
                    unsigned char b, g, r;
                    file.read(reinterpret_cast<char*>(&b), 1);
                    file.read(reinterpret_cast<char*>(&g), 1);
                    file.read(reinterpret_cast<char*>(&r), 1);
                    
                    // Convert RGB to grayscale using standard formula
                    // Gray = 0.299*R + 0.587*G + 0.114*B
                    unsigned char gray = static_cast<unsigned char>(0.299 * r + 0.587 * g + 0.114 * b);
                    image[y][x] = gray;
                }
                // Skip padding
                int padding = (4 - ((width * 3) % 4)) % 4;
                file.seekg(padding, ios::cur);
            }
        }
        
        file.close();
        return true;
    }
    
    // Write BMP file
    bool writeBMP(const string& filename, bool output24bit = false) {
        ofstream file(filename, ios::binary);
        if (!file.is_open()) {
            cout << "Error: Cannot create file " << filename << endl;
            return false;
        }
        
        if (output24bit) {
            // 24-bit RGB output
            int bytesPerPixel = 3;
            int padding = (4 - ((width * bytesPerPixel) % 4)) % 4;
            int rowSize = width * bytesPerPixel + padding;
            
            // Create BMP header for 24-bit
            BMPHeader header = {};
            header.signature[0] = 'B';
            header.signature[1] = 'M';
            header.fileSize = sizeof(BMPHeader) + height * rowSize;
            header.dataOffset = sizeof(BMPHeader);
            header.headerSize = 40;
            header.width = width;
            header.height = height;
            header.planes = 1;
            header.bitsPerPixel = 24;
            header.compression = 0;
            header.imageSize = height * rowSize;
            header.xPixelsPerMeter = 2835;
            header.yPixelsPerMeter = 2835;
            header.colorsUsed = 0;
            header.colorsImportant = 0;
            
            file.write(reinterpret_cast<char*>(&header), sizeof(header));
            
            // Write image data as RGB (BMP is stored bottom-up)
            for (int y = height - 1; y >= 0; y--) {
                for (int x = 0; x < width; x++) {
                    unsigned char gray = image[y][x];
                    // Write as BGR (Blue, Green, Red)
                    file.put(gray); // Blue
                    file.put(gray); // Green  
                    file.put(gray); // Red
                }
                // Write padding
                for (int p = 0; p < padding; p++) {
                    file.put(0);
                }
            }
        } else {
            // 8-bit grayscale output
            int padding = (4 - (width % 4)) % 4;
            int rowSize = width + padding;
            
            // Create BMP header for 8-bit
            BMPHeader header = {};
            header.signature[0] = 'B';
            header.signature[1] = 'M';
            header.fileSize = sizeof(BMPHeader) + height * rowSize;
            header.dataOffset = sizeof(BMPHeader);
            header.headerSize = 40;
            header.width = width;
            header.height = height;
            header.planes = 1;
            header.bitsPerPixel = 8;
            header.compression = 0;
            header.imageSize = height * rowSize;
            header.xPixelsPerMeter = 2835;
            header.yPixelsPerMeter = 2835;
            header.colorsUsed = 256;
            header.colorsImportant = 256;
            
            file.write(reinterpret_cast<char*>(&header), sizeof(header));
            
            // Write color palette (grayscale)
            for (int i = 0; i < 256; i++) {
                unsigned char palette[4] = {i, i, i, 0};
                file.write(reinterpret_cast<char*>(palette), 4);
            }
            
            // Write image data (BMP is stored bottom-up)
            for (int y = height - 1; y >= 0; y--) {
                file.write(reinterpret_cast<char*>(image[y].data()), width);
                // Write padding
                for (int p = 0; p < padding; p++) {
                    file.put(0);
                }
            }
        }
        
        file.close();
        return true;
    }
    
    // Convert to binary image using threshold
    void convertToBinary(int threshold = 128) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                image[y][x] = (image[y][x] > threshold) ? 255 : 0;
            }
        }
    }
    
    // Dilation operation with ball-shaped kernel
    void dilation(int kernelSize = 3) {
        vector<vector<unsigned char>> result(height, vector<unsigned char>(width, 0));
        int radius = kernelSize;
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                unsigned char maxVal = 0;
                
                // Check ball-shaped neighborhood (circular)
                for (int ky = -radius; ky <= radius; ky++) {
                    for (int kx = -radius; kx <= radius; kx++) {
                        // Check if point is within circle (ball shape)
                        if (ky * ky + kx * kx <= radius * radius) {
                            int ny = y + ky;
                            int nx = x + kx;
                            
                            if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                                maxVal = max(maxVal, image[ny][nx]);
                            }
                        }
                    }
                }
                
                result[y][x] = maxVal;
            }
        }
        
        image = result;
    }
    
    // Erosion operation with ball-shaped kernel
    void erosion(int kernelSize = 3) {
        vector<vector<unsigned char>> result(height, vector<unsigned char>(width, 255));
        int radius = kernelSize;
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                unsigned char minVal = 255;
                
                // Check ball-shaped neighborhood (circular)
                for (int ky = -radius; ky <= radius; ky++) {
                    for (int kx = -radius; kx <= radius; kx++) {
                        // Check if point is within circle (ball shape)
                        if (ky * ky + kx * kx <= radius * radius) {
                            int ny = y + ky;
                            int nx = x + kx;
                            
                            if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                                minVal = min(minVal, image[ny][nx]);
                            }
                        }
                    }
                }
                
                result[y][x] = minVal;
            }
        }
        
        image = result;
    }
    
    // Opening operation (erosion followed by dilation)
    void opening(int kernelSize = 3) {
        erosion(kernelSize);
        dilation(kernelSize);
    }
    
    // Closing operation (dilation followed by erosion)
    void closing(int kernelSize = 3) {
        dilation(kernelSize);
        erosion(kernelSize);
    }
    
    // Remove small objects (noise) based on area threshold
    void removeSmallObjects(int minArea = 100) {
        vector<vector<bool>> visited(height, vector<bool>(width, false));
        vector<vector<pair<int, int>>> objects;
        
        // Find all objects and their areas
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (image[y][x] == 255 && !visited[y][x]) {
                    vector<pair<int, int>> object;
                    floodFillWithCollection(x, y, visited, object);
                    objects.push_back(object);
                }
            }
        }
        
        // Remove small objects
        for (const auto& object : objects) {
            if (object.size() < minArea) {
                for (const auto& pixel : object) {
                    image[pixel.second][pixel.first] = 0;
                }
            }
        }
    }
    
    // Advanced separation using erosion to break connections
    void advancedSeparation() {
        // Create a copy for erosion
        vector<vector<unsigned char>> original = image;
        
        // Apply multiple erosions to break connections
        for (int i = 0; i < 5; i++) {
            erosion(3);
        }
        
        // Dilate back to restore object size
        for (int i = 0; i < 5; i++) {
            dilation(3);
        }
        
        // Use the original image as a mask to prevent over-erosion
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (original[y][x] == 0) {
                    image[y][x] = 0;
                }
            }
        }
    }
    
    // Distance-based separation to break connections
    void distanceBasedSeparation() {
        // Apply erosion to create gaps
        for (int i = 0; i < 2; i++) {
            erosion(3);
        }
        
        // Dilate back
        for (int i = 0; i < 1; i++) {
            dilation(3);
        }
    }
    
    // Skeletonization-like approach for better separation
    void skeletonSeparation() {
        // Apply more aggressive erosion to break larger bridges
        for (int i = 0; i < 7; i++) {
            erosion(3);
        }
        
        // Dilate back to restore some thickness
        for (int i = 0; i < 2; i++) {
            dilation(3);
        }
    }
    

    // Connected components labeling using flood fill
    int countObjects() {
        vector<vector<bool>> visited(height, vector<bool>(width, false));
        int objectCount = 0;
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (image[y][x] == 255 && !visited[y][x]) {
                    // Found a new object, flood fill to mark it
                    floodFill(x, y, visited);
                    objectCount++;
                }
            }
        }
        
        return objectCount;
    }
    
private:
    void floodFill(int startX, int startY, vector<vector<bool>>& visited) {
        queue<pair<int, int>> q;
        q.push({startX, startY});
        visited[startY][startX] = true;
        
        int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
        int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};
        
        while (!q.empty()) {
            int x = q.front().first;
            int y = q.front().second;
            q.pop();
            
            for (int i = 0; i < 8; i++) {
                int nx = x + dx[i];
                int ny = y + dy[i];
                
                if (nx >= 0 && nx < width && ny >= 0 && ny < height &&
                    image[ny][nx] == 255 && !visited[ny][nx]) {
                    visited[ny][nx] = true;
                    q.push({nx, ny});
                }
            }
        }
    }
    
    void floodFillWithCollection(int startX, int startY, vector<vector<bool>>& visited, vector<pair<int, int>>& object) {
        queue<pair<int, int>> q;
        q.push({startX, startY});
        visited[startY][startX] = true;
        object.push_back({startX, startY});
        
        int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
        int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};
        
        while (!q.empty()) {
            int x = q.front().first;
            int y = q.front().second;
            q.pop();
            
            for (int i = 0; i < 8; i++) {
                int nx = x + dx[i];
                int ny = y + dy[i];
                
                if (nx >= 0 && nx < width && ny >= 0 && ny < height &&
                    image[ny][nx] == 255 && !visited[ny][nx]) {
                    visited[ny][nx] = true;
                    object.push_back({nx, ny});
                    q.push({nx, ny});
                }
            }
        }
    }
    
public:
    // Get image dimensions
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
    // Get input bit depth
    int getInputBitDepth() const { return inputBitDepth; }
    
    // Get pixel value
    unsigned char getPixel(int x, int y) const {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            return image[y][x];
        }
        return 0;
    }
    
    // Set pixel value
    void setPixel(int x, int y, unsigned char value) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            image[y][x] = value;
        }
    }
};

int main() {
    cout << "Morphological Operations for Object Separation and Counting" << endl;
    cout << "=========================================================" << endl;
    
    // Process both test images
    vector<string> testImages = {"TestImage-even-width.bmp", "TestImage-odd-width.bmp"};
    
    for (const string& filename : testImages) {
        cout << "\nProcessing: " << filename << endl;
        cout << "----------------------------" << endl;
        
        ImageProcessor processor;
        
        // Read the image
        if (!processor.readBMP(filename)) {
            cout << "Failed to read " << filename << endl;
            continue;
        }
        
        cout << "Image dimensions: " << processor.getWidth() << " x " << processor.getHeight() << endl;
        cout << "Input format: " << processor.getInputBitDepth() << "-bit " << 
                (processor.getInputBitDepth() == 8 ? "grayscale" : "RGB") << " BMP" << endl;
        
        // Step 1: Convert to binary image with adaptive threshold
        cout << "Step 1: Converting to binary image..." << endl;
        int threshold = 140; // Slightly higher threshold to reduce noise
        cout << "  - Using threshold: " << threshold << endl;
        processor.convertToBinary(threshold);
        processor.writeBMP(filename.substr(0, filename.find('.')) + "_binary.bmp");
        processor.writeBMP(filename.substr(0, filename.find('.')) + "_binary_24bit.bmp", true);
        cout << "Binary image saved as: " << filename.substr(0, filename.find('.')) + "_binary.bmp" << endl;
        cout << "Binary image (24-bit) saved as: " << filename.substr(0, filename.find('.')) + "_binary_24bit.bmp" << endl;
        
        // Step 2: Apply aggressive morphological operations to separate objects
        cout << "Step 2: Applying aggressive morphological operations..." << endl;
        
        // Initial aggressive opening to remove noise and break connections
        cout << "  - Initial aggressive opening to remove noise and break connections..." << endl;
        processor.opening(3); // Radius 3 ball kernel to remove noise and break connections
        processor.writeBMP(filename.substr(0, filename.find('.')) + "_initial_opened.bmp");
        processor.writeBMP(filename.substr(0, filename.find('.')) + "_initial_opened_24bit.bmp", true);
        cout << "Initial opened image saved as: " << filename.substr(0, filename.find('.')) + "_initial_opened.bmp" << endl;
        cout << "Initial opened image (24-bit) saved as: " << filename.substr(0, filename.find('.')) + "_initial_opened_24bit.bmp" << endl;
        
        // Closing to fill holes within objects
        cout << "  - Closing to fill holes within objects..." << endl;
        processor.closing(3); // Radius 3 ball kernel to fill holes without connecting objects
        processor.writeBMP(filename.substr(0, filename.find('.')) + "_closed.bmp");
        processor.writeBMP(filename.substr(0, filename.find('.')) + "_closed_24bit.bmp", true);
        cout << "Closed image saved as: " << filename.substr(0, filename.find('.')) + "_closed.bmp" << endl;
        cout << "Closed image (24-bit) saved as: " << filename.substr(0, filename.find('.')) + "_closed_24bit.bmp" << endl;
        
        // Distance-based separation to break connections
        cout << "  - Distance-based separation to break connections..." << endl;
        processor.distanceBasedSeparation();
        processor.writeBMP(filename.substr(0, filename.find('.')) + "_distance_separated.bmp");
        processor.writeBMP(filename.substr(0, filename.find('.')) + "_distance_separated_24bit.bmp", true);
        cout << "Distance separated image saved as: " << filename.substr(0, filename.find('.')) + "_distance_separated.bmp" << endl;
        cout << "Distance separated image (24-bit) saved as: " << filename.substr(0, filename.find('.')) + "_distance_separated_24bit.bmp" << endl;
        
        // Remove small noise objects
        cout << "  - Removing small noise objects..." << endl;
        processor.removeSmallObjects(150); // Reduced threshold to keep more objects
        processor.writeBMP(filename.substr(0, filename.find('.')) + "_cleaned.bmp");
        processor.writeBMP(filename.substr(0, filename.find('.')) + "_cleaned_24bit.bmp", true);
        cout << "Cleaned image saved as: " << filename.substr(0, filename.find('.')) + "_cleaned.bmp" << endl;
        cout << "Cleaned image (24-bit) saved as: " << filename.substr(0, filename.find('.')) + "_cleaned_24bit.bmp" << endl;
        
        // Skeleton-based separation for final refinement
        cout << "  - Skeleton-based separation for final refinement..." << endl;
        processor.skeletonSeparation();
        processor.writeBMP(filename.substr(0, filename.find('.')) + "_skeleton_separated.bmp");
        processor.writeBMP(filename.substr(0, filename.find('.')) + "_skeleton_separated_24bit.bmp", true);
        cout << "Skeleton separated image saved as: " << filename.substr(0, filename.find('.')) + "_skeleton_separated.bmp" << endl;
        cout << "Skeleton separated image (24-bit) saved as: " << filename.substr(0, filename.find('.')) + "_skeleton_separated_24bit.bmp" << endl;

        // Step 3: Count objects
        cout << "Step 3: Counting objects..." << endl;
        int objectCount = processor.countObjects();
        cout << "Number of objects detected: " << objectCount << endl;
        
        // Save final result
        processor.writeBMP(filename.substr(0, filename.find('.')) + "_final.bmp");
        processor.writeBMP(filename.substr(0, filename.find('.')) + "_final_24bit.bmp", true);
        cout << "Final result saved as: " << filename.substr(0, filename.find('.')) + "_final.bmp" << endl;
        cout << "Final result (24-bit) saved as: " << filename.substr(0, filename.find('.')) + "_final_24bit.bmp" << endl;
    }
    
    cout << "\nProcessing complete!" << endl;
    cout << "\nControlled Erosion Separation Algorithm Summary:" << endl;
    cout << "1. Convert grayscale image to binary using higher threshold" << endl;
    cout << "2. Balanced opening (radius 3 ball) to remove noise and break connections" << endl;
    cout << "3. Closing (radius 3 ball) to fill holes within objects" << endl;
    cout << "4. Distance-based separation using ball-shaped kernels" << endl;
    cout << "5. Remove small objects based on area threshold" << endl;
    cout << "6. Enhanced skeleton-based separation (4 iterations, radius 3)" << endl;
    cout << "7. Use connected components analysis to count objects" << endl;
    
    return 0;
}
