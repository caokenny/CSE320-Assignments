#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    double *ptr = sf_malloc(sizeof(double));

    sf_varprint(ptr);

    sf_free(ptr);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
