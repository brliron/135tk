#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include "TFBMTool.h"

int convert_PNG_to_TFBM(LPCWSTR png, LPCWSTR out_tfbm)
{
  FILE *in = _wfopen(png, L"rb");
  if (!in) {
    _wperror(png);
    return 0;
  }
  uint8_t sig[8];
  fread(sig, 1, 8, in);
  if (!png_check_sig(sig, 8)) {
    wprintf(L"%s: invalid PNG signature\n", png);
    fclose(in);
    return 0;
  }

  png_structp png_ptr;
  png_infop info_ptr;

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info_ptr = png_create_info_struct(png_ptr);
  if (!png_ptr || !info_ptr || setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(in);
    return 0;
  }
  png_init_io(png_ptr, in);
  png_set_sig_bytes(png_ptr, 8);
  png_read_info(png_ptr, info_ptr);

  uint32_t width, height;
  int bit_depth, color_type;
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
  if (!(
	(bit_depth == 8  && color_type == PNG_COLOR_TYPE_PALETTE) ||
	(bit_depth == 24 && color_type == PNG_COLOR_TYPE_RGB) ||
	(bit_depth == 32 && color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	))
    {
      wprintf(L"Only the following color types are supported: 8-bits with palette, 24-bits RGB, 32-bits RGBA.\n");
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      fclose(in);
      return 0;
    }

  uint32_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);
  uint8_t **row_pointers = malloc(sizeof(uint8_t*) * height);
  uint8_t *image_data = malloc(rowbytes * height);
  for (uint32_t i = 0; i < height; i++)
    row_pointers[i] = image_data + i * rowbytes;
  png_read_image(png_ptr, row_pointers);
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  fclose(in);

  if (bit_depth == 24 || bit_depth == 32) {
    // RGB to BGR, or RGBA to BGRA
    for (uint32_t i = 0; i < height; i++) {
      for (uint32_t j = 0; i < width; i++) {
	uint8_t *pixel = &row_pointers[i][j * bit_depth / 8];
	uint8_t r = pixel[2];
	uint8_t b = pixel[0];
	pixel[0] = r;
        pixel[2] = b;
      }
    }
  }

  TFBM_header header;
  header.bpp = bit_depth;
  header.width = width;
  header.height = height;
  header.padding_width = rowbytes;
  FILE *fout = TFXX_open_write(out_tfbm, "TFBM", &header, sizeof(TFBM_header));
  TFXX_write(fout, (char*)image_data, rowbytes * height);
  free(image_data);
  free(row_pointers);
  return 1;
}
