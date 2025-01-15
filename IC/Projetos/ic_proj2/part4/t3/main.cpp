#include "videoCoder.h"
#include <iostream>

// função principal, controla a codificação e descodificação
int main(int argc, char **argv) {
    if (argc < 7) {
        std::cerr << "Uso: ./main <encode/decode> <input.y4m/output.bin> <output.bin/output.mp4> <iframe_interval> <block_size> <search_range>" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    std::string inputFile = argv[2];
    std::string outputFile = argv[3];
    int iframeInterval = std::stoi(argv[4]);
    int blockSize = std::stoi(argv[5]);
    int searchRange = std::stoi(argv[6]);

    try {
        VideoCoder coder(inputFile, outputFile, mode == "encode", iframeInterval, blockSize, searchRange);

        if (mode == "encode") {
            coder.encodeVideo(); // inicia a codificação
        } else if (mode == "decode") {
            coder.decodeVideo(); // inicia a descodificação
        } else {
            std::cerr << "Modo inválido. Use 'encode' ou 'decode'." << std::endl;
            return 1;
        }
    } catch (const std::exception &e) {
        std::cerr << "Erro: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
