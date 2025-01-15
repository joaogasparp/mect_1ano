// Golomb class
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <bitset>
#include <cmath>

// C++ can only write bytes not bits. So we need to write a byte at a time.
class Golomb {
    private:
        int min_bits;
        int max_bits;
        int n_values_with_min_bits;

        // Calcula os bits mínimos e máximos, assim como os valores com bits mínimos
        void calculateBits(int m) {
            if (m > 0) { // Verifica se m é válido
                // Calcula o número máximo de bits necessários (ceil(log2(m)))
                max_bits = static_cast<int>(ceil(log2(m)));

                // Calcula o número mínimo de bits (max_bits - 1)
                min_bits = max_bits - 1;

                // Calcula a quantidade de valores representáveis com min_bits
                n_values_with_min_bits = (1 << max_bits) - m; // 2^max_bits - m
            } else { // Caso m seja 0, todos os valores também são 0
                max_bits = 0;
                min_bits = 0;
                n_values_with_min_bits = 0;
            }
        }

        // Converte um número inteiro em uma string de bits com n bits de representação
        std::string remaindersBitString(int num, int n_bits_representation) {
            std::string result;
            result.reserve(n_bits_representation); // Reserva espaço para eficiência

            // Constrói a string de bits (da direita para a esquerda)
            for (int i = 0; i < n_bits_representation; i++) {
                result = ((num & 1) ? '1' : '0') + result; // Adiciona o bit no início
                num >>= 1; // Desloca o número para a direita (divide por 2)
            }

            return result; // Retorna a string resultante
        }

        // Converte uma string de bits em um número inteiro
        int bitStringToInt(const std::string& bit_string) {
            int result = 0;

            // Percorre cada caractere da string e converte para inteiro
            for (char bit : bit_string) {
                result = (result << 1) | (bit - '0'); // Multiplica por 2 e adiciona o bit atual
            }

            return result;
        }

    public:
        // constructor
        Golomb() { }

        // Decodifica uma string codificada para um valor com M fixo
        std::vector<int> decode(const std::string& encoded_string, int m) {
            calculateBits(m);                      // Calcula os parâmetros baseados em M
            std::vector<int> result;               // Armazena os valores decodificados
            size_t i = 0;                          // Posição atual no código

            // Percorre a string codificada
            while (i < encoded_string.length()) {
                int quotient = 0;

                // Conta o número de '0' para obter o quociente (codificado em código unário)
                while (i < encoded_string.length() && encoded_string[i] == '0') {
                    quotient++;
                    i++; // Avança para o próximo bit
                }

                i++; // Pula o '1' que marca o final do quociente

                int remainder = 0;                  // Inicializa o resto
                int j = 0;                          // Contador para bits do resto
                std::string tmp = "";               // Armazena o resto temporariamente

                // Calcula o resto se M não for 1
                if (m != 1) {
                    // Lê os primeiros min_bits do resto
                    while (j < min_bits && i < encoded_string.length()) {
                        tmp += encoded_string[i++];
                        j++;
                    }

                    // Converte a string temporária para inteiro
                    int res1 = bitStringToInt(tmp);

                    // Verifica se precisa ler mais um bit
                    if (res1 < n_values_with_min_bits) {
                        remainder = res1; // Usa o valor lido diretamente
                    } else if (i < encoded_string.length()) {
                        tmp += encoded_string[i++]; // Adiciona o bit extra
                        remainder = bitStringToInt(tmp) - n_values_with_min_bits;
                    }
                } else {
                    remainder = 0;
                    if (i < encoded_string.length()) i++; // Avança no código
                }

                // Calcula o resultado combinando quociente e resto
                int res = quotient * m + remainder;

                // Verifica o sinal do resultado
                if (i < encoded_string.length() && encoded_string[i] == '1') {
                    result.push_back(-res); // Negativo
                } else {
                    result.push_back(res);  // Positivo
                }

                i++; // Avança para o próximo bit
            }

            return result; // Retorna os valores decodificados
        }

