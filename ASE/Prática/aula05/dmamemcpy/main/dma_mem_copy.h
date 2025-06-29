#ifndef DMA_MEM_COPY_H
#define DMA_MEM_COPY_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Estrutura lógica para representar uma transferência de memória via DMA.
 * Esta estrutura é usada apenas na lógica da aplicação e não é diretamente usada pelo hardware.
 */
typedef struct dma_desc {
    uint32_t *src;           ///< Ponteiro para o buffer de origem
    uint32_t *dst;           ///< Ponteiro para o buffer de destino
    size_t size;             ///< Tamanho da transferência em bytes
    struct dma_desc *next;   ///< Permite encadeamento de descritores 
} dma_desc_t;

/**
 * @brief Inicializa o controlador GDMA, habilitando o clock e configurando os registradores.
 * Deve ser chamada antes de qualquer operação de transferência.
 */
void dma_init(void);

/**
 * @brief Inicia a transferência de memória utilizando GDMA, com base no descritor fornecido.
 * A transferência é dividida automaticamente em blocos se necessário.
 *
 * @param desc Ponteiro para o descritor de transferência
 */
void start_dma_transfer(dma_desc_t *desc);

#endif // DMA_MEM_COPY_H
