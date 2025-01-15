#ifndef PREDICTORS_H
#define PREDICTORS_H

#include <algorithm>

class Predictors {
public:
    enum Standards { JPEG_PL, JPEG_PA, JPEG_PAL, JPEG_ABC, JPEG_MBC, JPEG_LS };

    int predictors(Standards standard, int a, int b, int c) {
        switch (standard) {
            case JPEG_PL: return a;
            case JPEG_PA: return b;
            case JPEG_PAL: return c;
            case JPEG_ABC: return a + b - c;
            case JPEG_MBC: return a + (b - c) / 2;
            case JPEG_LS:
                if (c >= std::max(a, b)) return std::min(a, b);
                if (c <= std::min(a, b)) return std::max(a, b);
                return a + b - c;
            default: return 0;
        }
    }
};

#endif // PREDICTORS_H
