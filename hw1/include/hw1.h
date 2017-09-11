#ifndef HW_H
#define HW_H

#include "const.h"

extern int keyWasGiven;

int checkKeyValidF (int argc, char **argv);
int checkKeyValidP (int argc, char **argv);

int ePolyCipher(unsigned short mode);
void loadPolyTable(int rows, int columns);
void loadPolyTableWithKey(int rows, int columns);
int checkIfRepeating();
int encrypt(char input, int rows, int columns);
int checkIfInPAlphabet(char input);

#endif
