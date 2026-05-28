#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// BMP file header structures
#pragma pack(push, 1)
typedef struct {
    unsigned short bfType;
    unsigned int bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;
} BMPFileHeader;

typedef struct {
    unsigned int biSize;
    int biWidth;
    int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
} BMPInfoHeader;
#pragma pack(pop)

// Function prototypes
int readBMP(const char* filename, unsigned char** image, int* width, int* height, int* bitCount, int* topDown);
int writeBMP(const char* filename, unsigned char* image, int width, int height, int bitCount, int topDown);
void unsharpMasking(unsigned char* input, unsigned char* output, int width, int height, float amount, float radius);
void sobelOperator(unsigned char* input, unsigned char* output, int width, int height);
void laplacianOfGaussian(unsigned char* input, unsigned char* output, int width, int height, int maskSize, float sigma);
void gaussianBlur(unsigned char* input, unsigned char* output, int width, int height, float sigma);
void applyKernel(unsigned char* input, unsigned char* output, int width, int height, float* kernel, int kernelSize);
void normalizeImage(unsigned char* image, int width, int height);

int main() {
    printf("Image Processing for Edge Detection and Enhancement\n");
    
    // Image variables
    unsigned char *f1 = NULL, *f2 = NULL;  // Original images
    unsigned char *E1 = NULL, *E2 = NULL;  // Enhanced images (Unsharp Masking)
    unsigned char *Es1 = NULL, *Es2 = NULL; // Sobel edge images
    unsigned char *E1_1 = NULL, *E1_2 = NULL; // LOG edge images for f1
    unsigned char *E2_1 = NULL, *E2_2 = NULL; // LOG edge images for f2
    
    int width1, height1, width2, height2;
    
    // Read input images
    int bitCount1, bitCount2, topDown1, topDown2;
    printf("Reading input images\n");
    if (!readBMP("basel_gray.bmp", &f1, &width1, &height1, &bitCount1, &topDown1)) {
        printf("Error: Could not read basel_gray.bmp\n");
        return 1;
    }
    printf("Loaded basel_gray.bmp: %dx%d, %d-bit (top-down: %s)\n", width1, height1, bitCount1, topDown1 ? "yes" : "no");
    
    if (!readBMP("ant_gray.bmp", &f2, &width2, &height2, &bitCount2, &topDown2)) {
        printf("Error: Could not read ant_gray.bmp\n");
        return 1;
    }
    printf("Loaded ant_gray.bmp: %dx%d, %d-bit (top-down: %s)\n", width2, height2, bitCount2, topDown2 ? "yes" : "no");
    
    // Allocate memory for output images
    E1 = (unsigned char*)malloc(width1 * height1);
    E2 = (unsigned char*)malloc(width2 * height2);
    Es1 = (unsigned char*)malloc(width1 * height1);
    Es2 = (unsigned char*)malloc(width2 * height2);
    E1_1 = (unsigned char*)malloc(width1 * height1);
    E1_2 = (unsigned char*)malloc(width1 * height1);
    E2_1 = (unsigned char*)malloc(width2 * height2);
    E2_2 = (unsigned char*)malloc(width2 * height2);
    
    if (!E1 || !E2 || !Es1 || !Es2 || !E1_1 || !E1_2 || !E2_1 || !E2_2) {
        printf("Error: Memory allocation failed\n");
        return 1;
    }
    
    // Apply Unsharp Masking
    printf("Applying Unsharp Masking\n");
    unsharpMasking(f1, E1, width1, height1, 1.5f, 1.0f);
    unsharpMasking(f2, E2, width2, height2, 1.5f, 1.0f);
    printf("Unsharp Masking completed\n");
    
    // Apply Sobel Operator
    printf("Applying Sobel Operator\n");
    sobelOperator(f1, Es1, width1, height1);
    sobelOperator(f2, Es2, width2, height2);
    printf("Sobel Operator completed\n");
    
    // Apply Laplacian of Gaussian
    printf("Applying Laplacian of Gaussian\n");
    printf("LOG with 7x7 mask (sigma=1.4)...\n");
    laplacianOfGaussian(f1, E1_1, width1, height1, 7, 1.4f);
    laplacianOfGaussian(f2, E2_1, width2, height2, 7, 1.4f);
    
    printf("LOG with 11x11 mask (sigma=5.0)...\n");
    laplacianOfGaussian(f1, E1_2, width1, height1, 11, 5.0f);
    laplacianOfGaussian(f2, E2_2, width2, height2, 11, 5.0f);
    printf("Laplacian of Gaussian completed\n");
    
    // Save all output images
    printf("\nSaving output images\n");
    
    // Enhanced images (Unsharp Masking)
    writeBMP("basel_enhanced.bmp", E1, width1, height1, bitCount1, topDown1);
    writeBMP("ant_enhanced.bmp", E2, width2, height2, bitCount2, topDown2);
    printf("Enhanced images saved\n");
    
    // Sobel edge images
    writeBMP("basel_sobel.bmp", Es1, width1, height1, bitCount1, topDown1);
    writeBMP("ant_sobel.bmp", Es2, width2, height2, bitCount2, topDown2);
    printf("Sobel edge images saved\n");
    
    // LOG edge images
    writeBMP("basel_log_7x7.bmp", E1_1, width1, height1, bitCount1, topDown1);
    writeBMP("basel_log_11x11.bmp", E1_2, width1, height1, bitCount1, topDown1);
    writeBMP("ant_log_7x7.bmp", E2_1, width2, height2, bitCount2, topDown2);
    writeBMP("ant_log_11x11.bmp", E2_2, width2, height2, bitCount2, topDown2);
    printf("LOG edge images saved\n");
    
    printf("All processing completed successfully!\n");
    printf("Output files:\n");
    printf("- basel_enhanced.bmp, ant_enhanced.bmp (Unsharp Masking)\n");
    printf("- basel_sobel.bmp, ant_sobel.bmp (Sobel Operator)\n");
    printf("- basel_log_7x7.bmp, basel_log_11x11.bmp (LOG for basel)\n");
    printf("- ant_log_7x7.bmp, ant_log_11x11.bmp (LOG for ant)\n");
    
    // Clean up memory
    free(f1); free(f2);
    free(E1); free(E2);
    free(Es1); free(Es2);
    free(E1_1); free(E1_2);
    free(E2_1); free(E2_2);
    
    return 0;
}

