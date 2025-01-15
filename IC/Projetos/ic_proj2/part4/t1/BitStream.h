#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <fstream>
#include <string>
#include <cstdint>

class BitStream {
public:
    enum Mode { Read, Write };

    BitStream(const std::string& filename, Mode mode);
    ~BitStream();

    void writeBit(bool bit);
    bool readBit();

    void writeBits(uint32_t value, int bitCount);
    uint32_t readBits(int bitCount);

    void flush(); // Moved to public

private:
    std::fstream file;
    Mode mode;
    uint8_t buffer;
    int bitPos;

    void writeBuffer();
};

#endif // BITSTREAM_H
