#include <fstream>
#include <iostream>
#include <cctype>
#include <algorithm>
#include <map>
#include <locale>
#include <codecvt>
#include <regex>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <filesystem>
#include <sstream>
#include <string>
#include "histogram.h"

using namespace std;
namespace fs = std::filesystem;

wstring readFile(const fs::path& filepath) {
    wifstream infile(filepath);
    infile.imbue(locale(infile.getloc(), new codecvt_utf8<wchar_t>));

    if (!infile) {
        wcerr << L"Erro ao abrir o arquivo: " << filepath << endl;
        return L"";
    }

    wstring content((istreambuf_iterator<wchar_t>(infile)), istreambuf_iterator<wchar_t>());
    infile.close();
    return content;
}

wstring readAllFilesFromFolder(const string& folderPath) {
    wstringstream allContent;
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            wcout << L"Lendo arquivo: " << entry.path().filename().wstring() << endl;
            wstring fileContent = readFile(entry.path());
            allContent << fileContent << L"\n";
        }
    }
    return allContent.str();
}

void removePunctuation(wstring& data) {
    wstring punctuation = L"!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
    data.erase(remove_if(data.begin(), data.end(), [punctuation](wchar_t ch) {
        return punctuation.find(ch) != wstring::npos;
    }), data.end());
}

void convertToLowerCase(wstring& data) {
    for (wchar_t& ch : data) {
        ch = towlower(ch);
    }
}

void countCharacterFrequencies(const wstring& data, map<wchar_t, int>& freqMap) {
    for (const wchar_t& ch : data) {
        if (ch != L' ') {
            freqMap[ch]++;
        }
    }
    // //se 1 caracterer aparece menos de x vezes, ele não é contabilizado (para efeito de entropia e histograma)
    // for (auto it = freqMap.begin(); it != freqMap.end(); ) {
    //     if (it->second <= 15) {
    //         it = freqMap.erase(it);
    //     } else {
    //         ++it;
    //     }
    // }
}

void countWordFrequencies(const wstring& data, map<wstring, int>& freqMap, int& totalWords) {
    wstring word;
    totalWords = 0;
    for (const wchar_t& ch : data) {
        if (ch == L' ') {
            if (!word.empty()) {
                freqMap[word]++;
                totalWords++;
                word.clear();
            }
        } else {
            word += ch;
        }
    }
    if (!word.empty()) {
        freqMap[word]++;
        totalWords++;
    }

    // //se 1 palavra aparece menos de x vezes, ela não é contabilizada (para efeito de entropia e histograma)
    // for (auto it = freqMap.begin(); it != freqMap.end(); ) {
    //     if (it->second <= 50000) {
    //         it = freqMap.erase(it);
    //     } else {
    //         ++it;
    //     }
    // }
}

void removeXMLTags(wstring& data) {
    wregex xmlTagPattern(L"<[^>]*>");
    data = regex_replace(data, xmlTagPattern, L"");
}

double calculateEntropy(const map<wchar_t, int>& freqMap, int totalChars) {
    double entropy = 0.0;
    for (const auto& pair : freqMap) {
        double probability = static_cast<double>(pair.second) / totalChars;
        if (probability > 0) { 
            entropy -= probability * log2(probability);
        }
    }
    return entropy;
}

double calculateWordEntropy(const std::map<std::wstring, int>& freqMap, int totalWords) {
    double entropy = 0.0;
    for (const auto& pair : freqMap) {
        double probability = static_cast<double>(pair.second) / totalWords;
        if (probability > 0) {
            entropy -= probability * log2(probability);
        }
    }
    return entropy;
}

int menu() {
    int option;
    wcout << L"\nEscolha uma opção de processamento de texto:" << endl;
    wcout << L"1. Converter para minúsculas" << endl;
    wcout << L"2. Remover pontuação" << endl;
    wcout << L"3. Remover tags XML" << endl;
    wcout << L"4. Contar frequências de caracteres" << endl;
    wcout << L"5. Contar frequências de palavras" << endl;
    wcout << L"6. Exibir o texto" << endl;
    wcout << L"7. Exibir histogramas" << endl;
    wcout << L"0. Sair" << endl;
    wcout << L"Opção: ";
    wcin >> option;
    return option;
}