// Read BMP file
int readBMP(const char* filename, unsigned char** image, int* width, int* height, int* bitCount, int* topDown) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error: cannot open %s\n", filename);
        return 0;
    }

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    // Read headers
    fread(&fileHeader, sizeof(BMPFileHeader), 1, file);
    fread(&infoHeader, sizeof(BMPInfoHeader), 1, file);

    // Verify "BM"
    if (fileHeader.bfType != 0x4D42) {
        printf("Error: not a BMP file\n");
        fclose(file);
        return 0;
    }

    // Basic info
    int realHeight = infoHeader.biHeight;
    *width  = infoHeader.biWidth;
    *height = abs(realHeight);
    *bitCount = infoHeader.biBitCount;
    *topDown = (realHeight < 0);

    printf("Reading BMP: %dx%d, %d-bit (%s)\n",
           *width, *height, *bitCount, *topDown ? "top-down" : "bottom-up");

    // Support 8-bit and 24-bit uncompressed BMP
    if ((*bitCount != 8 && *bitCount != 24) || infoHeader.biCompression != 0) {
        printf("Error: only uncompressed 8-bit or 24-bit BMP supported\n");
        fclose(file);
        return 0;
    }

    // Allocate grayscale buffer
    *image = (unsigned char*)malloc((*width) * (*height));
    if (!*image) {
        fclose(file);
        return 0;
    }

    // Read color palette for 8-bit images
    unsigned char palette[256][4] = {0}; // BGR + reserved
    if (*bitCount == 8) {
        int paletteSize = (fileHeader.bfOffBits - 54) / 4;
        if (paletteSize > 256) paletteSize = 256;
        
        for (int i = 0; i < paletteSize; i++) {
            fread(&palette[i][0], 1, 4, file); // B, G, R, reserved
        }
    }

    // Seek to pixel array
    fseek(file, fileHeader.bfOffBits, SEEK_SET);

    if (*bitCount == 8) {
        // 8-bit palette-based
        int rowSize = ((*width + 3) / 4) * 4;
        int padding = rowSize - *width;

        for (int y = 0; y < *height; y++) {
            int destY = *topDown ? y : (*height - 1 - y);

            for (int x = 0; x < *width; x++) {
                unsigned char paletteIndex;
                fread(&paletteIndex, 1, 1, file);

                // Convert palette color to grayscale
                unsigned char b = palette[paletteIndex][0];
                unsigned char g = palette[paletteIndex][1];
                unsigned char r = palette[paletteIndex][2];
                
                (*image)[destY * (*width) + x] =
                    (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
            }

            // Skip padding
            if (padding > 0)
                fseek(file, padding, SEEK_CUR);
        }
    } else {
        // 24-bit RGB
        int rowSize = ((*width * 3 + 3) / 4) * 4;
        int padding = rowSize - (*width * 3);

        for (int y = 0; y < *height; y++) {
            int destY = *topDown ? y : (*height - 1 - y);

            for (int x = 0; x < *width; x++) {
                unsigned char b, g, r;
                fread(&b, 1, 1, file);
                fread(&g, 1, 1, file);
                fread(&r, 1, 1, file);

                // grayscale conversion
                (*image)[destY * (*width) + x] =
                    (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
            }

            // Skip padding
            if (padding > 0)
                fseek(file, padding, SEEK_CUR);
        }
    }

    fclose(file);
    return 1;
}


// Write BMP file (preserves original format)
int writeBMP(const char* filename, unsigned char* image, int width, int height, int bitCount, int topDown) {
    FILE* file = fopen(filename, "wb");
    if (!file) return 0;

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;
    
    if (bitCount == 8) {
        // 8-bit palette-based BMP
        int paletteSize = 256 * 4; // 256 colors * 4 bytes each
        int rowSize = ((width + 3) / 4) * 4;
        int imageSize = rowSize * height;
        int padding = rowSize - width;
        
        fileHeader.bfType = 0x4D42;
        fileHeader.bfOffBits = 54 + paletteSize;
        fileHeader.bfSize = 54 + paletteSize + imageSize;
        fileHeader.bfReserved1 = 0;
        fileHeader.bfReserved2 = 0;

        infoHeader.biSize = 40;
        infoHeader.biWidth = width;
        infoHeader.biHeight = topDown ? -height : height;
        infoHeader.biPlanes = 1;
        infoHeader.biBitCount = 8;
        infoHeader.biCompression = 0;
        infoHeader.biSizeImage = imageSize;
        infoHeader.biXPelsPerMeter = 0;
        infoHeader.biYPelsPerMeter = 0;
        infoHeader.biClrUsed = 256;
        infoHeader.biClrImportant = 0;

        // Write headers
        fwrite(&fileHeader, sizeof(BMPFileHeader), 1, file);
        fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, file);

        // Write grayscale palette
        for (int i = 0; i < 256; i++) {
            unsigned char paletteEntry[4] = {i, i, i, 0}; // B, G, R, reserved
            fwrite(paletteEntry, 1, 4, file);
        }

        // Write pixel data
        unsigned char* rowBuffer = (unsigned char*)malloc(rowSize);
        if (!rowBuffer) { fclose(file); return 0; }

        if (topDown) {
            // Top-down: write from top to bottom
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    rowBuffer[x] = image[y * width + x];
                }
                for (int p = 0; p < padding; p++)
                    rowBuffer[width + p] = 0;
                fwrite(rowBuffer, 1, rowSize, file);
            }
        } else {
            // Bottom-up: write from bottom to top
            for (int y = height - 1; y >= 0; y--) {
                for (int x = 0; x < width; x++) {
                    rowBuffer[x] = image[y * width + x];
                }
                for (int p = 0; p < padding; p++)
                    rowBuffer[width + p] = 0;
                fwrite(rowBuffer, 1, rowSize, file);
            }
        }

        free(rowBuffer);
    } else {
        // 24-bit RGB BMP
        int rowSize = ((width * 3 + 3) / 4) * 4;
        int imageSize = rowSize * height;
        int padding = rowSize - (width * 3);

        fileHeader.bfType = 0x4D42;
        fileHeader.bfOffBits = 54;
        fileHeader.bfSize = 54 + imageSize;
        fileHeader.bfReserved1 = 0;
        fileHeader.bfReserved2 = 0;

        infoHeader.biSize = 40;
        infoHeader.biWidth = width;
        infoHeader.biHeight = topDown ? -height : height;
        infoHeader.biPlanes = 1;
        infoHeader.biBitCount = 24;
        infoHeader.biCompression = 0;
        infoHeader.biSizeImage = imageSize;
        infoHeader.biXPelsPerMeter = 0;
        infoHeader.biYPelsPerMeter = 0;
        infoHeader.biClrUsed = 0;
        infoHeader.biClrImportant = 0;

        // Write headers
        fwrite(&fileHeader, sizeof(BMPFileHeader), 1, file);
        fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, file);

        // Write pixel data
        unsigned char* rowBuffer = (unsigned char*)malloc(rowSize);
        if (!rowBuffer) { fclose(file); return 0; }

        if (topDown) {
            // Top-down: write from top to bottom
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    unsigned char gray = image[y * width + x];
                    rowBuffer[x * 3 + 0] = gray; // B
                    rowBuffer[x * 3 + 1] = gray; // G
                    rowBuffer[x * 3 + 2] = gray; // R
                }
                for (int p = 0; p < padding; p++)
                    rowBuffer[width * 3 + p] = 0;
                fwrite(rowBuffer, 1, rowSize, file);
            }
        } else {
            // Bottom-up: write from bottom to top
            for (int y = height - 1; y >= 0; y--) {
                for (int x = 0; x < width; x++) {
                    unsigned char gray = image[y * width + x];
                    rowBuffer[x * 3 + 0] = gray; // B
                    rowBuffer[x * 3 + 1] = gray; // G
                    rowBuffer[x * 3 + 2] = gray; // R
                }
                for (int p = 0; p < padding; p++)
                    rowBuffer[width * 3 + p] = 0;
                fwrite(rowBuffer, 1, rowSize, file);
            }
        }

        free(rowBuffer);
    }

    fclose(file);
    return 1;
}

