#include <cuda_runtime.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_KERNEL_SIZE 169  // Support up to 13x13 kernels
#define MAX_BRIGHTNESS 255

typedef int pixel_t;

__constant__ float d_kernel[MAX_KERNEL_SIZE];

__global__ void convolution_kernel_cuda(const pixel_t *in, pixel_t *out,
                                        const int nx, const int ny, const int kn) {
    int m = blockIdx.x * blockDim.x + threadIdx.x; // coluna
    int n = blockIdx.y * blockDim.y + threadIdx.y; // linha

    const int khalf = kn / 2;

    if (m >= khalf && m < nx - khalf && n >= khalf && n < ny - khalf) {
        float pixel = 0.0f;
        size_t c = 0;

        for (int j = -khalf; j <= khalf; j++) {
            for (int i = -khalf; i <= khalf; i++) {
                int y = n - j;
                int x = m - i;
                pixel += in[y * nx + x] * d_kernel[c];
                c++;
            }
        }

        out[n * nx + m] = (pixel_t)pixel;
    }
}

void convolution_cuda(const pixel_t *in, pixel_t *out, const float *kernel,
                      const int nx, const int ny, const int kn) {
    assert(kn % 2 == 1);
[4:57 PM]
já estou cá amorix
￼
João Monteiro — 8:03 PM
@irin_eu
[8:03 PM]
o projeto de cle está feito acho
[8:03 PM]
está tudo no github
[8:03 PM]
# resultados do projeto canny cuda

## objetivo de cada task

**task 1 - gaussian filter**: aplicar um filtro gaussiano para reduzir ruído na imagem. o sigma controla o nível de desfoque.
Expand
RESULTADOS.md
2 KB
￼￼Choose FilesNo file chosen
￼
Message esgoto

    assert(nx > kn && ny > kn);
    assert(kn * kn <= MAX_KERNEL_SIZE);

    const size_t img_size = nx * ny * sizeof(pixel_t);
    const size_t kernel_size = kn * kn * sizeof(float);

    pixel_t *d_in = nullptr;
    pixel_t *d_out = nullptr;

    cudaSafeCall(cudaMalloc((void **)&d_in, img_size));
    cudaSafeCall(cudaMalloc((void **)&d_out, img_size));

    cudaSafeCall(cudaMemcpy(d_in, in, img_size, cudaMemcpyHostToDevice));
    cudaSafeCall(cudaMemcpyToSymbol(d_kernel, kernel, kernel_size));

    dim3 blockSize(16, 16);
    dim3 gridSize((nx + blockSize.x - 1) / blockSize.x,
                  (ny + blockSize.y - 1) / blockSize.y);

    convolution_kernel_cuda<<<gridSize, blockSize>>>(d_in, d_out, nx, ny, kn);
    cudaDeviceSynchronize();
    cudaCheckMsg("convolution_kernel_cuda failed");

    cudaSafeCall(cudaMemcpy(out, d_out, img_size, cudaMemcpyDeviceToHost));

    cudaFree(d_in);
    cudaFree(d_out);
}

// Kernel to find min and max values in an image
__global__ void min_max_kernel(const pixel_t *in, const int size, 
                               pixel_t *min_val, pixel_t *max_val) {
    extern __shared__ pixel_t shared_data[];
    pixel_t *shared_min = shared_data;
    pixel_t *shared_max = &shared_data[blockDim.x];
    
    int tid = threadIdx.x;
    int gid = blockIdx.x * blockDim.x + threadIdx.x;
    
    // Initialize shared memory
    shared_min[tid] = INT_MAX;
    shared_max[tid] = -INT_MAX;
    
    // Load data and find local min/max
    if (gid < size) {
        shared_min[tid] = in[gid];
        shared_max[tid] = in[gid];
    }
    
    // Process multiple elements per thread if needed
    for (int i = gid + blockDim.x * gridDim.x; i < size; i += blockDim.x * gridDim.x) {
        if (i < size) {
            shared_min[tid] = min(shared_min[tid], in[i]);
            shared_max[tid] = max(shared_max[tid], in[i]);
        }
    }
    
    __syncthreads();
    
    // Reduction in shared memory
    for (int s = blockDim.x / 2; s > 0; s >>= 1) {
        if (tid < s) {
            shared_min[tid] = min(shared_min[tid], shared_min[tid + s]);
            shared_max[tid] = max(shared_max[tid], shared_max[tid + s]);
        }
        __syncthreads();
    }
    
    // Write result
    if (tid == 0) {
        atomicMin(min_val, shared_min[0]);
        atomicMax(max_val, shared_max[0]);
    }
}

