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
        while (input != EOF) {
            input = fgetc(stdin);
            if (input == EOF) break;
            success = decryptMorseCode(input);
            if (success == 0) return 0;
        }
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
    printf("%s\n", buffer);
    return 1;
}

int decryptMorseCode(char input) {
    int fmkeyCounter = 0;
    int fracBackTrack = 0;
    if (input == 10) return 1;
    while(1) {
        if (input == *(fm_key + fmkeyCounter)) {
            while(**(fractionated_table + fmkeyCounter) != 0) {
                *(buffer + bufferCounter) = **(fractionated_table + fmkeyCounter);
                *(fractionated_table + fmkeyCounter) += 1;
                fracBackTrack++;
                bufferCounter++;
            }
            if (*(buffer + (bufferCounter - 1)) == 120) {
                printf("%s\n", buffer);
                secondDecryption();
                bufferCounter = 0;
            }
            *(fractionated_table + fmkeyCounter) -= fracBackTrack;
            fracBackTrack = 0;
            break;
        }
        else {
            fmkeyCounter++;
        }
    }
    return 1;
}

void secondDecryption() {
    bufferCounter = 0;
    int morseTableCounter = 0;
    //int bufferOriginalPosition = 0;
    int morseTableBackTrack = 0;
    while(1) {
        //printf("COMPARING %c == morse_table[%d] = %c\n", *(buffer + bufferCounter), morseTableCounter, **(morse_table + morseTableCounter));
        if (*(buffer + bufferCounter) == 120) bufferCounter++;
        if (*(buffer + bufferCounter) == **(morse_table + morseTableCounter)) {
            bufferCounter++;
            //bufferOriginalPosition++;
            *(morse_table + morseTableCounter) += 1;
            morseTableBackTrack++;
            if (*(buffer + bufferCounter) == 120 && **(morse_table + morseTableCounter) == 0) {
                printf("%c", morseTableCounter + 33);
                break;
            }
        }
        else {
            bufferCounter = 0;
            //bufferOriginalPosition = 0;
            *(morse_table + morseTableCounter) -= morseTableBackTrack;
            morseTableBackTrack = 0;
            morseTableCounter++;
        }
    }
}

int encryptMorseCode(char input) {
    if (!(1 & checkIfInFAlphabet(input))) {
        return 0;
    }
    int counter = 0;
    int morseTableCounter = 0;
    while (1) {
        if ((bufferCounter%3) == 0 && bufferCounter != 0) {
            secondEncryption();
            bufferCounter = 0;
            continue;
        }
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
    secondEncryption();
    return 1;
}

void secondEncryption() {
    int newBuffCounter = 0;
    int fracTableCounter = 0;
    int fracBackTrack = 0;
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
    if (**(morse_table + (input - 33)) != 0) {
        return 1;
    }
    return 0;
}