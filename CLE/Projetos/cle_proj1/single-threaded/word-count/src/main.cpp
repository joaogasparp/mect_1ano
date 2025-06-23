#include <getopt.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono> // Adicionando a biblioteca chrono

#include "word_count.h"

static option long_options[] = {
    {"lines",   no_argument, 0,  0 },
    {"words",   no_argument, 0,  0 },
    {"chars",   no_argument, 0,  0 },
    {0,         0,           0,  0 }
};

int main(int argc, char* argv[])
{
    auto start = std::chrono::high_resolution_clock::now(); // Iniciando o cronômetro

    WordCountFlags flags;
    while(1) {
        int option_index = 0;
        int c = getopt_long(argc, argv, "lwc",
                 long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'l': flags.lines = true; break;
            case 'w': flags.words = true; break;
            case 'c': flags.chars = true; break;
            case '?':
                // getopt_long already printed an error message
                break;
        }// end switch
    }// end while(1)

    if (!flags.lines && ! flags.words && !flags.chars)
        flags.lines = flags.words = flags.chars = true;

    std::vector<std::string> files;

    // check if there are any files
    if (optind < argc) {
        std::string fname = argv[optind];
        std::ifstream file(fname);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                files.push_back(line);
            }
            file.close();
        } else {
            std::cerr << "ERROR: Unable to open file: " << fname << std::endl;
            return 1;
        }
    } else {
        std::cout << "No files provided" << std::endl;
        return 1;
    }

    word_count(flags, files);

    auto end = std::chrono::high_resolution_clock::now(); // Parando o cronômetro
    std::chrono::duration<double> elapsed = end - start; // Calculando o tempo decorrido
    std::cout << "Execution time: " << elapsed.count() << " seconds" << std::endl; // Exibindo o tempo de execução

    return 0;
}
