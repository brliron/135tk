#pragma once

#include <stdio.h>
#include <string.h>

FILE *TFXX_open_read(char* fn, const char *in_magic, void *header, size_t header_size);
char *TFXX_read(FILE *f, size_t comp_size, size_t uncomp_size);
FILE *TFXX_open_write(char* fn, const char *magic, const void *header, size_t header_size);
void TFXX_write(FILE *f, const char *uncomp_data, size_t uncomp_size);
