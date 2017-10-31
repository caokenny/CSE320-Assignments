#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>

#include "sfish.h"
#include "debug.h"

int main(int argc, char *argv[], char* envp[]) {
    char* input;
    bool exited = false;

    if(!isatty(STDIN_FILENO)) {
        // If your shell is reading from a piped file
        // Don't have readline write anything to that file.
        // Such as the prompt or "user input"
        if((rl_outstream = fopen("/dev/null", "w")) == NULL){
            perror("Failed trying to open DEVNULL");
            exit(EXIT_FAILURE);
        }
    }

    do {

        input = readline(">> ");

        write(1, "\e[s", strlen("\e[s"));
        write(1, "\e[20;10H", strlen("\e[20;10H"));
        write(1, "SomeText", strlen("SomeText"));
        write(1, "\e[u", strlen("\e[u"));

        // If EOF is read (aka ^D) readline returns NULL
        if (strcmp(input, "help") == 0) HELP();
        else if (strcmp(input, "exit") == 0) break;
        else if (strcmp(input, "pwd") == 0) printf("%s\n", get_current_dir_name());
        else if (input[0] == 'c' && input[1] == 'd') {
            if (*(input + 3) == 0 || *(input + 3) == 32 || strlen(input) == 2) {
                if (getenv("PPATH") != get_current_dir_name()) setenv("PPATH", get_current_dir_name(), 1);
                chdir(getenv("HOME"));
            }
            else if (*(input + 3) == '-') {
                chdir(getenv("PPATH"));
            }
            else {
                if (getenv("PPATH") != get_current_dir_name()) setenv("PPATH", get_current_dir_name(), 1);
                chdir(input + 3);
            }
        }

        if(input == NULL) {
            continue;
        }


        // Currently nothing is implemented
        //printf(EXEC_NOT_FOUND, input);

        // You should change exit to a "builtin" for your hw.
        //exited = strcmp(input, "exit") == 0;

        // Readline mallocs the space for input. You must free it.
        rl_free(input);

    } while(!exited);

    printf("%s\n", getenv("PATH"));

    debug("%s", "user entered 'exit'");

    return EXIT_SUCCESS;
}
