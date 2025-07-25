
// Based on CUDA SDK template from NVIDIA

// includes, system
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <assert.h>
#include <float.h>

// utilities for safe cuda api calls copied from cuda sdk.

#define cudaSafeCall(err)           __cudaSafeCall      (err, __FILE__, __LINE__)
#define cudaCheckMsg(msg)           __cudaGetLastError  (msg, __FILE__, __LINE__)

inline void __cudaSafeCall(cudaError_t err, const char *file, const int line)
{
      if (cudaSuccess != err) {
    fprintf(stderr, "%s(%i) : cudaSafeCall() Runtime API error %d: %s.\n",
                  file, line, (int)err, cudaGetErrorString(err));
          exit(-1);
      }
}

inline void __cudaGetLastError( const char *errorMessage, const char *file, const int line )
{
    cudaError_t err = cudaGetLastError();
    if( cudaSuccess != err) {
        fprintf(stderr, "%s(%i) : cutilCheckMsg() CUTIL CUDA error : %s : (%d) %s.\n",
                file, line, errorMessage, (int)err, cudaGetErrorString( err ));
        exit(-1);
    }
}

#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((a)<(b))?(a):(b))

#define MAX_BRIGHTNESS 255

// Use int instead `unsigned char' so that we can
// store negative values.
typedef int pixel_t;

// include image functions
#include "image.c"

// convolution of in image to out image using kernel of kn width
void convolution(const pixel_t *in, pixel_t *out, const float *kernel,
                 const int nx, const int ny, const int kn)
{
    assert(kn % 2 == 1);    
    assert(nx > kn && ny > kn);
    const int khalf = kn / 2;

    for (int m = khalf; m < nx - khalf; m++)
        for (int n = khalf; n < ny - khalf; n++) {
            float pixel = 0.0;
            size_t c = 0;
            for (int j = -khalf; j <= khalf; j++)
                for (int i = -khalf; i <= khalf; i++) {
                    pixel += in[(n - j) * nx + m - i] * kernel[c];
                    c++;
                }

            out[n * nx + m] = (pixel_t)pixel;
        }
}

// determines min and max of in image
void min_max(const pixel_t *in, const int nx, const int ny, pixel_t *pmin, pixel_t *pmax)
{
    int min = INT_MAX, max = -INT_MAX;

    for (int m = 0; m < nx; m++)
        for (int n = 0; n < ny ; n++) {
            int pixel = in[n*nx + m];
            if (pixel < min)
                min = pixel;
            if (pixel > max)
                max = pixel;
        }
    *pmin = min; *pmax = max;
}

// normalizes inout image using min and max values
void normalize(  pixel_t *inout,
               const int nx, const int ny, const int kn,
               const int min, const int max)
{
    const int khalf = kn / 2;

    for (int m = khalf; m < nx - khalf; m++)
        for (int n = khalf; n < ny - khalf; n++) {

            pixel_t pixel = MAX_BRIGHTNESS * ((int)inout[n * nx + m] -(float) min) / ((float)max - (float)min);
            inout[n * nx + m] = pixel;
        }
}


/*
 * gaussianFilter:
 * http://www.songho.ca/dsp/cannyedge/cannyedge.html
 * determine size of kernel (odd #)
 * 0.0 <= sigma < 0.5 : 3
 * 0.5 <= sigma < 1.0 : 5
 * 1.0 <= sigma < 1.5 : 7
 * 1.5 <= sigma < 2.0 : 9
 * 2.0 <= sigma < 2.5 : 11
 * 2.5 <= sigma < 3.0 : 13 ...
 * kernelSize = 2 * int(2*sigma) + 3;
 */
void gaussian_filter(const pixel_t *in, pixel_t *out,
                     const int nx, const int ny, const float sigma)
{
    const int n = 2 * (int)(2 * sigma) + 3;
    const float mean = (float)floor(n / 2.0);
    float kernel[n * n]; // variable length array

    fprintf(stderr, "gaussian_filter: kernel size %d, sigma=%g\n",
            n, sigma);
    size_t c = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            kernel[c] = exp(-0.5 * (pow((i - mean) / sigma, 2.0) +
                            pow((j - mean) / sigma, 2.0)))
                / (2 * M_PI * sigma * sigma);
            c++;
        }

    convolution(in, out, kernel, nx, ny, n);
    pixel_t max, min;
    min_max(out, nx, ny, &min, &max);
    normalize(out, nx, ny, n, min, max);
}

// Canny non-maximum suppression
void non_maximum_supression(const pixel_t *after_Gx, const pixel_t * after_Gy, const pixel_t *G, pixel_t *nms,
                            const int nx, const int ny)
{
    for (int i = 1; i < nx - 1; i++)
        for (int j = 1; j < ny - 1; j++) {
            const int c = i + nx * j;
            const int nn = c - nx;
            const int ss = c + nx;
            const int ww = c + 1;
            const int ee = c - 1;
            const int nw = nn + 1;
            const int ne = nn - 1;
            const int sw = ss + 1;
            const int se = ss - 1;

            const float dir = (float)(fmod(atan2(after_Gy[c],
                                                 after_Gx[c]) + M_PI,
                                           M_PI) / M_PI) * 8;

            if (((dir <= 1 || dir > 7) && G[c] > G[ee] &&
                G[c] > G[ww]) || // 0 deg
                ((dir > 1 && dir <= 3) && G[c] > G[nw] &&
                G[c] > G[se]) || // 45 deg
                ((dir > 3 && dir <= 5) && G[c] > G[nn] &&
                G[c] > G[ss]) || // 90 deg
                ((dir > 5 && dir <= 7) && G[c] > G[ne] &&
                G[c] > G[sw]))   // 135 deg
                nms[c] = G[c];
            else
                nms[c] = 0;
        }
}

