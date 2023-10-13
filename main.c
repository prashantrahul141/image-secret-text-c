#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#define IHDR_CHUNK_TYPE 0x52444849
#define IDAT_CHUNK_TYPE 0x54414449
#define IEND_CHUNK_TYPE 0x444E4549
#define SECRET_DATA_CHUNK_TYPE

#define PNG_SIGNATURE_SIZE 8
const uint8_t PNG_SIGNATURE[PNG_SIGNATURE_SIZE] = {137, 80, 78, 71, 13, 10, 26, 10};

/// @brief returns help message string.
char *get_help_message()
{
  static char *help_message = "Storing Text in PNG Images using C. \
\nFile format supported : PNG.\n\
Arguments:\n\
\t - k : Method to use.\
\n\t - f : filename.\n";
  return help_message;
}

/// @brief  buffer from a file.
/// @param fileptr
/// @param buffer
/// @param buffer_size
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

/// @brief prints a buffer within a size limit.
/// @param buffer
/// @param size
void print_buffer_slice(uint8_t *buffer, size_t size_cap)
{
  for (size_t i = 0; i < size_cap; i++)
  {
    printf("%u ", buffer[i]);
  }
}

/// @brief Reverse bytes in a memory buffer.
/// @param buffer
/// @param size
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

int main(int argc, char *argv[])
{
  // for profiling.
  clock_t startTime, endTime;
  startTime = clock();

  // parsing command line arguments.
  int option;
  char *filename = NULL;
  while ((option = getopt(argc, argv, "hkf:")) != -1)
  {
    switch (option)
    {
    case 'h':
      printf(get_help_message());
      exit(0);

    case 'k':
      printf("k was entered.\n");
      break;

    case 'f':
      filename = optarg;
      printf("FILENAME : %s\n", filename);
      break;

    default:
      fprintf(stderr, "Could not parse these command line arguments.\n");
      break;
    }
  }

  // opening png file.
  FILE *fileptr = fopen(filename, "rb");

  // checking if file opened or not.
  if (!fileptr)
  {
    fprintf(stderr, "Could not open this file, make sure the image is in the "
                    "same directory as this executable.");
    exit(1);
  }

  // trying to read png signature.
  uint8_t signature[PNG_SIGNATURE_SIZE];
  read_buffer_from_file(fileptr, signature, sizeof(signature));

  if (memcmp(signature, PNG_SIGNATURE, 8) != 0)
  {
    printf("[ERROR] : Given image doesn\'t have the correct file signature.");
    exit(1);
  }

  bool read_buffer = true;
  while (read_buffer)
  {
    // reading length.
    uint32_t data_chunk_size;
    read_buffer_from_file(fileptr, &data_chunk_size, sizeof(data_chunk_size));
    reverse_bytes_order(&data_chunk_size, sizeof(data_chunk_size));

    // chunk type.
    uint8_t chunk_type[4];
    read_buffer_from_file(fileptr, chunk_type, sizeof(chunk_type));

    if (fseek(fileptr, data_chunk_size, SEEK_CUR) != 0)
    {
      fprintf(stderr, "Could not skip chunk");
      exit(1);
    }

    uint32_t chunk_crc;
    read_buffer_from_file(fileptr, &chunk_crc, sizeof(chunk_crc));

    printf("Chunk Size : %u\n", data_chunk_size);
    printf("Chunk Type : %.*s (0x%08X)\n", (int)sizeof(chunk_type), chunk_type, *(uint32_t *)chunk_type);
    printf("Chunk CRC : 0x%08X\n", chunk_crc);
    printf("--------------------------------------\n");

    // stop reading chunks if hit the IEND chunk which marks
    // the end of chunks according to png specification.
    if (*(uint32_t *)chunk_type == IEND_CHUNK_TYPE)
    {
      printf("Reached IEND Chunk.");
      read_buffer = false;
    }
  }
  // closes file.
  fclose(fileptr);

  // end time.
  endTime = clock();
  printf("\nTotal duration it took: %lfms\n",
         ((double)(endTime - startTime) * 1000.0) / CLOCKS_PER_SEC);

  return 0;
}