int main() {
    locale::global(locale(""));

    string folderPath = "en";
    wstring allText = readAllFilesFromFolder(folderPath);

    if (allText.empty()) {
        wcerr << L"Não foi possível ler os arquivos ou a pasta está vazia." << endl;
        return 1;
    }

    int option = -1;
    map<wchar_t, int> frequencyMap;
    map<wstring, int> wordFrequencyMap;
    int totalChars = allText.length();
    double entropy = 0.0;
    int totalWords = 0;
    std::ofstream leftFile;

    while (option != 0) {
        option = menu();
        switch (option) {
            case 1:
                wcout << L"\nConvertendo texto para minúsculas..." << endl;
                convertToLowerCase(allText);
                wcout << L"\nTexto convertido para minúsculas" << endl;
                break;
            case 2:
                wcout << L"\nRemovendo pontuação..." << endl;
                removePunctuation(allText);
                wcout << L"\nTexto sem pontuação" << endl;
                break;
            case 3:
                wcout << L"\nRemovendo tags XML..." << endl;
                removeXMLTags(allText);
                wcout << L"\nTexto sem tags XML" << endl;
                break;
            case 4:
                frequencyMap.clear();
                wcout << L"\nContando frequências de caracteres..." << endl;
                countCharacterFrequencies(allText, frequencyMap);
                wcout << L"\nFrequências de caracteres:" << endl;

                leftFile.open("caracteres_histogram_data.txt");
                if (!leftFile) {
                    wcerr << L"Erro ao abrir o arquivo caracteres_histogram_data.txt para gravação." << endl;
                    break;
                }
                for (const auto& pair : frequencyMap) {
                    wcout << pair.first << L": " << pair.second << endl;
                    leftFile << "'" << static_cast<char>(pair.first) << "' " << pair.second << "\n";
                }
                leftFile.close();

                entropy = calculateEntropy(frequencyMap, totalChars);
                wcout << L"Entropia H(X): " << entropy << endl;
                break;
            case 5:
                wordFrequencyMap.clear();
                wcout << L"\nContando frequências de palavras..." << endl;
                countWordFrequencies(allText, wordFrequencyMap, totalWords);
                wcout << L"\nFrequências de palavras:" << endl;
                wcout << L"Total de palavras: " << totalWords << endl;

                leftFile.open("words_histogram_data.txt");
                if (!leftFile) {
                    wcerr << L"Erro ao abrir o arquivo words_histogram_data.txt para gravação." << endl;
                    break;
                }
                for (const auto& pair : wordFrequencyMap) {
                    wcout << pair.first << L": " << pair.second << endl;
                    leftFile << convertWStringToString(pair.first) << " " << pair.second << "\n";
                }
                leftFile.close();

                entropy = calculateWordEntropy(wordFrequencyMap, totalWords);
                wcout << L"Entropia H(X): " << entropy << endl;
                break;
            case 6:
                wcout << L"\nTexto:" << endl;
                wcout << allText << endl;
                break;
            case 7:
                wcout << L"\nGerando histogramas..." << endl;

                createHistogramFromCharacterFrequencies(frequencyMap, "caracteres_histogram_data.txt");
                writeGnuplotScriptForCharacters("caracteres_histogram_data.txt", "histograma_de_caracteres", "plot_caracteres_histogram.gnuplot");

                createHistogramFromWordFrequencies(wordFrequencyMap, "words_histogram_data.txt");
                writeGnuplotScriptForWords("words_histogram_data.txt", "histograma_de_palavras", "plot_words_histogram.gnuplot");

                system("gnuplot plot_caracteres_histogram.gnuplot");
                system("gnuplot plot_words_histogram.gnuplot");

                std::remove("caracteres_histogram_data.txt");
                std::remove("words_histogram_data.txt");
                std::remove("plot_caracteres_histogram.gnuplot");
                std::remove("plot_words_histogram.gnuplot");

                wcout << L"Histogramas gerados." << endl;
                break;
            case 0:
                wcout << L"Sair do programa..." << endl;
                break;
            default:
                wcout << L"Opção inválida. Tente novamente." << endl;
        }
    }

    return 0;
}
