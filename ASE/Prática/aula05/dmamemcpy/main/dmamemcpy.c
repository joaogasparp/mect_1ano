#include <stdio.h>
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_async_memcpy.h"

#define buffer_size 4096
#define repeat_count 10000

static uint32_t srcbuffer[buffer_size] __attribute__((aligned(4)));
static uint32_t dstbuffer[buffer_size] __attribute__((aligned(4)));

static volatile bool transfer_done = false;

// callback da copia dma
static bool async_memcpy_callback(async_memcpy_handle_t handle, async_memcpy_event_t *event, void *args)
{
    transfer_done = true;
    return true;
}

// inicializa o dma
static async_memcpy_handle_t dma_init(void)
{
    async_memcpy_config_t config = ASYNC_MEMCPY_DEFAULT_CONFIG();
    async_memcpy_handle_t dma_chan = NULL;
    esp_err_t err = esp_async_memcpy_install(&config, &dma_chan);

    if (err != ESP_OK)
    {
        printf("Error initializing DMA: %d\n", err);
        return NULL;
    }

    return dma_chan;
}

// copia de memoria via dma
static void dma_memcpy(async_memcpy_handle_t dma_chan, void *dst, const void *src, size_t size)
{
    transfer_done = false;
    esp_async_memcpy(dma_chan, dst, (void *)src, size, async_memcpy_callback, NULL);
    while (!transfer_done)
        ;
}

void app_main(void)
{
    int i, j;
    for (i = 0; i < buffer_size; i++)
    {
        srcbuffer[i] = i;
    }

    async_memcpy_handle_t dma_chan = dma_init();
    if (dma_chan == NULL)
    {
        return;
    }

    int64_t startTime = esp_timer_get_time();

    for (j = 0; j < repeat_count; j++)
    {
        dma_memcpy(dma_chan, dstbuffer, srcbuffer, buffer_size * sizeof(uint32_t));
    }

    int64_t endTime = esp_timer_get_time();

    printf("Elapsed time: %lld microseconds\n", (endTime - startTime));

    for (i = 0; i < buffer_size; i++)
    {
        if (dstbuffer[i] != i)
        {
            printf("Destination buffer verification failed!\n");
            return;
        }
    }

    printf("Destination buffer verification succeeded!\n");
    esp_async_memcpy_uninstall(dma_chan);
}
