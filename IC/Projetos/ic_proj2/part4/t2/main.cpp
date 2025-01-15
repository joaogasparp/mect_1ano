#include "videoCoder.h"
#include <iostream>

int main(int argc, char **argv) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <encode|decode> <input> <output>" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    std::string input = argv[2];
    std::string output = argv[3];

    try {
        if (mode == "encode") {
            VideoCoder coder(input, output, true);
            coder.encodeVideo();
        } else if (mode == "decode") {
            VideoCoder coder(input, output, false);
            coder.decodeVideo();
        } else {
            std::cerr << "Invalid mode. Use 'encode' or 'decode'." << std::endl;
            return 1;
        }
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
