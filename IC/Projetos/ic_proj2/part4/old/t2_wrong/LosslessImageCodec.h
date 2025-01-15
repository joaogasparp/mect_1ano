#ifndef LOSSLESSIMAGECODEC_H
#define LOSSLESSIMAGECODEC_H

#include <opencv2/opencv.hpp>
#include <string>
#include "BitStream.h"
#include "Predictors.h"

class LosslessImageCodec {
private:
    int golombM;

public:
    explicit LosslessImageCodec(int golombM);
    int getGolombM() const;

    void encode(const cv::Mat& image, BitStream& bitStream, Predictors::Standards predictorType);
    void decode(BitStream& bitStream, cv::Mat& image, Predictors::Standards predictorType);
};

#endif // LOSSLESSIMAGECODEC_H
