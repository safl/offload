#define _GNU_SOURCE
#include <sys/mman.h>
#include <stdio.h>
#undef _GNU_SOURCE
#include <stdlib.h>
#include <inttypes.h>
#include <dlfcn.h>
#include <omp.h>

#include <miclord.h>

int main (int argc, char **argv)
{
    int device = 0;
    int nelem = 50000000;
    int offload = argc > 1 ? atoi(argv[1]) : 1;

    void (*voodoo)(double*, double*, double*, int, int, int);
    void* handle;
    char* error;

    printf("Loading library..\n");

    handle = dlopen("/others/safl/snippets/libvoodoo.so", RTLD_LAZY);
    if (!handle) {
        fputs (dlerror(), stderr);
        exit(1);
    }

    printf("Loading voodoo..\n");
    voodoo = dlsym(handle, "voodoo");
    if ((error = dlerror()) != NULL)  {
        fputs(error, stderr);
        exit(1);
    }
    mic_get_max_threads(device, offload);   // Initialize device
    printf("Started...\n");
                                            // Allocate memory
    double* a = (double*)mmap(0, nelem*sizeof(double), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    double* b = (double*)mmap(0, nelem*sizeof(double), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    double* c = (double*)mmap(0, nelem*sizeof(double), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    
    for(int i=0; i<nelem; ++i) {            // Initialize it
        a[i] = i;
        b[i] = i;
        //c[i] = i;
    }   

    mic_alloc(a, nelem, device, offload);   // Allocate buffers on device
    mic_alloc(b, nelem, device, offload);
    mic_alloc(c, nelem, device, offload);

    mic_push(a, nelem, device, offload);    // Copy inputs to device
    mic_push(a, nelem, device, offload);

    for (int i=0; i<10; ++i) {               // Do some computation
        voodoo(a, b, c, nelem, device, offload);
    }

    mic_pull_free(c, nelem, device, offload);    // Copy result back and free on device
    printf("c[nelem/2] = %f\n", c[nelem/2]);
    printf("Going down\n");

    munmap(a, nelem*sizeof(double));        // deallocate memory
    munmap(b, nelem*sizeof(double));
    munmap(c, nelem*sizeof(double));

    return 0;
}
