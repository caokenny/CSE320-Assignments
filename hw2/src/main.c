#include "debug.h"
#include "utf.h"
#include "wrappers.h"
#include <stdlib.h>
//#include <stdint.h>

int
main(int argc, char *argv[])
{
  int infile, outfile, in_flags, out_flags;
  //unsigned char c = 0x10;
  unsigned short s = 0xbeef;
  //unsigned int i = 0xdeadbeef;
  //unsigned long l = 0xdeadbeefdeadcafe;
  //reverse_bytes(&c, sizeof c);
  reverse_bytes(&s, sizeof s);
  //reverse_bytes(&i, sizeof i);
  //reverse_bytes(&l, sizeof l);
  parse_args(argc, argv); //parse command line args
  check_bom(); //check if BOM is valid
  print_state(); //print state of program
  in_flags = O_RDONLY;
  out_flags = O_WRONLY | O_CREAT;
  infile = Open(program_state->in_file, in_flags); //open stream
  outfile = Open(program_state->out_file, out_flags); //open stream
  lseek(SEEK_SET, program_state->bom_length, infile); /* Discard BOM */ //move pointer of file descriptor to discard BOM
  get_encoding_function()(infile, outfile);
  if(program_state != NULL) {
    close((uintptr_t)program_state);
  }
  //I think this is how this works
  close(outfile); //close outfile
  close(infile); //close infile
  return EXIT_SUCCESS;
}
