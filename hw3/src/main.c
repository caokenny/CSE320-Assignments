#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    void *a = sf_malloc(4000);
    void *b = sf_malloc(8000);
    sf_snapshot();
    void *c = sf_malloc(16000);
    void *d = sf_malloc(500);
    sf_snapshot();

    if (c == NULL) printf("C IS NULL\n");

    sf_free(a);
    sf_free(b);
    sf_free(d);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
