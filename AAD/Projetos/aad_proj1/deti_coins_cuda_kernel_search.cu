#include "md5.h"
#include "md5_cuda.h"

typedef unsigned int u32_t;

// Define constants
#ifndef MAX_DETI_COINS
#define MAX_DETI_COINS 1024
#endif

// Define a global memory buffer to store found DETI coins
__device__ u32_t deti_coin_buffer[MAX_DETI_COINS][13];
__device__ unsigned int deti_coin_count = 0;

__device__ u32_t deti_coin_power(const u32_t hash[4]) {
    int n = 0;

    if (hash[3] != 0u) {
        n = 32u - __clz(hash[3]);
    } else if (hash[2] != 0u) {
        n = 32u - __clz(hash[2]) + 32u;
    } else if (hash[1] != 0u) {
        n = 32u - __clz(hash[1]) + 64u;
    } else if (hash[0] != 0u) {
        n = 32u - __clz(hash[0]) + 96u;
    } else {
        n = 128u;
    }

    return n;
}

__device__ void hash_byte_reverse(u32_t hash[4]) {
    for (int i = 0; i < 4; i++) {
        hash[i] = __byte_perm(hash[i], 0, 0x0123); // Reverse bytes using CUDA intrinsics
    }
}


// Kernel function for DETI coin search
extern "C" __global__ __launch_bounds__(128, 1) void deti_coins_cuda_kernel_search(u32_t *template_data, u32_t *hash_output, u32_t num_attempts) {
    u32_t a, b, c, d, state[4], x[16];  // Registers for MD5 computation
    unsigned int tid = threadIdx.x + blockDim.x * blockIdx.x;

    // Load and modify the template
    u32_t local_template[13];
    for (int i = 0; i < 13; i++) {
        local_template[i] = template_data[i];
    }
    local_template[3] ^= tid;  // Example: XOR thread ID into template

    // Perform MD5 computations
    for (unsigned int attempt = 0; attempt < num_attempts; attempt++) {
        // Compute MD5 hash
        #define C(c)         (c)
        #define ROTATE(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
        #define DATA(idx)    local_template[idx]
        #define HASH(idx)    hash_output[tid * 4 + idx]
        #define STATE(idx)   state[idx]
        #define X(idx)       x[idx]
        CUSTOM_MD5_CODE();
        #undef C
        #undef ROTATE
        #undef DATA
        #undef HASH
        #undef STATE
        #undef X

        hash_byte_reverse(state);

        // Check DETI coin criteria
        if (deti_coin_power(state) >= 32) {
            unsigned int index = atomicAdd(&deti_coin_count, 1);
            if (index < MAX_DETI_COINS) {
                for (int i = 0; i < 13; i++) {
                    deti_coin_buffer[index][i] = local_template[i];
                }
            }
        }

        // Increment the template for the next attempt
        local_template[4]++;
    }

    // Write back the hash for debugging purposes
    for (int i = 0; i < 4; i++) {
        hash_output[tid * 4 + i] = state[i];
    }
}
