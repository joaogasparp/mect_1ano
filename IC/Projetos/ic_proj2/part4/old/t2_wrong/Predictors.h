#ifndef PREDICTORS_H
#define PREDICTORS_H

#include <opencv2/opencv.hpp>

namespace Predictors {
    enum class Standards {
        JPEG_LS
    };

    int predictPixel(const cv::Mat& image, int row, int col, Standards standard);
}

#endif // PREDICTORS_H