// Kernel to normalize image values
__global__ void normalize_kernel(pixel_t *inout, const int nx, const int ny, 
                                const int kn, const int min_val, const int max_val) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    const int khalf = kn / 2;
    
    if (x >= khalf && x < nx - khalf && y >= khalf && y < ny - khalf) {
        int idx = y * nx + x;
        float range = (float)(max_val - min_val);
        if (range > 0) {
            float normalized = MAX_BRIGHTNESS * ((float)inout[idx] - (float)min_val) / range;
            inout[idx] = (pixel_t)normalized;
        }
    }
}

void gaussian_filter_cuda(const pixel_t *in, pixel_t *out,
                         const int nx, const int ny, const float sigma) {
    const int n = 2 * (int)(2 * sigma) + 3;
    const float mean = (float)floor(n / 2.0);
    float *kernel = (float *)malloc(n * n * sizeof(float));
    
    fprintf(stderr, "gaussian_filter: kernel size %d, sigma=%g\n", n, sigma);
    
    // Generate Gaussian kernel
    size_t c = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            kernel[c] = exp(-0.5 * (pow((i - mean) / sigma, 2.0) +
                           pow((j - mean) / sigma, 2.0)))
                       / (2 * M_PI * sigma * sigma);
            c++;
        }
    }
    
    // Apply convolution
    convolution_cuda(in, out, kernel, nx, ny, n);
    
    // Find min and max values
    pixel_t *d_min, *d_max;
    cudaSafeCall(cudaMalloc(&d_min, sizeof(pixel_t)));
    cudaSafeCall(cudaMalloc(&d_max, sizeof(pixel_t)));
    
    pixel_t h_min = INT_MAX, h_max = -INT_MAX;
    cudaSafeCall(cudaMemcpy(d_min, &h_min, sizeof(pixel_t), cudaMemcpyHostToDevice));
    cudaSafeCall(cudaMemcpy(d_max, &h_max, sizeof(pixel_t), cudaMemcpyHostToDevice));
    
    // Allocate device memory for output if not already on device
    pixel_t *d_out;
    cudaSafeCall(cudaMalloc(&d_out, nx * ny * sizeof(pixel_t)));
    cudaSafeCall(cudaMemcpy(d_out, out, nx * ny * sizeof(pixel_t), cudaMemcpyHostToDevice));
    
    // Launch min_max kernel
    int blockSize = 256;
    int gridSize = (nx * ny + blockSize - 1) / blockSize;
    size_t sharedMemSize = 2 * blockSize * sizeof(pixel_t);
    
    min_max_kernel<<<gridSize, blockSize, sharedMemSize>>>(d_out, nx * ny, d_min, d_max);
    cudaDeviceSynchronize();
    cudaCheckMsg("min_max_kernel failed");
    
    // Get min and max values back
    cudaSafeCall(cudaMemcpy(&h_min, d_min, sizeof(pixel_t), cudaMemcpyDeviceToHost));
    cudaSafeCall(cudaMemcpy(&h_max, d_max, sizeof(pixel_t), cudaMemcpyDeviceToHost));
    
    // Launch normalize kernel
    dim3 blockSize2D(16, 16);
    dim3 gridSize2D((nx + blockSize2D.x - 1) / blockSize2D.x,
                    (ny + blockSize2D.y - 1) / blockSize2D.y);
    
    normalize_kernel<<<gridSize2D, blockSize2D>>>(d_out, nx, ny, n, h_min, h_max);
    cudaDeviceSynchronize();
    cudaCheckMsg("normalize_kernel failed");
    
    // Copy result back to host
    cudaSafeCall(cudaMemcpy(out, d_out, nx * ny * sizeof(pixel_t), cudaMemcpyDeviceToHost));
    
    // Clean up
    free(kernel);
    cudaFree(d_out);
    cudaFree(d_min);
    cudaFree(d_max);
}

