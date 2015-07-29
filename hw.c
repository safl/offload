#include <sys/mman.h>
#include <openacc.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

#if defined(_WIN32) || defined(_WIN64)
#include <sys/timeb.h>
#define gettime(a) _ftime(a)
#define usec(t1,t2) ((((t2).time-(t1).time)*1000+((t2).millitm-(t1).millitm))*100)
typedef struct _timeb timestruct;
#else
#include <sys/time.h>
#define gettime(a) gettimeofday(a,NULL)
#define usec(t1,t2) (((t2).tv_sec-(t1).tv_sec)*1000000+((t2).tv_usec-(t1).tv_usec))
typedef struct timeval timestruct;
#endif

#define acc_num_device_types 10

char* acc_device_type_text(acc_device_t device_type) {
    switch(device_type) {
        case acc_device_none:
            return "acc_device_none";
        case acc_device_default:
            return "acc_device_default";
        case acc_device_host:
            return "acc_device_host";
        case acc_device_not_host:
            return "acc_device_not_host";
        case acc_device_nvidia:
            return "acc_device_nvidia";
        case acc_device_radeon:
            return "acc_device_radeon";
        case acc_device_xeonphi:
            return "acc_device_xeonphi";
        case acc_device_pgi_opencl:
            return "acc_device_pgi_opencl";
        case acc_device_nvidia_opencl:
            return "acc_device_nvidia_opencl";
        case acc_device_opencl:
            return "acc_device_opencl";
        default:
            return "unknown";
    }
}

void acc_pprint_available_devices(void)
{
    acc_device_t device_types[acc_num_device_types] = {
        acc_device_none,
        acc_device_default,
        acc_device_host,
        acc_device_not_host,
        acc_device_nvidia,
        acc_device_radeon,
        acc_device_xeonphi,
        acc_device_pgi_opencl,
        acc_device_nvidia_opencl,
        acc_device_opencl
    };

    for(int i=0; i<acc_num_device_types; ++i) {
        acc_device_t device_type = device_types[i];
        int num_devices = acc_get_num_devices(device_type);
        printf("%d x %s\n", num_devices, acc_device_type_text(device_type));
    }
}

int main( int argc, char* argv[]) {

    int nelem = 100000;
    int iterations = 50000;
    int offload = 1;
    //int offload = 0;

    acc_pprint_available_devices();

    if (offload) {
        acc_init(acc_device_nvidia);
        acc_set_device_type(acc_device_nvidia);
    }

    double* restrict a = (double*)mmap(0, nelem*sizeof(double), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    double* restrict b = (double*)mmap(0, nelem*sizeof(double), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    double* restrict c = (double*)mmap(0, nelem*sizeof(double), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    #pragma omp parallel for
    for(int i=0; i<nelem; ++i) {
        a[i] = i;
        b[i] = i;
    }
    
    timestruct timer_started, timer_stopped;

    if (offload) {
        acc_copyin(a, nelem*sizeof(double));
        acc_copyin(b, nelem*sizeof(double));
        acc_copyin(c, nelem*sizeof(double));
    }
    gettime( &timer_started );
    for(int iteration=0; iteration<iterations; ++iteration) {
        #pragma acc parallel loop present(a, b, c) if(offload)
        for(int i=0; i<nelem; ++i) {
            c[i] = a[i] + b[i];
        }
    }
    if (offload) {
        acc_copyout(c, nelem*sizeof(double));
        acc_shutdown(acc_device_nvidia);
    }
    gettime( &timer_stopped );
    long long elapsed = usec(timer_started, timer_stopped);

    printf("HEj %f elapsed %ld\n", c[nelem/2], elapsed);

    return 0; 
}
