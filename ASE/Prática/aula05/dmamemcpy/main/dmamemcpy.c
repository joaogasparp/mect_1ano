// não funciona. acredito que haja uma limitação interna do gdma 
// no chip esp32c3 que não permite a transferência

#include <stdio.h>
#include <string.h>
#include "esp_timer.h"
#include "soc/gdma_reg.h"
#include "soc/gdma_struct.h"
#include "soc/system_reg.h"

#define BUFFER_SIZE 4096
#define REPEAT_COUNT 10000

static uint32_t srcBuffer[BUFFER_SIZE];
static uint32_t dstBuffer[BUFFER_SIZE];

typedef struct
{
    uint32_t dw0;
    uint32_t buffer_addr;
    uint32_t next_desc_addr;
} gdma_descriptor_t;

static gdma_descriptor_t tx_desc __attribute__((aligned(4)));
static gdma_descriptor_t rx_desc __attribute__((aligned(4)));

void app_main(void)
{
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        srcBuffer[i] = i;
    }

    REG_SET_BIT(SYSTEM_PERIP_CLK_EN0_REG, SYSTEM_DMA_CLK_EN);
    REG_CLR_BIT(SYSTEM_PERIP_RST_EN0_REG, SYSTEM_DMA_RST);
    REG_SET_BIT(GDMA_INT_ENA_CH0_REG, GDMA_IN_SUC_EOF_CH0_INT_ENA);

    int64_t startTime = esp_timer_get_time();
    uint32_t transfer_bytes = BUFFER_SIZE * sizeof(uint32_t);

    for (int j = 0; j < REPEAT_COUNT; j++)
    {
        REG_SET_BIT(GDMA_OUT_CONF0_CH0_REG, GDMA_OUT_RST_CH0);
        REG_CLR_BIT(GDMA_OUT_CONF0_CH0_REG, GDMA_OUT_RST_CH0);
        REG_SET_BIT(GDMA_IN_CONF0_CH0_REG, GDMA_IN_RST_CH0);
        REG_CLR_BIT(GDMA_IN_CONF0_CH0_REG, GDMA_IN_RST_CH0);

        tx_desc.dw0 = (1 << 31) | (1 << 30) | ((transfer_bytes & 0xFFF) << 12) | (transfer_bytes & 0xFFF);
        rx_desc.dw0 = (1 << 31) | (1 << 30) | ((transfer_bytes & 0xFFF) << 12) | (transfer_bytes & 0xFFF);

        tx_desc.buffer_addr = (uint32_t)srcBuffer;
        tx_desc.next_desc_addr = 0;

        rx_desc.buffer_addr = (uint32_t)dstBuffer;
        rx_desc.next_desc_addr = 0;

        REG_SET_BIT(GDMA_IN_CONF0_CH0_REG, GDMA_MEM_TRANS_EN_CH0);
        REG_WRITE(GDMA_OUT_LINK_CH0_REG, ((uint32_t)&tx_desc) & 0xFFFFF);
        REG_WRITE(GDMA_IN_LINK_CH0_REG, ((uint32_t)&rx_desc) & 0xFFFFF);
        REG_SET_BIT(GDMA_OUTLINK_START_CH0, GDMA_OUTLINK_START_CH0);
        REG_SET_BIT(GDMA_INLINK_START_CH0, GDMA_INLINK_START_CH0);

        while (!(REG_READ(GDMA_INT_RAW_CH0_REG) & GDMA_IN_SUC_EOF_CH0_INT_RAW))
            ;
        REG_WRITE(GDMA_INT_CLR_CH0_REG, GDMA_IN_SUC_EOF_CH0_INT_CLR);
    }

    int64_t endTime = esp_timer_get_time();
    printf("Elapsed time: %lld microseconds\n", (endTime - startTime));

    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        if (dstBuffer[i] != i)
        {
            printf("Destination buffer verification failed at i=%d!\n", i);
            return;
        }
    }
    printf("Destination buffer verification succeeded!\n");
}