// kernel para non-maximum suppression
__global__ void non_maximum_suppression_kernel(const pixel_t *after_Gx, const pixel_t *after_Gy, 
                                              const pixel_t *G, pixel_t *nms,
                                              const int nx, const int ny) {
    int i = blockIdx.x * blockDim.x + threadIdx.x + 1;  // começa em 1
    int j = blockIdx.y * blockDim.y + threadIdx.y + 1;  // começa em 1
    
    if (i < nx - 1 && j < ny - 1) {
        const int c = i + nx * j;
        const int nn = c - nx;  // north
        const int ss = c + nx;  // south
        const int ww = c + 1;   // west
        const int ee = c - 1;   // east
        const int nw = nn + 1;  // northwest
        const int ne = nn - 1;  // northeast
        const int sw = ss + 1;  // southwest
        const int se = ss - 1;  // southeast
        
        // calcula a direção do gradiente (0-7, mapeado para 0°, 45°, 90°, 135°)
        const float dir = (float)(fmod(atan2((float)after_Gy[c], (float)after_Gx[c]) + M_PI, M_PI) / M_PI) * 8;
        
        // verifica se é máximo local na direção do gradiente
        if (((dir <= 1 || dir > 7) && G[c] > G[ee] && G[c] > G[ww]) ||     // 0°
            ((dir > 1 && dir <= 3) && G[c] > G[nw] && G[c] > G[se]) ||     // 45°
            ((dir > 3 && dir <= 5) && G[c] > G[nn] && G[c] > G[ss]) ||     // 90°
            ((dir > 5 && dir <= 7) && G[c] > G[ne] && G[c] > G[sw])) {     // 135°
            nms[c] = G[c];
        } else {
            nms[c] = 0;
        }
    }
}

void non_maximum_suppression_cuda(const pixel_t *after_Gx, const pixel_t *after_Gy, 
                                 const pixel_t *G, pixel_t *nms,
                                 const int nx, const int ny) {
    const size_t size = nx * ny * sizeof(pixel_t);
    
    // aloca memória gpu
    pixel_t *d_after_Gx, *d_after_Gy, *d_G, *d_nms;
    cudaSafeCall(cudaMalloc(&d_after_Gx, size));
    cudaSafeCall(cudaMalloc(&d_after_Gy, size));
    cudaSafeCall(cudaMalloc(&d_G, size));
    cudaSafeCall(cudaMalloc(&d_nms, size));
    
    // copia dados para gpu
    cudaSafeCall(cudaMemcpy(d_after_Gx, after_Gx, size, cudaMemcpyHostToDevice));
    cudaSafeCall(cudaMemcpy(d_after_Gy, after_Gy, size, cudaMemcpyHostToDevice));
    cudaSafeCall(cudaMemcpy(d_G, G, size, cudaMemcpyHostToDevice));
    cudaSafeCall(cudaMemset(d_nms, 0, size));  // inicializa com zeros
    
    // configura grid e blocks
    dim3 blockSize(16, 16);
    dim3 gridSize((nx - 2 + blockSize.x - 1) / blockSize.x,  // -2 porque ignora bordas
                  (ny - 2 + blockSize.y - 1) / blockSize.y);
    
    // lança kernel
    non_maximum_suppression_kernel<<<gridSize, blockSize>>>(d_after_Gx, d_after_Gy, d_G, d_nms, nx, ny);
    cudaDeviceSynchronize();
    cudaCheckMsg("non_maximum_suppression_kernel failed");
    
    // copia resultado de volta
    cudaSafeCall(cudaMemcpy(nms, d_nms, size, cudaMemcpyDeviceToHost));
    
    // liberta memória
    cudaFree(d_after_Gx);
    cudaFree(d_after_Gy);
    cudaFree(d_G);
    cudaFree(d_nms);
}

