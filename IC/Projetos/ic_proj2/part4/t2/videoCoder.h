#ifndef VIDEOCODER_H
#define VIDEOCODER_H

#include "BitStream.h"
#include "Golomb.h"
#include "Predictors.h"
#include <opencv2/opencv.hpp>
#include <string>

class VideoCoder {
private:
    std::string input;
    std::string output;
    bool encodeMode;

public:
    VideoCoder(const std::string &inputFile, const std::string &outputFile, bool mode);
    void encodeVideo();
    void decodeVideo();
};

#endif
