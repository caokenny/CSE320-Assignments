#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    //double* ptr = sf_malloc(sizeof(long double));

    //*ptr = 320320320e-320;

    //printf("%f\n", *ptr);

    //sf_free(ptr);

    void *x = sf_malloc(sizeof(double) * 11);
    void *y = sf_malloc(sizeof(char));

    sf_varprint(x);

    sf_varprint(y);

    sf_free(x);
    sf_free(y);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
