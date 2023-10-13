#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

/// @brief  buffer from a file.
/// @param fileptr
/// @param buffer
/// @param buffer_size
void read_buffer_from_file(FILE *fileptr, void *buffer, size_t buffer_size);

/// @brief prints a buffer within a size limit.
/// @param buffer
/// @param size
void print_buffer_slice(uint8_t *buffer, size_t size);

/// @brief returns help message string.
char *get_help_message();

/// @brief Reverse bytes in a memory buffer.
/// @param buffer
/// @param size
void reverse_bytes_order(void *buffer, size_t size);