#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <iomanip>
#include <vector>
#include <map>
#include <cmath>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <cstdio>
#include <chrono>

// Variáveis globais
std::string chosenFile;
sf::SoundBuffer buffer;
unsigned int sampleRate;
unsigned int channelCount;
sf::Time duration;
const sf::Int16* samples;
std::size_t sampleCount;

// Função para listar arquivos na pasta 'audio_original' e permitir que o usuário escolha um arquivo
void chooseAudioFile() {
    std::vector<std::string> files;
    std::cout << "Available audio files:\n";
    for (const auto& entry : std::filesystem::directory_iterator("audio_original")) {
        if (entry.path().extension() == ".wav") {
            files.push_back(entry.path().filename().string());
        }
    }

    // Ordenar os arquivos em ordem alfabética
    std::sort(files.begin(), files.end());

    // Exibir os arquivos ordenados
    for (size_t i = 0; i < files.size(); ++i) {
        std::cout << (i + 1) << ". " << files[i] << "\n";
    }

    int choice;
    std::cout << "Choose a file number: ";
    std::cin >> choice;

    if (choice < 1 || static_cast<std::vector<std::string>::size_type>(choice) > files.size()) {
        std::cerr << "Invalid choice. Exiting...\n";
        exit(1);
    }

    chosenFile = "audio_original/" + files[choice - 1];
    std::cout << "Chosen file: " << chosenFile << "\n";

    if (!buffer.loadFromFile(chosenFile)) {
        std::cerr << "Error loading audio file: " << chosenFile << std::endl;
        exit(1);
    }

    // Inicializar as variáveis globais
    sampleRate = buffer.getSampleRate();
    channelCount = buffer.getChannelCount();
    duration = buffer.getDuration();
    samples = buffer.getSamples();
    sampleCount = buffer.getSampleCount();
}

// Função para carregar e exibir informações básicas do arquivo de áudio (T1)
void loadAudioFile() {
    std::cout << "\nAudio file information:" << std::endl;
    std::cout << "Sample rate: " << sampleRate << " Hz" << std::endl;
    std::cout << "Number of channels: " << channelCount << std::endl;
    std::cout << "Duration: " << std::fixed << std::setprecision(2) << duration.asSeconds() << " seconds" << std::endl;

    std::cout << "First 10 raw audio samples: ";
    for (std::size_t i = 0; i < std::min(static_cast<std::size_t>(10), sampleCount); ++i) {
        std::cout << samples[i] << " ";
    }
    std::cout << "\n";
}

// Função para visualizar a forma de onda do áudio (T2)
void visualizeWaveform() {
    sf::RenderWindow window(sf::VideoMode(1200, 600), "Waveform of Audio Sample");
    sf::VertexArray waveform(sf::LinesStrip, sampleCount);

    for (std::size_t i = 0; i < sampleCount; ++i) {
        float x = static_cast<float>(i) / sampleRate * window.getSize().x / duration.asSeconds();
        float y = window.getSize().y / 2 - samples[i] / 32768.0f * window.getSize().y / 2;
        waveform[i].position = sf::Vector2f(x, y);
        waveform[i].color = sf::Color::White;
    }

    sf::Sound sound;
    sound.setBuffer(buffer);
    sound.play();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear();
        window.draw(waveform);
        window.display();

        if (sound.getStatus() != sf::Sound::Playing) {
            window.close();
        }
    }
}

// Função para criar histograma dos valores de amplitude (T3)
std::map<int, int> createHistogram(const sf::Int16* samples, std::size_t sampleCount, unsigned int channelCount, int binSize) {
    std::map<int, int> histogram;
    for (std::size_t i = 0; i < sampleCount; i += channelCount) {
        int sample = samples[i];
        int bin = sample / binSize;
        histogram[bin]++;
    }
    return histogram;
}

