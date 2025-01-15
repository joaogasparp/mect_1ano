#include "BitStream.h"

BitStream::BitStream(const std::string& filename, Mode mode) : mode(mode), buffer(0), bitPos(0) {
    if (mode == Write) {
        file.open(filename, std::ios::binary | std::ios::out);
    } else {
        file.open(filename, std::ios::binary | std::ios::in);
    }
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }
}

BitStream::~BitStream() {
    if (mode == Write && bitPos > 0) {
        flush();
    }
    file.close();
}

void BitStream::writeBit(bool bit) {
    if (bit) {
        buffer |= (1 << (7 - bitPos));
    }
    bitPos++;
    if (bitPos == 8) {
        flush();
    }
}

bool BitStream::readBit() {
    if (bitPos == 0) {
        file.read(reinterpret_cast<char*>(&buffer), 1);
        if (file.eof()) throw std::runtime_error("End of file reached");
    }
    bool bit = (buffer >> (7 - bitPos)) & 1;
    bitPos = (bitPos + 1) % 8;
    return bit;
}

void BitStream::writeBits(uint32_t value, int bitCount) {
    for (int i = bitCount - 1; i >= 0; --i) {
        writeBit((value >> i) & 1);
    }
}

uint32_t BitStream::readBits(int bitCount) {
    uint32_t value = 0;
    for (int i = 0; i < bitCount; ++i) {
        value = (value << 1) | readBit();
    }
    return value;
}

void BitStream::flush() {
    file.write(reinterpret_cast<char*>(&buffer), 1);
    buffer = 0;
    bitPos = 0;
}
