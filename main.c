/*
   Original Repo : https://github.com/prashantrahul141/image-secret-text-c
   The PNG Specification : https://www.rfc-editor.org/rfc/rfc2083
*/

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// to enable debug mode.
// #define DEBUG

// DEBUG print macro.
#ifdef DEBUG
#define DEBUG_PRINT(x) printf x
#else
#define DEBUG_PRINT(x)                                                         \
  do {                                                                         \
  } while (0)
#endif

#define IHDR_CHUNK_TYPE 0x52444849
#define IDAT_CHUNK_TYPE 0x54414449
#define IEND_CHUNK_TYPE 0x444E4549
#define CRC_POLYNOMIAL 0x104C11DB7
#define LARGEST_32_BIT_VALUE 0xFFFFFFFF
char *SECRET_CHUNK_TYPE = "saNS";

#define PNG_SIGNATURE_SIZE 8
const uint8_t PNG_SIGNATURE[PNG_SIGNATURE_SIZE] = {137, 80, 78, 71,
                                                   13,  10, 26, 10};

uint32_t crc_table[256];

#define IDAT_CHUNK_COPY_SIZE (32 * 1024)
uint32_t idat_chunk_copy_data[IDAT_CHUNK_COPY_SIZE];

/// @brief returns help message string.
void get_help_message() {
  static char *help_message = "Storing Text in PNG Images using C. \
\nFile format supported : PNG.\n\
Arguments:\n\
\t - f : filename. takes <file path> as paramter.\n\
\t - e : Encode text from the given file.\n\
\t - d : Decode text in the given file.\n\
\t - t : Text which will be encoded in the image. takes <text> as paramters.";

  printf("%s", help_message);
}

/// @brief helper function to print bits of a char.
/// @param num
void print_bits_of_byte(void *p) {
  unsigned char *q = p;
  for (int iBit = CHAR_BIT; iBit > 0; --iBit) {
    // disable warning for bit variable.
#pragma GCC diagnostic ignored "-Wunused-variable"
    int bit = q[0] >> (iBit - 1);
    DEBUG_PRINT(("%c", '0' + (bit & 1)));
  }
}

/// @brief helper function to print bits of a mem buff.
/// @param unsigned int num
void print_bits(void *buff, size_t len) {
  char *buff_local = buff;
  for (int byte = 0; byte < len; byte++) {
    // DEBUG_PRINT(("(%d)", *buff_local));
    print_bits_of_byte(buff_local);
    DEBUG_PRINT((" "));
    buff_local++;
  }
  DEBUG_PRINT(("\n\n"));
}

/// @brief read buffer from a file.
/// @param fileptr
/// @param buffer
/// @param buffer_size
void read_buffer_from_file(FILE *fileptr, void *buffer, size_t buffer_size) {
  DEBUG_PRINT(("[READ_BUFFER_FROM_FILE] : %zu bytes\n", buffer_size));
  size_t n = fread(buffer, buffer_size, 1, fileptr);
  if (n != 1) {
    if (ferror(fileptr)) {
      fprintf(stderr, "[ERROR] : Could not read buffer.\n");
      exit(1);
    } else if (feof(fileptr)) {
      fprintf(stderr, "[ERROR] : could not read buffer, reached EOF.\n");
      exit(1);
    } else {
      printf("[ERROR] : Some unknown error occured while reading buffer.\n");
      exit(1);
    }
  }
}

/// @brief write buffer from a file.
/// @param fileptr
/// @param buffer
/// @param buffer_size
void write_buffer_to_file(FILE *fileptr, void *buffer, size_t buffer_size) {
  DEBUG_PRINT(("[WRITE_BUFFER_FROM_FILE] : %zu bytes\n", buffer_size));
  size_t n = fwrite(buffer, buffer_size, 1, fileptr);
  if (n != 1) {
    if (ferror(fileptr)) {
      fprintf(stderr, "[ERROR] : Could not read buffer.\n");
      exit(1);
    } else if (feof(fileptr)) {
      fprintf(stderr, "[ERROR] : could not write buffer, reached EOF.\n");
      exit(1);
    } else {
      printf("[ERROR] : Some unknown error occured while writing buffer.\n");
      exit(1);
    }
  }
}

/// @brief prints a buffer within a size limit.
/// @param buffer
/// @param size
void print_buffer_slice(char *buffer, size_t size_cap) {
  for (size_t i = 0; i < size_cap; i++) {
    printf("%c", buffer[i]);
  }
}

/// @brief Reverse bytes in a memory buffer.
/// @param buffer
/// @param size
void reverse_bytes_order(void *buffer_, size_t size_cap) {
  uint8_t *buffer = buffer_;
  for (size_t i = 0; i < size_cap / 2; i++) {
    uint8_t t = buffer[i];
    buffer[i] = buffer[size_cap - i - 1];
    buffer[size_cap - i - 1] = t;
  }
}

/// @brief Prints decoded result.
/// @param buffer
void print_decoded_result(char *buffer, size_t buffer_size) {
  printf("DECODED TEXT : ");
  print_buffer_slice(buffer, buffer_size);
  printf("\n\n");
}

