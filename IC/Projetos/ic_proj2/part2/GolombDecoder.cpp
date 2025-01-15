#include "../part1/BitStream.h"
#include <iostream>
#include <string>
#include <sndfile.hh>
#include "Golomb.h"
#include <cmath>
#include <vector>

using namespace std;

constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer para leitura de frames

int main(int argc, char** argv) {
    auto predict = [](int a, int b, int c) {
        return 3 * a - 3 * b + c;
    };

    clock_t start = clock();

    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <input file> <output file>\n";
        return 1;
    }

    int sampleRate = 44100;
    BitStream bs(argv[1], "r");

    auto readHeader = [&](size_t numBits) {
        vector<int> bits = bs.readBits(numBits);
        int value = 0;
        for (size_t i = 0; i < bits.size(); ++i) {
            value += bits[i] * pow(2, bits.size() - i - 1);
        }
        return value;
    };

    int nChannels = readHeader(16);
    int padding = readHeader(16);
    int q = readHeader(16);
    size_t nFrames = static_cast<size_t>(readHeader(32));
    int blockSize = readHeader(16);
    int num_zeros = readHeader(16);
    int m_size = readHeader(16);

    SndfileHandle sfhOut(argv[2], SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16, nChannels, sampleRate);

    vector<int> m_vector(m_size);
    for (int i = 0; i < m_size; ++i) {
        m_vector[i] = readHeader(16);
    }

    int totalBits = (bs.getFileSize() - (16 + 2 * m_size)) * 8;
    vector<int> encodedBits = bs.readBits(totalBits);

    string encoded = "";
    for (int bit : encodedBits) {
        encoded += to_string(bit);
    }
    encoded = encoded.substr(0, encoded.size() - num_zeros);

    Golomb g;
    vector<int> decoded = (m_size == 1) ? g.decode(encoded, m_vector[0]) : g.decodeMultiple(encoded, m_vector, blockSize);

    vector<short> samplesVector;

    if (nChannels == 1) {
        for (size_t i = 0; i < decoded.size(); ++i) {
            if (i >= 3) {
                int value = decoded[i] + predict(samplesVector[i - 1], samplesVector[i - 2], samplesVector[i - 3]);
                samplesVector.push_back(value);
            } else {
                samplesVector.push_back(decoded[i]);
            }
        }
    } else {
        for (size_t i = 0; i < nFrames; ++i) {
            if (i >= 3) {
                int value = decoded[i] + predict(samplesVector[i - 1], samplesVector[i - 2], samplesVector[i - 3]);
                samplesVector.push_back(value);
            } else {
                samplesVector.push_back(decoded[i]);
            }
        }

        for (size_t i = nFrames; i < decoded.size(); ++i) {
            if (i >= nFrames + 3) {
                int value = decoded[i] + predict(samplesVector[i - 1], samplesVector[i - 2], samplesVector[i - 3]);
                samplesVector.push_back(value);
            } else {
                samplesVector.push_back(decoded[i]);
            }
        }

        vector<short> merged;
        vector<short> leftChannel(samplesVector.begin(), samplesVector.begin() + nFrames);
        vector<short> rightChannel(samplesVector.begin() + nFrames, samplesVector.end());

        for (size_t i = 0; i < nFrames; ++i) {
            merged.push_back(leftChannel[i]);
            merged.push_back(rightChannel[i]);
        }
        samplesVector = merged;
    }

    if (q != 0) {
        for (auto& sample : samplesVector) {
            if (q > 1) {
                sample = (sample << (q - 1)) | 1;
            }
        }
    }

    if (padding > 0) {
        samplesVector.resize(samplesVector.size() - padding);
    }

    sfhOut.write(samplesVector.data(), samplesVector.size());
    bs.close();

    clock_t end = clock();
    double elapsed_secs = double(end - start) / CLOCKS_PER_SEC * 1000;
    cout << "Time elapsed: " << elapsed_secs << " ms" << endl;

    return 0;
}
