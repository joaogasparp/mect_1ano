#include <cuda_runtime.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_KERNEL_SIZE 169  
#define MAX_BRIGHTNESS 255
#define BLOCK_SIZE 32

typedef int pixel_t;

// kernels em memória constante (mais rápida)
__constant__ float d_kernel[MAX_KERNEL_SIZE];
__constant__ float d_gx[9];
__constant__ float d_gy[9];

// convolução otimizada com memória partilhada
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
    
    // carregar tile com padding
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

// magnitude do gradiente - igual ao código do professor mas na gpu
__global__ void gradient_magnitude_kernel(const pixel_t *after_Gx, const pixel_t *after_Gy, 
                                         pixel_t *G, const int nx, const int ny) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x > 0 && x < nx - 1 && y > 0 && y < ny - 1) {
        int idx = y * nx + x;
        // usar hypot como no código do professor (linha 274 do canny.cu)
        double gx = (double)after_Gx[idx];
        double gy = (double)after_Gy[idx];
        G[idx] = (pixel_t)hypot(gx, gy);
    }
}

// kernel sobel combinado para melhor eficiência de memória
__global__ void sobel_gradient_kernel(const pixel_t *in, pixel_t *Gx, pixel_t *Gy,
                                     const int nx, const int ny) {
    extern __shared__ pixel_t s_tile[];
    
    int tx = threadIdx.x;
    int ty = threadIdx.y;
    int x = blockIdx.x * blockDim.x + tx;
    int y = blockIdx.y * blockDim.y + ty;
    
    const int tile_size = blockDim.x + 2;
    
    // carregar memória partilhada com padding
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
        
        // aplicar filtros sobel
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

// non-maximum suppression - baseado no código do professor (linha 140-171 canny.cu)
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
    
    // carregar memória partilhada
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
        
        // cálculo da direção igual ao professor (linha 155-157 canny.cu)
        const float dir = (float)(fmod(atan2((double)s_Gy[s_idx], (double)s_Gx[s_idx]) + M_PI, M_PI) / M_PI) * 8;
        
        pixel_t curr = s_G[s_idx];
        bool keep = false;
        
        // verificação dos vizinhos igual ao professor (linha 159-167 canny.cu)
        if ((dir <= 1 || dir > 7) && curr > s_G[s_idx - 1] && curr > s_G[s_idx + 1]) {
            keep = true;  // 0 graus
        } else if (dir > 1 && dir <= 3 && curr > s_G[s_idx - s_width - 1] && curr > s_G[s_idx + s_width + 1]) {
            keep = true;  // 45 graus  
        } else if (dir > 3 && dir <= 5 && curr > s_G[s_idx - s_width] && curr > s_G[s_idx + s_width]) {
            keep = true;  // 90 graus
        } else if (dir > 5 && dir <= 7 && curr > s_G[s_idx - s_width + 1] && curr > s_G[s_idx + s_width - 1]) {
            keep = true;  // 135 graus
        }
        
        int out_idx = y * nx + x;
        nms[out_idx] = keep ? curr : 0;
    }
}

// kernel min-max adaptado do código original
__global__ void min_max_kernel(const pixel_t *in, const int size, 
                               pixel_t *min_val, pixel_t *max_val) {
    extern __shared__ pixel_t shared_data[];
    pixel_t *shared_min = shared_data;
    pixel_t *shared_max = &shared_data[blockDim.x];
    
    int tid = threadIdx.x;
    int gid = blockIdx.x * blockDim.x + threadIdx.x;
    
    shared_min[tid] = INT_MAX;
    shared_max[tid] = -INT_MAX;
    
    if (gid < size) {
        shared_min[tid] = in[gid];
        shared_max[tid] = in[gid];
    }
    
    // processar múltiplos elementos por thread
    for (int i = gid + blockDim.x * gridDim.x; i < size; i += blockDim.x * gridDim.x) {
        if (i < size) {
            shared_min[tid] = min(shared_min[tid], in[i]);
            shared_max[tid] = max(shared_max[tid], in[i]);
        }
    }
    
    __syncthreads();
    
    // redução em memória partilhada
    for (int s = blockDim.x / 2; s > 0; s >>= 1) {
        if (tid < s) {
            shared_min[tid] = min(shared_min[tid], shared_min[tid + s]);
            shared_max[tid] = max(shared_max[tid], shared_max[tid + s]);
        }
        __syncthreads();
    }
    
    if (tid == 0) {
        atomicMin(min_val, shared_min[0]);
        atomicMax(max_val, shared_max[0]);
    }
}

