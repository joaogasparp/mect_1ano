# Projeto 3 - Canny Edge Detector with CUDA Programming

## Resultados da Implementação

### Performance
**Tempos de execução obtidos:**
- Processamento em GPU: entre 1.69ms e 1.79ms (média ~1.74ms)


| Imagem | Pixels diferentes | Percentagem |
|--------|------------------|-------------|
| house.pgm | 2992 | 1.14% |
| jetplane.pgm | 9343 | 3.56% |
| lake.pgm | 14477 | 5.52% |
| livingroom.pgm | 7987 | 3.05% |
| mandril.pgm | 28464 | 10.86% |
| peppers_gray.pgm | 7937 | 3.03% |
| pirate.pgm | 14370 | 5.48% |
| walkbridge.pgm | 20893 | 7.97% |

## Implementação das Tasks

### Task 1 - Kernel para convolution() e gradientes verticais/horizontais

A função `convolution()` original aplica um kernel a uma imagem. Foi desenvolvido um kernel CUDA genérico para convolução:

```c
// canny-device.cu
__global__ void convolution_kernel_cuda(const pixel_t *in, pixel_t *out,
                                        const int nx, const int ny, const int kn) {
    int m = blockIdx.x * blockDim.x + threadIdx.x;
    int n = blockIdx.y * blockDim.y + threadIdx.y;
    
    const int khalf = kn / 2;
    
    if (m >= khalf && m < nx - khalf && n >= khalf && n < ny - khalf) {
        float pixel = 0.0f;
        size_t c = 0;
        
        for (int j = -khalf; j <= khalf; j++) {
            for (int i = -khalf; i <= khalf; i++) {
                pixel += in[(n - j) * nx + m - i] * d_kernel[c];
                c++;
            }
        }
        
        out[n * nx + m] = (pixel_t)pixel;
    }
}
```

Para os gradientes Sobel, foi criado um kernel otimizado que calcula ambos numa única passagem:

```c
// canny-device.cu
__global__ void sobel_gradient_kernel(const pixel_t *in, pixel_t *Gx, pixel_t *Gy,
                                     const int nx, const int ny) {
    extern __shared__ pixel_t s_tile[];
    
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    // load ao tile com bordas
    // ...
    
    if (x > 0 && x < nx - 1 && y > 0 && y < ny - 1) {
        float px = 0.0f, py = 0.0f;
        
        // aplicar filtros Sobel
        for (int j = 0; j < 3; j++) {
            for (int i = 0; i < 3; i++) {
                float val = s_tile[ly * tile_size + lx];
                px += val * d_gx[j * 3 + i];
                py += val * d_gy[j * 3 + i];
            }
        }
        
        Gx[y * nx + x] = (pixel_t)px;
        Gy[y * nx + x] = (pixel_t)py;
    }
}
```

Modificação em `cannyDevice()`:
```c
// substituir as duas chamadas de convolution por:
sobel_gradient_kernel<<<gridSize, blockSize, sharedMemSize>>>(d_gaussian, d_Gx, d_Gy, nx, ny);
```

### Task 2 - Kernel para non_maximum_suppression()

O NMS verifica se cada pixel é máximo local na direção do gradiente. Foi desenvolvido:

