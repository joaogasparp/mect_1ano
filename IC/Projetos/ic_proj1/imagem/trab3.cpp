#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <filesystem>
#include "ImageProcessor.cpp"

// handle menu operations
void findImages(const std::string& caminho) {
    std::cout << "Available images:" << std::endl;
    for (const auto& entry : std::filesystem::directory_iterator(caminho)) {
        if (entry.is_regular_file()) {
            std::string extensao = entry.path().extension().string();
            if (extensao == ".ppm" || extensao == ".jpg" || extensao == ".png") {
                std::cout << entry.path().filename().string() << std::endl;
            }
        }
    }
}

// choose an image
std::string escolherImagem() {
    std::string imagemEscolhida;
    std::cout << "Enter the image name (including the extension): ";
    std::cin >> imagemEscolhida;
    return imagemEscolhida;
}

// Function to compare two images
void compareImages(ImageProcessor& imgProc) {
    findImages("./");
    std::cout << "Select the first image:" << std::endl;
    std::string caminhoImagem1 = escolherImagem();

    findImages("./");
    std::cout << "Select the second image:" << std::endl;
    std::string caminhoImagem2 = escolherImagem();

    if (!imgProc.loadImages(caminhoImagem1, caminhoImagem2)) {
        std::cout << "Unable to load the images." << std::endl;
        return;
    }

    imgProc.compareImages();  // Compare the two images
}

// handle the menu for image analysis and processing
void mainMenuImage(ImageProcessor& imgProc) {
    int op;

    while (true) {
        std::cout << "\nSelect an option:" << std::endl;
        std::cout << "1 - Load an image" << std::endl;
        std::cout << "2 - Display original image" << std::endl;
        std::cout << "3 - Display channels and grayscale" << std::endl;
        std::cout << "4 - Generate the histogram (Gnuplot)" << std::endl;
        std::cout << "5 - Apply Gaussian Filter" << std::endl;
        std::cout << "6 - Perform uniform scalar quantization" << std::endl;
        std::cout << "7 - Return to the main menu" << std::endl;
        std::cout << "0 - Exit" << std::endl;

        std::cin >> op;

        switch (op) {
            case 1: {
                findImages("./");
                std::string caminhoImagem = escolherImagem();

                if (!imgProc.loadImage(caminhoImagem)) {
                    std::cout << "Unable to load the image: " << caminhoImagem << std::endl;
                    break;
                }
                std::cout << "Image loaded" << std::endl;
                break;
            }
            case 2: {
                imgProc.displayOriginalImage();
                break;
            }
            case 3: {
                imgProc.displayImageChannels();
                break;
            }
            case 4: {
                if (imgProc.calculateHistogram().empty()) {
                    std::cout << "No image loaded. Load one image first." << std::endl;
                } else {
                    imgProc.plotHistogramImage();
                }
                break;
            }
            case 5: {
                if (imgProc.calculateHistogram().empty()) {
                    std::cout << "No image loaded. Load one image first." << std::endl;
                } else {
                    int kernelSize;
                    std::cout << "Enter the kernel size for the Gaussian filter (odd number >= 3): ";
                    std::cin >> kernelSize;
                    imgProc.applyGaussianFilter(kernelSize);
                }
                break;
            }
            case 6: {
                if (imgProc.calculateHistogram().empty()) {
                    std::cout << "No image loaded. Load one image first." << std::endl;
                } else {
                    int quantizationLevels;
                    std::cout << "Enter the number of quantization levels (2, 4, 8, 16, 32, etc.): ";
                    std::cin >> quantizationLevels;

                    imgProc.quantizeImage(quantizationLevels);
                }
                break;
            }
            case 7: {
                return;  // Return to the main menu
            }
            case 0: {
                exit(0);  // Exit the program
            }
            default:
                std::cout << "Invalid option." << std::endl;
        }
    }
}

// Main function
int main() {
    ImageProcessor imgProc;  // Create an instance of ImageProcessor
    
    int opcao;
    while (true) {
        std::cout << "\nChoose an option:" << std::endl;
        std::cout << "1 - Analyze an image" << std::endl;
        std::cout << "2 - Compare two images" << std::endl;
        std::cout << "0 - Exit" << std::endl;
        std::cin >> opcao;

        switch (opcao) {
            case 1:
                mainMenuImage(imgProc);
                break;
            case 2:
                compareImages(imgProc);
                break;
            case 0:
                return 0;
            default:
                std::cout << "Invalid option." << std::endl;
        }
    }
}
