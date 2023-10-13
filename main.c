#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "utils.h"

#define PNG_SIGNATURE_SIZE 8
const uint8_t PNG_SIGNATURE[PNG_SIGNATURE_SIZE] = {137, 80, 78, 71, 13, 10, 26, 10};

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
      return 0;

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
  }
  // closes file.
  fclose(fileptr);

  // end time.
  endTime = clock();
  printf("\nTotal duration it took: %lfms\n",
         ((double)(endTime - startTime) * 1000.0) / CLOCKS_PER_SEC);

  return 0;
}