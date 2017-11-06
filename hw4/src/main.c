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

int parseLine(char *buf, char **argv, char *buf2, char *buf3, char *buf4);

volatile sig_atomic_t pid;
void sigchld_handler(int s) {
}

void sigint_handler(int s){
}

volatile pid_t childPID = (pid_t) -1;
void sigtstp_handler(int s) {
    insertElement(&jobs, (int)childPID);
    strcpy(jobNames[jobs.used-1], nameOfJob);
}

int main(int argc, char *argv[], char* envp[]) {
    initArray(&jobs, 1);
    signal(SIGCHLD, sigchld_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    sigset_t shellMask, shellPrev;
    sigemptyset(&shellMask);
    sigaddset(&shellMask, SIGTSTP);
    sigaddset(&shellMask, SIGINT);
    sigprocmask(SIG_BLOCK, &shellMask, &shellPrev);
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
        char *buf2 = NULL, *buf3 = NULL, *buf4 = NULL;
        int count = parseLine(input, argv, buf2, buf3, buf4);
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
                char *delim;
                if ((delim = strchr(argv[1], ' ')) != NULL) {
                    *delim = '\0';
                }
                setenv("PPATH", cwd, 1);
                chdir(argv[1]);
            }
        }
        else if (strstr(input, "kill") == input) {
            if (argv[1] == NULL) {
                free(prompt);
                free(input);
                continue;
            }
            if (strchr(argv[1], '%') != NULL) {
                char *buf = argv[1];
                buf++;
                int killPID = atoi(buf);
                int jobID = killPID;
                killPID = jobs.array[killPID];
                kill(killPID, SIGKILL);
                waitpid(killPID, NULL, 0);
                jobs.array[jobID] = 0;
            } else {
                int killPID;
                killPID = atoi(argv[1]);
                kill(killPID, SIGKILL);
                waitpid(killPID, NULL, 0);
                for (int i = 0; i < jobs.size; i++) {
                    if (jobs.array[i] == killPID) {
                        jobs.array[i] = 0;
                    }
                }
            }
        }
        else if (strstr(input, "jobs") == input) {
            for (int i = 0; i < jobs.size; i++) {
                if (jobs.array[i] != 0)
                    printf(JOBS_LIST_ITEM, i, jobNames[i]);
            }
        }
        else if (strstr(input, "fg") == input) {
            if (argv[1] == NULL) {
                free(prompt);
                free(input);
                continue;
            }
            char *buf = argv[1];
            buf++;
            int contPID = atoi(buf);
            contPID = jobs.array[contPID];
            kill(contPID, SIGCONT);
        }
        else {
            if (pipeCounter != 0) {
                sigset_t mask, prev;
                sigemptyset(&mask);
                sigaddset(&mask, SIGCHLD);
                sigprocmask(SIG_BLOCK, &mask, &prev);
                sigprocmask(SIG_UNBLOCK, &shellMask, NULL);
                int childStatus;
                if ((pid = fork()) == 0) {
                    signal(SIGTSTP, SIG_DFL);
                    sigset_t mask, prev;
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
                                while (strcmp(argv[k], "|") != 0) k++;
                                k++;
                                argv[0] = argv[k];
                                argv[1] = argv[k + 1];
                                for (int j = 2; j < count; j++) {
                                    argv[j] = 0;
                                }
                            } else {
                                for (int j = 2; j < count; j++) {
                                    if (strcmp(argv[j], "|") == 0) {
                                        argv[j] = 0;
                                        j++;
                                        while (argv[j] != NULL) {
                                            argv[j] = 0;
                                            j++;
                                        }
                                        break;
                                    }
                                }
                            }
                            if (execvp(argv[0], argv) < 0) {
                                printf("sfish exec error: %s: command not found\n", argv[0]);
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
                waitpid(pid, &childStatus, 0);
                sigsuspend(&prev);
                sigprocmask(SIG_UNBLOCK, &mask, NULL);
                sigprocmask(SIG_BLOCK, &shellMask, &shellPrev);
            } else {
                sigset_t mask, prev;
                sigemptyset(&mask);
                sigaddset(&mask, SIGCHLD);
                int childStatus;
                sigprocmask(SIG_BLOCK, &mask, &prev);
                sigprocmask(SIG_UNBLOCK, &shellMask, NULL);
                if ((pid = fork()) == 0) {
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
                                if (strchr(argv[j], '-') == NULL)
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
                                if (strchr(argv[j], '-') == NULL)
                                    argv[j] = 0;
                            }
                            break;
                        }
                    }
                    int redirectionCount = 0;
                    for (int i = 0; i < count - 2; i++) {
                        if (strcmp(argv[i], "<") == 0 || strcmp(argv[i], ">") == 0)
                            redirectionCount++;
                    }
                    if (redirectionCount > 2) printf(SYNTAX_ERROR, "sequence of redirects not supported");
                    else {
                        if (execvp(argv[0], argv)) {
                            printf(EXEC_NOT_FOUND, argv[0]);
                            exit(EXIT_FAILURE);
                        }
                    }
                }
                childPID = pid;
                nameOfJob = argv[0];

                waitpid(pid, &childStatus, WUNTRACED);
                sigsuspend(&prev);

                sigprocmask(SIG_UNBLOCK, &mask, NULL);
                sigprocmask(SIG_BLOCK, &shellMask, &shellPrev);

            }
        }

        // Readline mallocs the space for input. You must free it.
        free(prompt);
        rl_free(input);

        if (buf2 != NULL) free(buf2);
        if (buf3 != NULL) free(buf3);
        if (buf4 != NULL) free(buf4);
    } while(!exited);

    debug("%s", "user entered 'exit'");

    freeArray(&jobs);

    return EXIT_SUCCESS;
}