void writeGnuplotScript(const std::string& dataFile, const std::string& title, const std::string& outputFile) {
    std::ofstream scriptFile(outputFile);
    scriptFile << "set terminal png size 1200,800\n";
    scriptFile << "set output '" << title << ".png'\n";
    scriptFile << "set title 'Histogram of " << title << " Amplitude'\n";
    scriptFile << "set xlabel 'Amplitude Bin'\n";
    scriptFile << "set ylabel 'Count'\n";
    scriptFile << "plot '" << dataFile << "' using 1:2 with boxes title '" << title << " Channel'\n";
    scriptFile.close();
}

void createAmplitudeHistogram() {
    int binSize = 256;
    std::map<int, int> leftHistogram = createHistogram(samples, sampleCount, 1, binSize);
    std::map<int, int> rightHistogram, monoHistogram, sideHistogram;

    if (channelCount == 2) {
        rightHistogram = createHistogram(samples + 1, sampleCount, 2, binSize);
        for (std::size_t i = 0; i < sampleCount; i += 2) {
            int leftSample = samples[i];
            int rightSample = samples[i + 1];
            int monoSample = (leftSample + rightSample) / 2;
            int sideSample = (leftSample - rightSample) / 2;
            monoHistogram[monoSample / binSize]++;
            sideHistogram[sideSample / binSize]++;
        }
    }

    // Salvar os dados do histograma em arquivos separados
    std::ofstream leftFile("left_histogram_data.txt");
    for (const auto& [bin, count] : leftHistogram) {
        leftFile << bin * binSize << " " << count << "\n";
    }
    leftFile.close();

    std::ofstream rightFile("right_histogram_data.txt");
    for (const auto& [bin, count] : rightHistogram) {
        rightFile << bin * binSize << " " << count << "\n";
    }
    rightFile.close();

    std::ofstream monoFile("mono_histogram_data.txt");
    for (const auto& [bin, count] : monoHistogram) {
        monoFile << bin * binSize << " " << count << "\n";
    }
    monoFile.close();

    std::ofstream sideFile("side_histogram_data.txt");
    for (const auto& [bin, count] : sideHistogram) {
        sideFile << bin * binSize << " " << count << "\n";
    }
    sideFile.close();

    // Criar scripts do gnuplot para cada gráfico
    writeGnuplotScript("left_histogram_data.txt", "Left Channel", "plot_left_histogram.gnuplot");
    writeGnuplotScript("right_histogram_data.txt", "Right Channel", "plot_right_histogram.gnuplot");
    writeGnuplotScript("mono_histogram_data.txt", "Mono Channel", "plot_mono_histogram.gnuplot");
    writeGnuplotScript("side_histogram_data.txt", "Side Channel", "plot_side_histogram.gnuplot");

    // Chamar o gnuplot para gerar os gráficos
    system("gnuplot plot_left_histogram.gnuplot");
    system("gnuplot plot_right_histogram.gnuplot");
    system("gnuplot plot_mono_histogram.gnuplot");
    system("gnuplot plot_side_histogram.gnuplot");

    // Remover arquivos temporários
    std::remove("left_histogram_data.txt");
    std::remove("right_histogram_data.txt");
    std::remove("mono_histogram_data.txt");
    std::remove("side_histogram_data.txt");
    std::remove("plot_left_histogram.gnuplot");
    std::remove("plot_right_histogram.gnuplot");
    std::remove("plot_mono_histogram.gnuplot");
    std::remove("plot_side_histogram.gnuplot");
}

