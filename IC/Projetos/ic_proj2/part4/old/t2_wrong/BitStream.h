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

    void writeBits(uint32_t value, int bitCountToWrite);
    uint32_t readBits(int bitCountToRead);

    int remainingBits() const;
    void flush();
    bool eof() const;

private:
    void writeBuffer();
    void loadBuffer();

    std::ifstream inputStream;
    std::ofstream outputStream;
    Mode mode;
    uint64_t buffer;
    int bitCount;
    bool endOfFile;
};

#endif // BITSTREAM_H
