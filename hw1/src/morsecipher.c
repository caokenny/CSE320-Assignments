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

char *buffer = polybius_table;

int bufferCounter = 0;

int fMorseCipher(unsigned short mode) {
    char input;
    int success = 0;
    getAlphabetSize();
    if (keyWasGiven == 1)
        loadMorseKeyWithKey();
    else loadMorseKey();
    if (mode & 0x4000 && mode & 0x2000) {
        printf("HI\n");
    }
    else {
        while (input != EOF) {
            input = fgetc(stdin);
            if (input == EOF) break;
            if (input == 32) {
                if (*(buffer + (bufferCounter - 2)) != 120){
                    *(buffer + bufferCounter) = 120;
                    bufferCounter++;
                }
            }
            else {
                success = encryptMorseCode(input);
            }
            if (success == 0) return 0;
        }
    }
    return 1;
}

int encryptMorseCode(char input) {
    if (!(1 & checkIfInFAlphabet(input))) {
        printf("NOT IN ALPHABET\n");
        return 0;
    }
    int counter = 0;
    int morseTableCounter = 0;
    while (1) {
        if (input == 10) {
            *(buffer + bufferCounter) = 120;
            bufferCounter++;
            *(buffer + bufferCounter) = 10;
            bufferCounter++;
            break;
        }
        while (**(morse_table + (input - 33)) != 0){
            *(buffer + bufferCounter) = **(morse_table + (input - 33));
            bufferCounter++;
            *(morse_table + (input - 33)) += 1;
            morseTableCounter++;
        }
        *(morse_table + (input - 33)) -= morseTableCounter;
        morseTableCounter = 0;
        *(buffer + bufferCounter) = 120;
        bufferCounter++;
        return 1;
    }
    int newBuffCounter = 0;
    int fracTableCounter = 0;
    int fracBackTrack = 0;
    //if ((bufferCounter - 1) % 3 != 0) {
    //    bufferCounter -= ((bufferCounter - 1)%3);
    //}
    while (newBuffCounter != bufferCounter) {
        if (*(buffer + newBuffCounter) == **(fractionated_table + fracTableCounter)) {
            newBuffCounter++;
            *(fractionated_table + fracTableCounter) += 1;
            fracBackTrack++;
            if ((newBuffCounter%3) == 0 && newBuffCounter != 0){
                printf("%c", *(fm_key + fracTableCounter));
                *(fractionated_table + fracTableCounter) -= fracBackTrack;
                fracBackTrack = 0;
                fracTableCounter = 0;
            }
            if (*(buffer + newBuffCounter) == 10) {
                printf("\n");
                break;
            }
        }
        else {
            newBuffCounter -= (newBuffCounter % 3);
            *(fractionated_table + fracTableCounter) -= fracBackTrack;
            fracBackTrack = 0;
            fracTableCounter++;
        }
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

int checkIfInFAlphabet(char input) {
    if (input == 32 || input == 10) return 1;
    int fCounter = 0;
    while (*fm_alphabet != 0){
        if (input == *fm_alphabet){
            fm_alphabet -= fCounter;
            fCounter = 0;
            return 1;
        }
        else {
            fm_alphabet++;
            fCounter++;
        }
    }
    return 0;
}