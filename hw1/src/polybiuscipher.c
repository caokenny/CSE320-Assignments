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

int inputTaken = 0;

int decryptIndex = 0;


int ePolyCipher(unsigned short mode) {
    int success = 0;
    char input;
    int columns = 0x000F & mode; //bitmask to get number of columns
    int rows = 0x00F0 & mode; //bitmask to bits for rows
    rows = rows >> 4; //shift 4 bits right to get number of rows
    if (keyWasGiven == 1) {
        loadPolyTableWithKey(rows, columns);
    }
    else loadPolyTable(rows, columns);
    if (mode & 0x2000) {
        while (input != EOF) {
            if (inputTaken >= 2) {
                inputTaken = 0;
                decryptIndex = 0;
            }
            inputTaken++;
            input = fgetc(stdin);
            if (input == EOF) break;
            success = decrypt(input, rows, columns);
            if (success == 0) return 0;
        }
        return 1;
    }
    else {
        while (input != EOF) { //take input until EOF
            input = fgetc(stdin);
            if (input == EOF) break; //When EOF break
            success = encrypt(input, rows, columns); //call encrypt function
            if (success == 0) return 0; //if encrypt returns 0 return 0 to main
        }
        return 1; //otherwise return 1
    }
}

int decrypt(char input, int rows, int columns) {
    if (input == 32 || input == 9){
        printf("%c", input);
        inputTaken = 0;
        return 1;
    }
    if (input == 10) {
        printf("\n");
        inputTaken = 0;
        return 1;
    }
    if (input < 65) input -= 48;
    else input -= 55;
    if (inputTaken == 1){
        decryptIndex = input * columns;
    }
    if (inputTaken == 2) {
        int i = input % columns;
        decryptIndex += i;
        printf("%c", *(polybius_table + decryptIndex));
    }
    return 1;
}

int encrypt(char input, int rows, int columns) {
    int i = 0;
    int inputRow = -1;
    int inPAlphabet = 0;
    while (1) {
        if (i%columns == 0) inputRow++; //When we hit number of columns we go to next row
        if (input == 32 || input == 9) { //preserve white space
            printf("%c", input);
            break;
        }
        if (input == 10) { //preserve white space
            printf("%c\n", 32);
            break;
        }
        inPAlphabet = checkIfInPAlphabet(input); //check if input is in polybius_alphabet
        if (inPAlphabet == 0){ //if not in alphabet return 0
            return 0;
        }
        if (input == *(polybius_table + i)){ //if input is in table
            //print the hexadecimal
            printf("%X", inputRow);
            if (i%columns <= 9) printf("%X", (i%columns));
            if (i%columns > 9) printf("%X", (i%columns));
            break;
        }
        else i++;
    }
    return 1;
}

int checkIfInPAlphabet(char input) {
    int polyCounter = 0;
    while (*polybius_alphabet != 0){
        if (input == *polybius_alphabet){
            polybius_alphabet -= polyCounter;
            return 1;
        }
        else {
            polybius_alphabet++;
            polyCounter++;
        }
    }
    return 0;
}

void loadPolyTable(int rows, int columns) {
    int i = 0; //counter
    int alphabetSize = 0;
    while (*polybius_alphabet != 0){ //stop when we hit null term
        *(polybius_table + i) = *polybius_alphabet; //using pointer arith. fill in the table
        polybius_alphabet++;
        i++;
        alphabetSize++;
    }
    int extraSpace = rows * columns; //this is how much space we have left to fill in with null terms
    extraSpace = extraSpace - 94;
    while (extraSpace != 0){
        *(polybius_table + i) = 0;
        i++;
        extraSpace--;
    }
    polybius_alphabet -= alphabetSize;
}

void loadPolyTableWithKey(int rows, int columns) {
    int i = 0;
    int itRepeats = 0;
    int alphabetSize = 0;
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
        if (itRepeats == 1){
            polybius_alphabet++;
            alphabetSize++;
        }
        else {
            *(polybius_table + i) = *polybius_alphabet;
            polybius_alphabet++;
            i++;
            alphabetSize++;
        }
    }
    int extraSpace = rows * columns;
    extraSpace = extraSpace - 94;
    while (extraSpace != 0){
        *(polybius_table + i) = 0;
        i++;
        extraSpace--;
    }
    polybius_alphabet -= alphabetSize;
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