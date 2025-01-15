#include "Golomb.h"
#include <cmath>

Golomb::Golomb(int m) : m(m) {}

void Golomb::encode(BitStream &stream, int value) {
    int quotient = value / m;
    int remainder = value % m;

    for (int i = 0; i < quotient; i++) {
        stream.writeBit(1);
    }
    stream.writeBit(0);

    int b = std::floor(std::log2(m));
    int cutoff = (1 << (b + 1)) - m;

    if (remainder < cutoff) {
        stream.writeBits(remainder, b);
    } else {
        stream.writeBits(remainder + cutoff, b + 1);
    }
}

int Golomb::decode(BitStream &stream) {
    int quotient = 0;

    while (stream.readBit() == 1) {
        quotient++;
    }

    int b = std::floor(std::log2(m));
    int cutoff = (1 << (b + 1)) - m;
    int remainder = stream.readBits(b);

    if (remainder >= cutoff) {
        remainder = (remainder << 1) + stream.readBits(1) - cutoff;
    }

    return quotient * m + remainder;
}
