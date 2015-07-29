#include <sys/mman.h>
#include <openacc.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

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

    acc_pprint_available_devices();
    acc_init(acc_device_nvidia);
    acc_set_device_type(acc_device_nvidia);

    int nelem = 100;
    double* restrict a = (double*)mmap(0, nelem*sizeof(double), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    double* restrict b = (double*)mmap(0, nelem*sizeof(double), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    double* restrict c = (double*)mmap(0, nelem*sizeof(double), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    acc_create(a, nelem);
    acc_create(b, nelem);
    acc_create(c, nelem);

    #pragma acc parallel loop present(a, b, c)
    for(int i=0; i<nelem; ++i) {
        a[i] = b[i] + c[i];
    }

    acc_shutdown(acc_device_nvidia);

    return 0; 
}
