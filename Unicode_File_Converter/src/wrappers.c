#include "wrappers.h"
#include "debug.h"

void*
Malloc(size_t size)
{
  void* ret;
  if ((ret = malloc(size)) == NULL) {
    perror("Out of Memory");
    exit(EXIT_FAILURE);
  }
  return ret;
}

void*
Calloc(size_t nmemb, size_t size)
{
  void* ret;
  if ((ret = calloc(nmemb, size)) == NULL) {
    perror("Out of Memory");
    exit(EXIT_FAILURE);
  }
  return ret;
}

int
Open(char const* pathname, int flags)
{
  int fd;
  if ((fd = open(pathname, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
    perror("Could Not Open file");
    fprintf(stderr, "%s\n", pathname);
    exit(EXIT_FAILURE);
  }
  return fd;
}

ssize_t
read_to_bigendian(int fd, void* buf, size_t count)
{
  ssize_t bytes_read;

  bytes_read = read(fd, buf, count); //return bytes read from fd and store into buf
/*#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  reverse_bytes(buf, count);
#endif*/
  return bytes_read;
}

ssize_t
write_to_bigendian(int fd, void* buf, size_t count)
{
  ssize_t bytes_read;

/*#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  reverse_bytes(buf, count);
#endif*/
  bytes_read = write(fd, buf, count);
  return bytes_read;
}

void
reverse_bytes(void* bufp, size_t count)
{
  char* ptr = bufp;
  char temp;
  int i, j;
  for (i = (count - 1), j = 0; j < i; i--, j++, temp=~temp) {
    temp = ptr[j];
    ptr[j] = ptr[i];
    ptr[i] = temp;
  }
}

void
*memeset(void *s, int c, size_t n) {
  register char* stackpointer asm("esp"); //initialize stackpointer pointer with the value of the actual stackpointer
  memset(s, c, n);
  return stackpointer;
};

void
*memecpy(char *dest, char const *src, size_t n) {
  register char* stackpointer asm("esp"); //initialize stackpointer pointer with the value of the actual stackpointer
  memcpy(dest, src, n); //copy string from src to dest
  //printf("src = %s\n", *src);
  //printf("dest = %s\n", *dest);
  return stackpointer;
};
