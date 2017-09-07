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
unsigned short validargs(int argc, char **argv) {
    short x;
    short returnValue;
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
        argv++; //get argv[2]
        *argv += 1; //get the letter instead of -
        if (**argv == 101)
            return 0x4000; //When -f -e return second MSB as 1 and third MSB as 0
        if (**argv == 100)
            return 0x6000; //When -f -d return second MSB as 1 and third MSB as 1
        else return 0x0000;
    }
    if (**argv == 112) { //if -p flag
        argv++; //get argv[2]
        *argv += 1; //get the letter instead of -
        if (**argv == 101){ //-e flag
            returnValue = 0x0000;
            argv++; //get argv[3]
            *argv += 1;
            while(1) {
                if (**argv == 99) { //if -c flag
                    argv++; //get argv[4]
                    sscanf(*argv, "%hu", &x); //parses string to short
                    returnValue = returnValue | x; //bitwise or to change 0-4 LSB to the int given
                    argv++;
                    *argv += 1;
                }
                else if (**argv == 114) { //if -r flag
                    argv++;
                    sscanf(*argv, "%hu", &x);
                    short rows;
                    rows = x << 4;
                    returnValue = returnValue | rows;
                    argv++;
                    *argv += 1;
                }
                else break;
            }
            if (returnValue == 0x0000)
                return 0x00AA;
            else return returnValue;
        }
        else if (**argv == 100) { //if -d flag
            returnValue = 0x2000;
            argv++; //get argv[3]
            *argv += 1;
            while(1) {
                if (**argv == 99) { //if -c flag
                    argv++; //get argv[4]
                    sscanf(*argv, "%hu", &x); //parses string to short
                    returnValue = returnValue | x; //bitwise or to change 0-4 LSB to the int given
                    argv++;
                    *argv += 1;
                }
                else if (**argv == 114) { //if -r flag
                    argv++;
                    sscanf(*argv, "%hu", &x);
                    short rows;
                    rows = x << 4;
                    returnValue = returnValue | rows;
                    argv++;
                    *argv += 1;
                }
                else break;
            }
            if (returnValue == 0x2000)
                return 0x20AA;
            else return returnValue;
        }
    }
}
