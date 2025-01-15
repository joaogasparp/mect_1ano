#include "Golomb.h"
#include <cmath>
#include <stdexcept>

Golomb::Golomb(int m) : m(m), b(static_cast<int>(std::ceil(std::log2(m)))) {
    if (m <= 0) {
        throw std::invalid_argument("Golomb parameter m must be greater than 0.");
    }
}

int Golomb::mapToNonNegative(int value) const {
    return (value >= 0) ? 2 * value : -2 * value - 1;
}

int Golomb::mapToSigned(int value) const {
    return (value % 2 == 0) ? value / 2 : -(value / 2) - 1;
}

void Golomb::encode(int value, BitStream& bitStream) {
    int mappedValue = mapToNonNegative(value);
    int q = mappedValue / m;
    int r = mappedValue % m;

    // Write q unary bits
    for (int i = 0; i < q; i++) {
        bitStream.writeBits(1, 1);
    }
    bitStream.writeBits(0, 1);

    // Write r truncated binary
    if (r < (1 << b) - m) {
        bitStream.writeBits(r, b - 1);
    } else {
        bitStream.writeBits(r + (1 << b) - m, b);
    }
}

int Golomb::decode(BitStream& bitStream) {
    int q = 0;

    // Read q unary bits
    while (bitStream.readBits(1) == 1) {
        q++;
    }

    // Read r truncated binary
    int r;
    if (bitStream.remainingBits() < b - 1) {
        throw std::runtime_error("Not enough bits to decode.");
    }
    r = bitStream.readBits(b - 1);

    if (r >= (1 << b) - m) {
        if (bitStream.remainingBits() < 1) {
            throw std::runtime_error("Not enough bits to decode.");
        }
        r = (r << 1) + bitStream.readBits(1) - ((1 << b) - m);
    }

    int mappedValue = q * m + r;
    return mapToSigned(mappedValue);
}
