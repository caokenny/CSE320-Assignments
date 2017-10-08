#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    void *a = sf_malloc(4000);
    void *b = sf_malloc(8000);
    void *c = sf_malloc(16000);
    void *d = sf_malloc(32);

    sf_free(c);
    sf_free(b);
    sf_free(a);
    sf_free(d);


    /*double* ptr = sf_malloc(88);

    sf_varprint(ptr);

    *ptr = 320320320e-320;

    //printf("%f\n", *ptr);

    sf_free(ptr); */

    sf_mem_fini();

    return EXIT_SUCCESS;
}
