#include "Golomb.h"
#include <cmath>
#include <iostream>

Golomb::Golomb(int m, bool useInterleaving) : m(m), useInterleaving(useInterleaving) {}

void Golomb::encode(int number, BitStream& bitStream) {
    int quotient = std::abs(number) / m;
    int remainder = std::abs(number) % m;

    // Unary code for quotient
    for (int i = 0; i < quotient; ++i) {
        bitStream.writeBit(1);
    }
    bitStream.writeBit(0);

    // Binary code for remainder
    int remainderBits = std::ceil(std::log2(m));
    bitStream.writeBits(remainder, remainderBits);
}

int Golomb::decode(BitStream& bitStream) {
    // Decode unary code for quotient
    int quotient = 0;
    while (bitStream.readBit() == 1) {
        ++quotient;
    }

    // Decode binary code for remainder
    int remainderBits = std::ceil(std::log2(m));
    int remainder = bitStream.readBits(remainderBits);

    // Combine quotient and remainder
    int decodedNumber = quotient * m + remainder;

    // If interleaving is enabled, reverse the mapping for negative numbers
    return decodedNumber;
}
