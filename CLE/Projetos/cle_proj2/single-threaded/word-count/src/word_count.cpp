#include <iostream>
#include <fstream>
#include <sstream>
#include "utf-8.h"
#include "word_count.h"

void word_count(const WordCountFlags& flags, const std::vector<std::string>& files)
{
    int total_chars = 0;
    int total_lines = 0;
    int total_words = 0;

    for (const auto& file : files) {
        std::ifstream f(file);
        if (!f.is_open()) {
            std::cerr << "ERROR: could not open file " << file << std::endl;
            continue;
        }

        int chars = 0;
        int lines = 0;
        int words = 0;
        std::string line;
        while (std::getline(f, line)) {
            if (flags.lines) {
                lines++;
            }
            if (flags.words) {
                std::istringstream iss(line);
                std::string word;
                while (iss >> word) {
                    words++;
                }
            }
            if (flags.chars) {
                UTF8DecoderState state;
                uint32_t codepoint;
                for (char c : line) {
                    if (utf8_decode(state, static_cast<uint16_t>(c), &codepoint) > 0){
                        chars++;
                    }
                }
            }
        }

        f.close();

        total_lines += lines;
        total_words += words;
        total_chars += chars;

        std::cout << file << ": " << chars << " " << lines << " " << words << std::endl;
    }

    std::cout << "total: " << total_chars << " " << total_lines << " " << total_words << std::endl;
}
