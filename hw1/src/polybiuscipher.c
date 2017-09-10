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

int keyCounter = 0;


void ePolyCipher(unsigned short mode) {
    int columns = 0x000F & mode; //bitmask to get number of columns
    int rows = 0x00F0 & mode; //bitmask to bits for rows
    rows = rows >> 4; //shift 4 bits right to get number of rows
    printf("ROWS = %d, COLUMNS = %d\n", rows, columns);
    if (keyWasGiven == 1) {
        loadPolyTableWithKey(rows, columns);
    }
    else loadPolyTable(rows, columns);
    printf("%s\n", polybius_table);
}

void loadPolyTable(int rows, int columns) {
    int i = 0; //counter
    while (*polybius_alphabet != 0){ //stop when we hit null term
        *(polybius_table + i) = *polybius_alphabet; //using pointer arith. fill in the table
        polybius_alphabet++;
        i++;
    }
    int extraSpace = rows * columns; //this is how much space we have left to fill in with null terms
    extraSpace = extraSpace - 94;
    while (extraSpace != 0){
        *(polybius_table + i) = 0;
        i++;
        extraSpace--;
    }
}

void loadPolyTableWithKey(int rows, int columns) {
    int i = 0;
    int itRepeats = 0;
    while (*key != 0) {
        *(polybius_table + i) = *key;
        key++;
        i++;
        keyCounter++;
    }
    key -= keyCounter;
    keyCounter = 0;
    while (*polybius_alphabet != 0) {
        itRepeats = checkIfRepeating();
        if (itRepeats == 1)
            polybius_alphabet++;
        else {
            *(polybius_table + i) = *polybius_alphabet;
            polybius_alphabet++;
            i++;
        }
    }
    int extraSpace = rows * columns;
    extraSpace = extraSpace - 94;
    while (extraSpace != 0){
        *(polybius_table + i) = 0;
        i++;
        extraSpace--;
    }
    polybius_alphabet -= 94;
}

int checkIfRepeating() {
    while (*key != 0) {
        if (*polybius_alphabet == *key) {
            key -= keyCounter;
            keyCounter = 0;
            return 1;
        }
        else {
            key++;
            keyCounter++;
        }
    }
    key -= keyCounter;
    keyCounter = 0;
    return 0;
}