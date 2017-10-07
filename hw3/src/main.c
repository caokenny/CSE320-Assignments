#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    /* void *u = */ sf_malloc(1);          //32
    void *v = sf_malloc(LIST_1_MIN); //48
    void *w = sf_malloc(LIST_2_MIN); //160
    void *x = sf_malloc(LIST_3_MIN); //544
    void *y = sf_malloc(LIST_4_MIN); //2080
    /* void *z = */ sf_malloc(1); // 32

    /*double* ptr = sf_malloc(88);

    sf_varprint(ptr);

    *ptr = 320320320e-320;

    //printf("%f\n", *ptr);

    sf_free(ptr); */

    sf_mem_fini();

    return EXIT_SUCCESS;
}
