#include <inttypes.h>
void mic_alloc(double* data, size_t nelem, int dev, int offload);
void mic_push(double* data, size_t nelem, int dev, int offload);
void mic_push_alloc(double* data, size_t nelem, int dev, int offload);
void mic_pull(double* data, size_t nelem, int dev, int offload);
void mic_pull_free(double* data, size_t nelem, int dev, int offload);
int mic_get_max_threads(int dev, int offload);
