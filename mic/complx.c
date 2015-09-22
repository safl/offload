#include <inttypes.h>
#include <complex.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

int main (int argc, char **argv)
{
    int nelem = 5;

    float complex* data = (float complex*)malloc(sizeof(float complex)*nelem);    
    
    for(int i=0; i<nelem; ++i) {
        data[i] = i;
    }

    #pragma offload target(mic) in(data:length(nelem))
    for(int i=0; i<nelem; ++i) {
        printf("r=%f, i=%f\n", creal(data[i]), cimag(data[i]));
    }

    return 0;
}
