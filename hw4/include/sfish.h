#ifndef SFISH_H
#define SFISH_H

/* Format Strings */
#define EXEC_NOT_FOUND "sfish: %s: command not found\n"
#define JOBS_LIST_ITEM "[%d] %s\n"
#define STRFTIME_RPRMT "%a %b %e, %I:%M%p"
#define BUILTIN_ERROR  "sfish builtin error: %s\n"
#define SYNTAX_ERROR   "sfish syntax error: %s\n"
#define EXEC_ERROR     "sfish exec error: %s\n"

#define HELP() do{ \
fprintf(stderr, "%s\n", \
    "help                   Displays this help message\n" \
    "exit                   Exits bash shell\n" \
    "cd                     Changes working directory of the shell\n" \
    "   cd -                Changes the directory to the last directory the user was in\n" \
    "   cd                  Goes to the users home directory\n" \
    "pwd                    Prints the absolute path of the current working directory"); \
    exit(0); \
} while(0)


#endif
