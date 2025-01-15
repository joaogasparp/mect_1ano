#ifndef VIDEOCODER_H
#define VIDEOCODER_H

#include <string>
#include <opencv2/opencv.hpp>
#include "BitStream.h"

class VideoCoder {
public:
    VideoCoder(const std::string &inputFile, const std::string &outputFile, bool mode, 
               int iframeInterval, int blockSize, int searchRange);

    void encodeVideo();
    void decodeVideo();

private:
    void encodeIntraFrame(cv::Mat &frame, BitStream &bitStream);
    void encodeInterFrame(cv::Mat &current, cv::Mat &prevFrame, BitStream &bitStream);
    cv::Point searchBestMatch(const cv::Mat &block, const cv::Mat &ref, const cv::Rect &region);
    bool isIntraMode(const cv::Mat &block, const cv::Mat &predictedBlock);

    cv::Mat decodeIntraFrame(BitStream &bitStream);
    cv::Mat decodeInterFrame(BitStream &bitStream, const cv::Mat &prevFrame);

    std::string input;
    std::string output;
    bool encodeMode;
    int iframeInterval;
    int blockSize;
    int searchRange;

    const int frameWidth = 352;
    const int frameHeight = 240;
};

#endif
