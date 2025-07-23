#include <cuda_runtime.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_KERNEL_SIZE 169  
#define MAX_BRIGHTNESS 255
#define BLOCK_SIZE 32
#define TILE_SIZE 32

typedef int pixel_t;

__constant__ float d_kernel[MAX_KERNEL_SIZE];
__constant__ float d_gx[9];
__constant__ float d_gy[9];

// Optimized convolution using shared memory
__global__ void convolution_kernel_shared(const pixel_t *in, pixel_t *out,
                                         const int nx, const int ny, const int kn) {
    extern __shared__ pixel_t tile[];
    
    int tx = threadIdx.x;
    int ty = threadIdx.y;
    int bx = blockIdx.x;
    int by = blockIdx.y;
    
    const int khalf = kn / 2;
    const int tile_size = blockDim.x + 2 * khalf;
    
    int x = bx * blockDim.x + tx;
    int y = by * blockDim.y + ty;
    
    // Load tile with padding
    for (int dy = ty; dy < tile_size; dy += blockDim.y) {
        for (int dx = tx; dx < tile_size; dx += blockDim.x) {
            int gx = bx * blockDim.x + dx - khalf;
            int gy = by * blockDim.y + dy - khalf;
            
            if (gx >= 0 && gx < nx && gy >= 0 && gy < ny) {
                tile[dy * tile_size + dx] = in[gy * nx + gx];
            } else {
                tile[dy * tile_size + dx] = 0;
            }
        }
    }
    
    __syncthreads();
    
    if (x < nx && y < ny && x >= khalf && x < nx - khalf && y >= khalf && y < ny - khalf) {
        float pixel = 0.0f;
        
        for (int j = 0; j < kn; j++) {
            for (int i = 0; i < kn; i++) {
                int lx = tx + i;
                int ly = ty + j;
                pixel += tile[ly * tile_size + lx] * d_kernel[j * kn + i];
            }
        }
        
        out[y * nx + x] = (pixel_t)pixel;
    }
}

// Combined gradient magnitude kernel
__global__ void gradient_magnitude_kernel(const pixel_t *after_Gx, const pixel_t *after_Gy, 
                                         pixel_t *G, const int nx, const int ny) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x > 0 && x < nx - 1 && y > 0 && y < ny - 1) {
        int idx = y * nx + x;
        float gx = (float)after_Gx[idx];
        float gy = (float)after_Gy[idx];
        G[idx] = (pixel_t)sqrtf(gx * gx + gy * gy);
    }
}

// Combined convolution and gradient for better memory efficiency
__global__ void sobel_gradient_kernel(const pixel_t *in, pixel_t *Gx, pixel_t *Gy,
                                     const int nx, const int ny) {
    extern __shared__ pixel_t s_tile[];
    
    int tx = threadIdx.x;
    int ty = threadIdx.y;
    int x = blockIdx.x * blockDim.x + tx;
    int y = blockIdx.y * blockDim.y + ty;
    
    const int tile_size = blockDim.x + 2;
    
    // Load shared memory with padding
    for (int dy = ty; dy < tile_size; dy += blockDim.y) {
        for (int dx = tx; dx < tile_size; dx += blockDim.x) {
            int gx = blockIdx.x * blockDim.x + dx - 1;
            int gy = blockIdx.y * blockDim.y + dy - 1;
            
            if (gx >= 0 && gx < nx && gy >= 0 && gy < ny) {
                s_tile[dy * tile_size + dx] = in[gy * nx + gx];
            } else {
                s_tile[dy * tile_size + dx] = 0;
            }
        }
    }
    
    __syncthreads();
    
    if (x > 0 && x < nx - 1 && y > 0 && y < ny - 1) {
        float px = 0.0f, py = 0.0f;
        
        // Apply Sobel filters
        for (int j = 0; j < 3; j++) {
            for (int i = 0; i < 3; i++) {
                int lx = tx + i;
                int ly = ty + j;
                float val = s_tile[ly * tile_size + lx];
                px += val * d_gx[j * 3 + i];
                py += val * d_gy[j * 3 + i];
            }
        }
        
        int idx = y * nx + x;
        Gx[idx] = (pixel_t)px;
        Gy[idx] = (pixel_t)py;
    }
}

