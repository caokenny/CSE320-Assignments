#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define BWN   "\x1B[30m"
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

#include "sfish.h"
#include "debug.h"

typedef struct {
    int *array;
    size_t used;
    size_t size;
} Joblist;

Joblist jobs;
char jobNames[1024][30];
char *nameOfJob;

void initArray(Joblist *jobs, size_t initialSize) {
    jobs->array = (int*)malloc(initialSize * sizeof(int));
    jobs->used = 0;
    jobs->size = initialSize;
}

void insertElement(Joblist *jobs, int element) {
    if (jobs->used == jobs->size) {
        jobs->size *= 2;
        jobs->array = (int*)realloc(jobs->array, jobs->size * sizeof(int));
    }
    jobs->array[jobs->used++] = element;
}

void freeArray(Joblist *jobs) {
    free(jobs->array);
    jobs->array = NULL;
    jobs->used = jobs->size = 0;
}

int parseLine(char *buf, char **argv);

volatile sig_atomic_t pid;
void sigchld_handler(int s) {
}

void sigint_handler(int s){
}

volatile pid_t childPID = (pid_t) -1;
void sigtstp_handler(int s) {
    printf("SIGTSTP CAUGHT\n");
    insertElement(&jobs, (int)childPID);
    strcpy(jobNames[jobs.used-1], nameOfJob);
}

