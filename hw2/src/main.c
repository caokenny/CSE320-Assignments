#include "debug.h"
#include "utf.h"
#include "wrappers.h"
#include <stdlib.h>
//#include <stdint.h>

int
main(int argc, char *argv[])
{
  int infile, outfile, in_flags, out_flags;
  code_point_t cp1 = 0xc3a9;
  utf8_glyph_t g1;
  g1 = utf8_two_byte_encode(cp1);
  utf8_glyph_t compare;
  compare.bytes[1].byte = 0xa9;
  compare.bytes[0].byte = 0xc3;
  int x = memcmp(&g1, &compare, 2);
  printf("%d\n", x);
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
