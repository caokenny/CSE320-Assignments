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

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the program
 * and will return a unsigned short (2 bytes) that will contain the
 * information necessary for the proper execution of the program.
 *
 * IF -p is given but no (-r) ROWS or (-c) COLUMNS are specified this function
 * MUST set the lower bits to the default value of 10. If one or the other
 * (rows/columns) is specified then you MUST keep that value rather than assigning the default.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return Refer to homework document for the return value of this function.
 */

int keyWasGiven = 0;

unsigned short validargs(int argc, char **argv) {
    short x = 0;
    short returnValue;
    short rows = 10;
    short columns = 10;
    argv++;
    *argv += 1;
    //if -d, -e, -k, -r, or -c flag is first flag return error
    if (**argv == 100 || **argv == 101 || **argv == 107 || **argv == 114 || **argv == 99){
        return 0x0000;
    }
    if (**argv == 104){ //if -h flag first return 0x8000 for help menu
        return 0x8000;
    }
    if (**argv == 102) { //if -f flag
        if (argc - 2 == 0) return 0x0000; //return error too few arguments
        argv++; //get argv[2]
        *argv += 1; //get the letter instead of -
        if (**argv == 101) { //if -e flag
            if (argc - 3 == 0)
                return 0x4000; //When -f -e return second MSB as 1 and third MSB as 0
            argv++;
            *argv += 1;
            if (**argv == 107) { //if -k flag
                keyWasGiven = 1;
                if (argc - 5 != 0) return 0x0000;
                int kValid = checkKeyValidF(argc, argv);
                if (kValid == 1) return 0x4000;
            }
            else return 0x0000;
        }
        if (**argv == 100) { //if -d flag
            if (argc - 3 == 0)
                return 0x6000; //When -f -d return second MSB as 1 and third MSB as 1
            argv++;
            *argv += 1;
            if (**argv == 107) { //if -k flag
                keyWasGiven = 1;
                if (argc - 5 != 0) return 0x0000;
                int kValid = checkKeyValidF(argc, argv);
                if (kValid == 1) return 0x6000;
            }
            else return 0x0000;
        }
        else return 0x0000;
    }


    if (**argv == 112) { //if -p flag
        if (argc - 2 == 0) return 0x0000; //return error too few argument
        argv++; //get argv[2]
        *argv += 1; //get the letter instead of -
        if (**argv == 107) //if -k flag comes before -e or -d return an error
            return 0x0000;
        if (**argv == 101){ //-e flag
            returnValue = 0x0000;
            for (int i = 0; i < argc - 3; i++) {
                argv++; //get argv[3]
                *argv += 1;
                if (**argv == 99) { //if -c flag
                    argc -= 1;
                    argv++; //get argv[4]
                    sscanf(*argv, "%hu", &x); //parses string to short
                    if (x < 9 || x > 15)
                        return 0x0000;
                    columns = x;
                    returnValue = returnValue | x; //bitwise or to change 0-4 LSB to the int given
                }
                else if (**argv == 114) { //if -r flag
                    argc -= 1;
                    argv++;
                    sscanf(*argv, "%hu", &x);
                    if (x < 9 || x > 15)
                        return 0x0000;
                    rows = x << 4; //Shift bits 4 to the left
                    returnValue = returnValue | rows; //bitwise or to change 5-9 bits to the int given
                    rows = x;
                }
                else if (**argv == 107) { //if -k flag
                    keyWasGiven = 1;
                    argc -= 1;
                    int kValid = checkKeyValidP(argc, argv);
                    if (kValid == 0) return 0x0000;
                    argv++;
                }
                else return 0x0000;
            }
            if (!(1 & checkIfValidRowsCols(rows, columns))) {
                printf("Invalid rows/columns\n");
                return 0x0000;
            }
            if ((0x000F & returnValue) == 0) returnValue = returnValue | 0x000A;
            if ((0x00F0 & returnValue) == 0) returnValue = returnValue | 0x00A0;
            if (returnValue == 0x0000){
                return 0x00AA;
            }
            else{
                return returnValue;
            }
        }
        else if (**argv == 100) { //if -d flag
            returnValue = 0x2000;
            for (int i = 0; i < argc - 3; i++) {;
                argv++; //get argv[3]
                *argv += 1;
                if (**argv == 99) { //if -c flag
                    argc -= 1;
                    argv++; //get argv[4]
                    sscanf(*argv, "%hu", &x); //parses string to short
                    if (x < 9 || x > 15)
                        return 0x0000;
                    returnValue = returnValue | x; //bitwise or to change 0-4 LSB to the int given
                }
                else if (**argv == 114) { //if -r flag
                    argc -= 1;
                    argv++;
                    sscanf(*argv, "%hu", &x);
                    if (x < 9 || x > 15)
                        return 0x0000;
                    short rows;
                    rows = x << 4;
                    returnValue = returnValue | rows;
                }
                else if (**argv == 107) { //if -k flag
                    keyWasGiven = 1;
                    argc -= 1;
                    int kValid = checkKeyValidP(argc, argv);
                    if (kValid == 0) return 0x0000;
                    argv++;
                }
                else return 0x0000;
            }
            if ((0x000F & returnValue) == 0) returnValue = returnValue | 0x000A;
            if ((0x00F0 & returnValue) == 0) returnValue = returnValue | 0x00A0;
            if (returnValue == 0x2000)
                return 0x20AA;
            else return returnValue;
        }
    }
    return 0x0000;
}

