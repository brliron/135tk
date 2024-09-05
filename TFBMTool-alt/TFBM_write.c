#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include "TFBMTool.h"

int export_palette(LPCWSTR png, png_structp png_ptr, png_infop info_ptr, LPCWSTR out_tfpa)
{
  png_colorp plte;
  png_bytep tRNS;
  int plte_size;
  int tRNS_size;

  if (png_get_PLTE(png_ptr, info_ptr, &plte, &plte_size) != PNG_INFO_PLTE || plte_size <= 0) {
    fwprintf(stderr, L"%S: no palette found in 8-bit PNG.\n", png);
    return 1;
  }
  if (png_get_tRNS(png_ptr, info_ptr, &tRNS, &tRNS_size, NULL) != PNG_INFO_tRNS || tRNS_size <= 0) {
    fwprintf(stderr, L"Warning: %S: no transparency information found in 8-bit PNG.\n"
      "Assuming 0 is fully transparent and everything else is fully opaque.", png);
    tRNS_size = -1;
  }

  uint16_t *plte_out_16 = malloc((2 + 4) * 256);
  uint8_t *plte_out_8 = (uint8_t*)(plte_out_16 + 256);

  for (int i = 0; i < 256; i++) {
    png_color color;
    png_byte alpha;
    if (i < plte_size) {
      color = plte[i];
    }
    else {
      color.red = 0;
      color.green = 0;
      color.blue = 0;
    }
    if (i < tRNS_size) {
      alpha = tRNS[i];
    }
    else if (i == 0) {
      alpha = 0;
    }
    else {
      alpha = 255;
    }

    // ARGB5551
    unsigned int a = (alpha > 0x20) ? 1 : 0;
    unsigned int r = color.red   * 32 / 256;
    unsigned int g = color.green * 32 / 256;
    unsigned int b = color.blue  * 32 / 256;
    uint16_t c = (uint16_t)((a << 15) + (r << 10) + (g << 5) + b);
    plte_out_16[i] = c;

    // BGRA8888
    plte_out_8[i * 4 + 2] = color.red;
    plte_out_8[i * 4 + 1] = color.green;
    plte_out_8[i * 4 + 0] = color.blue;
    plte_out_8[i * 4 + 3] = alpha;
  }

  FILE *fout = TFXX_open_write(out_tfpa, "TFPA", NULL, 0);
  TFXX_write(fout, (char*)plte_out_16, (2 + 4) * 256);

  return 0;
}

int convert_PNG_to_TFBM(LPCWSTR png, LPCWSTR out_tfbm, LPCWSTR out_tfpa)
{
  FILE *in = _wfopen(png, L"rb");
  if (!in) {
    _wperror(png);
    return 0;
  }
  uint8_t sig[8];
  fread(sig, 1, 8, in);
  if (!png_check_sig(sig, 8)) {
    fwprintf(stderr, L"%S: invalid PNG signature\n", png);
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
  if (bit_depth != 8 || !(
	color_type == PNG_COLOR_TYPE_PALETTE ||
	color_type == PNG_COLOR_TYPE_RGB ||
	color_type == PNG_COLOR_TYPE_RGB_ALPHA
	))
    {
      fwprintf(stderr, L"Only the following color types are supported: 8-bits with palette, 24-bits RGB, 32-bits RGBA.\n");
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      fclose(in);
      return 0;
    }

  if (bit_depth == 8)
    export_palette(png, png_ptr, info_ptr, out_tfpa);

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
      for (uint32_t j = 0; j < width; j++) {
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
  header.padding_width = width; // TODO: add padding pixels
  FILE *fout = TFXX_open_write(out_tfbm, "TFBM", &header, sizeof(TFBM_header));
  TFXX_write(fout, (char*)image_data, rowbytes * height);
  free(image_data);
  free(row_pointers);
  return 1;
}
