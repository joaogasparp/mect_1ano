#include "Predictors.h"

cv::Mat Predictors::calculateResiduals(const cv::Mat& image, Standards standard) {
    cv::Mat residuals(image.size(), CV_32SC1);

    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            int predicted = 0;

            if (standard == Standards::JPEG_LS) {
                // Simple JPEG-LS predictor
                if (i > 0 && j > 0) {
                    predicted = image.at<uchar>(i, j - 1); // Predict based on left pixel
                }
            }

            residuals.at<int>(i, j) = static_cast<int>(image.at<uchar>(i, j)) - predicted;
        }
    }

    return residuals;
}

int Predictors::predictPixel(const cv::Mat& image, int row, int col, Standards standard) {
    if (standard == Standards::JPEG_LS) {
        // Example: Use left pixel as prediction
        return (col > 0) ? image.at<uchar>(row, col - 1) : 0;
    }
    return 0; // Default prediction
}

void Predictors::reconstructImage(cv::Mat& image, Standards standard) {
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            int predicted = 0;

            if (standard == Standards::JPEG_LS) {
                // Simple JPEG-LS predictor
                if (i > 0 && j > 0) {
                    predicted = image.at<int>(i, j - 1); // Predict based on left pixel
                }
            }

            image.at<int>(i, j) += predicted;
        }
    }

    image.convertTo(image, CV_8UC1); // Convert back to 8-bit unsigned format
}
