#ifndef HW_H
#define HW_H

#include "const.h"

extern int keyWasGiven;

int checkKeyValidF (int argc, char **argv);
int checkKeyValidP (int argc, char **argv);

int polyCipher(unsigned short mode);
int checkIfValidRowsCols(short rows, short columns);
void loadPolyTable(int rows, int columns);
void loadPolyTableWithKey(int rows, int columns);
int checkIfRepeating();
int encrypt(char input, int rows, int columns);
int checkIfInPAlphabet(char input);
int decrypt(char input, int rows, int columns);

int fMorseCipher(unsigned short mode);
int encryptMorseCode(char input);
int decryptMorseCode(char input);
void loadMorseKey();
void loadMorseKeyWithKey();
void getAlphabetSize();
int checkIfRepeatingMorse();
int checkIfInFAlphabet(char input);
void secondEncryption();
void secondDecryption();

#endif
