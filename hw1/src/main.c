#include <stdlib.h>

#include "hw1.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    unsigned short mode;

    mode = validargs(argc, argv);

    debug("Mode: 0x%X", mode);

    if (mode == 0x0000) {
        USAGE(*argv, EXIT_FAILURE);
    }

    if(mode & 0x8000) {
        USAGE(*argv, EXIT_SUCCESS);
    }

    if (mode & 0x4000) {
        printf("THIS IS SUPPOSED TO DO SOMETHING WITH -f and -e FLAG\n");
    }

    if (mode & 0x6000) {
        printf("THIS IS SUPPOSED TO DO SOMETHING WITH -f and -d FLAG\n");
    }

    if (mode & 0x2000) {
        printf("THIS DOES SOMETHING WITH -p and -d FLAG\n");
    }

    if (mode & 0x00FF) {
        ePolyCipher(mode);
    }

    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */