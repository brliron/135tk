#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <png.h>

static int inflate_bytes(char* file_in, size_t size_in, char* file_out, size_t size_out)
{
	int ret;
	z_stream strm;

	strm.zalloc = NULL;
	strm.zfree = NULL;
	strm.opaque = NULL;
	strm.next_in = (unsigned char*)file_in;
	strm.avail_in = size_in;
	strm.next_out = (unsigned char*)file_out;
	strm.avail_out = size_out;
	ret = inflateInit(&strm);
	if (ret != Z_OK) {
		return ret;
	}
	do {
		ret = inflate(&strm, Z_NO_FLUSH);
		if (ret != Z_OK) {
			break;
		}
	} while (ret != Z_STREAM_END);
	inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : ret;
}

FILE *TFXX_open(const char *fn, const char *in_magic, void *header, size_t header_size)
{
  FILE *f = fopen(fn, "rb");
  if (f == NULL) {
    perror(fn);
    return NULL;
  }

  char magic[4];
  uint8_t version;
  fread(magic, 4, 1, f);
  fread(&version, 1, 1, f);
  if (memcmp(in_magic, magic, 4) != 0 || version != 0) {
    printf("Error: %s: wrong magic or version\n", fn);
    fclose(f);
    return NULL;
  }

  fread(header, header_size, 1, f);

  return f;
}

char *TFXX_read(FILE *f, size_t comp_size, size_t uncomp_size)
{
  if (!f) {
    return NULL;
  }

  char *comp_data = malloc(comp_size);
  fread(comp_data, comp_size, 1, f);
  fclose(f);

  char *uncomp_data = malloc(uncomp_size);
  if (inflate_bytes(comp_data, comp_size, uncomp_data, uncomp_size) != Z_OK) {
    printf("inflate error\n");
    free(uncomp_data);
    uncomp_data = NULL;
  }
  free(comp_data);

  return uncomp_data;
}

#pragma pack(push, 1)
typedef struct {
  uint8_t bpp;
  uint32_t width;
  uint32_t height;
  uint32_t padding_width;
  uint32_t comp_size;
} TFBM_header;
#pragma pack(pop)

int convert_TFBM(const char *tfbm, const char *tfpa, const char *out_png)
{
  TFBM_header header;
  FILE *f = TFXX_open(tfbm, "TFBM", &header, sizeof(header));
  char *data = TFXX_read(f, header.comp_size, header.padding_width * header.height);
  if (!data) {
    return 0;
  }

  png_structp png_ptr;
  png_infop info_ptr;

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info_ptr = png_create_info_struct(png_ptr);
  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return 0;
  }

  FILE *out = fopen(out_png, "wb");
  if (!out) {
    perror(out_png);
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
  png_set_IHDR(png_ptr, info_ptr, header.width, header.height, header.bpp, color_type,
	       PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  png_color *palette = NULL;
  png_byte *tRNS = NULL;
  if (header.bpp == 8) {
    if (!tfpa) {
      printf("Error: no palette given for a 8-bits with palette TFBM image.\n");
      png_destroy_write_struct(&png_ptr, &info_ptr);
      free(data);
      fclose(out);
      return 0;
    }

    uint32_t comp_size;
    FILE *f = TFXX_open(tfpa, "TFPA", &comp_size, sizeof(comp_size));
    char *plt_data = TFXX_read(f, comp_size, 256 * 2 + 256 * 4);
    if (!plt_data) {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      free(data);
      fclose(out);
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
    png_set_PLTE(png_ptr, info_ptr, palette, 256);
    png_set_tRNS(png_ptr, info_ptr, tRNS, 256, NULL);

    free(plt_data);
  }
  else if (header.bpp == 24 || header.bpp == 32) {
    // BGR to RGB, or BGRA to RGBA
    for (uint32_t i = 0; i < header.height; i++) {
      char *line = &data[i * header.padding_width];
      for (uint32_t j = 0; i < header.width; i++) {
	char *pixel = &line[j * header.bpp / 8];
	uint8_t r = pixel[2];
	uint8_t b = pixel[0];
	pixel[0] = r;
        pixel[2] = b;
      }
    }
  }

  png_write_info(png_ptr, info_ptr);

  for (uint32_t i = 0; i < header.height; i++) {
    png_write_row(png_ptr, (png_bytep)(data + i * header.padding_width));
  }
  png_write_end(png_ptr, NULL);

  if (palette) {
    png_free(png_ptr, palette);
  }
  if (tRNS) {
    png_free(png_ptr, tRNS);
  }
  png_destroy_write_struct(&png_ptr, &info_ptr);
  free(data);
  fclose(out);
  return 1;
}

int main(int ac, char **av)
{
  if (ac < 2) {
    printf("Usage: %s in.[bmp|png] [palette.bmp]\n", av[0]);
    return 0;
  }

  convert_TFBM(av[1], av[2], av[1]);
  return 0;
}
