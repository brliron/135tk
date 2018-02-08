#ifndef TFBMTOOL_H_
# define TFBMTOOL_H_

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

FILE *TFXX_open(const char *fn, const char *in_magic, void *header, size_t header_size);
char *TFXX_read(FILE *f, size_t comp_size, size_t uncomp_size);
int convert_TFBM_to_PNG(const char *tfbm, const char *tfpa, const char *out_png);

#endif /* !TFBMTOOL_H_ */