// Função para quantização uniforme do áudio (T4)
void quantizeAudio() {
    std::vector<sf::Int16> quantizedSamples(sampleCount);

    int bits;
    std::cout << "Enter the number of bits for quantization (e.g., 8, 16): ";
    std::cin >> bits;

    int levels = std::pow(2, bits);
    int step = 65536 / levels;

    auto start = std::chrono::high_resolution_clock::now();

    for (std::size_t i = 0; i < sampleCount; ++i) {
        quantizedSamples[i] = (samples[i] / step) * step;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "\nQuantization took: " << elapsed.count() << " seconds" << std::endl;

    sf::SoundBuffer quantizedBuffer;
    quantizedBuffer.loadFromSamples(quantizedSamples.data(), sampleCount, channelCount, sampleRate);

    std::filesystem::create_directory("audio_quantized");

    std::string quantizedFileName = "audio_quantized/quantized_" + std::filesystem::path(chosenFile).filename().string();
    if (!quantizedBuffer.saveToFile(quantizedFileName)) {
        std::cerr << "Error saving quantized audio file: " << quantizedFileName << std::endl;
        return;
    }
    std::cout << "Quantized audio file saved as: " << quantizedFileName << std::endl;

    sf::RenderWindow window(sf::VideoMode(1200, 600), "Waveform of Audio Sample");

    sf::VertexArray originalWaveform(sf::LinesStrip, sampleCount);
    sf::VertexArray quantizedWaveform(sf::LinesStrip, sampleCount);

    for (std::size_t i = 0; i < sampleCount; ++i) {
        float x = static_cast<float>(i) / sampleRate * window.getSize().x / duration.asSeconds();
        float yOriginal = window.getSize().y / 2 - samples[i] / 32768.0f * window.getSize().y / 2;
        float yQuantized = window.getSize().y / 2 - quantizedSamples[i] / 32768.0f * window.getSize().y / 2;
        originalWaveform[i].position = sf::Vector2f(x, yOriginal);
        originalWaveform[i].color = sf::Color::White;
        quantizedWaveform[i].position = sf::Vector2f(x, yQuantized);
        quantizedWaveform[i].color = sf::Color::Red;
    }

    sf::Sound sound;
    sound.setBuffer(buffer);
    sound.play();

    sf::Sound quantizedSound;
    quantizedSound.setBuffer(quantizedBuffer);
    quantizedSound.play();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear();
        window.draw(originalWaveform);
        window.draw(quantizedWaveform);
        window.display();

        if (sound.getStatus() != sf::Sound::Playing && quantizedSound.getStatus() != sf::Sound::Playing) {
            window.close();
        }
    }
}

// Função para calcular MSE e SNR entre amostras de áudio (T5)
void compareAudioSamples() {
    std::vector<sf::Int16> quantizedSamples(sampleCount);

    int bits;
    std::cout << "Enter the number of bits for quantization (e.g., 8, 16): ";
    std::cin >> bits;

    int levels = std::pow(2, bits);
    int step = 65536 / levels;

    for (std::size_t i = 0; i < sampleCount; ++i) {
        quantizedSamples[i] = (samples[i] / step) * step;
    }

    double mse = 0.0;
    double signalPower = 0.0;
    double noisePower = 0.0;

    for (std::size_t i = 0; i < sampleCount; ++i) {
        double error = samples[i] - quantizedSamples[i];
        mse += error * error;
        signalPower += samples[i] * samples[i];
        noisePower += error * error;
    }

    mse /= sampleCount;
    double snr = 10 * std::log10(signalPower / noisePower);

    std::cout << "\nMean Squared Error (MSE): " << mse << std::endl;
    std::cout << "Signal-to-Noise Ratio (SNR): " << snr << " dB" << std::endl;
}

// Função para exibir o menu e chamar as funções correspondentes
void displayMenu() {
    int choice;
    do {
        std::cout << "\nMenu:\n";
        std::cout << "1. Load and display audio file information (T1)\n";
        std::cout << "2. Visualize audio waveform (T2)\n";
        std::cout << "3. Create amplitude histogram (T3)\n";
        std::cout << "4. Quantize audio and visualize waveform (T4)\n";
        std::cout << "5. Compare original and quantized audio samples (T5)\n";
        std::cout << "6. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                loadAudioFile();
                break;
            case 2:
                visualizeWaveform();
                break;
            case 3:
                createAmplitudeHistogram();
                std::cout << "\nHistograms created and saved as .png files in the current directory." << std::endl;
                break;
            case 4:
                quantizeAudio();
                break;
            case 5:
                compareAudioSamples();
                break;
            case 6:
                std::cout << "Exiting...\n";
                break;
            default:
                std::cerr << "Invalid choice. Please try again.\n";
                break;
        }
    } while (choice != 6);
}

int main() {
    chooseAudioFile();
    displayMenu();
    return 0;
}
