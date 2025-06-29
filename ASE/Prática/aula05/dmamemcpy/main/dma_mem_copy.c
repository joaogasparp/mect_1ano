#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "dma_mem_copy.h"
#include "esp_timer.h"
#include "soc/gdma_reg.h"
#include "soc/system_reg.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include <stdbool.h>
#include <stddef.h>

#define TAG "DMA_MEMCPY"              // Prefix for log
#define BUFFER_SIZE 4096              // Number of uint32_t elements (each 4 bytes) = 16 KB total
#define MAX_DMA_BLOCK 4092            // Maximum number of bytes per GDMA descriptor (safe hardware limit)
#define SIZE (BUFFER_SIZE * sizeof(uint32_t)) // Total size in bytes to copy

uint32_t *src = NULL;
uint32_t *dst = NULL;

// GDMA hardware descriptor structure
typedef struct lldesc {
    volatile uint32_t size : 12;       // Capacity of the block (max 4095 bytes)
    volatile uint32_t length : 12;     // Actual number of bytes to transfer
    volatile uint32_t reserved : 5;    // Reserved bits
    volatile uint32_t eof : 1;         // End of Frame (set for last descriptor)
    volatile uint32_t owner : 1;       // 1 = GDMA owns descriptor; 0 = CPU
    uint8_t *buf;                      // Pointer to the data buffer
    struct lldesc *next;               // Pointer to next descriptor (linked list)
} lldesc_t;

bool dma_transfer_in_progress = false;

// Initializes the DMA controller and channel 0
void dma_init() {
    // Enable DMA peripheral clock
    REG_SET_BIT(SYSTEM_PERIP_CLK_EN1_REG, SYSTEM_DMA_CLK_EN);
    // Remove DMA peripheral from reset
    REG_CLR_BIT(SYSTEM_PERIP_RST_EN1_REG, SYSTEM_DMA_RST);

    // Reset DMA channel 0 (both TX(IN) and RX(OUT) directions)
    REG_SET_BIT(GDMA_OUT_CONF0_CH0_REG, GDMA_OUT_RST_CH0);
    REG_SET_BIT(GDMA_IN_CONF0_CH0_REG, GDMA_IN_RST_CH0);
    REG_CLR_BIT(GDMA_OUT_CONF0_CH0_REG, GDMA_OUT_RST_CH0);
    REG_CLR_BIT(GDMA_IN_CONF0_CH0_REG, GDMA_IN_RST_CH0);

    // Set channel 0 to use memory-to-memory mode (peripheral 3)
    REG_WRITE(GDMA_OUT_PERI_SEL_CH0_REG, 3);
    REG_WRITE(GDMA_IN_PERI_SEL_CH0_REG, 3);

    // Enable burst transfer mode for better performance
    REG_SET_BIT(GDMA_OUT_CONF0_CH0_REG, GDMA_OUT_DATA_BURST_EN_CH0);
    REG_SET_BIT(GDMA_IN_CONF0_CH0_REG, GDMA_IN_DATA_BURST_EN_CH0);

    // ESP_LOGI(TAG, "DMA initialized");
}

