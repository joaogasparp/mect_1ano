#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <fstream>
#include <string>

class BitStream {
private:
    char buffer = 0x00;
    int bufSize = 0;

public:
    std::fstream fs;

    BitStream();
    ~BitStream();

    void openFile(const std::string &filename, std::ios::openmode mode);
    void writeBit(int bit);
    void flushBuffer();
    int readBit();
    void writeBits(uint64_t value, int n);
    uint64_t readBits(int n);
    bool isEndOfFile();
    void alignToByte(); // Align bitstream to byte boundary
};

#endif
