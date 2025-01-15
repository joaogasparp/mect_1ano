#ifndef LOSSLESSVIDEOCODEC_H
#define LOSSLESSVIDEOCODEC_H

#include <opencv2/opencv.hpp>
#include "BitStream.h"
#include "LosslessImageCodec.h"

struct VideoHeader {
    int width;
    int height;
    int totalFrames;
    int golombM;
    double fps;
};

class LosslessVideoCodec {
private:
    LosslessImageCodec imageCodec;
    void encodeFrame(const cv::Mat& frame, BitStream& bitStream);
    void decodeFrame(BitStream& bitStream, cv::Mat& frame);

public:
    LosslessVideoCodec(int golombM);
    void encodeVideo(const std::string& inputPath, const std::string& outputPath);
    void decodeVideo(const std::string& inputPath, const std::string& outputPath);
};

#endif // LOSSLESSVIDEOCODEC_H
