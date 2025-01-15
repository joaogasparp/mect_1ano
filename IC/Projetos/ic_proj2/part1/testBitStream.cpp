#include "BitStream.h"
#include <iostream>
#include <fstream>
#include <vector>

int main() {
    // Abrir o ficheiro de texto como entrada
    std::ifstream inputFile("input.txt");
    if (!inputFile) {
        std::cerr << "Erro ao abrir input.txt" << std::endl;
        return 1;
    }

    // Criar um ficheiro binário para escrita
    BitStream bsWrite("output.bin", "w");
    char bit;

    // Ler bits do ficheiro de texto e escrever no ficheiro binário
    while (inputFile >> bit) {
        bsWrite.writeBit(bit - '0'); // Converte char para int (0 ou 1)
    }
    bsWrite.close();
    inputFile.close();

    // Ler o ficheiro binário e verificar os bits
    BitStream bsRead("output.bin", "r");
    std::vector<int> bitsLidos = bsRead.readBits(16); // Lê 16 bits
    bsRead.close();

    // Mostrar os bits lidos
    std::cout << "Bits lidos: ";
    for (int bit : bitsLidos) {
        std::cout << bit;
    }
    std::cout << std::endl;

    return 0;
}