// kernel para detetar edges fortes (acima de tmax)
__global__ void first_edges_kernel(const pixel_t *nms, pixel_t *edges,
                                  const int nx, const int ny, const int tmax) {
    int i = blockIdx.x * blockDim.x + threadIdx.x + 1;  // começa em 1
    int j = blockIdx.y * blockDim.y + threadIdx.y + 1;  // começa em 1
    
    if (i < nx - 1 && j < ny - 1) {
        int c = j * nx + i;
        
        if (nms[c] >= tmax) {
            edges[c] = MAX_BRIGHTNESS;  // marca como edge forte
        } else {
            edges[c] = 0;
        }
    }
}

void first_edges_cuda(const pixel_t *nms, pixel_t *edges,
                     const int nx, const int ny, const int tmax) {
    const size_t size = nx * ny * sizeof(pixel_t);
    
    // aloca memória gpu
    pixel_t *d_nms, *d_edges;
    cudaSafeCall(cudaMalloc(&d_nms, size));
    cudaSafeCall(cudaMalloc(&d_edges, size));
    
    // copia dados para gpu e inicializa edges com zeros
    cudaSafeCall(cudaMemcpy(d_nms, nms, size, cudaMemcpyHostToDevice));
    cudaSafeCall(cudaMemset(d_edges, 0, size));
    
    // configura grid e blocks
    dim3 blockSize(16, 16);
    dim3 gridSize((nx - 2 + blockSize.x - 1) / blockSize.x,
                  (ny - 2 + blockSize.y - 1) / blockSize.y);
    
    // lança kernel
    first_edges_kernel<<<gridSize, blockSize>>>(d_nms, d_edges, nx, ny, tmax);
    cudaDeviceSynchronize();
    cudaCheckMsg("first_edges_kernel failed");
    
    // copia resultado de volta
    cudaSafeCall(cudaMemcpy(edges, d_edges, size, cudaMemcpyDeviceToHost));
    
    // liberta memória
    cudaFree(d_nms);
    cudaFree(d_edges);
}

// kernel para hysteresis - conecta edges fracos a edges fortes
__global__ void hysteresis_edges_kernel(const pixel_t *nms, pixel_t *edges,
                                       const int nx, const int ny, const int tmin,
                                       int *changed) {
    int i = blockIdx.x * blockDim.x + threadIdx.x + 1;  // começa em 1
    int j = blockIdx.y * blockDim.y + threadIdx.y + 1;  // começa em 1
    
    if (i < nx - 1 && j < ny - 1) {
        int t = j * nx + i;
        
        // se é edge fraco (entre tmin e tmax) e ainda não foi marcado
        if (nms[t] >= tmin && edges[t] == 0) {
            // verifica os 8 vizinhos
            int nbs[8];
            nbs[0] = t - nx;     // north
            nbs[1] = t + nx;     // south
            nbs[2] = t + 1;      // west
            nbs[3] = t - 1;      // east
            nbs[4] = nbs[0] + 1; // northwest
            nbs[5] = nbs[0] - 1; // northeast
            nbs[6] = nbs[1] + 1; // southwest
            nbs[7] = nbs[1] - 1; // southeast
            
            // se algum vizinho é edge, marca este pixel também
            for (int k = 0; k < 8; k++) {
                if (edges[nbs[k]] != 0) {
                    edges[t] = MAX_BRIGHTNESS;
                    atomicAdd(changed, 1);  // indica que houve mudança
                    break;
                }
            }
        }
    }
}