// DMA memory copy
void start_dma_transfer(dma_desc_t *desc) {

    // if (!desc || !desc->src || !desc->dst || desc->size == 0) {
    //     ESP_LOGE(TAG, "Invalid descriptor.");
    //     return;
    // }

    size_t remaining = desc->size;
    uint8_t *src_ptr = (uint8_t *)desc->src;
    uint8_t *dst_ptr = (uint8_t *)desc->dst;

    // Calculate how many blocks (descriptors) are needed
    int num_blocks = (remaining + MAX_DMA_BLOCK - 1) / MAX_DMA_BLOCK;

    // Allocate physical hardware descriptors for TX and RX
    lldesc_t *tx_descs = heap_caps_calloc(num_blocks, sizeof(lldesc_t), MALLOC_CAP_DMA);
    lldesc_t *rx_descs = heap_caps_calloc(num_blocks, sizeof(lldesc_t), MALLOC_CAP_DMA);

    // if (!tx_descs || !rx_descs) {
    //     ESP_LOGE(TAG, "Failed to allocate DMA descriptors.");
    //     return;
    // }

    // Fill in each TX/RX descriptor and see the appropriate chunk size
    for (int i = 0; i < num_blocks; i++) {
        size_t chunk_size = (remaining > MAX_DMA_BLOCK) ? MAX_DMA_BLOCK : remaining;

        // Configure TX (source) descriptor
        tx_descs[i].size = chunk_size;
        tx_descs[i].length = chunk_size;
        tx_descs[i].eof = (i == num_blocks - 1); // Mark last block
        tx_descs[i].owner = 1;                   // GDMA owns it
        tx_descs[i].buf = src_ptr;
        tx_descs[i].next = (i == num_blocks - 1) ? NULL : &tx_descs[i + 1]; // Linked list for DMA: point to next descriptor, or NULL if last

        // Configure RX (destination) descriptor
        rx_descs[i].size = chunk_size;
        rx_descs[i].length = chunk_size;
        rx_descs[i].eof = (i == num_blocks - 1);
        rx_descs[i].owner = 1;
        rx_descs[i].buf = dst_ptr;
        rx_descs[i].next = (i == num_blocks - 1) ? NULL : &rx_descs[i + 1];

        // Update pointers and remaining size
        src_ptr += chunk_size;
        dst_ptr += chunk_size;
        remaining -= chunk_size;
    }

    // Set the descriptors in GDMA registers
    REG_WRITE(GDMA_OUT_LINK_CH0_REG, (uint32_t)&tx_descs[0]);
    REG_WRITE(GDMA_IN_LINK_CH0_REG, (uint32_t)&rx_descs[0]);

    // Enable memory-to-memory transfer mode
    REG_SET_BIT(GDMA_IN_CONF0_CH0_REG, GDMA_MEM_TRANS_EN_CH0);

    // Start the GDMA transfer
    REG_SET_BIT(GDMA_OUT_LINK_CH0_REG, GDMA_OUTLINK_START_CH0);
    REG_SET_BIT(GDMA_IN_LINK_CH0_REG, GDMA_INLINK_START_CH0);

    dma_transfer_in_progress = true;

    // Wait (polling) for the transfer to finish
    while (!(REG_READ(GDMA_INT_RAW_CH0_REG) & GDMA_IN_SUC_EOF_CH0_INT_RAW)) {
    }

    // Clear interrupt flag
    REG_SET_BIT(GDMA_INT_CLR_CH0_REG, GDMA_IN_SUC_EOF_CH0_INT_CLR);
    dma_transfer_in_progress = false;

    // ESP_LOGI(TAG, "DMA transfer complete.");

    // Free descriptor memory
    free(tx_descs);
    free(rx_descs);
}

void app_main(void) {
    // Allocate memory for source and destination buffers (DMA-compatible)
    src = heap_caps_malloc(SIZE, MALLOC_CAP_DMA);
    dst = heap_caps_malloc(SIZE, MALLOC_CAP_DMA);

    if (!src || !dst) {
        ESP_LOGE(TAG, "Failed to allocate memory");
        return;
    }

    // Initialize source buffer with known values, and zero in destination buffer
    for (int i = 0; i < BUFFER_SIZE; i++) {
        src[i] = i;
        dst[i] = 0;
    }

    dma_init();

    // Create logical DMA transfer descriptor
    dma_desc_t desc = {
        .src = src,
        .dst = dst,
        .size = SIZE,
        .next = NULL
    };

    // Measure time of DMA transfer
    int64_t start = esp_timer_get_time();
    start_dma_transfer(&desc);
    int64_t end = esp_timer_get_time();

    ESP_LOGI(TAG, "DMA took %lld us", end - start);

    // Verify if successful
    bool success = true;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (dst[i] != src[i]) {
            ESP_LOGE(TAG, "Copy error at index %d: expected %" PRIu32 ", got %" PRIu32, i, src[i], dst[i]);
            success = false;
            break;
        }
    }

    if (success) {
        ESP_LOGI(TAG, "Copy verified successfully!");
    } else {
        ESP_LOGE(TAG, "Memory copy failed!");
    }

    // Free allocated buffers
    free(src);
    free(dst);
}
