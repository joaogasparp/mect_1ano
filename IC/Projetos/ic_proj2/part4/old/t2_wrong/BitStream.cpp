#include "BitStream.h"
#include <stdexcept>
#include <iostream>
#include <bitset>

BitStream::BitStream(const std::string& filename, Mode mode)
    : mode(mode), buffer(0), bitCount(0), endOfFile(false) {
    if (mode == Write) {
        outputStream.open(filename, std::ios::binary | std::ios::out);
        if (!outputStream.is_open()) {
            throw std::runtime_error("Failed to open output file: " + filename);
        }
    } else if (mode == Read) {
        inputStream.open(filename, std::ios::binary | std::ios::in);
        if (!inputStream.is_open()) {
            throw std::runtime_error("Failed to open input file: " + filename);
        }
        loadBuffer();
    }
}

BitStream::~BitStream() {
    if (mode == Write && bitCount > 0) {
        flush();
    }
    if (inputStream.is_open()) {
        inputStream.close();
    }
    if (outputStream.is_open()) {
        outputStream.close();
    }
}

void BitStream::writeBits(uint32_t value, int bitCountToWrite) {
    if (bitCountToWrite <= 0 || bitCountToWrite > 32) {
        throw std::invalid_argument("Invalid bitCountToWrite: must be between 1 and 32.");
    }

    buffer = (buffer << bitCountToWrite) | (value & ((1U << bitCountToWrite) - 1));
    bitCount += bitCountToWrite;

    while (bitCount >= 8) {
        writeBuffer();
    }
}

uint32_t BitStream::readBits(int bitCountToRead) {
    if (bitCountToRead <= 0 || bitCountToRead > 32) {
        throw std::invalid_argument("Invalid bitCountToRead: must be between 1 and 32.");
    }

    uint32_t value = 0;

    while (bitCount < bitCountToRead) {
        if (endOfFile) {
            throw std::runtime_error("End of file reached.");
        }
        loadBuffer();
    }

    value = (buffer >> (bitCount - bitCountToRead)) & ((1U << bitCountToRead) - 1);
    bitCount -= bitCountToRead;

    return value;
}

void BitStream::writeBuffer() {
    uint8_t byte = (buffer >> (bitCount - 8)) & 0xFF;
    outputStream.put(byte);
    bitCount -= 8;
}

void BitStream::loadBuffer() {
    if (inputStream.eof()) {
        endOfFile = true;
        return;
    }

    uint8_t byte = inputStream.get();
    if (inputStream.eof()) {
        endOfFile = true;
        return;
    }

    buffer = (buffer << 8) | byte;
    bitCount += 8;
}

int BitStream::remainingBits() const {
    if (mode == Read) {
        return bitCount;
    } else {
        throw std::runtime_error("remainingBits is only valid in Read mode.");
    }
}

void BitStream::flush() {
    while (bitCount > 0) {
        writeBuffer();
    }
}

bool BitStream::eof() const {
    return (mode == Read && bitCount == 0 && endOfFile);
}