// Optimized NMS with shared memory
__global__ void nms_shared_kernel(const pixel_t *Gx, const pixel_t *Gy, const pixel_t *G, 
                                  pixel_t *nms, const int nx, const int ny) {
    extern __shared__ pixel_t s_data[];
    
    int tx = threadIdx.x;
    int ty = threadIdx.y;
    int x = blockIdx.x * blockDim.x + tx;
    int y = blockIdx.y * blockDim.y + ty;
    
    const int s_width = blockDim.x + 2;
    pixel_t *s_G = s_data;
    pixel_t *s_Gx = &s_data[s_width * (blockDim.y + 2)];
    pixel_t *s_Gy = &s_data[2 * s_width * (blockDim.y + 2)];
    
    // Load shared memory
    for (int dy = ty; dy < blockDim.y + 2; dy += blockDim.y) {
        for (int dx = tx; dx < s_width; dx += blockDim.x) {
            int gx = blockIdx.x * blockDim.x + dx - 1;
            int gy = blockIdx.y * blockDim.y + dy - 1;
            int idx = dy * s_width + dx;
            
            if (gx >= 0 && gx < nx && gy >= 0 && gy < ny) {
                int g_idx = gy * nx + gx;
                s_G[idx] = G[g_idx];
                s_Gx[idx] = Gx[g_idx];
                s_Gy[idx] = Gy[g_idx];
            } else {
                s_G[idx] = 0;
                s_Gx[idx] = 0;
                s_Gy[idx] = 0;
            }
        }
    }
    
    __syncthreads();
    
    if (x > 0 && x < nx - 1 && y > 0 && y < ny - 1) {
        int s_idx = (ty + 1) * s_width + (tx + 1);
        
        float angle = atan2f((float)s_Gy[s_idx], (float)s_Gx[s_idx]);
        angle = angle < 0 ? angle + M_PI : angle;
        int dir = __float2int_rn(angle * 4.0f / M_PI) % 4;
        
        pixel_t curr = s_G[s_idx];
        pixel_t n1, n2;
        
        switch(dir) {
            case 0: // horizontal
                n1 = s_G[s_idx - 1];
                n2 = s_G[s_idx + 1];
                break;
            case 1: // diagonal /
                n1 = s_G[s_idx - s_width - 1];
                n2 = s_G[s_idx + s_width + 1];
                break;
            case 2: // vertical
                n1 = s_G[s_idx - s_width];
                n2 = s_G[s_idx + s_width];
                break;
            case 3: // diagonal \
                n1 = s_G[s_idx - s_width + 1];
                n2 = s_G[s_idx + s_width - 1];
                break;
        }
        
        int out_idx = y * nx + x;
        nms[out_idx] = (curr >= n1 && curr >= n2) ? curr : 0;
    }
}

// Combined edge tracking kernel
__global__ void edge_tracking_kernel(const pixel_t *nms, pixel_t *edges,
                                    const int nx, const int ny, 
                                    const int tmin, const int tmax) {
    extern __shared__ pixel_t s_nms[];
    
    int tx = threadIdx.x;
    int ty = threadIdx.y;
    int x = blockIdx.x * blockDim.x + tx;
    int y = blockIdx.y * blockDim.y + ty;
    
    const int s_width = blockDim.x + 2;
    
    // Load shared memory
    for (int dy = ty; dy < blockDim.y + 2; dy += blockDim.y) {
        for (int dx = tx; dx < s_width; dx += blockDim.x) {
            int gx = blockIdx.x * blockDim.x + dx - 1;
            int gy = blockIdx.y * blockDim.y + dy - 1;
            
            if (gx >= 0 && gx < nx && gy >= 0 && gy < ny) {
                s_nms[dy * s_width + dx] = nms[gy * nx + gx];
            } else {
                s_nms[dy * s_width + dx] = 0;
            }
        }
    }
    
    __syncthreads();
    
    if (x > 0 && x < nx - 1 && y > 0 && y < ny - 1) {
        int s_idx = (ty + 1) * s_width + (tx + 1);
        pixel_t val = s_nms[s_idx];
        
        if (val >= tmax) {
            edges[y * nx + x] = MAX_BRIGHTNESS;
        } else if (val >= tmin) {
            // Check 8-connected neighbors
            bool hasStrongNeighbor = false;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dx == 0 && dy == 0) continue;
                    if (s_nms[(ty + 1 + dy) * s_width + (tx + 1 + dx)] >= tmax) {
                        hasStrongNeighbor = true;
                        break;
                    }
                }
                if (hasStrongNeighbor) break;
            }
            edges[y * nx + x] = hasStrongNeighbor ? MAX_BRIGHTNESS : 0;
        } else {
            edges[y * nx + x] = 0;
        }
    }
}

