#ifndef DETI_COINS_CPU_AVX2_SEARCH
#define DETI_COINS_CPU_AVX2_SEARCH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "search_utilities.h"
#include "md5_cpu_avx2.h"

static void deti_coins_cpu_avx2_search(u32_t n_random_words)
{
    u32_t n, idx, lane;
    u64_t n_attempts, n_coins;
    coin_t coin[8];
    u32_t hash[8][4u];
    static u32_t interleaved_test_data[13u * 8u] __attribute__((aligned(32)));
    static u32_t interleaved_test_hash[4u * 8u] __attribute__((aligned(32)));
    u32_t v1 = 0x20202020, v2 = 0x20202020;

    for (lane = 0; lane < 8; lane++)
    {
        snprintf(coin[lane].coin_as_chars, 53, "DETI coin %1d%40s\n", lane, "");
    }

    for (n_attempts = n_coins = 0ul; stop_request == 0; n_attempts += 8)
    {
        for (lane = 0u; lane < 8u; lane++)
            for (idx = 0u; idx < 13u; idx++)
                interleaved_test_data[8u * idx + lane] = coin[lane].coin_as_ints[idx];

        md5_cpu_avx2((v8si *)interleaved_test_data, (v8si *)interleaved_test_hash);

        for (lane = 0u; lane < 8u; lane++)
        {
            for (idx = 0u; idx < 4u; idx++)
                hash[lane][idx] = interleaved_test_hash[8u * idx + lane];

            hash_byte_reverse(hash[lane]);

            n = deti_coin_power(hash[lane]);

            if (n >= 32u)
            {
                save_deti_coin(coin[lane].coin_as_ints);
                n_coins++;
                printf("coin found: %s", coin[lane].coin_as_chars);
            }
        }

        for (lane = 0u; lane < 8u; lane++)
        {
            for (idx = 10u; idx < 53u && coin[lane].coin_as_chars[idx] == (u08_t)126; idx++)
                coin[lane].coin_as_chars[idx] = ' ';
            if (idx < 53u)
                coin[lane].coin_as_chars[idx]++;
        }

        v1 = next_value_to_try(v1);
        if (v1 == 0x20202020)
            v2 = next_value_to_try(v2);

        for (lane = 0; lane < 8; lane++)
        {
            coin[lane].coin_as_ints[5] = v1;
            coin[lane].coin_as_ints[6] = v2;
        }
    }

    STORE_DETI_COINS();
    printf("deti_coins_cpu_avx2_search: %lu DETI coin%s found in %lu attempt%s (expected %.2f coins)\n", n_coins, (n_coins == 1ul) ? "" : "s", n_attempts, (n_attempts == 1ul) ? "" : "s", (double)n_attempts / (double)(1ul << 32));
}

#endif
