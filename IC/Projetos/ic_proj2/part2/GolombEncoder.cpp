#include "Golomb.h"
#include <iostream>
#include <string>
#include <vector>
#include <sndfile.hh>
#include "../part1/BitStream.h"
#include <string.h>
#include <numeric>
#include <algorithm>

using namespace std;

constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer for reading frames

// Function to predict the next value in the sequence based on 3 previous values
int predict(int a, int b, int c) {
    return 3 * a - 3 * b + c;
}

// Function to calculate m based on u
int calc_m(int u) {
    return static_cast<int>(-1 / log(static_cast<double>(u) / (1 + u)));
}

int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 6) {
        cerr << "Usage: " << argv[0] << " <input file> <output file> <m | bs> [auto] [q] \n";
        return 1;
    }

    int q = 0;
    bool autoMode = false;
    bool quantization = false;

    if (argc >= 5) {
        if (strcmp(argv[4], "auto") == 0) {
            autoMode = true;
        } else {
            q = atoi(argv[4]);
            quantization = true;
        }
    }

    if (argc == 6) {
        q = atoi(argv[5]);
        quantization = true;
    }

    if (q > 16 || q < 0) {
        cerr << "[q] must be between 1 and 15\n";
        return 1;
    }

    SndfileHandle sfhIn{argv[1]};
    if (sfhIn.error() || (sfhIn.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV || 
        (sfhIn.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
        cerr << "Error: invalid input file or format\n";
        return 1;
    }

    string output = argv[2];
    short m = atoi(argv[3]);
    clock_t start = clock();
    short bs = m;

    size_t nFrames = static_cast<size_t>(sfhIn.frames());
    size_t nChannels = static_cast<size_t>(sfhIn.channels());
    vector<short> samples(nChannels * nFrames);
    sfhIn.readf(samples.data(), nFrames);
    size_t nBlocks = static_cast<size_t>(ceil(static_cast<double>(nFrames) / bs));

    samples.resize(nBlocks * bs * nChannels);
    int padding = samples.size() - nFrames * nChannels;

    vector<short> left_samples(samples.size() / 2);
    vector<short> right_samples(samples.size() / 2);

    if (quantization) {
        for (auto &sample : samples) {
            sample >>= q; // Remove the q least significant bits
        }
    }

    if (nChannels > 1) {
        for (size_t i = 0; i < samples.size() / 2; ++i) {
            left_samples[i] = samples[i * nChannels];
            right_samples[i] = samples[i * nChannels + 1];
        }
    }

    vector<int> m_vector;
    vector<int> valuesToBeEncoded;

    auto process_samples = [&](const vector<short>& channel_samples) {
        for (size_t i = 0; i < channel_samples.size(); ++i) {
            if (i >= 3) {
                int difference = channel_samples[i] - predict(channel_samples[i - 1], channel_samples[i - 2], channel_samples[i - 3]);
                valuesToBeEncoded.push_back(difference);
            } else {
                valuesToBeEncoded.push_back(channel_samples[i]);
            }

            if (i % bs == 0 && i != 0) {
                int sum = accumulate(valuesToBeEncoded.end() - bs, valuesToBeEncoded.end(), 0, [](int acc, int val) { return acc + abs(val); });
                int u = round(sum / static_cast<double>(bs));
                m = calc_m(u);
                m = static_cast<short>(max(static_cast<int>(m), 1));
                m_vector.push_back(m);
            }
        }
    };

    if (nChannels < 2) {
        process_samples(samples);
    } else {
        process_samples(left_samples);
        process_samples(right_samples);
    }

    string encodedString;
    Golomb g;

    if (!autoMode) {
        for (const auto &value : valuesToBeEncoded) {
            encodedString += g.encode(value, m);
        }
    } else {
        int m_index = 0;
        for (size_t i = 0; i < valuesToBeEncoded.size(); ++i) {
            if (i % bs == 0 && i != 0) m_index++;
            encodedString += g.encode(valuesToBeEncoded[i], m_vector[m_index]);
        }
    }

    BitStream bitStream(output, "w");
    vector<int> bits;
    vector<int> encoded_bits(encodedString.begin(), encodedString.end());
    
    // Convert encoded string to bits
    for (auto &bit : encoded_bits) {
        bit -= '0';
    }

    int count_zeros = (8 - (encoded_bits.size() % 8)) % 8;
    encoded_bits.insert(encoded_bits.end(), count_zeros, 0);

    // Prepare header bits
    auto add_bits = [&](int value, int bit_count) {
        for (int i = bit_count - 1; i >= 0; --i) {
            bits.push_back((value >> i) & 1);
        }
    };

    add_bits(sfhIn.channels(), 16);
    add_bits(padding, 16);
    add_bits(q, 16);
    add_bits(samples.size() / 2, 32);
    add_bits(bs, 16);
    add_bits(count_zeros, 16);
    
    if (!autoMode) {
        m_vector.clear();
        m_vector.push_back(m);
    }
    add_bits(m_vector.size(), 16);
    
    for (const auto &m_value : m_vector) {
        add_bits(m_value, 16);
    }

    bits.insert(bits.end(), encoded_bits.begin(), encoded_bits.end());
    bitStream.writeBits(bits);
    bitStream.close();

    // End the timer
    clock_t end = clock();
    double elapsed_secs = double(end - start) / CLOCKS_PER_SEC * 1000;
    cout << "Execution time: " << elapsed_secs << " ms" << endl;

    return 0;
}
