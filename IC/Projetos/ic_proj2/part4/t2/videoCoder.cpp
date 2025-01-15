#include "videoCoder.h"
#include <opencv2/opencv.hpp>
#include <algorithm>

// construtor videocoder: responsável por inicializar os ficheiros de entrada e saída, 
// assim como definir o modo (encode ou decode).
VideoCoder::VideoCoder(const std::string &inputFile, const std::string &outputFile, bool mode)
    : input(inputFile), output(outputFile), encodeMode(mode) {}

// funct codificar vídeo.
// abre um input video file, aplica predicter e golomb nos resíduos 
// e grava o resultado num ficheiro binário (output.bin)
// heard: dimensões do vídeo e processa cada frame e canal de cor.
void VideoCoder::encodeVideo() {
    cv::VideoCapture cap(input);
    if (!cap.isOpened()) throw std::runtime_error("Erro ao abrir o input video file.");

    int frameWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int frameHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));

    BitStream bitStream;
    bitStream.openFile(output, std::ios::out | std::ios::binary);

    bitStream.writeBits(frameWidth, 16); // guardar largura
    bitStream.writeBits(frameHeight, 16); // guardar altura

    cv::Mat frame;
    while (cap.read(frame)) {
        std::vector<int16_t> residuals;

        for (int row = 0; row < frame.rows; ++row) {
            for (int col = 0; col < frame.cols; ++col) {
                for (int channel = 0; channel < 3; ++channel) {
                    int a = 0, b = 0, c = 0; // substituir pelos valores reais
                    int prediction = Predictors().predictJPEG_LS(a, b, c);
                    int residual = frame.at<cv::Vec3b>(row, col)[channel] - prediction;
                    residuals.push_back(residual);
                }
            }
        }

        // calcular o valor de m
        double mean = 0.0;
        for (int16_t residual : residuals) mean += std::abs(residual);
        mean /= residuals.size();
        int m = std::max(1, (int)std::round(-1.0 / std::log2(mean / (mean + 1.0))));

        // armazenar m no bitstream
        bitStream.writeBits(m, 16);

        // codificar os resíduos
        Golomb golomb(m);
        for (int16_t residual : residuals) {
            golomb.encode(bitStream, residual);
        }
    }

    bitStream.flushBuffer(); // finalizar gravação
}

// descodifica vídeo com golomb e predição
void VideoCoder::decodeVideo() {
    BitStream bitStream;
    bitStream.openFile(input, std::ios::in | std::ios::binary);

    int frameWidth = bitStream.readBits(16); // ler largura
    int frameHeight = bitStream.readBits(16); // ler altura

    cv::VideoWriter writer(output, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 30, cv::Size(frameWidth, frameHeight), true);
    if (!writer.isOpened()) throw std::runtime_error("Erro ao abrir o output video file.");

    cv::Mat frame(frameHeight, frameWidth, CV_8UC3);
    while (!bitStream.isEndOfFile()) {
        std::vector<int16_t> residuals;

        // ler o valor de m do bitstream
        int m = bitStream.readBits(16);

        // descodificar resíduos
        Golomb golomb(m);
        for (int row = 0; row < frame.rows; ++row) {
            for (int col = 0; col < frame.cols; ++col) {
                for (int channel = 0; channel < 3; ++channel) {
                    int residual = golomb.decode(bitStream);
                    residuals.push_back(residual);
                }
            }
        }

        // reconstruir o frame a partir dos resíduos e da predição
        int index = 0;
        for (int row = 0; row < frame.rows; ++row) {
            for (int col = 0; col < frame.cols; ++col) {
                for (int channel = 0; channel < 3; ++channel) {
                    int a = 0, b = 0, c = 0; // substituir pelos valores reais
                    int prediction = Predictors().predictJPEG_LS(a, b, c);
                    frame.at<cv::Vec3b>(row, col)[channel] = std::clamp(prediction + residuals[index++], 0, 255);
                }
            }
        }

        writer.write(frame); // escrever frame reconstruído
    }
}
