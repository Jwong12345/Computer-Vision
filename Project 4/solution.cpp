#include <iostream>
#include <vector>
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

// Function to perform 8-point DCT
vector<double> dct8(const vector<double>& input) {
    vector<double> output(8, 0.0);
    for (int k = 0; k < 8; ++k) {
        for (int n = 0; n < 8; ++n) {
            output[k] += input[n] * cos(M_PI * (2 * n + 1) * k / 16.0);
        }
        if (k == 0) {
            output[k] *= (1.0 / sqrt(8.0));
        } else {
            output[k] *= (sqrt(2.0) / sqrt(8.0));
        }
    }
    return output;
}

// Function to perform 16-point DCT
vector<double> dct16(const vector<double>& input) {
    vector<double> output(16, 0.0);
    for (int k = 0; k < 16; ++k) {
        for (int n = 0; n < 16; ++n) {
            output[k] += input[n] * cos(M_PI * (2 * n + 1) * k / 32.0);
        }
        if (k == 0) {
            output[k] *= (1.0 / sqrt(16.0));
        } else {
            output[k] *= (sqrt(2.0) / sqrt(16.0));
        }
    }
    return output;
}

// Function to plot coefficients using OpenCV
void plotCoefficients(const vector<double>& coefficients, const string& title) {
    int n = coefficients.size();
    Mat plot(400, 600, CV_8UC3, Scalar(255, 255, 255));

    // Find min and max values for scaling
    double minVal = *min_element(coefficients.begin(), coefficients.end());
    double maxVal = *max_element(coefficients.begin(), coefficients.end());

    // Calculate y coordinate for zero line
    int zeroY = 200 - (0 - minVal) / (maxVal - minVal) * 200;

    // Draw horizontal line at zero
    line(plot, Point(0, zeroY), Point(600, zeroY), Scalar(0, 255, 0), 1);

    // Add y-axis labels
    putText(plot, "Positive", Point(10, 50), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0), 1);
    putText(plot, "Negative", Point(10, 350), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0), 1);

    // Plot coefficients
    for (int i = 0; i < n - 1; ++i) {
        Point pt1, pt2;
        pt1.x = i * 600 / n;
        pt1.y = 200 - (coefficients[i] - minVal) / (maxVal - minVal) * 200;
        pt2.x = (i + 1) * 600 / n;
        pt2.y = 200 - (coefficients[i + 1] - minVal) / (maxVal - minVal) * 200;
        line(plot, pt1, pt2, Scalar(0, 0, 255), 2);
    }

    putText(plot, title, Point(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 0), 2);
    imshow(title, plot);
    waitKey(0);
}

int main() {
    // Input sequence
    vector<vector<double>> input = {
        {10, 11, 12, 11, 12, 13, 12, 11},
        {10, -10, 8, -7, 8, -8, 7, -7}
    };

    // Transform each row separately
    vector<double> row1_dct = dct8(input[0]);
        cout << "DCT of row1: ";
        for (double val : row1_dct){
            cout << val << " ";
        }
        cout << endl;
    vector<double> row2_dct = dct8(input[1]);
        cout << "DCT of row2: ";
        for (double val : row2_dct){
            cout << val << " ";
        }
        cout << endl;

    // Plot the resulting 16 transform coefficients
    vector<double> combined_rows_dct = row1_dct;
    combined_rows_dct.insert(combined_rows_dct.end(), row2_dct.begin(), row2_dct.end());
    cout <<  "Combined Rows DCT: ";
        for (double val : combined_rows_dct){
            cout << val << " ";
        }
        cout << endl;
    plotCoefficients(combined_rows_dct, "DCT of Rows Separately");

    // Combine all 16 numbers into a single vector and transform it using a 16-point DCT
    vector<double> combined_input = input[0];
    combined_input.insert(combined_input.end(), input[1].begin(), input[1].end());
    vector<double> combined_dct = dct16(combined_input);

    // Plot the 16 transform coefficients
    cout << "DCT of combined input: ";
    for (double val : combined_dct) {
        cout << val << " ";
    }
    cout << endl;
    plotCoefficients(combined_dct, "DCT of Combined Vector");

    return 0;
}