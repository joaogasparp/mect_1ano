#include "BitStream.h"
#include <iostream>

// construtor padrão
BitStream::BitStream() = default;

// destrutor, fecha o ficheiro se estiver aberto
BitStream::~BitStream() {
    if (fs.is_open()) {
        fs.close();
    }
}

// abre o ficheiro com o modo especificado (leitura ou escrita)
void BitStream::openFile(const std::string &filename, std::ios::openmode mode) {
    fs.open(filename, mode);
    if (!fs.is_open()) {
        throw std::runtime_error("Erro ao abrir o ficheiro: " + filename);
    }
}

// escreve 1 bit no buffer; quando cheio, escreve o byte completo no ficheiro
void BitStream::writeBit(int bit) {
    buffer = (buffer << 1) | (bit & 1);
    bufSize++;
    if (bufSize == 8) { // verifica se o buffer está cheio
        fs.write(&buffer, 1); // escreve no ficheiro
        buffer = 0x00;        // reseta o buffer
        bufSize = 0;
    }
}

// garante que os bits restantes no buffer são escritos no ficheiro
void BitStream::flushBuffer() {
    if (bufSize > 0) {
        buffer <<= (8 - bufSize); // alinha os bits restantes
        fs.write(&buffer, 1);    // escreve no ficheiro
        buffer = 0x00;           // limpa o buffer
        bufSize = 0;
    }
}

// lê 1 bit do buffer, carrega mais bytes do ficheiro se necessário
int BitStream::readBit() {
    if (bufSize == 0) { // carrega mais dados se o buffer estiver vazio
        fs.read(&buffer, 1);
        if (fs.eof()) { // verifica fim do ficheiro
            return -1;
        }
        bufSize = 8; // reinicia o tamanho do buffer
    }
    int bit = (buffer >> (bufSize - 1)) & 1; // extrai o bit mais significativo
    bufSize--;
    return bit;
}

// escreve vários bits (n) no buffer
void BitStream::writeBits(uint64_t value, int n) {
    for (int i = n - 1; i >= 0; i--) {
        writeBit((value >> i) & 1);
    }
}

// lê vários bits (n) do buffer
uint64_t BitStream::readBits(int n) {
    uint64_t value = 0;
    for (int i = 0; i < n; i++) {
        int bit = readBit();
        if (bit == -1) { // erro se o ficheiro acabar inesperadamente
            throw std::runtime_error("Fim do ficheiro durante a leitura de bits.");
        }
        value = (value << 1) | bit;
    }
    return value;
}

// verifica se chegou ao fim do ficheiro
bool BitStream::isEndOfFile() {
    return fs.eof();
}

// alinha o buffer ao próximo byte
void BitStream::alignToByte() {
    buffer = 0x00;
    bufSize = 0;
}
