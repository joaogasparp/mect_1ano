#ifndef GOLOMB_H
#define GOLOMB_H

#include "BitStream.h"

class Golomb {
public:
    explicit Golomb(int m);

    void encode(int value, BitStream& bitStream);
    int decode(BitStream& bitStream);

    int mapToNonNegative(int value) const;
    int mapToSigned(int value) const;

private:
    int m;
    int b;
};

#endif // GOLOMB_H
