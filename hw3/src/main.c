#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    void *ptr = sf_malloc(5000);

    sf_varprint(ptr);

    sf_snapshot();

    sf_free(ptr);

    sf_snapshot();


    /*double* ptr = sf_malloc(88);

    sf_varprint(ptr);

    *ptr = 320320320e-320;

    //printf("%f\n", *ptr);

    sf_free(ptr); */

    sf_mem_fini();

    return EXIT_SUCCESS;
}