/// @brief reverse bits of a single byte.
/// @param n
uint8_t reverse_bits_char(uint8_t b) {
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4; // 0xF0 : 11110000 ; 0x0F : 00001111
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2; // 0xCC : 11001100 ; 0x33 : 00110011
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1; // 0xAA : 10101010 ; 0x55 : 01010101
  return b;
}

/// @brief precomputes crc table.
void make_crc_table() {
  for (int32_t n = 0; n < 256; n++) {
    uint32_t c = (unsigned long)n;
    for (int32_t k = 0; k < 8; k++) {
      if (c & 1)
        c = 0xedb88320L ^ (c >> 1);
      else
        c = c >> 1;
    }
    crc_table[n] = c;
  }
}

/// @brief update a running CRC with the bytes buf[0..len-1]
/// the crc should be init at all 1's
/// @param crc
/// @param buf
/// @param len
uint32_t update_crc(uint32_t crc, uint8_t *buf, int32_t len) {
  uint32_t c = crc;
  for (size_t n = 0; n < len; n++) {
    c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
  }
  return c;
}

/// @brief looks up crc and returns it
/// @param buf
/// @param len
uint32_t crc(uint8_t *buf, int32_t len) {
  return update_crc(0xFFFFFFFFL, buf, len) ^ 0xFFFFFFFFL;
}

