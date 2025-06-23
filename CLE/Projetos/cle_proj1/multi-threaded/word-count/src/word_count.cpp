#include "word_count.h"
#include <fstream>
#include <sstream>
#include <iostream>

// Implementação da função de contagem de palavras
WordCountResult word_count(const std::string &file) {
    std::ifstream in(file);
    if (!in.is_open()) {
        std::cerr << "Error opening file: " << file << std::endl;
        return {0, 0, 0};
    }

    WordCountResult result = {0, 0, 0};
    std::string line;
    
    while (std::getline(in, line)) {
        result.lines++;
        result.chars += line.size();
        std::istringstream iss(line);
        std::string word;
        while (iss >> word) {
            result.words++;
        }
    }
    in.close();
    return result;
}
