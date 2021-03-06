#include "debug.h"
#include "utf.h"
#include "wrappers.h"
#include <stdlib.h>
//#include <stdint.h>

int
main(int argc, char *argv[])
{
  int infile, outfile, in_flags, out_flags, infileInode, outfileInode, checkInode;
  struct stat file_stat;
  parse_args(argc, argv); //parse command line args. This fills infile and outfile with proper file names and sets encoding_to
  check_bom(); //check if BOM is valid also sets program_state->encoding_from
  print_state(); //print state of program
  in_flags = O_RDONLY;
  out_flags = O_WRONLY | O_CREAT;
  infile = Open(program_state->in_file, in_flags); //open stream
  outfile = Open(program_state->out_file, out_flags); //open stream
  checkInode = fstat(infile, &file_stat);
  if (checkInode < 0) {
    printf("ERROR\n");
  }
  infileInode = file_stat.st_ino;
  checkInode = fstat(outfile, &file_stat);
  if (checkInode < 0){
    printf("ERROR\n");
  }
  outfileInode = file_stat.st_ino;
  if (infileInode == outfileInode) {
    if (program_state != NULL) {
      free(program_state);
    }
    close(infile);
    close(outfile);
    exit(EXIT_FAILURE);
  }
  else {
    close(outfile);
    out_flags = O_WRONLY | O_TRUNC | O_CREAT;
    outfile = Open(program_state->out_file, out_flags);
  }
  get_encoding_function()(infile, outfile);
  if(program_state != NULL) {
    free(program_state);
  }
  close(outfile); //close outfile
  close(infile); //close infile
  return EXIT_SUCCESS;
}
