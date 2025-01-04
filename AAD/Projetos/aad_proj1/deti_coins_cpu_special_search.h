#ifndef DETI_COINS_CPU_SPECIAL_SEARCH
#define DETI_COINS_CPU_SPECIAL_SEARCH

static void deti_coins_cpu_special_search(void)
{
    u32_t n, idx, coin[13u], hash[4u];
    u64_t n_attempts, n_coins;
    u08_t *bytes;

    bytes = (u08_t *)&coin[0];
    bytes[0u] = 'D';
    bytes[1u] = 'E';
    bytes[2u] = 'T';
    bytes[3u] = 'I';
    bytes[4u] = ' ';
    bytes[5u] = 'c';
    bytes[6u] = 'o';
    bytes[7u] = 'i';
    bytes[8u] = 'n';
    bytes[9u] = ' ';
    bytes[10u] = 'B';
    bytes[11u] = 'R';
    bytes[12u] = 'A';
    bytes[13u] = 'I';
    bytes[14u] = 'N';
    bytes[15u] = 'S';
    bytes[16u] = ' ';
    bytes[17u] = 'O';
    bytes[18u] = 'U';
    bytes[19u] = 'T';
    bytes[20u] = ' ';
    for (idx = 21u; idx < 13u * 4u - 1u; idx++)
        bytes[idx] = ' ';
    bytes[13u * 4u - 1u] = '\n';

    for (n_attempts = n_coins = 0ul; stop_request == 0; n_attempts++)
    {
        md5_cpu(coin, hash);
        hash_byte_reverse(hash);
        n = deti_coin_power(hash);

        if (n >= 32u)
        {
            save_deti_coin(coin);
            n_coins++;
        }

        for (idx = 21u; idx < 13u * 4u - 1u && bytes[idx] == (u08_t)126; idx++)
            bytes[idx] = ' ';
        if (idx < 13u * 4u - 1u)
            bytes[idx]++;
    }
    STORE_DETI_COINS();
    printf("deti_coins_cpu_search: %lu DETI coin%s found in %lu attempt%s (expected %.2f coins)\n", n_coins, (n_coins == 1ul) ? "" : "s", n_attempts, (n_attempts == 1ul) ? "" : "s", (double)n_attempts / (double)(1ul << 32));
}

#endif