// ENTRY POINT.
int main(int argc, char *argv[]) {
  if (argc <= 1) {
    get_help_message();
    exit(0);
  }

  // for profiling.
  double startTime, endTime;
  startTime = (double)clock() / CLOCKS_PER_SEC;

  // parsing command line arguments.
  int option;
  char *filename = NULL;
  char *encoding_data = NULL;
  bool DECODING_MODE = true;
  while ((option = getopt(argc, argv, "het:df:")) != -1) {
    switch (option) {
    case 'h':
      get_help_message();
      exit(0);

    case 'e':
      printf("ENCODING MODE.\n");
      DECODING_MODE = false;
      break;

    case 't':
      if (!DECODING_MODE) {
        encoding_data = optarg;
        printf("DATA TO BE ENCODED IN THE IMAGE : %s (%li bytes).\n",
               encoding_data, strlen(encoding_data));
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
      fprintf(stderr,
              "[ERROR] : Could not parse these command line arguments.\n");
      exit(1);
      break;
    }
  }

  // opening png file.
  FILE *input_file_ptr = fopen(filename, "rb");
  FILE *output_file_ptr;

  if (!input_file_ptr) {
    fprintf(stderr, "Could not open input file. exiting.\n");
    exit(1);
  }

  if (!DECODING_MODE) {
    // in encoding mode.
    if (strlen(encoding_data) <= 0) {
      fprintf(stderr, "[ERROR] : Encoding text cannot be 0 characters long.\n");
      exit(1);
    }

    output_file_ptr = fopen("output.png", "wb");
    if (!output_file_ptr) {
      fprintf(stderr, "Could not create output file.\n");
      exit(1);
    }

    // precompute crc table
    make_crc_table();
  }

  // trying to read png signature.
  uint8_t signature[PNG_SIGNATURE_SIZE];
  read_buffer_from_file(input_file_ptr, signature, sizeof(signature));
  if (!DECODING_MODE) {
    write_buffer_to_file(output_file_ptr, signature, sizeof(signature));
  }

  if (memcmp(signature, PNG_SIGNATURE, 8) != 0) {
    printf("[ERROR] : Given image doesn\'t have the correct file signature.");
    exit(1);
  }

  bool read_buffer = true;
  bool done_encoding = false;
  bool found_secret_chunk = false;
  while (read_buffer) {
    // reading length.
    uint32_t data_chunk_size;
    read_buffer_from_file(input_file_ptr, &data_chunk_size,
                          sizeof(data_chunk_size));

    // chunk type.
    uint8_t chunk_type[4];
    read_buffer_from_file(input_file_ptr, chunk_type, sizeof(chunk_type));

    if (!DECODING_MODE) {
      // writing output files in encoding mode.
      write_buffer_to_file(output_file_ptr, &data_chunk_size,
                           sizeof(data_chunk_size));
      write_buffer_to_file(output_file_ptr, chunk_type, sizeof(chunk_type));
    }

    reverse_bytes_order(&data_chunk_size, sizeof(data_chunk_size));

    if (!DECODING_MODE) {
      // when in encoding mode.
      float iterations = data_chunk_size / (float)sizeof(idat_chunk_copy_data);
      uint32_t temp_data_chunk_size = data_chunk_size;
      DEBUG_PRINT(("Iterations required : %f for IDAT_SIZE : %lu and "
                   "DATA_CHUNK_SIZE : %d\n",
                   iterations, sizeof(idat_chunk_copy_data), data_chunk_size));
      if (iterations > 1.0f) {
        // when there are more data than holding capacity.
        while (true) {
          if (temp_data_chunk_size > sizeof(idat_chunk_copy_data)) {
            read_buffer_from_file(input_file_ptr, &idat_chunk_copy_data,
                                  sizeof(idat_chunk_copy_data));
            write_buffer_to_file(output_file_ptr, &idat_chunk_copy_data,
                                 sizeof(idat_chunk_copy_data));
            temp_data_chunk_size =
                (size_t)(temp_data_chunk_size -
                         (size_t)sizeof(idat_chunk_copy_data));
          } else {
            read_buffer_from_file(input_file_ptr, &idat_chunk_copy_data,
                                  temp_data_chunk_size);
            write_buffer_to_file(output_file_ptr, &idat_chunk_copy_data,
                                 temp_data_chunk_size);
            break; // just to be sure.
          }
        }
      } else {
        if (data_chunk_size != 0) {
          // when there are less data than holding capacity.
          DEBUG_PRINT(("writing entire chunk in once.\n"));
          read_buffer_from_file(input_file_ptr, &idat_chunk_copy_data,
                                data_chunk_size);
          write_buffer_to_file(output_file_ptr, &idat_chunk_copy_data,
                               data_chunk_size);
        }
      }
    } else {
      // decoding acutal text.
      if (*(char *)chunk_type == *SECRET_CHUNK_TYPE) {
        DEBUG_PRINT(("Chunk found.\n"));
        found_secret_chunk = true;
        char *secret_data_buffer = (char *)malloc(data_chunk_size);
        read_buffer_from_file(input_file_ptr, secret_data_buffer,
                              data_chunk_size);
        print_decoded_result(secret_data_buffer, data_chunk_size);
        free(secret_data_buffer);
        fclose(input_file_ptr);
        exit(0);
      } else {
        // when decoding if the chunk type is not saNS we can just skip it.
        if (fseek(input_file_ptr, data_chunk_size, SEEK_CUR) != 0) {
          fprintf(stderr, "Failed to seek input file data chunk.");
          exit(1);
        }
      }
    }

    uint32_t chunk_crc;
    read_buffer_from_file(input_file_ptr, &chunk_crc, sizeof(chunk_crc));

    // secret chunk stuff.
    if (!DECODING_MODE) {
      write_buffer_to_file(output_file_ptr, &chunk_crc, sizeof(chunk_crc));

      // writing secret text to output image.
      if (*(uint32_t *)chunk_type == IDAT_CHUNK_TYPE && !done_encoding) {
        printf("ENCODING SECRET CHUNK.\n");

        // writing  length chunk.
        uint32_t secret_chunk_size = strlen(encoding_data);
        reverse_bytes_order(&secret_chunk_size, sizeof(secret_chunk_size));
        write_buffer_to_file(output_file_ptr, &secret_chunk_size,
                             sizeof(secret_chunk_size));
        reverse_bytes_order(&secret_chunk_size, sizeof(secret_chunk_size));

        // writing  type chunk.
        write_buffer_to_file(output_file_ptr, SECRET_CHUNK_TYPE, 4);

        // writing  data chunk.
        write_buffer_to_file(output_file_ptr, encoding_data, secret_chunk_size);

        // crc of the data.
        // combining type and data chunks.
        uint32_t new_buffer_size =
            strlen(SECRET_CHUNK_TYPE) + strlen(encoding_data);
        uint8_t *new_buffer = (uint8_t *)malloc(new_buffer_size);
        strcpy((char *)new_buffer, SECRET_CHUNK_TYPE);
        strcat((char *)new_buffer, encoding_data);

        // crc of the new buffer ( chunk type + chunk data)
        uint32_t secret_chunk_crc = crc(new_buffer, new_buffer_size);
        free(new_buffer);

        // reverse bytes.
        reverse_bytes_order(&secret_chunk_crc, sizeof(secret_chunk_crc));

        write_buffer_to_file(output_file_ptr, &secret_chunk_crc,
                             sizeof(secret_chunk_crc));

        done_encoding = true;
      }
    }

    // printing some info about the current chunk.
    DEBUG_PRINT(("--------------------------------------\n"));
    DEBUG_PRINT(("Chunk Data Size : %u\n", data_chunk_size));
    DEBUG_PRINT(("Chunk Type : %.*s (0x%08X)\n", (int)sizeof(chunk_type),
                 chunk_type, *(uint32_t *)chunk_type));
    DEBUG_PRINT(("Chunk CRC : 0x%08X\n", chunk_crc));
    DEBUG_PRINT(("--------------------------------------\n"));

    // stop reading chunks if hit the IEND chunk which marks
    // the end of chunks according to png specification.
    if (*(uint32_t *)chunk_type == IEND_CHUNK_TYPE) {
      DEBUG_PRINT(("Reached IEND Chunk.\n"));
      read_buffer = false;
    }
  }

  // closes file.
  fclose(input_file_ptr);
  if (!DECODING_MODE) {
    printf("ENCODING COMPLETE.\n");
    fclose(output_file_ptr);
  } else {
    if (!found_secret_chunk) {
      printf("Could\'nt found any secret chunk in the given image, make sure "
             "the image you\'re providing does have a secret text created "
             "using this program.\n");
    }
  }

  // end time.
  endTime = (double)clock() / CLOCKS_PER_SEC;
  printf("Total duration it took: %lfs\n", endTime - startTime);

  return 0;
}
