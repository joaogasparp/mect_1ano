// deti_coins_cpu_search() using web assembly

#ifndef DETI_COINS_WEB_ASSEMBLY
#define DETI_COINS_WEB_ASSEMBLY

typedef unsigned int u32_t;
#include <stdio.h>
#include <time.h>
#include "md5.h"

#define next_value_to_try(v)              \
  do                                      \
  {                                       \
    v++;                                  \
    if ((v & 0xFF) == 0x7F)               \
    {                                     \
      v += 0xA1;                          \
      if (((v >> 8) & 0xFF) == 0x7F)      \
      {                                   \
        v += 0xA1 << 8;                   \
        if (((v >> 16) & 0xFF) == 0x7F)   \
        {                                 \
          v += 0xA1 << 16;                \
          if (((v >> 24) & 0xFF) == 0x7F) \
          {                               \
            v += 0xA1 << 24;              \
          }                               \
        }                                 \
      }                                   \
    }                                     \
  } while (0)

static void deti_coins_web_assembly(void)
{
  u32_t coin[13u], hash[4u], n_attempts, n_coins, v1, v2;
  clock_t t;

  coin[0] = 0x49544544u;
  coin[1] = 0x696f6320u;
  coin[2] = 0x6e20206eu;
  coin[3] = 0x65626d75u;
  coin[4] = 0x666f2072u;
  coin[5] = 0x65687420u;
  coin[6] = 0x74746120u;
  coin[7] = 0x74706d65u;
  coin[8] = 0x305b2020u;
  coin[9] = 0x30303030u;
  v1 = coin[10] = 0x33333530u;
  v2 = coin[11] = 0x35383238u;
  coin[12] = 0x0a5d3137u;

  t = clock();
  for (n_attempts = n_coins = 0u; stop_request == 0; n_attempts++)
  {

    { // one message -> one MD5 hash
      u32_t a, b, c, d, state[4], x[16];
#define C(c) (c)
#define ROTATE(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#define DATA(idx) coin[idx]
#define HASH(idx) hash[idx]
#define STATE(idx) state[idx]
#define X(idx) x[idx]
      CUSTOM_MD5_CODE();
#undef C
#undef ROTATE
#undef DATA
#undef HASH
#undef STATE
#undef X
    }

    if (hash[3] == 0u)
    {
      for (u32_t i = 0; i < 13; i++)
        printf("0x%08x%c", coin[i], (i == 12) ? '\n' : ' ');
      n_coins++;
    }

    next_value_to_try(v1);
    coin[10] = v1;
    if (v1 == 0x20202020)
    {
      next_value_to_try(v2);
      coin[11] = v2;
    }
  }
  t = clock() - t;
  printf("deti_coins_cpu_search using web assembly: %u DETI coin%s found in %u attempt%s (expected %.2f coins) in %.3fs\n", n_coins, (n_coins == 1u) ? "" : "s", n_attempts, (n_attempts == 1u) ? "" : "s", (double)n_attempts / (double)(1ul << 32), ((double)t) / CLOCKS_PER_SEC);
}

#endif
