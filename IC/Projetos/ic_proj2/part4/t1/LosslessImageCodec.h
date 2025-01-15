#ifndef LOSSLESSIMAGECODEC_H
#define LOSSLESSIMAGECODEC_H

#include <opencv2/opencv.hpp>
#include "Golomb.h"
#include "BitStream.h"
#include "Predictors.h"
#include <vector>

class LosslessImageCodec {
public:
    explicit LosslessImageCodec(int golombM);
    void encode(const cv::Mat& image, const std::string& outputFile, Predictors::Standards predictorType);
    cv::Mat decode(const std::string& inputFile, Predictors::Standards predictorType);

private:
    int m;
    Predictors predictors;

    int mapToNonNegative(int n);
    int mapToSigned(int n);
    std::vector<std::vector<int>> calculateResiduals(const cv::Mat& image, Predictors::Standards standard);
    cv::Mat reconstructImage(const std::vector<std::vector<int>>& residuals, Predictors::Standards standard);
};

#endif // LOSSLESSIMAGECODEC_H
