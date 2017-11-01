#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "sfish.h"
#include "debug.h"

int parseLine(char *buf, char **argv);

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
        //if (strstr(get_current_dir_name(), getenv("HOME")) != NULL) input = readline(~)
        char *prompt = calloc(strlen(get_current_dir_name()) + 1, sizeof(char));
        char *workingDir = get_current_dir_name();
        if (strstr(get_current_dir_name(), getenv("HOME")) != NULL) {
            strcat(prompt, "~");
            strcat(prompt, workingDir + strlen(getenv("HOME")));
            strcat(prompt, " :: kencao >> ");
        } else {
            strcat(prompt, get_current_dir_name());
            strcat(prompt, " :: kencao >> ");
        }
        input = readline(prompt);
        if (input == NULL) {
            free(prompt);
            free(input);
            continue;
        }

        parseLine(input, argv);
        if (getenv("PPATH") == NULL) {
            setenv("PPATH", get_current_dir_name(), 1);
        }
        //write(1, "\e[s", strlen("\e[s"));
        //write(1, "\e[20;10H", strlen("\e[20;10H"));
        //write(1, "SomeText", strlen("SomeText"));
        //write(1, "\e[u", strlen("\e[u"));

        // If EOF is read (aka ^D) readline returns NULL
        if (strcmp(argv[0], "help") == 0) HELP();
        else if (strcmp(argv[0], "exit") == 0) break;
        else if (strcmp(argv[0], "pwd") == 0) printf("%s\n", get_current_dir_name());
        else if (strcmp(argv[0], "cd") == 0) {
            if (argv[1] == NULL || strcmp(argv[1], " ") == 0) {
                //if (getenv("PPATH") != get_current_dir_name()) setenv("PPATH", get_current_dir_name(), 1);
                chdir(getenv("HOME"));
            }
            else if (strcmp(argv[1], "-") == 0) {
                chdir(getenv("PPATH"));
            }
            else {
                if (getenv("PPATH") != get_current_dir_name()) setenv("PPATH", get_current_dir_name(), 1);
                chdir(argv[1]);
            }
        }
        else {
            pid_t pid;
            int childStatus;
            //stdout = fmemopen(argv[1], strlen(argv[1]) + 1, "r+");
            if ((pid = fork()) == 0) {
                if (strcmp(argv[0], "ls") == 0) {

                }
                else if (strcmp(argv[0], "echo") == 0) {
                    if (strcmp(argv[2], ">") == 0) {
                        int fd;
                        fd = open(argv[3], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
                        dup2(fd, 1);
                        argv[2] = 0;
                        argv[3] = 0;
                    }
                }
                else if (strcmp(argv[0], "cat") == 0) {
                    if (strcmp(argv[1], "<") == 0) {
                        int fd;
                        fd = open(argv[2], O_RDONLY);
                        argv[1] = 0;
                        dup2(fd, 0);
                    }
                }
                else if (strcmp(argv[0], "grep") == 0) {
                    if (strcmp(argv[2], "<") == 0) {
                        int fd = open(argv[3], O_RDONLY);
                        argv[2] = 0;
                        dup2(fd, 0);
                    }
                }
                execvp(argv[0], argv);
            }
            waitpid(pid, &childStatus, 0);
        }


        // Currently nothing is implemented
        //printf(EXEC_NOT_FOUND, input);

        // You should change exit to a "builtin" for your hw.
        //exited = strcmp(input, "exit") == 0;

        // Readline mallocs the space for input. You must free it.
        free(prompt);
        free(input);
    } while(!exited);

    debug("%s", "user entered 'exit'");

    return EXIT_SUCCESS;
}

int parseLine(char *buf, char **argv) {
    char *delim;
    int argc;
    int bg;

    buf[strlen(buf)] = ' ';
    while (*buf && (*buf == ' '))
        buf++;
    argc = 0;
    /*delim = strchr(buf, ' ');
    argv[argc++] = buf;
    *delim = '\0';
    buf = delim + 1;
    while (*buf && (*buf == ' ')) buf++;*/
    while ((delim = strchr(buf, ' '))) {
        if (buf[0] != '"') {
            argv[argc++] = buf;
            *delim = '\0';
            buf = delim + 1;
            while (*buf && (*buf == ' '))
                buf++;
        }
        else {
            delim = strchr(buf + 1, '"');
            argv[argc++] = buf + 1;
            *delim = '\0';
            buf = delim + 1;
            while (*buf && (*buf == ' '))
                buf++;
        }
    }
    argv[argc] = NULL;

    if (argc == 0) return 1;

    if ((bg = (*argv[argc-1] == '&')) != 0) argv[--argc] = NULL;

    return bg;
}
