#include "Predictors.h"

// predictor jpeg-ls para estimar valores de píxeis
int Predictors::predictJPEG_LS(int a, int b, int c) {
    int pred = a + b - c;
    int minVal = std::min(a, b);
    int maxVal = std::max(a, b);
    if (c >= maxVal) pred = minVal;
    else if (c <= minVal) pred = maxVal;
    return pred;
}