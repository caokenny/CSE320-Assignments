#include "hw1.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif


void ePolyCipher(unsigned short mode) {
    int columns = 0x000F & mode; //bitmask to get number of columns
    int rows = 0x00F0 & mode; //bitmask to bits for rows
    rows = rows >> 4; //shift 4 bits right to get number of rows
    printf("ROWS = %d, COLUMNS = %d\n", rows, columns);
    loadPolyTable();
}

void loadPolyTable() {
    int i = 0;
    polybius_alphabet--;
    printf("%d\n", *polybius_alphabet);
    while (*polybius_alphabet != 0){
        *(polybius_table + i) = *polybius_alphabet;
        polybius_alphabet++;
        i++;
    }
    printf("%s\n", polybius_table);
}