#ifndef WORD_COUNT_H
#define WORD_COUNT_H

#include <string>
#include <vector>
#include <unordered_map>

// Estrutura para armazenar o resultado da contagem de palavras
struct WordCountResult {
    int words = 0;
    int lines = 0;
    int chars = 0;
};

// Flags para opções futuras (se existirem)
struct WordCountFlags {
    bool count_lines = false;
    bool count_chars = false;
};

// Função de contagem de palavras
WordCountResult word_count(const std::string &file);

#endif // WORD_COUNT_H
 