// edges found in first pass for nms > tmax
void first_edges(const pixel_t *nms, pixel_t *reference,
                 const int nx, const int ny, const int tmax)
{

    size_t c = 1;
    for (int j = 1; j < ny - 1; j++) {
        for (int i = 1; i < nx - 1; i++) {
            if (nms[c] >= tmax) { // trace edges
                reference[c] = MAX_BRIGHTNESS;
            }
            c++;
        }
        c+=2; // because borders are not considered
    }
}


// edges found in after first passes for nms > tmin && neighbor is edge
void hysteresis_edges(const pixel_t *nms, pixel_t *reference,
                      const int nx, const int ny, const int tmin, bool *pchanged)
{
    // Tracing edges with hysteresis . Non-recursive implementation.
    for (int i = 1; i < nx - 1; i++) {
        for (int j = 1; j < ny - 1; j++) {
            size_t t = i + j * nx;

            int nbs[8]; // neighbours
            nbs[0] = t - nx;     // nn
            nbs[1] = t + nx;     // ss
            nbs[2] = t + 1;      // ww
            nbs[3] = t - 1;      // ee
            nbs[4] = nbs[0] + 1; // nw
            nbs[5] = nbs[0] - 1; // ne
            nbs[6] = nbs[1] + 1; // sw
            nbs[7] = nbs[1] - 1; // se

            if (nms[t] >= tmin && reference[t] == 0) {
                for(int k = 0; k < 8; k++)
                    if (reference[nbs[k]] != 0) {
                        reference[t] = MAX_BRIGHTNESS;
                        *pchanged = true;
                    }
            }
        }
    }
}

/*
 * Links:
 * http://en.wikipedia.org/wiki/Canny_edge_detector
 * http://www.tomgibara.com/computer-vision/CannyEdgeDetector.java
 * http://fourier.eng.hmc.edu/e161/lectures/canny/node1.html
 * http://www.songho.ca/dsp/cannyedge/cannyedge.html
 *
 * Note: T1 and T2 are lower and upper thresholds.
 */

//canny edge detector code to run on the host
void cannyHost( const int *h_idata, const int w, const int h,
               const int tmin,            // tmin canny parameter
               const int tmax,            // tmax canny parameter
               const float sigma,         // sigma canny parameter
               int * reference)
{
    const int nx = w;
    const int ny = h;

    pixel_t *G        = (pixel_t *) calloc(nx * ny, sizeof(pixel_t));
    pixel_t *after_Gx = (pixel_t *) calloc(nx * ny, sizeof(pixel_t));
    pixel_t *after_Gy = (pixel_t *) calloc(nx * ny, sizeof(pixel_t));
    pixel_t *nms      = (pixel_t *) calloc(nx * ny, sizeof(pixel_t));

    if (G == NULL || after_Gx == NULL || after_Gy == NULL ||
        nms == NULL || reference == NULL) {
        fprintf(stderr, "canny_edge_detection:"
                " Failed memory allocation(s).\n");
        exit(1);
    }

    // Gaussian filter
    gaussian_filter(h_idata, reference, nx, ny, sigma);

    const float Gx[] = {-1, 0, 1,
        -2, 0, 2,
        -1, 0, 1};

    // Gradient along x
    convolution(reference, after_Gx, Gx, nx, ny, 3);

    const float Gy[] = { 1, 2, 1,
        0, 0, 0,
        -1,-2,-1};

    // Gradient along y
    convolution(reference, after_Gy, Gy, nx, ny, 3);

    // Merging gradients
    for (int i = 1; i < nx - 1; i++)
        for (int j = 1; j < ny - 1; j++) {
            const int c = i + nx * j;
            G[c] = (pixel_t)(hypot((double)(after_Gx[c]), (double)( after_Gy[c]) ));
        }

    // Non-maximum suppression, straightforward implementation.
    non_maximum_supression(after_Gx, after_Gy, G, nms, nx, ny);

    // edges with nms >= tmax
    memset(reference, 0, sizeof(pixel_t) * nx * ny);
    first_edges(nms, reference, nx, ny, tmax);

    // edges with nms >= tmin && neighbor is edge
    bool changed;
    do {
        changed = false;
        hysteresis_edges(nms, reference, nx, ny, tmin, &changed);
    } while (changed==true);

    free(after_Gx);
    free(after_Gy);
    free(G);
    free(nms);
}

//==============================
// Your code goes into this file
#include "canny-device.cu"
//==============================