int main(int argc, char *argv[], char* envp[]) {
    initArray(&jobs, 1);
    signal(SIGCHLD, sigchld_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    char *input;
    bool exited = false;
    char *color = "";

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
        char *prompt = calloc(strlen(cwd) + 10, sizeof(char));
        if (strstr(cwd, getenv("HOME")) != NULL) {
            if (strcmp(color, "RED") == 0) strcat(prompt, RED "~");
            else if (strcmp(color, "GRN") == 0) strcat(prompt, GRN "~");
            else if (strcmp(color, "YEL") == 0) strcat(prompt, YEL "~");
            else if (strcmp(color, "BLU") == 0) strcat(prompt, BLU "~");
            else if (strcmp(color, "MAG") == 0) strcat(prompt, MAG "~");
            else if (strcmp(color, "CYN") == 0) strcat(prompt, CYN "~");
            else if (strcmp(color, "WHT") == 0) strcat(prompt, WHT "~");
            else if (strcmp(color, "BWN") == 0) strcat(prompt, BWN "~");
            else strcat(prompt, "~");
            strcat(prompt, cwd + strlen(getenv("HOME")));
            strcat(prompt, " :: kencao >> " RESET);
        } else {
            if (strcmp(color, "RED") == 0) strcat(prompt, RED "");
            else if (strcmp(color, "GRN") == 0) strcat(prompt, GRN "");
            else if (strcmp(color, "YEL") == 0) strcat(prompt, YEL "");
            else if (strcmp(color, "BLU") == 0) strcat(prompt, BLU "");
            else if (strcmp(color, "MAG") == 0) strcat(prompt, MAG "");
            else if (strcmp(color, "CYN") == 0) strcat(prompt, CYN "");
            else if (strcmp(color, "WHT") == 0) strcat(prompt, WHT "");
            else if (strcmp(color, "BWN") == 0) strcat(prompt, BWN "");
            else strcat(prompt, "");
            strcat(prompt, cwd);
            strcat(prompt, " :: kencao >> " RESET);
        }
        input = readline(prompt);
        if (strcmp(input, "") == 0) {
            free(prompt);
            free(input);
            continue;
        }
        if (getenv("PPATH") == NULL) {
            setenv("PPATH", cwd, 1);
        }
        int count = parseLine(input, argv);
        int pipeCounter = 0;
        for (int i = 0; i < count; i++) {
            if (strcmp(argv[i], "|") == 0) pipeCounter++;
        }
        //printf("%s\n", argv[1]++);

        //write(1, "\e[s", strlen("\e[s"));
        //write(1, "\e[20;10H", strlen("\e[20;10H"));
        //write(1, "SomeText", strlen("SomeText"));
        //write(1, "\e[u", strlen("\e[u"));

        // If EOF is read (aka ^D) readline returns NULL
        if (strstr(input, "help") == input) {
            //pid_t pid;
            if (count == 1) {
                if ((pid = fork()) == 0){
                    HELP();
                    exit(EXIT_SUCCESS);
                }
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
                        close(fd);
                    }
                    exit(EXIT_SUCCESS);
                }
            }
            waitpid(pid, NULL, 0);
            /*int wpid = waitpid(pid, &childStatus, 0);
            if (WIFEXITED(childStatus)) {
                printf("Child %d exited with status %d\n", wpid, WEXITSTATUS(childStatus));
            }*/
        }
        else if (strstr(input, "color") == input) {
            if (strcmp(argv[1], "RED") == 0) {
                color = "RED";
            }
            else if (strcmp(argv[1], "GRN") == 0) {
                color = "GRN";
            }
            else if (strcmp(argv[1], "YEL") == 0) {
                color = "YEL";
            }
            else if (strcmp(argv[1], "BLU") == 0) {
                color = "BLU";
            }
            else if (strcmp(argv[1], "MAG") == 0) {
                color = "MAG";
            }
            else if (strcmp(argv[1], "CYN") == 0) {
                color = "CYN";
            }
            else if (strcmp(argv[1], "WHT") == 0) {
                color = "WHT";
            }
            else if (strcmp(argv[1], "BWN") == 0) {
                color = "BWN";
            }
            else {
                color = "RESET";
            }
            free(prompt);
            free(input);
            continue;

        }
        else if (strstr(input, "exit") == input) break;
        else if (strstr(input, "pwd") == input) {
            //pid_t pid;
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
        else if (strstr(input, "kill") == input) {
            if (strchr(argv[1], '%') != NULL) {
                char *buf = argv[1];
                buf++;
                int killPID = atoi(buf);
                killPID = jobs.array[killPID];
                kill(killPID, SIGKILL);
                waitpid(killPID, NULL, 0);
            } else {
                int killPID;
                killPID = atoi(argv[1]);
                kill(killPID, SIGKILL);
                waitpid(killPID, NULL, 0);
            }
        }
        else if (strstr(input, "jobs") == input) {
            for (int i = 0; i < jobs.size; i++) {
                printf(JOBS_LIST_ITEM, i, jobNames[i]);
            }
        }
        else {
            if (pipeCounter != 0) {
                sigset_t mask, prev;
                //signal(SIGTSTP, sigtstp_handler);
                sigemptyset(&mask);
                sigaddset(&mask, SIGCHLD);
                //pid_t pid;
                sigprocmask(SIG_BLOCK, &mask, &prev);
                if ((pid = fork()) == 0) {
                    sigset_t mask, prev;
                    //signal(SIGTSTP, sigtstp_handler);
                    sigemptyset(&mask);
                    sigaddset(&mask, SIGCHLD);
                    int pipeEnds1[2];
                    pid_t pid[pipeCounter + 1];
                    pipe(pipeEnds1);
                    for (int i = 0; i < pipeCounter + 1; i++) {
                        sigprocmask(SIG_BLOCK, &mask, &prev);
                        if ((pid[i] = fork()) == 0) {
                            dup2(pipeEnds1[1], STDOUT_FILENO);
                            close(pipeEnds1[1]);
                            close(pipeEnds1[0]);
                            if (i != 0) {
                                int k = i + 1;
                                while (k%3 != 0) k++;
                                argv[0] = argv[k];
                                argv[1] = argv[k + 1];
                                for (int j = 2; j < count; j++) {
                                    argv[j] = 0;
                                }
                            } else {
                                for (int j = 2; j < count; j++) {
                                    argv[j] = 0;
                                }
                            }
                            if (execvp(argv[0], argv) < 0) {
                                printf(EXEC_NOT_FOUND, argv[0]);
                                exit(EXIT_FAILURE);
                            }
                        }
                        dup2(pipeEnds1[0], STDIN_FILENO);
                        close(pipeEnds1[1]);
                        close(pipeEnds1[0]);
                    }
                    for (int i = 0; i < pipeCounter + 1; i++) {
                        waitpid(pid[i], NULL, 0);
                    }
                    exit(0);
                }
                pid = 0;
                while (!pid)
                    sigsuspend(&prev);
                sigprocmask(SIG_UNBLOCK, &mask, NULL);
                wait(NULL);
            } else {
                sigset_t mask, prev;
                sigemptyset(&mask);
                sigaddset(&mask, SIGCHLD);
                //pid_t pid;
                int childStatus;
                sigprocmask(SIG_BLOCK, &mask, &prev);
                if ((pid = fork()) == 0) {
                    //sigprocmask(SIG_UNBLOCK, &mask, NULL);
                    signal(SIGTSTP, SIG_DFL);
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
                    if (execvp(argv[0], argv)) {
                        printf(EXEC_NOT_FOUND, argv[0]);
                        exit(EXIT_FAILURE);
                    }
                }
                //sigprocmask(SIG_UNBLOCK, &prev, NULL);
                childPID = pid;
                nameOfJob = argv[0];

                waitpid(pid, &childStatus, WUNTRACED);
                //setpgid(pid, pid);
                //tcsetpgrp(STDIN_FILENO, pid);
                //pid = 0;
                //while(!pid){
                    sigsuspend(&prev);
                //}

                sigprocmask(SIG_UNBLOCK, &mask, NULL);

            }
        }


        // Currently nothing is implemented
        //printf(EXEC_NOT_FOUND, input);

        // You should change exit to a "builtin" for your hw.
        //exited = strcmp(input, "exit") == 0;

        // Readline mallocs the space for input. You must free it.
        free(prompt);
        rl_free(input);
    } while(!exited);

    debug("%s", "user entered 'exit'");

    freeArray(&jobs);

    return EXIT_SUCCESS;
}

int parseLine(char *buf, char **argv) {
    char *delim;
    int argc;
    int bg;

    buf[strlen(buf) + 1] = '\0';
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
                    /*char direction[2];
                    direction[0] = buf[0];
                    direction[1] = '\0';*/
                    argv[argc++] = buf;
                    *(buf + 1) = '\0';
                    buf += 2;
                    while (*buf && (*buf == ' '))
                        buf++;
                    delim = strchr(buf, ' ');
                    argv[argc++] = buf;
                    *delim = '\0';
                    buf = delim + 1;
                    while (*buf && (*buf == ' '))
                        buf++;
                    break;
                }
            }
            delim = strchr(buf, '>');
            if (delim == NULL) delim = strchr(buf, '<');
            if (delim == NULL) delim = strchr(buf, '|');
            if (delim == NULL) {
                buf[strlen(buf) - 1] = '\0';
                argv[argc++] = buf;
                buf += strlen(buf);
            } else {
                argv[argc++] = buf;
                *delim = '\0';
                buf = delim + 1;
                while (*buf && (*buf == ' '))
                    buf++;
            }
        }
    }
    argv[argc] = NULL;

    if (argc == 0) return 1;

    if ((bg = (*argv[argc-1] == '&')) != 0) argv[--argc] = NULL;

    return argc;
}