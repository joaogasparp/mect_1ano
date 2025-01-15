#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

class BitStream {
    private:
        std::fstream file;               // Ficheiro manipulado
        std::string fileName;           // Nome do ficheiro
        std::string fileMode;           // Modo de operação (leitura ou escrita)
        int currentBitPos = 0;          // Posição atual do bit
        int currentArrayPos = 0;        // Posição atual no vetor de bits
        int fileSize = 0;               // Tamanho do ficheiro

    public:
        // Construtor
        BitStream(const std::string& name, const std::string& mode) {
            fileName = name;
            fileMode = mode;

            // Determina o modo de abertura do ficheiro
            std::ios::openmode openMode;

            if (mode == "r") {
                openMode = std::ios::in | std::ios::binary; // Modo leitura
            } 
            else if (mode == "w") {
                openMode = std::ios::out | std::ios::binary | std::ios::trunc; // Modo escrita
            } 
            else {
                std::cerr << "Invalid mode: " << mode << std::endl;
                return;
            }

            // Abre o ficheiro no modo especificado
            file.open(fileName, openMode);

            // Verifica se o ficheiro foi aberto
            if (!file.is_open()) {
                std::cerr << "Failed to open file: " << fileName << std::endl;
                return;
            }

            // Configura tamanho no modo leitura
            if (mode == "r") {
                currentBitPos = 0;
                fileSize = getFileSize();
            } else if (mode == "w") {
                currentBitPos = 0;
                currentArrayPos = 0;
            }
        }

        // Função para obter o tamanho do ficheiro
        int getFileSize() const {
            std::ifstream inFile(fileName, std::ios::binary | std::ios::ate); // Abre no final
            if (!inFile.is_open()) {
                std::cerr << "Failed to open file for size calculation: " << fileName << std::endl;
                return -1; // Retorna -1 em caso de erro
            }
            return static_cast<int>(inFile.tellg()); // Tamanho do ficheiro
        }

        // Fecha o ficheiro
        void close() {
            if (file.is_open()) {
                file.close();
            }
        }

        // Escreve um bit no ficheiro
        void writeBit(int bit) {
            static char buffer = 0;   // Buffer para armazenar bits temporários
            static int count = 0;     // Contador para rastrear o número de bits no buffer

            // Define o bit na posição apropriada, se for 1
            if (bit) {
                buffer |= (1 << (7 - count));
            }
            count++;

            // Escreve o buffer no ficheiro quando estiver cheio (8 bits)
            if (count == 8) {
                file.put(buffer);  // Escreve o byte completo no ficheiro
                buffer = 0;        // Reinicia o buffer
                count = 0;         // Reinicia o contador
            }
        }

        // Lê um bit do ficheiro
        int readBit() {
            static char buffer = 0;  // Buffer para armazenar bits lidos
            static int count = 8;    // Inicializa para forçar a leitura de um novo byte na primeira chamada

            // Lê um novo byte do ficheiro, se todos os bits foram processados
            if (count == 8) {
                file.get(buffer);  // Lê um novo byte do ficheiro
                count = 0;         // Reinicia o contador de bits
            }

            // Extrai o bit na posição atual
            int bit = (buffer >> (7 - count)) & 1;
            count++;  // Avança para o próximo bit
            return bit;
        }

        // Escreve um vetor de bits no ficheiro
        void writeBits(const std::vector<int>& bits) {
            for (int bit : bits) {
                writeBit(bit);  // Escreve cada bit individualmente
            }
        }

        // Lê vários bits do ficheiro
        std::vector<int> readBits(int n) {
            std::vector<int> bits;  // Vetor para armazenar os bits lidos
            bits.reserve(n);        // Reserva espaço para evitar realocações desnecessárias

            for (int i = 0; i < n; ++i) {
                bits.push_back(readBit());  // Lê e adiciona cada bit ao vetor
            }
            return bits;
        }
};

#endif // BITSTREAM_H
