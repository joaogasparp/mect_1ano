#include "histogram.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <locale>
#include <codecvt>
#include <fstream>
#include <map>

void createHistogramFromCharacterFrequencies(const std::map<wchar_t, int>& freqMap, const std::string& filename) {
    std::ofstream outFile(filename);
    for (const auto& [ch, freq] : freqMap) {
        outFile << "'" << static_cast<char>(ch) << "' " << freq << "\n";
    }
    outFile.close();
}

std::string convertWStringToString(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

void createHistogramFromWordFrequencies(const std::map<std::wstring, int>& freqMap, const std::string& filename) {
    std::ofstream outFile(filename);
    for (const auto& [word, freq] : freqMap) {
        outFile << convertWStringToString(word) << " " << freq << "\n";
    }
    outFile.close();
}

void writeGnuplotScriptForWords(const std::string& dataFile, const std::string& title, const std::string& outputFile) {
    std::ofstream scriptFile(outputFile);
    scriptFile << "set terminal png size 1980,800\n";
    scriptFile << "set output '" << title << ".png'\n";
    scriptFile << "set title '" << title << "'\n";
    scriptFile << "set xlabel 'Palavras'\n";
    scriptFile << "set ylabel 'Frequência'\n";
    scriptFile << "set style fill solid 1.0\n";
    scriptFile << "set boxwidth 0.8\n";
    scriptFile << "set xtics rotate by -45\n";
    scriptFile << "set yrange [0:*]\n";
    scriptFile << "plot '" << dataFile << "' using 2:xtic(1) with boxes lc rgb 'blue' title '" << title << "'\n";
    scriptFile.close();
}

void writeGnuplotScriptForCharacters(const std::string& dataFile, const std::string& title, const std::string& outputFile) {
    std::ofstream scriptFile(outputFile);
    scriptFile << "set terminal png size 1980,800\n";
    scriptFile << "set output '" << title << ".png'\n";
    scriptFile << "set title '" << title << "'\n";
    scriptFile << "set xlabel 'Caracteres'\n";
    scriptFile << "set ylabel 'Frequência'\n";
    scriptFile << "set style fill solid 1.0\n";
    scriptFile << "set boxwidth 0.8\n";
    scriptFile << "set xtics rotate by -45\n";
    scriptFile << "plot '" << dataFile << "' using 2:xtic(1) with boxes lc rgb 'blue' title '" << title << "'\n";
    scriptFile.close();
}