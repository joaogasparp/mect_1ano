#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

using namespace std;
using namespace cv;

// função para calcular o mse entre duas imagens
double calculateMSEBetween(const cv::Mat &img1, const cv::Mat &img2) {
    if (img1.empty() || img2.empty() || img1.size() != img2.size() || img1.type() != img2.type()) {
        cerr << "erro: imagens vazias ou tamanhos/tipos diferentes." << endl;
        return -1.0;
    }

    double mse = 0.0;
    for (int y = 0; y < img1.rows; y++) {
        for (int x = 0; x < img1.cols; x++) {
            for (int c = 0; c < img1.channels(); c++) {
                double diff = img1.at<Vec3b>(y, x)[c] - img2.at<Vec3b>(y, x)[c];
                mse += diff * diff;
            }
        }
    }
    mse /= (double)(img1.total() * img1.channels());
    return mse;
}

// função para calcular o psnr entre duas imagens
double calculatePSNRBetween(const cv::Mat &img1, const cv::Mat &img2) {
    double mse = calculateMSEBetween(img1, img2);
    if (mse <= 0) {
        return 100.0; // imagens idênticas
    }

    double psnr = 10.0 * log10((255 * 255) / mse);
    return psnr;
}

// função principal para comparar dois vídeos
void compareVideos(const string &originalPath, const string &reconstructedPath) {
    VideoCapture originalVideo(originalPath);
    VideoCapture reconstructedVideo(reconstructedPath);

    if (!originalVideo.isOpened() || !reconstructedVideo.isOpened()) {
        cerr << "erro: não foi possível abrir os vídeos." << endl;
        return;
    }

    Mat originalFrame, reconstructedFrame;
    int frameCount = 0;
    double totalMSE = 0.0;
    double totalPSNR = 0.0;

    while (true) {
        originalVideo >> originalFrame;
        reconstructedVideo >> reconstructedFrame;

        if (originalFrame.empty() || reconstructedFrame.empty()) {
            break; // fim de um dos vídeos
        }

        // verifica dimensões dos frames
        if (originalFrame.size() != reconstructedFrame.size() || originalFrame.type() != reconstructedFrame.type()) {
            cerr << "erro: tamanho ou tipo de frame incompatível no frame " << frameCount << endl;
            return;
        }

        double mse = calculateMSEBetween(originalFrame, reconstructedFrame);
        double psnr = calculatePSNRBetween(originalFrame, reconstructedFrame);

        totalMSE += mse;
        totalPSNR += psnr;
        frameCount++;

        cout << "frame " << frameCount << ": mse = " << mse << ", psnr = " << psnr << " dB" << endl;
    }

    if (frameCount > 0) {
        cout << "\n=== resumo ===" << endl;
        cout << "total de frames comparados: " << frameCount << endl;
        cout << "mse médio: " << (totalMSE / frameCount) << endl;
        cout << "psnr médio: " << (totalPSNR / frameCount) << " dB" << endl;
    } else {
        cerr << "erro: nenhum frame foi comparado." << endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: ./compare <vídeo_original> <vídeo_reconstruído>" << endl;
        return 1;
    }

    string originalPath = argv[1];
    string reconstructedPath = argv[2];

    compareVideos(originalPath, reconstructedPath);

    return 0;
}