        //decode function for dynamic Ms (changing Ms)
        std::vector<int> decodeMultiple(std::string encoded_string, std::vector<int> m_vector, int block_size) {
            std::vector<int> result;
            // bit position in the encoded string
            int i = 0;
            // m index on the vector for the current block
            int m_i = 0;
            // blocksize bits counter
            int count = 0;
            calculateBits(m_vector[m_i]);
            while((long unsigned int) i < encoded_string.length()) {
                    int quotient = 0;
                    // count the number of 0s in the encoded string to get the quotient (written in unary code)
                    while (encoded_string[i] == '0') {
                        quotient++;
                        // next bit
                        i++;
                    }
                    // skip a bit (the 1 in the unary code, that represents the end of the quotient)
                    i++;
                    int remainder = 0;
                    // counter for the number of bits read from the encoded string in the remainder part (binary code)
                    int j = 0;
                    std::string tmp = "";

                    // if m is 1, the remainder is 0;
                    // besides that if m is 0, it's forced as 1 (to avoid division by 0)
                    if (m_vector[m_i] != 1){
                        while (j < min_bits) {
                            tmp += encoded_string[i];
                            // next bit
                            i++;
                            // next bit of the remainder part (binary code)
                            j++;
                        }
                        
                        // convert the temporary string to integer to get the remainder
                        int res1 = bitStringToInt(tmp);

                        // if the remainder has a value that is greater than the number of values with min_bits (which corresponds to the max value with min_bits)
                        // the next bit must be read as part of the remainder
                        // if the value is smaller, the remainder is the value read so far
                        if (res1 < n_values_with_min_bits) {
                            remainder = res1;
                        } else {
                            tmp += encoded_string[i];
                            // next bit
                            i++;
                            remainder = bitStringToInt(tmp) - n_values_with_min_bits;
                        }
                    } else {
                        remainder = 0;
                        // next bit
                        i++;
                    }

                    // result value without sign
                    int res = quotient * m_vector[m_i] + remainder;

                    // if the encoded string has a 1 in the end of the remainder, the result is negative, otherwise it's positive
                    if (encoded_string[i] == '1') {
                        result.push_back(-(res));
                    } else {
                        result.push_back(res);
                    }

                    // next bit
                    i++;

                    count++;
                    // if the block size is reached, the m index is incremented
                    if (count == block_size) {
                        // next m index
                        m_i++;
                        // reset blocksize bits counter
                        count = 0;
                        calculateBits(m_vector[m_i]);
                    }  
                }
            return result;
        }

        // Codifica um número usando codificação de Golomb com parâmetro M
        std::string encode(int num, int m) {
            // Calcula os bits necessários para codificar com base em M
            calculateBits(m);

            // Inicializa a string de resultado
            std::string result;

            // Calcula o quociente e o resto usando codificação de Golomb
            int quotient = 0, remainder = 0;
            if (m > 0) { // Evita divisões por zero
                quotient = abs(num) / m;       // Quociente (parte inteira da divisão)
                remainder = abs(num) % m;     // Resto (parte fracionária)
            }

            // Adiciona o quociente em código unário ('0's seguidos por '1')
            result.append(quotient, '0');     // Repete '0' pelo valor do quociente
            result += '1';                    // Marca o fim do quociente com '1'

            // Codifica o resto em binário, dependendo do valor de M
            if (m != 1) { // Quando M é 1, o resto sempre será 0
                if (remainder < n_values_with_min_bits) {
                    // Usa o número mínimo de bits para codificar o resto
                    result += remaindersBitString(remainder, min_bits);
                } else {
                    // Codifica com o número máximo de bits, ajustando o valor
                    result += remaindersBitString(remainder + n_values_with_min_bits, max_bits);
                }
            } else {
                result += '0'; // Adiciona '0' fixo quando M é 1
            }

            // Adiciona o bit de sinal: '1' para negativo e '0' para positivo
            result += (num < 0) ? '1' : '0';

            return result; // Retorna a string codificada
        }
};
