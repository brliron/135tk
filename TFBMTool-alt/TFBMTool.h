#ifndef TFBMTOOL_H_
# define TFBMTOOL_H_

# include <Windows.h>
# include <stdio.h>
# include <stdint.h>

#pragma pack(push, 1)
typedef struct {
  uint8_t bpp;
  uint32_t width;
  uint32_t height;
  uint32_t padding_width;
  uint32_t comp_size;
} TFBM_header;
#pragma pack(pop)

FILE *TFXX_open_read(LPCWSTR fn, const char *in_magic, void *header, size_t header_size);
char *TFXX_read(FILE *f, size_t comp_size, size_t uncomp_size);
FILE *TFXX_open_write(LPCWSTR fn, const char *magic, const void *header, size_t header_size);
void TFXX_write(FILE *f, const char *uncomp_data, size_t uncomp_size);
int convert_TFBM_to_PNG(LPCWSTR tfbm, LPCWSTR tfpa, LPCWSTR out_png);
int convert_PNG_to_TFBM(LPCWSTR png, LPCWSTR out_tfbm);

#endif /* !TFBMTOOL_H_ */