// Unsharp Masking
void unsharpMasking(unsigned char* input, unsigned char* output, int width, int height, float amount, float radius) {
    unsigned char* blurred = (unsigned char*)malloc(width * height);
    gaussianBlur(input, blurred, width, height, radius);
    
    for (int i = 0; i < width * height; i++) {
        int sharpened = input[i] + amount * (input[i] - blurred[i]);
        output[i] = (unsigned char)(sharpened < 0 ? 0 : (sharpened > 255 ? 255 : sharpened));
    }
    
    free(blurred);
}

// Sobel Operator
void sobelOperator(unsigned char* input, unsigned char* output, int width, int height) {
    // Sobel kernels
    int sobelX[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    int sobelY[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
    
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int gx = 0, gy = 0;
            
            // Apply Sobel kernels
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int pixel = input[(y + ky) * width + (x + kx)];
                    gx += pixel * sobelX[ky + 1][kx + 1];
                    gy += pixel * sobelY[ky + 1][kx + 1];
                }
            }
            
            // Calculate gradient magnitude
            int magnitude = (int)sqrt(gx * gx + gy * gy);
            output[y * width + x] = (unsigned char)(magnitude > 255 ? 255 : magnitude);
        }
    }
    
    // Handle borders
    for (int y = 0; y < height; y++) {
        output[y * width] = 0;
        output[y * width + width - 1] = 0;
    }
    for (int x = 0; x < width; x++) {
        output[x] = 0;
        output[(height - 1) * width + x] = 0;
    }
}