void cannyDevice(const int *h_idata, const int w, const int h,
                 const int tmin, const int tmax,
                 const float sigma,
                 int *h_odata) {
    const int nx = w;
    const int ny = h;
    const size_t size = nx * ny * sizeof(pixel_t);
    
    // Allocate all GPU memory at once
    pixel_t *d_input, *d_gaussian, *d_Gx, *d_Gy, *d_G, *d_nms, *d_edges;
    cudaSafeCall(cudaMalloc(&d_input, size));
    cudaSafeCall(cudaMalloc(&d_gaussian, size));
    cudaSafeCall(cudaMalloc(&d_Gx, size));
    cudaSafeCall(cudaMalloc(&d_Gy, size));
    cudaSafeCall(cudaMalloc(&d_G, size));
    cudaSafeCall(cudaMalloc(&d_nms, size));
    cudaSafeCall(cudaMalloc(&d_edges, size));
    
    // Copy input to GPU
    cudaSafeCall(cudaMemcpy(d_input, h_idata, size, cudaMemcpyHostToDevice));
    
    // Create Gaussian kernel
    const int n = 2 * (int)(2 * sigma) + 3;
    const float mean = (float)floor(n / 2.0);
    float *kernel = (float *)malloc(n * n * sizeof(float));
    
    size_t c = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            kernel[c] = exp(-0.5 * (pow((i - mean) / sigma, 2.0) +
                           pow((j - mean) / sigma, 2.0)))
                       / (2 * M_PI * sigma * sigma);
            c++;
        }
    }
    
    // Copy kernels to constant memory
    cudaSafeCall(cudaMemcpyToSymbol(d_kernel, kernel, n * n * sizeof(float)));
    
    const float Gx[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    const float Gy[] = {1, 2, 1, 0, 0, 0, -1, -2, -1};
    cudaSafeCall(cudaMemcpyToSymbol(d_gx, Gx, 9 * sizeof(float)));
    cudaSafeCall(cudaMemcpyToSymbol(d_gy, Gy, 9 * sizeof(float)));
    
    dim3 blockSize(BLOCK_SIZE, BLOCK_SIZE);
    dim3 gridSize((nx + blockSize.x - 1) / blockSize.x,
                  (ny + blockSize.y - 1) / blockSize.y);
    
    // 1. Gaussian filter with shared memory
    size_t sharedMemSize = (BLOCK_SIZE + 2 * (n/2)) * (BLOCK_SIZE + 2 * (n/2)) * sizeof(pixel_t);
    convolution_kernel_shared<<<gridSize, blockSize, sharedMemSize>>>(d_input, d_gaussian, nx, ny, n);
    
    // 2. Sobel gradients with shared memory
    sharedMemSize = (BLOCK_SIZE + 2) * (BLOCK_SIZE + 2) * sizeof(pixel_t);
    sobel_gradient_kernel<<<gridSize, blockSize, sharedMemSize>>>(d_gaussian, d_Gx, d_Gy, nx, ny);
    
    // 3. Gradient magnitude
    gradient_magnitude_kernel<<<gridSize, blockSize>>>(d_Gx, d_Gy, d_G, nx, ny);
    
    // 4. Non-maximum suppression with shared memory
    sharedMemSize = 3 * (BLOCK_SIZE + 2) * (BLOCK_SIZE + 2) * sizeof(pixel_t);
    nms_shared_kernel<<<gridSize, blockSize, sharedMemSize>>>(d_Gx, d_Gy, d_G, d_nms, nx, ny);
    
    // 5. Edge tracking (combines first edges and hysteresis in one pass)
    sharedMemSize = (BLOCK_SIZE + 2) * (BLOCK_SIZE + 2) * sizeof(pixel_t);
    edge_tracking_kernel<<<gridSize, blockSize, sharedMemSize>>>(d_nms, d_edges, nx, ny, tmin, tmax);
    
    // For remaining weak edges, run hysteresis iterations
    pixel_t *d_edges_tmp;
    cudaSafeCall(cudaMalloc(&d_edges_tmp, size));
    
    int *d_changed;
    cudaSafeCall(cudaMalloc(&d_changed, sizeof(int)));
    
    // Simplified hysteresis kernel
    __global__ void hysteresis_pass(const pixel_t *nms, pixel_t *edges_in, pixel_t *edges_out,
                                   const int nx, const int ny, const int tmin, int *changed) {
        int x = blockIdx.x * blockDim.x + threadIdx.x;
        int y = blockIdx.y * blockDim.y + threadIdx.y;
        
        if (x > 0 && x < nx - 1 && y > 0 && y < ny - 1) {
            int idx = y * nx + x;
            
            if (edges_in[idx] == 0 && nms[idx] >= tmin) {
                // Check neighbors
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (edges_in[(y + dy) * nx + (x + dx)] == MAX_BRIGHTNESS) {
                            edges_out[idx] = MAX_BRIGHTNESS;
                            atomicAdd(changed, 1);
                            return;
                        }
                    }
                }
            }
            edges_out[idx] = edges_in[idx];
        }
    }
    
    // Run hysteresis iterations
    int h_changed;
    int iter = 0;
    do {
        h_changed = 0;
        cudaSafeCall(cudaMemcpy(d_changed, &h_changed, sizeof(int), cudaMemcpyHostToDevice));
        cudaSafeCall(cudaMemcpy(d_edges_tmp, d_edges, size, cudaMemcpyDeviceToDevice));
        
        hysteresis_pass<<<gridSize, blockSize>>>(d_nms, d_edges_tmp, d_edges, nx, ny, tmin, d_changed);
        
        cudaSafeCall(cudaMemcpy(&h_changed, d_changed, sizeof(int), cudaMemcpyDeviceToHost));
        iter++;
    } while (h_changed > 0 && iter < 10);
    
    // Copy result back
    cudaSafeCall(cudaMemcpy(h_odata, d_edges, size, cudaMemcpyDeviceToHost));
    
    // Cleanup
    free(kernel);
    cudaFree(d_input);
    cudaFree(d_gaussian);
    cudaFree(d_Gx);
    cudaFree(d_Gy);
    cudaFree(d_G);
    cudaFree(d_nms);
    cudaFree(d_edges);
    cudaFree(d_edges_tmp);
    cudaFree(d_changed);
}