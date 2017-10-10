#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    void *x = sf_malloc(LIST_3_MIN); //544
    void *y = sf_malloc(LIST_4_MIN); //2080

    sf_free(x);
    sf_free(y);

    sf_snapshot();

    //sf_snapshot();

    //*ptr = 320320320e-320;

    //printf("%f\n", *ptr);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
