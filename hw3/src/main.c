#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    void *a = sf_malloc(16368);

    sf_snapshot();

    sf_varprint(a);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
