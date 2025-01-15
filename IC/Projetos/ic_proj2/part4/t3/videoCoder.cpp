#include "videoCoder.h"
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>

VideoCoder::VideoCoder(const std::string &inputFile, const std::string &outputFile, bool mode, 
                       int iframeInterval, int blockSize, int searchRange)
    : input(inputFile), output(outputFile), encodeMode(mode), 
      iframeInterval(iframeInterval), blockSize(blockSize), searchRange(searchRange) {}

void VideoCoder::encodeVideo() {
    cv::VideoCapture cap(input);
    if (!cap.isOpened()) {
        throw std::runtime_error("Failed to open video.");
    }

    BitStream bitStream;
    bitStream.openFile(output, std::ios::out | std::ios::binary);

    cv::Mat prevFrame, frame;
    int frameIndex = 0;

    while (cap.read(frame)) {
        cv::resize(frame, frame, cv::Size(frameWidth, frameHeight)); // Resize to 352x240
        if (frameIndex % iframeInterval == 0) {
            encodeIntraFrame(frame, bitStream);
        } else {
            encodeInterFrame(frame, prevFrame, bitStream);
        }

        prevFrame = frame.clone();
        frameIndex++;
    }

    bitStream.flushBuffer();
    cap.release();
    std::cout << "Encoding completed successfully." << std::endl;
}

void VideoCoder::decodeVideo() {
    BitStream bitStream;
    bitStream.openFile(input, std::ios::in | std::ios::binary);

    cv::VideoWriter writer(output, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 
                           30, cv::Size(frameWidth, frameHeight));

    if (!writer.isOpened()) {
        throw std::runtime_error("Failed to open output video.");
    }

    cv::Mat prevFrame, currentFrame;
    while (!bitStream.isEndOfFile()) {
        bitStream.alignToByte();

        int isIFrame = bitStream.readBit();
        if (isIFrame) {
            currentFrame = decodeIntraFrame(bitStream);
        } else {
            currentFrame = decodeInterFrame(bitStream, prevFrame);
        }

        writer.write(currentFrame);
        prevFrame = currentFrame.clone();
    }

    writer.release();
    std::cout << "Decoding completed successfully." << std::endl;
}


// esta função codifica um frame I (independente de outros frames).
// marca o frame como I-frame com um bit de controlo (1) no bitstream.
// calcula o tamanho total do frame (número de pixels * tamanho de cada pixel).
// verifica se o tamanho do frame é o esperado (no caso de teste 352x240 com 3 canais de cor).
// escreve o tamanho do frame (em 32 bits) no bitstream e depois 
// guarda os dados do frame (os valores de cada pixel).
// alinha o bitstream ao próximo byte (flushBuffer) para evitar erros.
void VideoCoder::encodeIntraFrame(cv::Mat &frame, BitStream &bitStream) {
    bitStream.writeBit(1); // marcar i-frame
    size_t frameSize = frame.total() * frame.elemSize();
    std::cout << "Writing frame size: " << frameSize << " to bitstream." << std::endl;

    if (frameSize != static_cast<size_t>(frameWidth * frameHeight * 3)) {
        throw std::runtime_error("Unexpected frame size during encoding.");
    }

    bitStream.writeBits(static_cast<uint64_t>(frameSize), 32); // Write frame size
    bitStream.fs.write(reinterpret_cast<const char*>(frame.data), frameSize);

    // Align to the next byte boundary after writing the frame
    bitStream.flushBuffer();

    // Debugging pixel data
    std::cout << "First pixel during encoding (BGR): "
              << static_cast<int>(frame.data[0]) << ", "
              << static_cast<int>(frame.data[1]) << ", "
              << static_cast<int>(frame.data[2]) << std::endl;
}


// esta função codifica um frame P (previsão com base no frame anterior).
// percorre o frame atual em blocos (tamanho definido por blockSize).
// para cada bloco:
// - verifica se está dentro dos limites do frame (caso contrário, ignora).
// - compara o bloco atual com o correspondente no frame anterior.
// - se a diferença for grande (isIntraMode), marca como bloco intra e guarda diretamente os dados.
// - se a diferença for pequena, marca como bloco inter, calcula o vetor de movimento para o bloco mais semelhante 
//   no frame anterior (searchBestMatch) e guarda o vetor e os dados do bloco no bitstream.

void VideoCoder::encodeInterFrame(cv::Mat &current, cv::Mat &prevFrame, BitStream &bitStream) {
    bitStream.writeBit(0); // Mark as P-frame

    for (int i = 0; i < current.rows; i += blockSize) {
        for (int j = 0; j < current.cols; j += blockSize) {
            cv::Rect blockRegion(j, i, blockSize, blockSize);
            if (blockRegion.x + blockRegion.width > current.cols || 
                blockRegion.y + blockRegion.height > current.rows) {
                continue;
            }

            cv::Mat block = current(blockRegion);
            cv::Mat predictedBlock = prevFrame(blockRegion);

            if (isIntraMode(block, predictedBlock)) {
                bitStream.writeBit(1); // marcar como intra block
                bitStream.fs.write(reinterpret_cast<const char*>(block.data), block.total() * block.elemSize());
            } else {
                bitStream.writeBit(0); // marcar como inter block
                cv::Point motionVector = searchBestMatch(block, prevFrame, blockRegion);
                bitStream.writeBits(motionVector.x, 16);
                bitStream.writeBits(motionVector.y, 16);
                bitStream.fs.write(reinterpret_cast<const char*>(block.data), block.total() * block.elemSize());
            }
        }
    }
}

