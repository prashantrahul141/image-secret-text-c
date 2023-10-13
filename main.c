#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

// to enable debug mode.
#define DEBUG
// DEBUG print macro.
#ifdef DEBUG
#define DEBUG_PRINT(x) printf x
#else
#define DEBUG_PRINT(x) \
  do                   \
  {                    \
  } while (0)
#endif

#define IHDR_CHUNK_TYPE 0x52444849
#define IDAT_CHUNK_TYPE 0x54414449
#define IEND_CHUNK_TYPE 0x444E4549
#define SECRET_DATA_CHUNK_TYPE

#define PNG_SIGNATURE_SIZE 8
const uint8_t PNG_SIGNATURE[PNG_SIGNATURE_SIZE] = {137, 80, 78, 71, 13, 10, 26, 10};

#define IDAT_CHUNK_COPY_SIZE (32 * 1024)
uint32_t idat_chunk_copy_data[IDAT_CHUNK_COPY_SIZE];

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

/// @brief read buffer from a file.
/// @param fileptr
/// @param buffer
/// @param buffer_size
void read_buffer_from_file(FILE *fileptr, void *buffer, size_t buffer_size)
{
  DEBUG_PRINT(("[READ_BUFFER_FROM_FILE] : %d\n", buffer_size));
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

/// @brief write buffer from a file.
/// @param fileptr
/// @param buffer
/// @param buffer_size
void write_buffer_to_file(FILE *fileptr, void *buffer, size_t buffer_size)
{

  DEBUG_PRINT(("[WRITE_BUFFER_FROM_FILE] : %d\n", buffer_size));
  size_t n = fwrite(buffer, buffer_size, 1, fileptr);
  if (n != 1)
  {
    if (ferror(fileptr))
    {
      fprintf(stderr, "[ERROR] : Could not read buffer.\n");
      exit(1);
    }
    else if (feof(fileptr))
    {
      fprintf(stderr, "[ERROR] : could not write buffer, reached EOF.\n");
      exit(1);
    }
    else
    {
      printf("[ERROR] : Some unknown error occured while writing buffer.\n");
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
  double startTime, endTime;
  startTime = (double)clock() / CLOCKS_PER_SEC;

  // parsing command line arguments.
  int option;
  char *filename = NULL;
  char *encoding_data = NULL;
  bool DECODING_MODE = true;
  while ((option = getopt(argc, argv, "het:df:")) != -1)
  {
    switch (option)
    {
    case 'h':
      printf(get_help_message());
      exit(0);

    case 'e':
      printf("ENCODING MODE.\n");
      DECODING_MODE = false;
      break;

    case 't':
      if (!DECODING_MODE)
      {
        encoding_data = optarg;
        printf("DATA TO BE ENCODED IN THE IMAGE : %s (%d).\n", encoding_data, strlen(encoding_data));
      }
      break;

    case 'd':
      printf("DECODING MODE.\n");
      DECODING_MODE = true;
      break;

    case 'f':
      filename = optarg;
      printf("FILENAME : %s\n", filename);
      break;

    default:
      fprintf(stderr, "[ERROR] : Could not parse these command line arguments.\n");
      exit(1);
      break;
    }
  }

  // opening png file.
  FILE *input_file_ptr = fopen(filename, "rb");
  FILE *output_file_ptr;

  if (!DECODING_MODE)
  {
    // in encoding mode.
    if (strlen(encoding_data) <= 0)
    {
      fprintf(stderr, "[ERROR] : Encoding text cannot be 0 characters long.\n");
      exit(1);
    }

    output_file_ptr = fopen("output.png", "wb");
    if (!output_file_ptr)
    {
      fprintf(stderr, "Could not create output file.\n");
      exit(1);
    }
  }

  // trying to read png signature.
  uint8_t signature[PNG_SIGNATURE_SIZE];
  read_buffer_from_file(input_file_ptr, signature, sizeof(signature));
  if (!DECODING_MODE)
  {
    write_buffer_to_file(output_file_ptr, signature, sizeof(signature));
  }

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
    read_buffer_from_file(input_file_ptr, &data_chunk_size, sizeof(data_chunk_size));

    // chunk type.
    uint8_t chunk_type[4];
    read_buffer_from_file(input_file_ptr, chunk_type, sizeof(chunk_type));

    if (!DECODING_MODE)
    {
      // writing output files in encoding mode.
      write_buffer_to_file(output_file_ptr, &data_chunk_size, sizeof(data_chunk_size));
      write_buffer_to_file(output_file_ptr, chunk_type, sizeof(chunk_type));
    }

    reverse_bytes_order(&data_chunk_size, sizeof(data_chunk_size));

    if (!DECODING_MODE)
    {
      // when in encoding mode.
      float iterations = data_chunk_size / (float)sizeof(idat_chunk_copy_data);
      uint32_t temp_data_chunk_size = data_chunk_size;
      DEBUG_PRINT(("Iterations required : %f for IDAT_SIZE : %d and DATA_CHUNK_SIZE : %d\n", iterations, sizeof(idat_chunk_copy_data), data_chunk_size));
      if (iterations > 1.0f)
      {
        // when there are more data than holding capacity.
        for (;;)
        {
          if (temp_data_chunk_size > sizeof(idat_chunk_copy_data))
          {
            read_buffer_from_file(input_file_ptr, &idat_chunk_copy_data, sizeof(idat_chunk_copy_data));
            write_buffer_to_file(output_file_ptr, &idat_chunk_copy_data, sizeof(idat_chunk_copy_data));
            temp_data_chunk_size = (size_t)(temp_data_chunk_size - (size_t)sizeof(idat_chunk_copy_data));
          }
          else
          {
            read_buffer_from_file(input_file_ptr, &idat_chunk_copy_data, temp_data_chunk_size);
            write_buffer_to_file(output_file_ptr, &idat_chunk_copy_data, temp_data_chunk_size);
            break; // just to be sure.
          }
        }
      }
      else
      {
        if (data_chunk_size != 0)
        {
          // when there are less data than holding capacity.
          DEBUG_PRINT(("writing entire chunk in once.\n"));
          read_buffer_from_file(input_file_ptr, &idat_chunk_copy_data, data_chunk_size);
          write_buffer_to_file(output_file_ptr, &idat_chunk_copy_data, data_chunk_size);
        }
      }
    }
    else
    {
      // when in decoding mode. No need to write output file we
      // can just seek the chunk.
      if (fseek(input_file_ptr, data_chunk_size, SEEK_CUR) != 0)
      {
        fprintf(stderr, "Failed to seek input file data chunk.");
        exit(1);
      }
    }

    uint32_t chunk_crc;
    read_buffer_from_file(input_file_ptr, &chunk_crc, sizeof(chunk_crc));
    if (!DECODING_MODE)
    {
      write_buffer_to_file(output_file_ptr, &chunk_crc, sizeof(chunk_crc));
    }

    DEBUG_PRINT(("--------------------------------------\n"));
    DEBUG_PRINT(("Chunk Data Size : %u\n", data_chunk_size));
    DEBUG_PRINT(("Chunk Type : %.*s (0x%08X)\n", (int)sizeof(chunk_type), chunk_type, *(uint32_t *)chunk_type));
    DEBUG_PRINT(("Chunk CRC : 0x%08X\n", chunk_crc));
    DEBUG_PRINT(("--------------------------------------\n"));

    // stop reading chunks if hit the IEND chunk which marks
    // the end of chunks according to png specification.
    if (*(uint32_t *)chunk_type == IEND_CHUNK_TYPE)
    {
      DEBUG_PRINT(("Reached IEND Chunk.\n"));
      read_buffer = false;
    }
  }

  // closes file.
  fclose(input_file_ptr);
  if (!DECODING_MODE)
  {
    fclose(output_file_ptr);
  }

  // end time.
  endTime = (double)clock() / CLOCKS_PER_SEC;

  printf("\nTotal duration it took: %lfms\n",
         endTime - startTime);

  return 0;
}