#include "LosslessVideoCodec.h"
#include <stdexcept>
#include <iostream>

LosslessVideoCodec::LosslessVideoCodec(int golombM) : imageCodec(golombM) {}

void LosslessVideoCodec::encodeFrame(const cv::Mat& frame, BitStream& bitStream) {
    imageCodec.encode(frame, bitStream, Predictors::Standards::JPEG_LS);
}

void LosslessVideoCodec::decodeFrame(BitStream& bitStream, cv::Mat& frame) {
    imageCodec.decode(bitStream, frame, Predictors::Standards::JPEG_LS);
}

void LosslessVideoCodec::encodeVideo(const std::string& inputPath, const std::string& outputPath) {
    cv::VideoCapture cap(inputPath);
    if (!cap.isOpened()) {
        throw std::runtime_error("Failed to open input video file.");
    }

    VideoHeader header = {
        static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH)),
        static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT)),
        static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT)),
        imageCodec.getGolombM(),
        cap.get(cv::CAP_PROP_FPS)
    };

    BitStream bitStream(outputPath, BitStream::Mode::Write);

    // Write video header
    bitStream.writeBits(header.width, 16);
    bitStream.writeBits(header.height, 16);
    bitStream.writeBits(header.totalFrames, 32);
    bitStream.writeBits(header.golombM, 8);
    bitStream.writeBits(static_cast<int>(header.fps), 16);

    cv::Mat frame;
    int frameCount = 0;

    while (cap.read(frame)) {
        std::cout << "Encoding frame " << ++frameCount << "/" << header.totalFrames << std::endl;
        encodeFrame(frame, bitStream);
    }

    cap.release();
    std::cout << "Video encoding completed. Total frames: " << frameCount << std::endl;
}

void LosslessVideoCodec::decodeVideo(const std::string& inputPath, const std::string& outputPath) {
    BitStream bitStream(inputPath, BitStream::Mode::Read);

    // Read video header
    VideoHeader header = {
        bitStream.readBits(16),
        bitStream.readBits(16),
        bitStream.readBits(32),
        bitStream.readBits(8),
        static_cast<double>(bitStream.readBits(16))
    };

    cv::VideoWriter writer(outputPath, cv::VideoWriter::fourcc('a', 'v', 'c', '1'),
                           header.fps, cv::Size(header.width, header.height));

    cv::Mat frame(header.height, header.width, CV_8UC3);
    int frameCount = 0;

    while (frameCount < header.totalFrames) {
        std::cout << "Decoding frame " << ++frameCount << "/" << header.totalFrames << std::endl;
        decodeFrame(bitStream, frame);
        writer.write(frame);
    }

    writer.release();
    std::cout << "Video decoding completed. Total frames: " << frameCount << std::endl;
}
