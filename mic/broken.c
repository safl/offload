#define _GNU_SOURCE
#include <sys/mman.h>
#include <stdio.h>
#undef _GNU_SOURCE
#include <stdlib.h>
#include <inttypes.h>
#include <dlfcn.h>
#include <omp.h>
#include <miclord.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <omp.h>
#include <math.h>

//
// Operations are split out to make them easier to spot with OFFLOAD_REPORT=3
//

// Just some random function doing some work...
void voodoo(double* a, double* b, double* c, int nelem, int device, int offload)
{
    int mthreads;
    #pragma offload target(mic:device) if(offload) 
    {
        mthreads = omp_get_max_threads();
    }
    const int64_t nworkers = nelem > mthreads ? mthreads : 1;
    const int64_t work_split= nelem / nworkers;
    const int64_t work_spill= nelem % nworkers;

    #pragma offload                                     \
        target(mic:device)                              \
        in(a:length(0) alloc_if(0) free_if(0)),         \
        in(b:length(0) alloc_if(0) free_if(0)),         \
        in(c:length(0) alloc_if(0) free_if(0))          \
        in(work_split)                                  \
        in(work_spill)                                  \
        if(offload)
    {
        #pragma omp parallel num_threads(nworkers)
        {
            for(int i=0; i<100; ++i) {
                const int tid = omp_get_thread_num();

                int64_t work=0, work_offset=0, work_end=0;
                if (tid < work_spill) {
                    work = work_split + 1;
                    work_offset = tid * work;
                } else {
                    work = work_split;
                    work_offset = tid * work + work_spill;
                }
                work_end = work_offset + work;

                if (work) {
                    for (int eidx=work_offset; eidx<work_end; ++eidx) {
                        c[eidx] = ((a[eidx] + b[eidx]) / 100.0);
                    }
                }
            }
        }
    }
}

// Used for "initializing" the MIC
int mic_get_max_threads(int device, int offload)
{
    int mthreads;
    #pragma offload target(mic:device)  \
        out(mthreads)                   \
        if (offload)
    {
        mthreads = omp_get_max_threads();
    }
    return mthreads;
}

// Used for managing memory on the MIC
void mic_alloc(double* data, size_t nelem, int device, int offload)
{
    #pragma offload_transfer                                    \
            target(mic:device)                                  \
            nocopy(data:length(nelem) alloc_if(1) free_if(0))   \
            if (offload)
}

void mic_push(double* data, size_t nelem, int device, int offload)
{
    #pragma offload_transfer                                    \
            target(mic:device)                                  \
            in(data:length(nelem) alloc_if(0) free_if(0))       \
            if (offload)
}

void mic_push_alloc(double* data, size_t nelem, int device, int offload)
{
    #pragma offload_transfer                                    \
            target(mic:device)                                  \
            in(data:length(nelem) alloc_if(1) free_if(0))       \
            if (offload)
}

void mic_pull(double* data, size_t nelem, int device, int offload)
{
    #pragma offload_transfer                                    \
            target(mic:device)                                  \
            out(data:length(nelem) alloc_if(0) free_if(0))      \
            if (offload)
}

void mic_pull_free(double* data, size_t nelem, int device, int offload)
{
    #pragma offload_transfer                                    \
            target(mic:device)                                  \
            out(data:length(nelem) alloc_if(0) free_if(1))      \
            if (offload)
}

int main (int argc, char **argv)
{
    int device = 0;                             // The mic deice
    int iter = 10;
    int nelem = 50000000;                       // number of elements in the arrays
    int offload = argc > 1 ? atoi(argv[1]) : 1; // Control offloading

    void* handle;
    char* error;

    printf("Initializing device(%d)... offload(%d)?\n", device, offload);
    mic_get_max_threads(device, offload);       // Initialize device

    printf("Allocating on host...\n");
                                                // Allocate memory on host
    double* a = (double*)mmap(0, nelem*sizeof(double), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    double* b = (double*)mmap(0, nelem*sizeof(double), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    double* c = (double*)mmap(0, nelem*sizeof(double), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    
    printf("Initializing on host...\n");
    for(int i=0; i<nelem; ++i) {                // Initialize it
        a[i] = i;
        b[i] = i;
        //c[i] = i;
    }

    printf("Allocating on device...\n");
    mic_alloc(a, nelem, device, offload);       // Allocate buffers on device
    mic_alloc(b, nelem, device, offload);
    mic_alloc(c, nelem, device, offload);

    printf("Copying to device...\n");
    mic_push(a, nelem, device, offload);        // Copy input-buffers to device
    mic_push(a, nelem, device, offload);

    printf("Doing some work on device...\n");
    for (int i=0; i<iter; ++i) {                // Call the 
        voodoo(a, b, c, nelem, device, offload);
    }
    
    printf("Retrieving and deallocating data device...\n");
    mic_pull_free(c, nelem, device, offload);   // Copy result back and free on device

    printf("Bla bla...\n");
    printf("c[nelem/2] = %f\n", c[nelem/2]);

    printf("Deallocating on host...");

    munmap(a, nelem*sizeof(double));            // Deallocate memory on host
    munmap(b, nelem*sizeof(double));
    munmap(c, nelem*sizeof(double));

    printf("Bye!");

    return 0;
}