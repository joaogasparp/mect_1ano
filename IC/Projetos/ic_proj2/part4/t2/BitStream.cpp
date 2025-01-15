#include "BitStream.h"
#include <stdexcept>

BitStream::BitStream() {}

BitStream::~BitStream() {
    if (fs.is_open()) {
        fs.close();
    }
}

void BitStream::openFile(const std::string &filename, std::ios::openmode mode) {
    fs.open(filename, mode);
    if (!fs.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
}

bool BitStream::isEndOfFile() {
    return fs.eof();
}

void BitStream::writeBit(int bit) {
    buffer |= (bit << (7 - (bufSize % 8)));
    bufSize++;
    if (bufSize == 8) {
        fs.put(buffer);
        buffer = 0x00;
        bufSize = 0;
    }
}

void BitStream::flushBuffer() {
    if (bufSize > 0) {
        fs.put(buffer);
        buffer = 0x00;
        bufSize = 0;
    }
}

int BitStream::readBit() {
    if (rBufSize == 0) {
        fs.get(rBuffer);
        rBufSize = 8;
    }
    int bit = (rBuffer >> (rBufSize - 1)) & 1;
    rBufSize--;
    return bit;
}

void BitStream::writeBits(uint64_t value, int n) {
    for (int i = n - 1; i >= 0; i--) {
        writeBit((value >> i) & 1);
    }
}

uint64_t BitStream::readBits(int n) {
    uint64_t value = 0;
    for (int i = 0; i < n; i++) {
        value = (value << 1) | readBit();
    }
    return value;
}

void BitStream::writeString(const std::string &s) {
    for (char c : s) {
        writeBits(static_cast<uint64_t>(c), 8);
    }
}

std::string BitStream::readString(int length) {
    std::string s;
    for (int i = 0; i < length; i++) {
        s += static_cast<char>(readBits(8));
    }
    return s;
}
