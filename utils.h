#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

void read_buffer_from_file(FILE *fileptr, uint8_t *buffer, size_t buffer_size);

void print_buffer_slice(uint8_t *buffer, size_t size);
char *get_help_message();
