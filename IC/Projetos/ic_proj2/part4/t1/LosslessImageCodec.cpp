#include "LosslessImageCodec.h"
#include <cmath>
#include <algorithm>
#include <iostream>

LosslessImageCodec::LosslessImageCodec(int golombM) : m(golombM) {}

int LosslessImageCodec::mapToNonNegative(int n) {
    return (n >= 0) ? 2 * n : (-2 * n - 1);
}

int LosslessImageCodec::mapToSigned(int n) {
    return (n % 2 == 0) ? (n / 2) : (-(n + 1) / 2);
}

std::vector<std::vector<int>> LosslessImageCodec::calculateResiduals(const cv::Mat& image, Predictors::Standards standard) {
    std::vector<std::vector<int>> residuals(image.channels());
    int rows = image.rows;
    int cols = image.cols;

    residuals[0].push_back(rows);
    residuals[0].push_back(cols);

    for (int c = 0; c < image.channels(); ++c) {
        residuals[c].reserve(rows * cols + 2);
        if (c > 0) {
            residuals[c].push_back(rows);
            residuals[c].push_back(cols);
        }

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                uchar actual = image.at<cv::Vec3b>(i, j)[c];
                int left = (j > 0) ? image.at<cv::Vec3b>(i, j - 1)[c] : 0;
                int above = (i > 0) ? image.at<cv::Vec3b>(i - 1, j)[c] : 0;
                int diag = (i > 0 && j > 0) ? image.at<cv::Vec3b>(i - 1, j - 1)[c] : 0;

                int predicted = predictors.predictors(standard, left, above, diag);
                int residual = static_cast<int>(actual) - predicted;
                residuals[c].push_back(mapToNonNegative(residual));

                // Log residuals for debugging
                std::cout << "Pixel[" << i << "][" << j << "] Actual: " << (int)actual
                          << ", Predicted: " << predicted
                          << ", Residual: " << residual << "\n";
            }
        }
    }
    return residuals;
}

void LosslessImageCodec::encode(const cv::Mat& image, const std::string& outputFile, Predictors::Standards predictorType) {
    BitStream bitStream(outputFile, BitStream::Write);
    auto residuals = calculateResiduals(image, predictorType);

    bitStream.writeBits(image.channels(), 8);
    for (const auto& channelResiduals : residuals) {
        bitStream.writeBits(channelResiduals[0], 16);
        bitStream.writeBits(channelResiduals[1], 16);

        double mean = 0.0;
        for (size_t i = 2; i < channelResiduals.size(); ++i) {
            mean += channelResiduals[i];
        }
        mean /= (channelResiduals.size() - 2);

        m = std::max(1, static_cast<int>(std::round(-1 / std::log2(mean / (mean + 1)))));
        m = 1 << static_cast<int>(std::ceil(std::log2(m)));

        bitStream.writeBits(m, 16);

        Golomb golombCoder(m);
        for (size_t i = 2; i < channelResiduals.size(); ++i) {
            golombCoder.encode(channelResiduals[i], bitStream);
        }
    }

    bitStream.flush();
}

cv::Mat LosslessImageCodec::reconstructImage(const std::vector<std::vector<int>>& residuals, Predictors::Standards standard) {
    int rows = residuals[0][0];
    int cols = residuals[0][1];
    int channels = residuals.size();

    cv::Mat reconstructed = cv::Mat::zeros(rows, cols, CV_8UC3);

    for (int c = 0; c < channels; ++c) {
        int idx = 2;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                int left = (j > 0) ? reconstructed.at<cv::Vec3b>(i, j - 1)[c] : 0;
                int above = (i > 0) ? reconstructed.at<cv::Vec3b>(i - 1, j)[c] : 0;
                int diag = (i > 0 && j > 0) ? reconstructed.at<cv::Vec3b>(i - 1, j - 1)[c] : 0;

                int predicted = predictors.predictors(standard, left, above, diag);
                int pixel = predicted + residuals[c][idx];
                pixel = ((pixel % 256) + 256) % 256;
                reconstructed.at<cv::Vec3b>(i, j)[c] = static_cast<uchar>(pixel);

                // Log reconstruction for debugging
                std::cout << "Pixel[" << i << "][" << j << "] Predicted: " << predicted
                          << ", Residual: " << residuals[c][idx]
                          << ", Reconstructed: " << pixel << "\n";

                idx++;
            }
        }
    }
    return reconstructed;
}

cv::Mat LosslessImageCodec::decode(const std::string& inputFile, Predictors::Standards predictorType) {
    BitStream bitStream(inputFile, BitStream::Read);

    int numChannels = bitStream.readBits(8);
    std::vector<std::vector<int>> residuals(numChannels);

    for (int c = 0; c < numChannels; ++c) {
        int rows = bitStream.readBits(16);
        int cols = bitStream.readBits(16);

        m = bitStream.readBits(16);
        Golomb golombCoder(m);

        std::vector<int> channelResiduals;
        channelResiduals.push_back(rows);
        channelResiduals.push_back(cols);
        channelResiduals.reserve(rows * cols + 2);

        for (int i = 0; i < rows * cols; ++i) {
            int encoded = golombCoder.decode(bitStream);
            channelResiduals.push_back(mapToSigned(encoded));
        }
        residuals[c] = std::move(channelResiduals);
    }

    return reconstructImage(residuals, predictorType);
}