// Laplacian of Gaussian
void laplacianOfGaussian(unsigned char* input, unsigned char* output, int width, int height, int maskSize, float sigma) {
    // Create LOG kernel
    float* kernel = (float*)malloc(maskSize * maskSize * sizeof(float));
    int center = maskSize / 2;
    float sum = 0.0f;
    
    for (int y = 0; y < maskSize; y++) {
        for (int x = 0; x < maskSize; x++) {
            int dx = x - center;
            int dy = y - center;
            float r2 = dx * dx + dy * dy;
            float gaussian = exp(-r2 / (2 * sigma * sigma));
            float laplacian = (r2 - 2 * sigma * sigma) / (sigma * sigma * sigma * sigma);
            kernel[y * maskSize + x] = gaussian * laplacian;
            sum += kernel[y * maskSize + x];
        }
    }
    
    // Normalize kernel
    for (int i = 0; i < maskSize * maskSize; i++) {
        kernel[i] -= sum / (maskSize * maskSize);
    }
    
    applyKernel(input, output, width, height, kernel, maskSize);
    normalizeImage(output, width, height);
    
    free(kernel);
}

// Gaussian Blur
void gaussianBlur(unsigned char* input, unsigned char* output, int width, int height, float sigma) {
    int kernelSize = (int)(6 * sigma) + 1;
    if (kernelSize % 2 == 0) kernelSize++;
    
    float* kernel = (float*)malloc(kernelSize * sizeof(float));
    int center = kernelSize / 2;
    float sum = 0.0f;
    
    // Create 1D Gaussian kernel
    for (int i = 0; i < kernelSize; i++) {
        int x = i - center;
        kernel[i] = exp(-(x * x) / (2 * sigma * sigma));
        sum += kernel[i];
    }
    
    // Normalize kernel
    for (int i = 0; i < kernelSize; i++) {
        kernel[i] /= sum;
    }
    
    // Apply horizontal blur
    unsigned char* temp = (unsigned char*)malloc(width * height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float value = 0.0f;
            for (int k = 0; k < kernelSize; k++) {
                int kx = x + k - center;
                if (kx >= 0 && kx < width) {
                    value += input[y * width + kx] * kernel[k];
                }
            }
            temp[y * width + x] = (unsigned char)value;
        }
    }
    
    // Apply vertical blur
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float value = 0.0f;
            for (int k = 0; k < kernelSize; k++) {
                int ky = y + k - center;
                if (ky >= 0 && ky < height) {
                    value += temp[ky * width + x] * kernel[k];
                }
            }
            output[y * width + x] = (unsigned char)value;
        }
    }
    
    free(kernel);
    free(temp);
}

// Apply convolution kernel
void applyKernel(unsigned char* input, unsigned char* output, int width, int height, float* kernel, int kernelSize) {
    int center = kernelSize / 2;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float value = 0.0f;
            for (int ky = 0; ky < kernelSize; ky++) {
                for (int kx = 0; kx < kernelSize; kx++) {
                    int px = x + kx - center;
                    int py = y + ky - center;
                    
                    if (px >= 0 && px < width && py >= 0 && py < height) {
                        value += input[py * width + px] * kernel[ky * kernelSize + kx];
                    }
                }
            }
            output[y * width + x] = (unsigned char)(value < 0 ? 0 : (value > 255 ? 255 : value));
        }
    }
}

// Normalize image to 0-255 range
void normalizeImage(unsigned char* image, int width, int height) {
    int min = 255, max = 0;
    
    // Find min and max values
    for (int i = 0; i < width * height; i++) {
        if (image[i] < min) min = image[i];
        if (image[i] > max) max = image[i];
    }
    
    // Normalize if range is not 0-255
    if (max > min) {
        float scale = 255.0f / (max - min);
        for (int i = 0; i < width * height; i++) {
            image[i] = (unsigned char)((image[i] - min) * scale);
        }
    }
}