int parseLine(char *buf, char **argv, char *buf2, char *buf3, char *buf4) {
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
        }
        else if (buf[0] == '-') {
            delim = strchr(buf, ' ');
            argv[argc++] = buf;
            *delim = '\0';
            buf = delim + 1;
            while (*buf && (*buf == ' '))
                buf++;
        }
        else {

            for (int i = 0; i < strlen(buf); i++) {
                if ((buf[i] == '<' || buf[i] == '>' || buf[i] == '|') && i == 0) {
                    if (*(buf + 1) != ' ') {
                        buf2 = (char*)malloc(strlen(buf) + 1);
                        strcpy(buf2, buf);
                        delim = strchr(buf2, buf[i]);
                        argv[argc++] = buf2;
                        delim++;
                        *delim = '\0';
                        buf++;
                        while(*buf && (*buf == ' '))
                            buf++;
                        break;
                    } else {
                        delim = strchr(buf, buf[i]);
                        argv[argc++] = buf;
                        delim++;
                        *delim = '\0';
                        buf = delim + 1;
                        while (*buf && (*buf == ' '))
                            buf++;
                        break;
                    }
                }
                else if (buf[i] == '<' || buf[i] == '>' || buf[i] == '|') {
                    delim = strchr(buf, buf[i]);
                    if (*(delim - 1) != ' ') {
                        buf3 = (char*)malloc(strlen(buf) + 1);
                        strcpy(buf3, buf);
                        delim = strchr(buf3, buf[i]);
                        argv[argc++] = buf3;
                        *delim = '\0';
                        delim = strchr(buf, buf[i]);
                        buf = delim;
                        while (*buf && (*buf == ' '))
                            buf++;
                    } else {
                        argv[argc++] = buf;
                        delim--;
                        *delim = '\0';
                        buf = delim + 1;
                        while (*buf && (*buf == ' '))
                            buf++;
                    }
                    if (*(buf + 1) != ' ') {
                        buf4 = (char*)malloc(strlen(buf) + 1);
                        strcpy(buf4, buf);
                        argv[argc++] = buf4;
                        *(buf4 + 1) = '\0';
                        buf++;
                        while (*buf && (*buf == ' '))
                            buf++;
                    } else {
                        argv[argc++] = buf;
                        *(buf + 1) = '\0';
                        buf += 2;
                        while (*buf && (*buf == ' '))
                            buf++;
                    }
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
                delim = strchr(buf, ' ');
                if (*(delim + 1) == '\0') {
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
