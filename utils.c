#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

void read_buffer_from_file(FILE *fileptr, uint8_t *buffer, size_t buffer_size)
{
  size_t n = fread(buffer, buffer_size, 1, fileptr);
  if (n != 1)
  {
    if (ferror(fileptr))
    {
      fprintf(stderr, "[ERROR] : Could not read buffer.\n");
      exit(1);
    }
    else if (feof(fileptr))
    {
      fprintf(stderr, "[ERROR] : could not read buffer, reached EOF.\n");
      exit(1);
    }
    else
    {
      printf("[ERROR] : Some unknown error occured while reading buffer.\n");
      exit(1);
    }
  }
}