```c
// canny-device.cu
__global__ void nms_shared_kernel(const pixel_t *Gx, const pixel_t *Gy, const pixel_t *G, 
                                  pixel_t *nms, const int nx, const int ny) {
    extern __shared__ pixel_t s_data[];
    
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    // dividir shared memory para 3 arrays
    const int s_width = blockDim.x + 2;
    pixel_t *s_G = s_data;
    pixel_t *s_Gx = &s_data[s_width * (blockDim.y + 2)];
    pixel_t *s_Gy = &s_data[2 * s_width * (blockDim.y + 2)];
    
    // dar load aos dados...
    __syncthreads();
    
    if (x > 0 && x < nx - 1 && y > 0 && y < ny - 1) {
        int s_idx = (threadIdx.y + 1) * s_width + (threadIdx.x + 1);
        
        // cálculo da direção (igual ao professor)
        const float dir = (float)(fmod(atan2((double)s_Gy[s_idx], 
                                            (double)s_Gx[s_idx]) + M_PI, M_PI) / M_PI) * 8;
        
        pixel_t curr = s_G[s_idx];
        bool keep = false;
        
        // verificar vizinhos conforme direção
        if ((dir <= 1 || dir > 7) && curr > s_G[s_idx - 1] && curr > s_G[s_idx + 1]) {
            keep = true;  // 0 graus
        } else if (dir > 1 && dir <= 3 && curr > s_G[s_idx - s_width - 1] && 
                   curr > s_G[s_idx + s_width + 1]) {
            keep = true;  // 45 graus  
        } else if (dir > 3 && dir <= 5 && curr > s_G[s_idx - s_width] && 
                   curr > s_G[s_idx + s_width]) {
            keep = true;  // 90 graus
        } else if (dir > 5 && dir <= 7 && curr > s_G[s_idx - s_width + 1] && 
                   curr > s_G[s_idx + s_width - 1]) {
            keep = true;  // 135 graus
        }
        
        nms[y * nx + x] = keep ? curr : 0;
    }
}
```

Modificação em `cannyDevice()`:
```c
// calcular magnitude primeiro
gradient_magnitude_kernel<<<gridSize, blockSize>>>(d_Gx, d_Gy, d_G, nx, ny);

// depois aplicar Non Maximum Ssuppression
size_t sharedMemSize = 3 * (BLOCK_SIZE + 2) * (BLOCK_SIZE + 2) * sizeof(pixel_t);
nms_shared_kernel<<<gridSize, blockSize, sharedMemSize>>>(d_Gx, d_Gy, d_G, d_nms, nx, ny);
```

### Task 3 - Kernel para first_edges()

Este kernel marca pixels com valor >= tmax como edges definitivos:

```c
// canny-device.cu
__global__ void first_edges_kernel(const pixel_t *nms, pixel_t *edges,
                                  const int nx, const int ny, const int tmax) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x >= 1 && x < nx - 1 && y >= 1 && y < ny - 1) {
        int idx = y * nx + x;
        edges[idx] = (nms[idx] >= tmax) ? MAX_BRIGHTNESS : 0;
    }
}
```

Modificação em `cannyDevice()`:
```c
// inicializar edges com zeros
cudaMemset(d_edges, 0, size);

// marcar edges fortes
first_edges_kernel<<<gridSize, blockSize>>>(d_nms, d_edges, nx, ny, tmax);
```

### Task 4 - Kernel para hysteresis_edges()

Este kernel conecta edges fracos (>= tmin) a edges fortes através dos vizinhos:

```c
// canny-device.cu
__global__ void hysteresis_pass(const pixel_t *nms, pixel_t *edges,
                               const int nx, const int ny, const int tmin, int *changed) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int j = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (i >= 1 && i < nx - 1 && j >= 1 && j < ny - 1) {
        int t = j * nx + i;
        
        if (edges[t] == 0 && nms[t] >= tmin) {
            // neighbours (igual ao professor)
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
```

Modificação em `cannyDevice()`:
```c
// alocar contador de mudanças
int *d_changed;
cudaMalloc(&d_changed, sizeof(int));

// executar hysteresis iterativamente
int h_changed;
do {
    h_changed = 0;
    cudaMemcpy(d_changed, &h_changed, sizeof(int), cudaMemcpyHostToDevice);
    
    hysteresis_pass<<<gridSize, blockSize>>>(d_nms, d_edges, nx, ny, tmin, d_changed);
    cudaDeviceSynchronize();
    
    cudaMemcpy(&h_changed, d_changed, sizeof(int), cudaMemcpyDeviceToHost);
} while (h_changed > 0);
```

### Task 5 - Kernels para gaussian_filter()

