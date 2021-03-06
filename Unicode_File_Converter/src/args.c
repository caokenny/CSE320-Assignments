#include "debug.h"
#include "utf.h"
#include "wrappers.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *STR_UTF16BE  = "UTF16BE";
const char *STR_UTF16LE = "UTF16LE";
const char *STR_UTF8  = "UTF8";

int opterr;
int optopt;
int optind;
char *optarg;

state_t *program_state;

void
parse_args(int argc, char *argv[])
{
  int i;
  int wrongFormat = 0;
  char option;
  char *joined_argv;

  joined_argv = join_string_array(argc, argv); //joined_argv = charArray base address
  info("argc: %d argv: %s", argc, joined_argv);
  free(joined_argv); //deallocate memory allocated by charArray

  program_state = Calloc(1, sizeof(state_t)); //Allocate memory for 1 element of size(state_t)
  for (i = 0; optind < argc; ++i) {
    debug("%d opterr: %d", i, opterr);
    debug("%d optind: %d", i, optind);
    debug("%d optopt: %d", i, optopt);
    debug("%d argv[optind]: %s", i, argv[optind]);
    if ((option = getopt(argc, argv, "+e:")) != -1) { //get command line arguments
      switch (option) {
        case 'e': {
          info("Encoding Argument: %s", optarg); //if e do this
          if (optarg[1] == 104) {
            USAGE(argv[0]);
            if (program_state != NULL) {
                free(program_state);
            }
            exit(EXIT_SUCCESS);
          }
          if ((program_state->encoding_to = determine_format(optarg)) == 0){
            wrongFormat = 1;
            print_state();
          }
          break;
        }
        case '?': {
          if (optopt != 'h'){
            fprintf(stderr, KRED "-%c is not a supported argument\n" KNRM, optopt);
            if(program_state != NULL) {
                free(program_state);
            }
            exit(EXIT_FAILURE);
          }
          else {
            USAGE(argv[0]);
            if(program_state != NULL) {
                free(program_state);
            }
            exit(EXIT_SUCCESS);
          }
        }
        default: {
          break;
        }
      }
    }
    elsif(argv[optind] != NULL) //if argv[optind] != null
    {
      if (optind < 3) {
        free(program_state);
        exit(EXIT_FAILURE);
      }
      if (program_state->in_file == NULL) { //(*program_state).in_file
        program_state->in_file = argv[optind]; //(*program_state).in_file = argv[optind]
      }
      elsif(program_state->out_file == NULL)
      {
        program_state->out_file = argv[optind];
      }
      optind++;
    }
  }
  if (program_state->in_file == NULL || program_state->out_file == NULL || wrongFormat == 1 || argc == 1 || argc != 5) {
    if(program_state != NULL) {
        free(program_state);
    }
    exit(EXIT_FAILURE);
  }
}

format_t
determine_format(char *argument)
{
  if (strcmp(argument, STR_UTF16LE) == 0)
    return UTF16LE;
  if (strcmp(argument, STR_UTF16BE) == 0)
    return UTF16BE;
  if (strcmp(argument, STR_UTF8) == 0)
    return UTF8;
  return 0;
}

char*
bom_to_string(format_t bom){
  switch(bom){
    case UTF8: return "STR_UTF8";
    case UTF16BE: return "STR_UTF16BE";
    case UTF16LE: return "STR_UTF16LE";
  }
  return "UNKNOWN";
}

char*
join_string_array(int count, char *array[])
{
  char *ret; //char pointer ret
  int i;
  int len = 0, str_len, cur_str_len;

  str_len = array_size(count, array); //returns size of the array
  ret = (char*) malloc(str_len);

  for (i = 0; i < count; ++i) {
    cur_str_len = strlen(array[i]);
    memecpy(ret + len, array[i], cur_str_len); //copy string from array[i] to charArray[i]
    len += cur_str_len;
    memecpy(ret + len, " ", 1); //copy empty space into charArray[i]
    len += 1;
  }
  return ret; //return pointer to charArray
}

int
array_size(int count, char *array[])
{
  int i, sum = 1; /* NULL terminator */
  for (i = 0; i < count; ++i) {
    sum += strlen(array[i]);
    ++sum; /* For the spaces */
  }
  return sum;
}

void
print_state()
{
//errorcase:
  if (program_state == NULL) {
    error("program_state is %p", (void*)program_state);
    exit(EXIT_FAILURE);
  }
  info("program_state {\n"
         "  format_t encoding_to = 0x%X;\n"
         "  format_t encoding_from = 0x%X;\n"
         "  char *in_file = '%s';\n"
         "  char *out_file = '%s';\n"
         "};\n",
         program_state->encoding_to, program_state->encoding_from,
         program_state->in_file, program_state->out_file);
}
