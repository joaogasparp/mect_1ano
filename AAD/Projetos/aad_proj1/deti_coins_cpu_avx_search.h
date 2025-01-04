#ifndef DETI_COINS_CPU_AVX_SEARCH
#define DETI_COINS_CPU_AVX_SEARCH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "search_utilities.h"

static void deti_coins_cpu_avx_search(u32_t n_random_words)
{
    u32_t n, idx, lane;
    u64_t n_attempts, n_coins;
    coin_t coin[4];
    u32_t hash[4][4u];
    static u32_t interleaved_test_data[13u * 4u] __attribute__((aligned(16)));
    static u32_t interleaved_test_hash[4u * 4u] __attribute__((aligned(16)));
    u32_t v1 = 0x20202020, v2 = 0x20202020;

    for (lane = 0; lane < 4; lane++)
    {
        snprintf(coin[lane].coin_as_chars, 53, "DETI coin %1d%40s\n", lane, "");
    }

    for (n_attempts = n_coins = 0ul; stop_request == 0; n_attempts += 4)
    {
        for (lane = 0u; lane < 4u; lane++)
            for (idx = 0u; idx < 13u; idx++)
                interleaved_test_data[4u * idx + lane] = coin[lane].coin_as_ints[idx];

        md5_cpu_avx((v4si *)interleaved_test_data, (v4si *)interleaved_test_hash);

        for (lane = 0u; lane < 4u; lane++)
        {
            for (idx = 0u; idx < 4u; idx++)
                hash[lane][idx] = interleaved_test_hash[4u * idx + lane];

            hash_byte_reverse(hash[lane]);

            n = deti_coin_power(hash[lane]);

            if (n >= 32u)
            {
                save_deti_coin(coin[lane].coin_as_ints);
                n_coins++;
                printf("hash saved: 0x%8x 0x%8x 0x%8x 0x%8x\n", hash[lane][0], hash[lane][1], hash[lane][2], hash[lane][3]);
            }
        }

        for (lane = 0u; lane < 4u; lane++)
        {
            for (idx = 10u; idx < 53u && coin[lane].coin_as_chars[idx] == (u08_t)126; idx++)
                coin[lane].coin_as_chars[idx] = ' ';
            if (idx < 53u)
                coin[lane].coin_as_chars[idx]++;
        }

        v1 = next_value_to_try(v1);
        if (v1 == 0x20202020)
            v2 = next_value_to_try(v2);

        for (lane = 0; lane < 4; lane++)
        {
            coin[lane].coin_as_ints[5] = v1;
            coin[lane].coin_as_ints[6] = v2;
        }
    }

    STORE_DETI_COINS();
    printf("deti_coins_cpu_avx_search: %lu DETI coin%s found in %lu attempt%s (expected %.2f coins)\n", n_coins, (n_coins == 1ul) ? "" : "s", n_attempts, (n_attempts == 1ul) ? "" : "s", (double)n_attempts / (double)(1ul << 32));
}

#endif