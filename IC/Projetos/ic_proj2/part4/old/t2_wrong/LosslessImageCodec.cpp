#include "LosslessImageCodec.h"
#include "Predictors.h"
#include "Golomb.h"

LosslessImageCodec::LosslessImageCodec(int golombM) : golombM(golombM) {}


void LosslessImageCodec::encode(const cv::Mat& image, BitStream& bitStream, Predictors::Standards predictorType) {
    Golomb golomb(golombM);

    for (int i = 0; i < image.rows; ++i) {
        for (int j = 0; j < image.cols; ++j) {
            int predicted = Predictors::predictPixel(image, i, j, predictorType); // Prediction logic
            int value = image.at<uchar>(i, j) - predicted; // Residual calculation
            int mappedValue = golomb.mapToNonNegative(value);
            bitStream.writeBits(mappedValue, 16); // Assume residuals are mapped to 16 bits
        }
    }
}

void LosslessImageCodec::decode(BitStream& bitStream, cv::Mat& image, Predictors::Standards predictorType) {
    Golomb golomb(golombM);

    for (int i = 0; i < image.rows; ++i) {
        for (int j = 0; j < image.cols; ++j) {
            int mappedValue = bitStream.readBits(16); // Read 16 bits
            int residual = golomb.mapToSigned(mappedValue);
            int predicted = Predictors::predictPixel(image, i, j, predictorType); // Prediction logic
            image.at<uchar>(i, j) = static_cast<uchar>(predicted + residual); // Reconstruct pixel
        }
    }
}
