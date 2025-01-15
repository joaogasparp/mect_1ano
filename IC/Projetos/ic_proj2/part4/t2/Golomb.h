#ifndef GOLOMB_H
#define GOLOMB_H

#include "BitStream.h"

class Golomb {
private:
    int m;

public:
    Golomb(int m);

    void encode(BitStream &stream, int value);
    int decode(BitStream &stream);
};

#endif
