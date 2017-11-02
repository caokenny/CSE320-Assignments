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
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        char *prompt = calloc(strlen(cwd) + 1, sizeof(char));
        if (strstr(cwd, getenv("HOME")) != NULL) {
            strcat(prompt, "~");
            strcat(prompt, cwd + strlen(getenv("HOME")));
            strcat(prompt, " :: kencao >> ");
        } else {
            strcat(prompt, cwd);
            strcat(prompt, " :: kencao >> ");
        }
        input = readline(prompt);
        if (strcmp(input, "") == 0) {
            free(prompt);
            free(input);
            continue;
        }

        int count = parseLine(input, argv);
        if (getenv("PPATH") == NULL) {
            setenv("PPATH", cwd, 1);
        }
        //write(1, "\e[s", strlen("\e[s"));
        //write(1, "\e[20;10H", strlen("\e[20;10H"));
        //write(1, "SomeText", strlen("SomeText"));
        //write(1, "\e[u", strlen("\e[u"));

        // If EOF is read (aka ^D) readline returns NULL
        if (strstr(input, "help") == input) {
            pid_t pid;
            int childStatus;
            if (count == 1) {
                if ((pid = fork()) == 0)
                    HELP();
            } else {
                if ((pid = fork()) == 0) {
                    if (strcmp(argv[1], ">") == 0) {
                        int fd;
                        char *buf =
"help                  Displays this help message\n\
exit                  Exits bash shell\n\
cd                    Changes working directory of the shell\n\
    cd -              Changes the directory to the last directory the user was in\n\
    cd                Goes to the users home directory\n\
pwd                   Prints the absolute path of the current working directory";\
                        fd = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
                        write(fd, buf, strlen(buf));
                    }
                }
            }
            waitpid(pid, &childStatus, 0);
            /*int wpid = waitpid(pid, &childStatus, 0);
            if (WIFEXITED(childStatus)) {
                printf("Child %d exited with status %d\n", wpid, WEXITSTATUS(childStatus));
            }*/
            return EXIT_SUCCESS;
        }
        else if (strstr(input, "exit") == input) break;
        else if (strstr(input, "pwd") == input) {
            pid_t pid;
            int childStatus;
            if (count == 1) {
                if ((pid = fork()) == 0) {
                    printf("%s\n", cwd);
                    exit(0);
                }
            } else {
                if ((pid = fork()) == 0) {
                    if (strcmp(argv[1], ">") == 0) {
                        int fd;
                        fd = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
                        write(fd, cwd, strlen(cwd));
                        exit(0);
                    }
                }
            }
            waitpid(pid, &childStatus, 0);
            //int wpid = waitpid(pid, &childStatus, 0);
            /*if (WIFEXITED(childStatus)) {
                printf("Child %d exited with status %d\n", wpid, WEXITSTATUS(childStatus));
            }*/
        }
        else if (strstr(input, "cd") == input) {
            if (count == 1) {
                setenv("PPATH", cwd, 1);
                chdir(getenv("HOME"));
            }
            else if (*(input + 3) == 0 || *(input + 3) == 32) {
                setenv("PPATH", cwd, 1);
                chdir(getenv("HOME"));
            }
            else if (*(input + 3) == '-') {
                chdir(getenv("PPATH"));
                setenv("PPATH", cwd, 1);
            }
            else {
                setenv("PPATH", cwd, 1);
                chdir(input + 3);
            }
        }
        else {
            //int count = parseLine(input, argv);
            pid_t pid;
            int childStatus;
            while (1) {
                if ((pid = fork()) == 0) {
                    for (int i = 0; i < count; i++) {
                        if (strcmp(argv[i], "<") == 0) {
                            int fd;
                            fd = open(argv[i + 1], O_RDONLY);
                            dup2(fd, 0);
                            for (int j = i; j < count; j++){
                                if (strcmp(argv[j], ">") == 0) {
                                    int fd2;
                                    fd2 = open(argv[j + 1], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
                                    dup2(fd2, 1);
                                }
                                argv[j] = 0;
                            }
                            break;
                        }
                        if (strcmp(argv[i], ">") == 0) {
                            int fd;
                            fd = open(argv[i + 1], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
                            dup2(fd, 1);
                            for (int j = i; j < count; j++){
                                if (strcmp(argv[j], "<") == 0) {
                                    int fd2;
                                    fd2 = open(argv[j + 1], O_RDONLY);
                                    dup2(fd2, 0);
                                }
                                argv[j] = 0;
                            }
                            break;
                        }
                    }
                    execvp(argv[0], argv);
                }
                waitpid(pid, &childStatus, 0);
                break;
            }
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
    argv[0] = NULL;
    while ((delim = strchr(buf, ' '))) {
        if (buf[0] != '"' && argv[0] == NULL) {
            argv[argc++] = buf;
            *delim = '\0';
            buf = delim + 1;
            while (*buf && (*buf == ' '))
                buf++;
        }
        else if (buf[0] == '"') {
            delim = strchr(buf + 1, '"');
            argv[argc++] = buf + 1;
            *delim = '\0';
            buf = delim + 1;
            while (*buf && (*buf == ' '))
                buf++;
        } else {
            for (int i = 0; i < strlen(buf); i++) {
                if ((buf[i] == '<' || buf[i] == '>' || buf[i] == '|') && i == 0) {
                    delim = strchr(buf, buf[i]);
                    argv[argc++] = buf;
                    delim++;
                    *delim = '\0';
                    buf = delim + 1;
                    while (*buf && (*buf == ' '))
                        buf++;
                    break;
                }
                else if (buf[i] == '<' || buf[i] == '>' || buf[i] == '|') {
                    delim = strchr(buf, buf[i]);
                    argv[argc++] = buf;
                    delim--;
                    *delim = '\0';
                    buf = delim + 1;
                    while (*buf && (*buf == ' '))
                        buf++;
                    break;
                }
            }
            delim = strchr(buf, ' ');
            argv[argc++] = buf;
            *delim = '\0';
            buf = delim + 1;
            while (*buf && (*buf == ' '))
                buf++;
        }
    }
    argv[argc] = NULL;

    if (argc == 0) return 1;

    if ((bg = (*argv[argc-1] == '&')) != 0) argv[--argc] = NULL;

    return argc;
}
