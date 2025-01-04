
#if defined(__GNUC__) && defined(__AVX2__)
#ifndef MD5_CPU_AVX2
#define MD5_CPU_AVX2

#include <immintrin.h>

typedef __m256i v8si;

static void md5_cpu_avx2(v8si *interleaved8_data, v8si *interleaved8_hash)
{
    v8si a, b, c, d, interleaved8_state[4], interleaved8_x[16];
#define C(c) _mm256_set1_epi32(c)
#define ROTATE(x, n) _mm256_or_si256(_mm256_slli_epi32(x, n), _mm256_srli_epi32(x, 32 - (n)))
#define DATA(idx) interleaved8_data[idx]
#define HASH(idx) interleaved8_hash[idx]
#define STATE(idx) interleaved8_state[idx]
#define X(idx) interleaved8_x[idx]
    CUSTOM_MD5_CODE();
#undef C
#undef ROTATE
#undef DATA
#undef HASH
#undef STATE
#undef X
}

static void test_md5_cpu_avx2(void)
{
#define N_TIMING_TESTS 1000000u
    static u32_t interleaved_test_data[13u * 8u] __attribute__((aligned(32)));
    static u32_t interleaved_test_hash[4u * 8u] __attribute__((aligned(32)));
    u32_t n, lane, idx, *htd, *hth;

    if (N_MESSAGES % 8u != 0u)
    {
        fprintf(stderr, "test_md5_cpu_avx2: N_MESSAGES is not a multiple of 8\n");
        exit(1);
    }
    htd = &host_md5_test_data[0u];
    hth = &host_md5_test_hash[0u];
    for (n = 0u; n < N_MESSAGES; n += 8u)
    {
        for (lane = 0u; lane < 8u; lane++)
        {
            for (idx = 0u; idx < 13u; idx++)
                interleaved_test_data[13u * lane + idx] = htd[13u * (n + lane) + idx];
        }
        md5_cpu_avx2((v8si *)interleaved_test_data, (v8si *)interleaved_test_hash);
        for (lane = 0u; lane < 8u; lane++)
        {
            for (idx = 0u; idx < 4u; idx++)
                hth[4u * (n + lane) + idx] = interleaved_test_hash[4u * lane + idx];
        }
    }

#if N_TIMING_TESTS > 0u
    time_measurement();
    for (n = 0u; n < N_TIMING_TESTS; n++)
    {
        md5_cpu_avx2((v8si *)interleaved_test_data, (v8si *)interleaved_test_hash);
    }
    time_measurement();
    printf("time per md5 hash ( avx2): %7.3fns %7.3fns\n", cpu_time_delta_ns() / (double)(8u * N_TIMING_TESTS), wall_time_delta_ns() / (double)(8u * N_TIMING_TESTS));
#endif
#undef N_TIMING_TESTS
}

#endif
#endif