**Kernel de convolução com shared memory**:
```c
// canny-device.cu
__global__ void convolution_kernel_shared(const pixel_t *in, pixel_t *out,
                                         const int nx, const int ny, const int kn) {
    extern __shared__ pixel_t tile[];
    
    const int khalf = kn / 2;
    const int tile_size = blockDim.x + 2 * khalf;
    
    // load cooperativo do tile
    for (int dy = threadIdx.y; dy < tile_size; dy += blockDim.y) {
        for (int dx = threadIdx.x; dx < tile_size; dx += blockDim.x) {
            int gx = blockIdx.x * blockDim.x + dx - khalf;
            int gy = blockIdx.y * blockDim.y + dy - khalf;
            
            if (gx >= 0 && gx < nx && gy >= 0 && gy < ny) {
                tile[dy * tile_size + dx] = in[gy * nx + gx];
            } else {
                tile[dy * tile_size + dx] = 0;
            }
        }
    }
    
    __syncthreads();
    
    // aplicar convolução
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x >= khalf && x < nx - khalf && y >= khalf && y < ny - khalf) {
        float pixel = 0.0f;
        
        for (int j = 0; j < kn; j++) {
            for (int i = 0; i < kn; i++) {
                pixel += tile[(threadIdx.y + j) * tile_size + (threadIdx.x + i)] * 
                         d_kernel[j * kn + i];
            }
        }
        
        out[y * nx + x] = (pixel_t)pixel;
    }
}
```

**Kernel para encontrar min/max**:
```c
// canny-device.cu
__global__ void min_max_kernel(const pixel_t *in, const int size, 
                               pixel_t *min_val, pixel_t *max_val) {
    extern __shared__ pixel_t shared_data[];
    pixel_t *shared_min = shared_data;
    pixel_t *shared_max = &shared_data[blockDim.x];
    
    int tid = threadIdx.x;
    int gid = blockIdx.x * blockDim.x + threadIdx.x;
    
    // inicializar e carregar
    shared_min[tid] = (gid < size) ? in[gid] : INT_MAX;
    shared_max[tid] = (gid < size) ? in[gid] : -INT_MAX;
    
    // processar múltiplos elementos
    for (int i = gid + blockDim.x * gridDim.x; i < size; i += blockDim.x * gridDim.x) {
        shared_min[tid] = min(shared_min[tid], in[i]);
        shared_max[tid] = max(shared_max[tid], in[i]);
    }
    
    __syncthreads();
    
    // redução
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
```

**Kernel para normalizar**:
```c
// canny-device.cu
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
```

Modificação em `cannyDevice()`:
```c
// gerar kernel gaussiano (igual ao professor)
const int n = 2 * (int)(2 * sigma) + 3;
float *kernel = (float *)malloc(n * n * sizeof(float));
// ... preencher kernel ...

// copiar para constant memory
cudaMemcpyToSymbol(d_kernel, kernel, n * n * sizeof(float));

// aplicar convolução
size_t sharedMemSize = (BLOCK_SIZE + 2 * (n/2)) * (BLOCK_SIZE + 2 * (n/2)) * sizeof(pixel_t);
convolution_kernel_shared<<<gridSize, blockSize, sharedMemSize>>>(d_input, d_gaussian, nx, ny, n);

// encontrar min/max
min_max_kernel<<<gridSize1D, blockSize1D, sharedMem1D>>>(d_gaussian, nx * ny, d_min, d_max);

// normalizar
normalize_kernel<<<gridSize, blockSize>>>(d_gaussian, nx, ny, n, h_min, h_max);
```

## Instruções de Compilação e Execução

### Compilação
```bash
make clean
make
```

### Testar Individualmente
```bash
./canny -i <imagem.pgm> -s <sigma> -n <tmin> -x <tmax>
```

**Parâmetros:**
- `-i`: imagem PGM de entrada
- `-s`: sigma do filtro gaussiano (default: 1.0)
- `-n`: threshold mínimo (default: 45)
- `-x`: threshold máximo (default: 50)

### Testar para todas
```bash
./test_all_images.sh
```