// normalizar imagem - baseado no código do professor (linha 88-100 canny.cu)
__global__ void normalize_kernel(pixel_t *inout, const int nx, const int ny, 
                                const int kn, const int min_val, const int max_val) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    const int khalf = kn / 2;
    
    if (x >= khalf && x < nx - khalf && y >= khalf && y < ny - khalf) {
        int idx = y * nx + x;
        float range = (float)(max_val - min_val);
        if (range > 0) {
            // fórmula igual ao professor (linha 97 canny.cu)
            float normalized = MAX_BRIGHTNESS * ((float)inout[idx] - (float)min_val) / range;
            inout[idx] = (pixel_t)normalized;
        }
    }
}

// primeira passagem - edges fortes (baseado linha 173-188 canny.cu)
__global__ void first_edges_kernel(const pixel_t *nms, pixel_t *edges,
                                  const int nx, const int ny, const int tmax) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x >= 1 && x < nx - 1 && y >= 1 && y < ny - 1) {
        int idx = y * nx + x;
        edges[idx] = (nms[idx] >= tmax) ? MAX_BRIGHTNESS : 0;
    }
}

// hysteresis - baseado no código do professor (linha 192-219 canny.cu)
__global__ void hysteresis_pass(const pixel_t *nms, pixel_t *edges,
                               const int nx, const int ny, const int tmin, int *changed) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int j = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (i >= 1 && i < nx - 1 && j >= 1 && j < ny - 1) {
        int t = j * nx + i;
        
        if (edges[t] == 0 && nms[t] >= tmin) {
            // vizinhos igual ao professor (linha 200-207 canny.cu)
            int nbs[8];
            nbs[0] = t - nx;     // nn
            nbs[1] = t + nx;     // ss  
            nbs[2] = t + 1;      // ww
            nbs[3] = t - 1;      // ee
            nbs[4] = nbs[0] + 1; // nw
            nbs[5] = nbs[0] - 1; // ne
            nbs[6] = nbs[1] + 1; // sw
            nbs[7] = nbs[1] - 1; // se
            
            for (int k = 0; k < 8; k++) {
                if (edges[nbs[k]] != 0) {
                    edges[t] = MAX_BRIGHTNESS;
                    atomicAdd(changed, 1);
                    break;
                }
            }
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
    
    // alocar toda a memória gpu de uma vez (otimização)
    pixel_t *d_input, *d_gaussian, *d_Gx, *d_Gy, *d_G, *d_nms, *d_edges;
    cudaSafeCall(cudaMalloc(&d_input, size));
    cudaSafeCall(cudaMalloc(&d_gaussian, size));
    cudaSafeCall(cudaMalloc(&d_Gx, size));
    cudaSafeCall(cudaMalloc(&d_Gy, size));
    cudaSafeCall(cudaMalloc(&d_G, size));
    cudaSafeCall(cudaMalloc(&d_nms, size));
    cudaSafeCall(cudaMalloc(&d_edges, size));
    
    // copiar input para gpu
    cudaSafeCall(cudaMemcpy(d_input, h_idata, size, cudaMemcpyHostToDevice));
    
    // criar kernel gaussiano - igual ao professor (linha 118-131 canny.cu)
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
    
    // copiar kernels para memória constante (mais rápida)
    cudaSafeCall(cudaMemcpyToSymbol(d_kernel, kernel, n * n * sizeof(float)));
    
    // sobel igual ao professor (linha 256-266 canny.cu)
    const float Gx[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    const float Gy[] = {1, 2, 1, 0, 0, 0, -1, -2, -1};
    cudaSafeCall(cudaMemcpyToSymbol(d_gx, Gx, 9 * sizeof(float)));
    cudaSafeCall(cudaMemcpyToSymbol(d_gy, Gy, 9 * sizeof(float)));
    
    // configuração otimizada - blocos de 32x32
    dim3 blockSize(BLOCK_SIZE, BLOCK_SIZE);
    dim3 gridSize((nx + blockSize.x - 1) / blockSize.x,
                  (ny + blockSize.y - 1) / blockSize.y);
    
    // 1. filtro gaussiano com memória partilhada
    fprintf(stderr, "gaussian_filter: kernel size %d, sigma=%g\n", n, sigma);
    size_t sharedMemSize = (BLOCK_SIZE + 2 * (n/2)) * (BLOCK_SIZE + 2 * (n/2)) * sizeof(pixel_t);
    convolution_kernel_shared<<<gridSize, blockSize, sharedMemSize>>>(d_input, d_gaussian, nx, ny, n);
    
    // normalizar após gaussiano - igual ao professor
    pixel_t *d_min, *d_max;
    cudaSafeCall(cudaMalloc(&d_min, sizeof(pixel_t)));
    cudaSafeCall(cudaMalloc(&d_max, sizeof(pixel_t)));
    
    pixel_t h_min = INT_MAX, h_max = -INT_MAX;
    cudaSafeCall(cudaMemcpy(d_min, &h_min, sizeof(pixel_t), cudaMemcpyHostToDevice));
    cudaSafeCall(cudaMemcpy(d_max, &h_max, sizeof(pixel_t), cudaMemcpyHostToDevice));
    
    // encontrar min e max
    int blockSize1D = 256;
    int gridSize1D = (nx * ny + blockSize1D - 1) / blockSize1D;
    size_t sharedMem1D = 2 * blockSize1D * sizeof(pixel_t);
    
    min_max_kernel<<<gridSize1D, blockSize1D, sharedMem1D>>>(d_gaussian, nx * ny, d_min, d_max);
    cudaDeviceSynchronize();
    
    cudaSafeCall(cudaMemcpy(&h_min, d_min, sizeof(pixel_t), cudaMemcpyDeviceToHost));
    cudaSafeCall(cudaMemcpy(&h_max, d_max, sizeof(pixel_t), cudaMemcpyDeviceToHost));
    
    // normalizar
    normalize_kernel<<<gridSize, blockSize>>>(d_gaussian, nx, ny, n, h_min, h_max);
    cudaDeviceSynchronize();
    
    cudaFree(d_min);
    cudaFree(d_max);
    
    // 2. gradientes sobel com memória partilhada
    sharedMemSize = (BLOCK_SIZE + 2) * (BLOCK_SIZE + 2) * sizeof(pixel_t);
    sobel_gradient_kernel<<<gridSize, blockSize, sharedMemSize>>>(d_gaussian, d_Gx, d_Gy, nx, ny);
    
    // 3. magnitude do gradiente
    gradient_magnitude_kernel<<<gridSize, blockSize>>>(d_Gx, d_Gy, d_G, nx, ny);
    
    // 4. non-maximum suppression com memória partilhada
    sharedMemSize = 3 * (BLOCK_SIZE + 2) * (BLOCK_SIZE + 2) * sizeof(pixel_t);
    nms_shared_kernel<<<gridSize, blockSize, sharedMemSize>>>(d_Gx, d_Gy, d_G, d_nms, nx, ny);
    
    // 5. primeira passagem edges (fortes >= tmax)
    cudaSafeCall(cudaMemset(d_edges, 0, size));
    first_edges_kernel<<<gridSize, blockSize>>>(d_nms, d_edges, nx, ny, tmax);
    
    // 6. hysteresis para edges fracos
    int *d_changed;
    cudaSafeCall(cudaMalloc(&d_changed, sizeof(int)));
    
    // iterar até não haver mudanças - igual ao professor (linha 285-289 canny.cu)
    int h_changed;
    do {
        h_changed = 0;
        cudaSafeCall(cudaMemcpy(d_changed, &h_changed, sizeof(int), cudaMemcpyHostToDevice));
        
        hysteresis_pass<<<gridSize, blockSize>>>(d_nms, d_edges, nx, ny, tmin, d_changed);
        cudaDeviceSynchronize();
        
        cudaSafeCall(cudaMemcpy(&h_changed, d_changed, sizeof(int), cudaMemcpyDeviceToHost));
    } while (h_changed > 0);
    
    // copiar resultado para cpu
    cudaSafeCall(cudaMemcpy(h_odata, d_edges, size, cudaMemcpyDeviceToHost));
    
    // libertar memória
    free(kernel);
    cudaFree(d_input);
    cudaFree(d_gaussian);
    cudaFree(d_Gx);
    cudaFree(d_Gy);
    cudaFree(d_G);
    cudaFree(d_nms);
    cudaFree(d_edges);
    cudaFree(d_changed);
}