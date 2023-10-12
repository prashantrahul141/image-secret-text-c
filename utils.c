#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

char *get_help_message()
{
  static char *help_message = "Storing Text in PNG Images using C. \
\nFile format supported : PNG.\n\
Arguments:\n\
\t - k : Method to use.\
\n\t - f : filename.\n";
  return help_message;
}

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

void print_buffer_slice(uint8_t *buffer, size_t size)
{
  for (size_t i = 0; i < size; i++)
  {
    printf("%u ", buffer[i]);
  }
}