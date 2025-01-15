#include "LosslessVideoCodec.h"
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <encode|decode> <input_file> <output_file>" << endl;
        return 1;
    }

    string mode = argv[1];
    string inputFile = argv[2];
    string outputFile = argv[3];

    LosslessVideoCodec codec;

    try {
        if (mode == "encode") {
            cout << "Encoding video..." << endl;
            codec.encodeVideo(inputFile, outputFile);
        } else if (mode == "decode") {
            cout << "Decoding video..." << endl;
            codec.decodeVideo(inputFile, outputFile);
        } else {
            cerr << "Invalid mode. Use 'encode' or 'decode'." << endl;
            return 1;
        }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
