#ifndef SEARCH_UTILITIES
#define SEARCH_UTILITIES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef unsigned int u32_t;

typedef union
{
    u32_t coin_as_ints[13];
    char coin_as_chars[53];
} coin_t;

u32_t next_value_to_try(u32_t v)
{
    do
    {
        v++;
        if ((v & 0xFF) == 0x7E)
        {
            v += 0xA1;
            if (((v >> 8) & 0xFF) == 0x7E)
            {
                v += 0xA1 << 8;
                if (((v >> 16) & 0xFF) == 0x7E)
                {
                    v += 0xA1 << 16;
                    if (((v >> 24) & 0xFF) == 0x7E)
                    {
                        v += 0xA1 << 24;
                        return 0x20202020;
                    }
                    return v;
                }
                return v;
            }
            return v;
        }
    } while (0);
    return v;
}

u32_t next_value_to_try_other(u32_t v)
{
    v++;
    if ((v & 0x000000FF) != 0x0000007F)
        return v;
    v += 0x000000A1;
    if ((v & 0x0000FF00) != 0x00007F00)
        return v;
    v += 0x0000A100;
    if ((v & 0x00FF0000) != 0x007F0000)
        return v;
    v += 0x00A10000;
    if ((v & 0xFF000000) != 0x7F000000)
        return v;
    v += 0xA1000000;
    return 0x20202020;
}

void test_next_value_to_try_ascii()
{
    u32_t initial_values[] = {0x20202079, 0x2020207A, 0x2020207B, 0x2020207C, 0x2020207D};
    int num_initial_values = sizeof(initial_values) / sizeof(initial_values[0]);
    int num_iterations = 1000000;
    int num_tests = 10;

    double total_time = 0.0;

    for (int t = 0; t < num_tests; t++)
    {
        for (int j = 0; j < num_initial_values; j++)
        {
            u32_t v = initial_values[j];
            struct timespec start_time, end_time;

            clock_gettime(CLOCK_MONOTONIC, &start_time);

            for (int i = 0; i < num_iterations; i++)
            {
                v = next_value_to_try(v);
            }

            clock_gettime(CLOCK_MONOTONIC, &end_time);

            double time_taken = (end_time.tv_sec - start_time.tv_sec) * 1e9 + (end_time.tv_nsec - start_time.tv_nsec);
            total_time += time_taken;
        }
    }

    double average_time = total_time / (num_tests * num_initial_values);
    printf("time executing test_next_value_to_try_ascii: %.3f ns\n", average_time);
}

u32_t next_value_to_try_opencl(u32_t v) {
    do {
        v++;
        if ((v & 0xFF) == 0x7E) {
            v += 0xA1;
            if (((v >> 8) & 0xFF) == 0x7E) {
                v += 0xA1 << 8;
                if (((v >> 16) & 0xFF) == 0x7E) {
                    v += 0xA1 << 16;
                    if (((v >> 24) & 0xFF) == 0x7E) {
                        v += 0xA1 << 24;
                        return 0x20202020;
                    }
                    return v;
                }
                return v;
            }
            return v;
        }
    } while (0);
    return v;
}


#endif
