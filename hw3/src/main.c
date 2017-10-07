#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    double* ptr = sf_malloc(sizeof(long double));

    void *y = sf_malloc(sizeof(char));

    sf_varprint(ptr);

    sf_varprint(y);

    sf_snapshot();

    *ptr = 320320320e-320;

    //printf("%f\n", *ptr);

    sf_free(ptr);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
