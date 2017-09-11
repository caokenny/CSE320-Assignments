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

int alphabetSize = 0;

int fMorseCipher() {
    char input;
    int success = 0;
    getAlphabetSize();
    if (keyWasGiven == 1)
        loadMorseKeyWithKey();
    else loadMorseKey();
    while (input != EOF) {
        input = fgetc(stdin);
        if (input == EOF) break;
        if (input == 10) break;
        if (input != 32)
            success = encryptMorseCode(input);
        else printf("x");
        if (success == 0) return 0;
    }
    return 1;
}

int encryptMorseCode(char input) {
    int counter = 0;
    while (1) {
        if (*(fm_key + counter) == input) {
            printf("%sx", *(morse_table + (counter + 32)));
            break;
        }
        else counter++;
    }
    return 1;
}

void loadMorseKey() {
    int i = 0;
    while (*fm_alphabet != 0){
        *(fm_key + i) = *fm_alphabet;
        fm_alphabet++;
        i++;
    }
    fm_alphabet -= alphabetSize;
}

void loadMorseKeyWithKey() {
    int i = 0;
    int keyCounter = 0;
    while (*key != 0) {
        *(fm_key + i) = *key;
        key++;
        i++;
        keyCounter++;
    }
    key -= keyCounter;
    while (*fm_alphabet != 0) {
        if (1 & checkIfRepeatingMorse()) {
            fm_alphabet++;
        }
        else {
            *(fm_key + i) = *fm_alphabet;
            fm_alphabet++;
            i++;
        }
    }
    fm_alphabet -= alphabetSize;
}

void getAlphabetSize(){
    while (*fm_alphabet != 0) {
        fm_alphabet++;
        alphabetSize++;
    }
    fm_alphabet -= alphabetSize;
}

int checkIfRepeatingMorse() {
    int keyBackTrack = 0;
    while (*key != 0) {
        if (*fm_alphabet == *key) {
            key -= keyBackTrack;
            return 1;
        }
        key++;
        keyBackTrack++;
    }
    key -= keyBackTrack;
    return 0;
}