void hysteresis_edges_cuda(const pixel_t *nms, pixel_t *edges,
                          const int nx, const int ny, const int tmin) {
    const size_t size = nx * ny * sizeof(pixel_t);
    
    // aloca memória gpu
    pixel_t *d_nms, *d_edges;
    int *d_changed;
    cudaSafeCall(cudaMalloc(&d_nms, size));
    cudaSafeCall(cudaMalloc(&d_edges, size));
    cudaSafeCall(cudaMalloc(&d_changed, sizeof(int)));
    
    // copia dados para gpu
    cudaSafeCall(cudaMemcpy(d_nms, nms, size, cudaMemcpyHostToDevice));
    cudaSafeCall(cudaMemcpy(d_edges, edges, size, cudaMemcpyHostToDevice));
    
    // configura grid e blocks
    dim3 blockSize(16, 16);
    dim3 gridSize((nx - 2 + blockSize.x - 1) / blockSize.x,
                  (ny - 2 + blockSize.y - 1) / blockSize.y);
    
    // repete até não haver mais mudanças (como no código cpu)
    int h_changed;
    do {
        h_changed = 0;
        cudaSafeCall(cudaMemcpy(d_changed, &h_changed, sizeof(int), cudaMemcpyHostToDevice));
        
        // lança kernel
        hysteresis_edges_kernel<<<gridSize, blockSize>>>(d_nms, d_edges, nx, ny, tmin, d_changed);
        cudaDeviceSynchronize();
        cudaCheckMsg("hysteresis_edges_kernel failed");
        
        // verifica se houve mudanças
        cudaSafeCall(cudaMemcpy(&h_changed, d_changed, sizeof(int), cudaMemcpyDeviceToHost));
    } while (h_changed > 0);
    
    // copia resultado de volta
    cudaSafeCall(cudaMemcpy(edges, d_edges, size, cudaMemcpyDeviceToHost));
    
    // liberta memória
    cudaFree(d_nms);
    cudaFree(d_edges);
    cudaFree(d_changed);
}

void cannyDevice(const int *h_idata, const int w, const int h,
                 const int tmin, const int tmax,
                 const float sigma,
                 int *h_odata) {
    const int nx = w;
    const int ny = h;

    pixel_t *after_Gx = (pixel_t *)calloc(nx * ny, sizeof(pixel_t));
    pixel_t *after_Gy = (pixel_t *)calloc(nx * ny, sizeof(pixel_t));
    pixel_t *filtered = (pixel_t *)calloc(nx * ny, sizeof(pixel_t));
    pixel_t *G = (pixel_t *)calloc(nx * ny, sizeof(pixel_t));
    pixel_t *nms = (pixel_t *)calloc(nx * ny, sizeof(pixel_t));

    if (after_Gx == NULL || after_Gy == NULL || filtered == NULL || 
        G == NULL || nms == NULL || h_odata == NULL) {
        fprintf(stderr, "cannyDevice: Failed memory allocation(s).\n");
        exit(1);
    }

    // passo 1: filtro gaussiano
    gaussian_filter_cuda(h_idata, filtered, nx, ny, sigma);

    // passo 2: gradientes sobel
    const float Gx[] = {-1, 0, 1,
                        -2, 0, 2,
                        -1, 0, 1};

    const float Gy[] = { 1,  2,  1,
                         0,  0,  0,
                        -1, -2, -1};

    convolution_cuda(filtered, after_Gx, Gx, nx, ny, 3);
    convolution_cuda(filtered, after_Gy, Gy, nx, ny, 3);

    // passo 3: magnitude do gradiente
    for (int i = 1; i < nx - 1; i++) {
        for (int j = 1; j < ny - 1; j++) {
            const int c = i + nx * j;
            G[c] = (pixel_t)(hypot((double)after_Gx[c], (double)after_Gy[c]));
        }
    }

    // passo 4: non-maximum suppression
    non_maximum_suppression_cuda(after_Gx, after_Gy, G, nms, nx, ny);

    // passo 5: first edges (edges fortes com threshold tmax)
    memset(h_odata, 0, nx * ny * sizeof(pixel_t));  // inicializa output com zeros
    first_edges_cuda(nms, h_odata, nx, ny, tmax);

    // passo 6: hysteresis (conecta edges fracos aos fortes)
    hysteresis_edges_cuda(nms, h_odata, nx, ny, tmin);

    // agora temos o resultado completo do canny edge detector

    free(after_Gx);
    free(after_Gy);
    free(filtered);
    free(G);
    free(nms);
}
