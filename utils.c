#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

char *get_help_message()
{
  static char *help_message = "Storing Text in PNG Images using C. \
\nFile format supported : PNG.\n\
Arguments:\n\
\t - k : Method to use.\
\n\t - f : filename.\n";
  return help_message;
}

void read_buffer_from_file(FILE *fileptr, void *buffer, size_t buffer_size)
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

void print_buffer_slice(uint8_t *buffer, size_t size_cap)
{
  for (size_t i = 0; i < size_cap; i++)
  {
    printf("%u ", buffer[i]);
  }
}

void reverse_bytes_order(void *buffer_, size_t size_cap)
{
  uint8_t *buffer = buffer_;
  for (size_t i = 0; i < size_cap / 2; i++)
  {
    uint8_t t = buffer[i];
    buffer[i] = buffer[size_cap - i - 1];
    buffer[size_cap - i - 1] = t;
  }
}