// print command line format
void usage(char *command)
{
    printf("Usage: %s [-h] [-d device] [-i inputfile] [-o outputfile] [-r referenceFile] [-s sigma] [-t threshold]\n",command);
}

// main
int main( int argc, char** argv)
{
    // default command line options
    int deviceId = 0;
    char *fileIn=(char *)"images/lake.pgm",*fileOut=(char *)"out.pgm",*referenceOut=(char *)"reference.pgm";
    int tmin = 45, tmax = 50;
    float sigma=1.0f;

    // parse command line arguments
    int opt;
    while( (opt = getopt(argc,argv,"d:i:o:r:n:x:s:h")) !=-1)
    {
        switch(opt)
        {

            case 'd':  // device
                if(sscanf(optarg,"%d",&deviceId)!=1)
                {
                    usage(argv[0]);
                    exit(1);
                }
                break;

            case 'i': // input image filename
                if(strlen(optarg)==0)
                {
                    usage(argv[0]);
                    exit(1);
                }

                fileIn = strdup(optarg);
                break;
            case 'o': // output image (from device) filename
                if(strlen(optarg)==0)
                {
                    usage(argv[0]);
                    exit(1);
                }
                fileOut = strdup(optarg);
                break;
            case 'r': // output image (from host) filename
                if(strlen(optarg)==0)
                {
                    usage(argv[0]);
                    exit(1);
                }
                referenceOut = strdup(optarg);
                break;
            case 'n': // tmin
                if(strlen(optarg)==0 || sscanf(optarg,"%d",&tmin)!=1)
                {
                    usage(argv[0]);
                    exit(1);
                }
                break;
            case 'x': // tmax
                if(strlen(optarg)==0 || sscanf(optarg,"%d",&tmax)!=1)
                {
                    usage(argv[0]);
                    exit(1);
                }
                break;
            case 's': // sigma
                if(strlen(optarg)==0 || sscanf(optarg,"%f",&sigma)!=1)
                {
                    usage(argv[0]);
                    exit(1);
                }
                break;
            case 'h': // help
                usage(argv[0]);
                exit(0);
                break;

        }
    }

    // Banner
    int deviceCount = 0;
    cudaSafeCall( cudaGetDeviceCount(&deviceCount));

    if (deviceCount == 0){
        fprintf(stderr, "No CUDA-capable device found.");
        return 0;
    }

    printf("Available devices\n");
    for (int i = 0; i < deviceCount; ++i){
        cudaDeviceProp deviceProp;
        cudaSafeCall(cudaGetDeviceProperties(&deviceProp, i));

        if (i == deviceId) printf("->");
        else printf("  ");

        printf("%d: %s (compute %d.%d)\n", i, deviceProp.name, deviceProp.major, deviceProp.minor);
    }
    printf("\n");

    // select cuda device
    cudaSafeCall( cudaSetDevice( deviceId ) );

    // create events to measure host canny detector time and device canny detector time
    cudaEvent_t startH, stopH, startD, stopD;
    cudaEventCreate(&startH);
    cudaEventCreate(&stopH);
    cudaEventCreate(&startD);
    cudaEventCreate(&stopD);

    // allocate host memory
    int* h_idata=NULL;
    unsigned int h,w;

    //load pgm
    if (loadPGM(fileIn, (uint32_t**)&h_idata, &w, &h) != 1) {
        exit(1);
    }

    // allocate mem for the result on host side
    int* h_odata = (int*) calloc( h*w, sizeof(unsigned int));
    int* reference = (int*) calloc( h*w, sizeof(unsigned int));

    // detect edges at host
    cudaEventRecord( startH, 0 );
    cannyHost(h_idata, w, h, tmin, tmax, sigma, reference);
    cudaEventRecord( stopH, 0 );
    cudaEventSynchronize( stopH );

    // detect edges at GPU
    cudaEventRecord( startD, 0 );
    cannyDevice(h_idata, w, h, tmin, tmax, sigma, h_odata);
    cudaEventRecord( stopD, 0 );
    cudaEventSynchronize( stopD );

    // check if kernel execution generated and error
    cudaCheckMsg("Kernel execution failed");

    float timeH, timeD;
    cudaEventElapsedTime( &timeH, startH, stopH );
    printf( "Host processing time: %f (ms)\n", timeH);
    cudaEventElapsedTime( &timeD, startD, stopD );
    printf( "Device processing time: %f (ms)\n", timeD);

    // save output images
    if (savePGM(referenceOut, (unsigned int *)reference, w, h) != 1) {
        exit(1);
    }

    if (savePGM(fileOut,(unsigned int *) h_odata, w, h) != 1) {
        exit(1);
    }

    int hist = 0;
    for (int i = 0; i < w * h; ++i){
        int delta = abs(reference[i] - h_odata[i]);
        if (delta != 0)
            hist++;
    }
    printf("\nNumber of different pixels: %d/%d (%.2f%%)\n", hist, w*h, hist/(float)(w*h) * 100.0f);

    // cleanup memory
    if (h_idata != nullptr)
        free( h_idata);

    free( h_odata);
    free( reference);

    cudaDeviceReset();
}