int checkIfValidRowsCols(short rows, short columns) {
    int alphabetSize = 0;
    while (*polybius_alphabet != 0) {
        alphabetSize++;
        polybius_alphabet++;
    }
    polybius_alphabet -= alphabetSize;
    if (rows * columns < alphabetSize) return 0;
    else return 1;
}

int checkKeyValidF(int argc, char **argv) {
    argv++; //get next element
    key = *argv;
    char *keyCompare = *argv; //variable that holds the key so we can compare
    int keyCounter = 0; //counter
    int keyBacktrack = 0;
    int k = 0;
    int alphabetCounter = 0;
    while (*key != 0) { //while *key isn't NULL
        keyCompare++; //increment keyCompare by 1 so we get the next value so we don't compare the same letters
        while (*keyCompare != 0){ //while keyCompare isn't NULL
            if (*key == *keyCompare) return 0; //here we compare one letter in the key to the rest of the string
            keyCounter++; //increment counter so we know how much to backtrack later
            keyCompare++; //increment to the next letter in the key
        }
        while (*fm_alphabet != 0){
            if (*key == *fm_alphabet) {
                k = 1;
                break;
            }
            else {
                fm_alphabet++;
                alphabetCounter++;
            }
        }
        if (k != 1) return 0;
        k = 0;
        fm_alphabet -= alphabetCounter;
        alphabetCounter = 0;
        keyCompare -= keyCounter; //backtrack keyCompare
        keyCounter = 0; //set counter back to 0
        key++; //get the next letter in the key to compare
        keyBacktrack++;
    }
    key -= keyBacktrack;
    return 1;
}

int checkKeyValidP (int argc, char **argv){
    argv++; //get next element
    key = *argv;
    char *keyCompare = *argv; //variable that holds the key so we can compare
    int keyCounter = 0; //counter
    int keyBacktrack = 0;
    int k = 0;
    int alphabetCounter = 0;
    while (*key != 0) { //while *key isn't NULL
        keyCompare++; //increment keyCompare by 1 so we get the next value so we don't compare the same letters
        while (*keyCompare != 0){ //while keyCompare isn't NULL
            if (*key == *keyCompare) return 0; //here we compare one letter in the key to the rest of the string
            keyCounter++; //increment counter so we know how much to backtrack later
            keyCompare++; //increment to the next letter in the key
        }
        while (*polybius_alphabet != 0){
            if (*key == *polybius_alphabet) {
                k = 1;
                break;
            }
            else {
                polybius_alphabet++;
                alphabetCounter++;
            }
        }
        if (k != 1) return 0;
        k = 0;
        polybius_alphabet -= alphabetCounter;
        alphabetCounter = 0;
        keyCompare -= keyCounter; //backtrack keyCompare
        keyCounter = 0; //set counter back to 0
        key++; //get the next letter in the key to compare
        keyBacktrack++;
    }
    key -= keyBacktrack;
    return 1;
}