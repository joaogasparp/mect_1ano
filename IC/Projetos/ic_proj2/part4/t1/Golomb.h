#ifndef GOLOMB_H
#define GOLOMB_H

#include "BitStream.h"

class Golomb {
public:
    Golomb(int m, bool useInterleaving = false);
    void encode(int number, BitStream& bitStream);
    int decode(BitStream& bitStream);

private:
    int m; // Golomb parameter
    bool useInterleaving; // Whether to use interleaving for negative numbers
};

#endif // GOLOMB_H
