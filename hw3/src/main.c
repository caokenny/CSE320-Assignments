#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    void* x = sf_malloc(4080);
    sf_snapshot();
     sf_malloc(4080);
     sf_snapshot();
     sf_malloc(4080);
    void* y = sf_malloc(4080);

    sf_snapshot();

    sf_free(x);
    sf_free(y);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
