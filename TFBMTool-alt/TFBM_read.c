#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include "TFBMTool.h"

int convert_TFBM_to_PNG(LPCWSTR tfbm, LPCWSTR tfpa, LPCWSTR out_png)
{
  png_structp png_ptr;
  png_infop info_ptr;

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info_ptr = png_create_info_struct(png_ptr);
  if (!png_ptr || !info_ptr || setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return 0;
  }

  TFBM_header header;
  char *data = NULL;
  {
    FILE *f = TFXX_open_read(tfbm, "TFBM", &header, sizeof(header));
    data = TFXX_read(f, header.comp_size, header.padding_width * header.height * header.bpp / 8);
    if (!data) {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return 0;
    }
  }

  png_color *palette = NULL;
  png_byte *tRNS = NULL;
  if (header.bpp == 8) {
    if (!tfpa) {
      fwprintf(stderr, L"Error: no palette given for a 8-bits with palette TFBM image.\n");
      png_destroy_write_struct(&png_ptr, &info_ptr);
      free(data);
      return 0;
    }

    uint32_t comp_size;
    FILE *f = TFXX_open_read(tfpa, "TFPA", &comp_size, sizeof(comp_size));
    char *plt_data = TFXX_read(f, comp_size, 256 * 2 + 256 * 4);
    if (!plt_data) {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      free(data);
      return 0;
    }

    palette = png_malloc(png_ptr, 256 * sizeof(png_color));
    tRNS    = png_malloc(png_ptr, 256);
    for (int i = 0; i < 256; i++) {
      palette[i].red   = plt_data[256 * 2 + i * 4 + 2];
      palette[i].green = plt_data[256 * 2 + i * 4 + 1];
      palette[i].blue  = plt_data[256 * 2 + i * 4 + 0];
      tRNS[i]          = plt_data[256 * 2 + i * 4 + 3];
    }

    free(plt_data);
  }

  FILE *out = _wfopen(out_png, L"wb");
  if (!out) {
    _wperror(out_png);
    if (palette) png_free(png_ptr, palette);
    if (tRNS) png_free(png_ptr, tRNS);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    free(data);
    return 0;
  }
  png_init_io(png_ptr, out);

  int color_type;
  if (header.bpp == 8) {
    color_type = PNG_COLOR_TYPE_PALETTE;
  }
  else if (header.bpp == 24) {
    color_type = PNG_COLOR_TYPE_RGB;
  }
  else if (header.bpp == 32) {
    color_type = PNG_COLOR_TYPE_RGB_ALPHA;
  }
  else {
    fwprintf(stderr, L"%s: unsupported bit depth: %d (only 8, 24 and 32 are supported).\n", out_png, header.bpp);
    if (palette) png_free(png_ptr, palette);
    if (tRNS) png_free(png_ptr, tRNS);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    free(data);
    return 0;
  }
  png_set_IHDR(png_ptr, info_ptr, header.width, header.height, 8, color_type,
	       PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  if (header.bpp == 8) {
    png_set_PLTE(png_ptr, info_ptr, palette, 256);
    png_set_tRNS(png_ptr, info_ptr, tRNS, 256, NULL);
  }
  else if (header.bpp == 24 || header.bpp == 32) {
    // BGR to RGB, or BGRA to RGBA
    for (uint32_t i = 0; i < header.height; i++) {
      char *line = &data[i * header.padding_width * header.bpp / 8];
      for (uint32_t j = 0; j < header.width; j++) {
	uint8_t *pixel = (uint8_t*)&line[j * header.bpp / 8];
	uint8_t r = pixel[2];
	uint8_t b = pixel[0];
	pixel[0] = r;
        pixel[2] = b;
      }
    }
  }

  png_write_info(png_ptr, info_ptr);

  for (uint32_t i = 0; i < header.height; i++) {
    png_write_row(png_ptr, (png_bytep)(data + i * header.padding_width * header.bpp / 8));
  }
  png_write_end(png_ptr, NULL);

  if (palette) png_free(png_ptr, palette);
  if (tRNS) png_free(png_ptr, tRNS);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  free(data);
  fclose(out);
  return 1;
}