// descodifica um frame I (independente) a partir do bitstream
// lê o tamanho do frame (em bits), verifica se corresponde às dimensões esperadas
// se o tamanho do frame for maior, ignora o padding extra
// lê os dados do frame diretamente do bitstream e retorna o frame reconstruído
cv::Mat VideoCoder::decodeIntraFrame(BitStream &bitStream) {
    uint64_t frameSize = bitStream.readBits(32); // frame size
    std::cout << "Read frame size: " << frameSize << " from bitstream." << std::endl;

    cv::Mat frame(frameHeight, frameWidth, CV_8UC3); //  352x240
    uint64_t expectedFrameSize = static_cast<uint64_t>(frameWidth * frameHeight * 3);

    if (frameSize > expectedFrameSize) {
        std::cout << "Detected padding. Ignoring extra " 
                  << (frameSize - expectedFrameSize) << " bytes." << std::endl;
        frameSize = expectedFrameSize; // ajdust frame size to expected dimensions
    } else if (frameSize != expectedFrameSize) {
        throw std::runtime_error("Frame size mismatch between encoded and expected dimensions.");
    }

    // ler frame data
    bitStream.fs.read(reinterpret_cast<char*>(frame.data), frameSize);

    std::cout << "First pixel during decoding (BGR): "
              << static_cast<int>(frame.data[0]) << ", "
              << static_cast<int>(frame.data[1]) << ", "
              << static_cast<int>(frame.data[2]) << std::endl;

    return frame;
}

// descodifica um frame P (dependente do frame anterior) a partir do bitstream.
// percorre o frame em blocos e verifica se cada bloco é intra (independente) ou inter (referência ao frame anterior). 
// - se for intra, lê os dados diretamente do bitstream
// - se for inter, calcula a posição do bloco correspondente no frame anterior através de vetores 
// e soma o bloco de referência com os dados lidos
cv::Mat VideoCoder::decodeInterFrame(BitStream &bitStream, const cv::Mat &prevFrame) {
    cv::Mat frame(frameHeight, frameWidth, CV_8UC3);

    for (int i = 0; i < frame.rows; i += blockSize) {
        for (int j = 0; j < frame.cols; j += blockSize) {
            cv::Rect blockRegion(j, i, blockSize, blockSize);
            if (blockRegion.x + blockRegion.width > frame.cols || 
                blockRegion.y + blockRegion.height > frame.rows) {
                continue;
            }

            int isIntraBlock = bitStream.readBit();

            if (isIntraBlock) {
                cv::Mat block = frame(blockRegion);
                bitStream.fs.read(reinterpret_cast<char*>(block.data), block.total() * block.elemSize());
            } else {
                int motionX = bitStream.readBits(16);
                int motionY = bitStream.readBits(16);

                cv::Rect refRegion(blockRegion.x + motionX, blockRegion.y + motionY, blockSize, blockSize);
                if (refRegion.x < 0 || refRegion.y < 0 || 
                    refRegion.x + refRegion.width > prevFrame.cols || 
                    refRegion.y + refRegion.height > prevFrame.rows) {
                    continue;
                }

                cv::Mat refBlock = prevFrame(refRegion);
                cv::Mat block = frame(blockRegion);
                bitStream.fs.read(reinterpret_cast<char*>(block.data), block.total() * block.elemSize());
                block += refBlock;
            }
        }
    }

    return frame;
}

bool VideoCoder::isIntraMode(const cv::Mat &block, const cv::Mat &predictedBlock) {
    double error = cv::norm(block, predictedBlock, cv::NORM_L2);
    return error > 50.0; 
}

cv::Point VideoCoder::searchBestMatch(const cv::Mat &block, const cv::Mat &ref, const cv::Rect &region) {
    cv::Point bestMatch(0, 0);
    double minError = DBL_MAX;

    for (int dy = -searchRange; dy <= searchRange; ++dy) {
        for (int dx = -searchRange; dx <= searchRange; ++dx) {
            cv::Rect searchRegion(region.x + dx, region.y + dy, blockSize, blockSize);

            if (searchRegion.x < 0 || searchRegion.y < 0 ||
                searchRegion.x + searchRegion.width > ref.cols ||
                searchRegion.y + searchRegion.height > ref.rows) {
                continue;
            }

            cv::Mat candidate = ref(searchRegion);
            double error = cv::norm(block, candidate, cv::NORM_L2);

            if (error < minError) {
                minError = error;
                bestMatch = cv::Point(dx, dy);
            }
        }
    }

    return bestMatch;
}
