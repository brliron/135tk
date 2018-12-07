#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include "bmpfont_create.h"

typedef enum
  {
    // Plain RGBA output
    UNPACKED_RGBA,
    // The mode used by the game. Each color channel is used as a grayscale image.
    PACKED_RGBA,
    // Grayscale output
    UNPACKED_GRAYSCALE,
    // Grayscale output, with:
    // (0;0) containing the 1st pixel of the 1st character,
    // (1;0) containing the 1st pixel of the 2st character,
    // (2;0) containing the 1st pixel of the 3st character,
    // (3;0) containing the 1st pixel of the 4st character,
    // (4;0) containing the 2st pixel of the 1st character,
    // etc. It is the format used by thcrap (to make conversion to packed RGBA easier).
    PACKED_GRAYSCALE,

    INVALID_FORMAT
  } OutputType;

#pragma pack(push, 1)
typedef struct
{
  int16_t x;
  int16_t y;
  uint8_t width;
  uint8_t height;
  uint8_t y_offset;
  uint8_t channel;
} CharDetail;
#pragma pack(pop)

typedef struct
{
  unsigned int x;
  unsigned int y;
  int channel;
  unsigned int w;
  unsigned int h;
  unsigned int char_w;
  unsigned int line_h;

  struct {
    void* (*init)();
    int   (*consume_option)(void*, const char*, const char*);
    int   (*consume_option_binary)(void*, const char*, const void*, size_t);
    void  (*put_char)(void*, WCHAR, BYTE**, int*, int*);
    void  (*free)(void*);
  }            func;
  void        *graphics;
  HMODULE      hMod;
  const char  *exe;
  char         chars_list[65536];
  int          chars_count;
  OutputType   output_type;
  int          export_png;
  const char  *out_fn;
  BYTE       **out_mem;
  size_t      *out_size;
} State;

int put_char(State *state, WCHAR c, BYTE** dest, CharDetail* charDetail)
{
  int w;
  int h;
  BYTE  *buffer =      (BYTE*) malloc(256 * 256 * 4);
  BYTE **buffer_rows = (BYTE**)malloc(256 * sizeof(BYTE*));
  memset(buffer, 0, 256 * 256 * 4);
  int i;
  for (i = 0; i < 256; i++)
    buffer_rows[i] = &buffer[256 * 4 * i];

  state->func.put_char(state->graphics, c, buffer_rows, &w, &h);
  if (c == L' ')
    w += 8;

  if (state->x + w >= state->w)
    {
      free(buffer);
      free(buffer_rows);
      return 0;
    }
  if (w > (int)state->char_w)
    state->char_w = w;
  if (h > (int)state->line_h)
    state->line_h = h;

  int x;
  int y;
  for (x = 0; x < w; x++)
    for (y = 0; y < h; y++) {
      int x2 = (state->x + x) * 4 + state->channel;
      int y2 = state->y + y;
      switch (state->output_type)
	{
	case UNPACKED_RGBA:
	  dest[y2][x2 + 0] = buffer_rows[y][x * 4 + 0];
	  dest[y2][x2 + 1] = buffer_rows[y][x * 4 + 1];
	  dest[y2][x2 + 2] = buffer_rows[y][x * 4 + 2];
	  dest[y2][x2 + 3] = buffer_rows[y][x * 4 + 3];
	  break;
	case UNPACKED_GRAYSCALE:
	  dest[y2][state->x + x] = buffer_rows[y][x * 4];
	  break;
	case PACKED_RGBA:
	case PACKED_GRAYSCALE:
	  dest[y2][x2] = buffer_rows[y][x * 4];
	  break;
	case INVALID_FORMAT:
	  break;
	}
    }

  charDetail->x        = state->x;
  charDetail->y        = state->y;
  charDetail->width    = w;
  charDetail->height   = h;
  charDetail->y_offset = 0;
  charDetail->channel  = state->channel;

  free(buffer_rows);
  free(buffer);
  return 1;
}

int export_to_PNG(uint8_t** rows, size_t width, size_t height, char* out_png)
{
  png_structp png_ptr;
  png_infop info_ptr;

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info_ptr = png_create_info_struct(png_ptr);
  if (!png_ptr || !info_ptr || setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return 0;
  }

  FILE *out = fopen(out_png, "wb");
  if (!out) {
    perror(out_png);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return 0;
  }
  png_init_io(png_ptr, out);

  // TODO: change bit depth to 4
  png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_GRAY,
	       PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  uint8_t* out_data = malloc(width * height);
  uint8_t** out_rows = malloc(height * sizeof(uint8_t*));
  for (uint32_t i = 0; i < height; i++) {
    out_rows[i] = &out_data[i * width];
    for (uint32_t j = 0; j < width; j++)
      out_rows[i][j] = rows[i][j];
  }

  png_write_info(png_ptr, info_ptr);

  for (uint32_t i = 0; i < height; i++) {
    png_write_row(png_ptr, out_rows[i]);
  }
  png_write_end(png_ptr, NULL);

  png_destroy_write_struct(&png_ptr, &info_ptr);
  free(out_rows);
  free(out_data);
  fclose(out);
  return 1;
}

void *bmpfont_init()
{
  State *bmpfont = malloc(sizeof(State));
  memset(bmpfont, 0, sizeof(State));
  bmpfont->exe = "bmpfont";
  bmpfont->output_type = INVALID_FORMAT;
  for (uint16_t c = 0; c < 65535; c++)
    bmpfont->chars_list[c] = 1;
  return bmpfont;
}

void usage(State *state)
{
  printf("Usage: %s option...\n"
	 "  --help            Display this help message.\n"
	 "  --format format   unpacked_rgba, packed_rgba, unpacked_grayscale\n"
         "                    or packed_grayscale.\n"
	 "                    unpacked_rgba and unpacked_grayscale can be opened\n"
	 "                    with an image editor.\n"
	 "                    packed_rgba is the format used by the game.\n"
	 "                    packed_grayscale is the format expected by thcrap.\n"
	 "                    It also creates a PNG output.\n"
	 "  --out output_file Output file name (required).\n"
	 "  --plugin plugin   Plugin to be used to render the texts (required).\n"
	 "  --png true        Exports the file as png (requires --format packed_grayscale).\n"
	 , state->exe);
  if (state->func.consume_option)
    {
      printf("\nPlugin-specific options:\n");
      state->func.consume_option(state->graphics, "--help", NULL);
    }
  else if (!state->func.init)
    printf("For plugin-specific options, use --plugin [plugin] --help\n");
}

int init_plugin(State *state, const char* fn)
{
  state->hMod = LoadLibrary(fn);
  if (!state->hMod)
    {
      printf("%s: can't open plugin %s\n", state->exe, fn);
      return 0;
    }

  state->func.init                  = (void*(*)())                                        GetProcAddress(state->hMod, "graphics_init");
  state->func.consume_option        = (int  (*)(void*, const char*, const char*))         GetProcAddress(state->hMod, "graphics_consume_option");
  state->func.consume_option_binary = (int  (*)(void*, const char*, const void*, size_t)) GetProcAddress(state->hMod, "graphics_consume_option_binary");
  state->func.put_char              = (void (*)(void*, WCHAR, BYTE**, int*, int*))        GetProcAddress(state->hMod, "graphics_put_char");
  state->func.free                  = (void (*)(void*))                                   GetProcAddress(state->hMod, "graphics_free");

  int error = 0;
  if (!state->func.init || !state->func.free || !state->func.put_char)
    {
      printf("%s: the %s plugin doesn's define all the functions needed by this program.\n", state->exe, fn);
      error = 1;
    }
  else
    {
      state->graphics = state->func.init();
      if (state->graphics == NULL)
	error = 1;
    }

  if (error)
    {
      FreeLibrary(state->hMod);
      state->func.init                  = NULL;
      state->func.consume_option        = NULL;
      state->func.consume_option_binary = NULL;
      state->func.put_char              = NULL;
      state->func.free                  = NULL;
      state->hMod                       = NULL;
      return 0;
    }
  return 1;
}

int bmpfont_add_option(void *bmpfont, const char *name, const char *value)
{
  State *state = (State*)bmpfont;

  if (strcmp(name, "--exe") == 0)
    state->exe = value;
  else if (strcmp(name, "--help") == 0)
    {
      usage(state);
      return 0;
    }
  else if (strcmp(name, "--plugin") == 0)
    {
      if (!init_plugin(state, value))
	return 0;
    }
  else if (strcmp(name, "--format") == 0)
    {
      if      (strcmp(value, "unpacked_rgba") == 0)
        state->output_type = UNPACKED_RGBA;
      else if (strcmp(value, "packed_rgba") == 0)
        state->output_type = PACKED_RGBA;
      else if (strcmp(value, "unpacked_grayscale") == 0)
        state->output_type = UNPACKED_GRAYSCALE;
      else if (strcmp(value, "packed_grayscale") == 0)
        state->output_type = PACKED_GRAYSCALE;
      else
        {
          printf("%s: unknown format %s\n", state->exe, value);
          return 0;
        }
    }
  else if (strcmp(name, "--out") == 0)
    state->out_fn = value;
  else if (strcmp(name, "--png") == 0)
    state->export_png = 1;
  else
    {
      if (!state->func.consume_option)
	{
	  fprintf(stderr, "%s: unrecognized option '%s'\n"
			  "Try '%s --help' for more information.",
		  state->exe, name, state->exe);
	  return 0;
	}
      if (!state->func.consume_option(state->graphics, name, value))
	return 0;
    }

  return 1;
}

int bmpfont_add_option_binary(void *bmpfont, const char *name, void *value, size_t value_size)
{
  State *state = (State*)bmpfont;

  if (strcmp(name, "--out-buffer") == 0)
    {
      if (value_size != sizeof(void*))
        {
          printf("%s: binary option %s needs a value_size of %d\n", state->exe, name, sizeof(void*));
          return 0;
        }
      state->out_mem = (BYTE**)value;
    }
  else if (strcmp(name, "--out-size") == 0)
    {
      if (value_size != sizeof(void*))
        {
          printf("%s: binary option %s needs a value_size of %d\n", state->exe, name, sizeof(size_t));
          return 0;
        }
      state->out_size = (size_t*)value;
    }
  else if (strcmp(name, "--out-size") == 0)
    {
      char *chars_list = (char*)value;
      for (uint16_t i = 0; i < 65535 && i < value_size; i++)
	state->chars_list[i] = chars_list[i];
    }
  else
    {
      if (!state->func.consume_option_binary)
	{
	  fprintf(stderr, "%s: unrecognized binary option '%s'\n"
			  "Check the header files for more information.",
		  state->exe, name);
	  return 0;
	}
      if (!state->func.consume_option_binary(state->graphics, name, value, value_size))
	return 0;
    }

  return 1;
}

int check_state(State *state)
{
  // Did we get all our required arguments?
  if (state->output_type == INVALID_FORMAT)
    printf("--format is required\n\n");
  if (!state->out_fn && !state->out_mem)
    printf("--out is required\n\n"); // If using as a library, --out-mem can be used instead
  if (!state->func.init)
    printf("--plugin is required\n\n");
  if (state->output_type == INVALID_FORMAT || !state->out_fn || !state->func.init)
    {
      usage(state);
      return 0;
    }

  // Ask the same question to our plugin
  if (state->func.consume_option && state->func.consume_option(state->graphics, NULL, NULL) == 0)
    return 0;

  return 1;
}

static void make_bmpfont(State *state, BYTE **pData, BYTE ***pRows, CharDetail **pCharDetails)
{
  state->x = 0;
  state->y = 0;
  state->channel = 0;
  state->w = 2048;
  state->h = 1024;
  state->char_w = 0;
  state->line_h = 0;

  BYTE  *data = (BYTE*) malloc(state->w * state->h * 4);
  BYTE **rows = (BYTE**)malloc(state->h * sizeof(BYTE*));
  memset(data, 0, state->w * state->h * 4);
  for (unsigned int i = 0; i < state->h; i++)
    rows[i] = data + i * state->w * 4;
  CharDetail *charDetails = (CharDetail*)malloc(65536 * sizeof(CharDetail));

  for (uint16_t c = 0; c < 65535; c++)
    {
      if (!state->chars_list[c])
	continue;
      if (put_char(state, c, rows, &charDetails[c]) == 0)
	{
	  // TODO: clear (x;y) -> (x+char_w;y+line_h)
	  if (state->y + state->line_h + 256 > state->h)
	    {
	      state->h += 1024;
	      data = (BYTE*) realloc(data, state->w * state->h * 4);
	      rows = (BYTE**)realloc(rows, state->h * sizeof(BYTE*));
	      for (unsigned int i = 0; i < state->h; i++)
		rows[i] = data + i * state->w * 4;
	    }
	  state->y += state->line_h;
	  state->x = 0;
	  state->line_h = 0;

	  // Redraw all the characters in the current cell
	  state->char_w = 0;
	  while (state->channel > 0)
	    {
	      c--;
	      state->channel--;
	    }
	  c--;
	  continue;
	}

      if (state->output_type != UNPACKED_RGBA && state->output_type != UNPACKED_GRAYSCALE)
	state->channel++;
      else
	state->channel += 4;
      if (state->channel == 4)
	{
	  state->x += state->char_w;
	  state->char_w = 0;
	  state->channel = 0;
	}
      state->chars_count++;
    }

  state->h = state->y + state->line_h;
  *pData = data;
  *pRows = rows;
  *pCharDetails = charDetails;
}

static void fill_bmp_headers(State *state, BITMAPFILEHEADER *header, BITMAPINFOHEADER *info)
{
  int bpp;
  int palette_size;
  int bitmap_size;

  if (state->output_type == UNPACKED_RGBA || state->output_type == PACKED_RGBA)
    {
      bpp = 32;
      palette_size = 0;
      bitmap_size = state->w * state->h * 4;
    }
  else
    {
      bpp = 8;
      palette_size = 256 * 4;
      bitmap_size = state->w * state->h;
    }

  if (state->output_type == PACKED_GRAYSCALE)
    state->w *= 4;

  header->bfType = 'B' | ('M' << 8);
  header->bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + palette_size + bitmap_size;
  header->bfReserved1 = 0;
  header->bfReserved2 = 0;
  header->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + palette_size;

  info->biSize = sizeof(BITMAPINFOHEADER);
  info->biWidth = state->w;
  info->biHeight = state->h;
  info->biPlanes = 1;
  info->biBitCount = bpp;
  info->biCompression = BI_RGB;
  info->biSizeImage = 0;
  info->biXPelsPerMeter = 0;
  info->biYPelsPerMeter = 0;
  info->biClrUsed = 0;
  info->biClrImportant = 0;
}

int bmpfont_run(void *bmpfont)
{
  State *state = (State*)bmpfont;

  if (!check_state(state))
    return 0;

  BYTE        *data;
  BYTE       **rows;
  CharDetail  *charDetails;
  make_bmpfont(state, &data, &rows, &charDetails);
  state->func.free(state->graphics);

  BITMAPFILEHEADER header;
  BITMAPINFOHEADER info;
  fill_bmp_headers(state, &header, &info);

  size_t output_size = header.bfSize;
  if (state->output_type == PACKED_RGBA || state->output_type == PACKED_GRAYSCALE)
    output_size += 4 + state->chars_count * (2 + sizeof(CharDetail));
  BYTE  *output = malloc(output_size);
  BYTE  *output_ptr = output;

  // Header
  memcpy(output_ptr, &header, sizeof(header));
  output_ptr += sizeof(header);
  memcpy(output_ptr, &info, sizeof(info));
  output_ptr += sizeof(info);

  // Palette
  if (state->output_type == UNPACKED_GRAYSCALE || state->output_type == PACKED_GRAYSCALE)
    {
      uint8_t c = 0;
      do
	{
	  output_ptr[0] = c;
	  output_ptr[1] = c;
	  output_ptr[2] = c;
	  output_ptr[3] = 0;
	  output_ptr += 4;
	  c++;
	} while (c != 0);
    }

  // Bitmap
  int line;
  for (line = state->h - 1; line >= 0; line--)
    {
      memcpy(output_ptr, rows[line], state->w * info.biBitCount / 8);
      output_ptr += state->w * info.biBitCount / 8;
    }

  // Metadatas
  if (state->output_type == PACKED_RGBA || state->output_type == PACKED_GRAYSCALE)
    {
      uint16_t unk = 0x0215; // I don't know what is that, so I take the bytes in spell_font.bmp for now.
      uint16_t nb_chars = state->chars_count;
      memcpy(output_ptr,     &unk, 2);
      memcpy(output_ptr + 2, &nb_chars, 2);
      output_ptr += 4;
      for (uint16_t c = 0; c < 65535; c++) {
          if (state->chars_list[c]) {
              memcpy(output_ptr, &c, 2);
              output_ptr += 2;
          }
      }
      for (uint16_t c = 0; c < 65535; c++) {
          if (state->chars_list[c]) {
              memcpy(output_ptr, &charDetails[c], sizeof(CharDetail));
              output_ptr += sizeof(CharDetail);
          }
      }
    }

  // File output
  if (state->out_fn)
    {
      FILE *fout = fopen(state->out_fn, "wb");
      if (!fout)
        {
          perror(state->out_fn);
          return 1;
        }
      fwrite(output, output_size, 1, fout);
      fclose(fout);
    }

  // PNG output for thcrap
  if (state->output_type == PACKED_GRAYSCALE && state->export_png && state->out_fn)
    {
      char *path_png = (char*)malloc(strlen(state->out_fn) + 5);
      strcpy(path_png, state->out_fn);
      strcat(path_png, ".png");
      export_to_PNG(rows, state->w, state->h, path_png);
      free(path_png);
    }

  // Metadata output
  if (state->output_type == PACKED_GRAYSCALE && state->out_fn)
    {
      // We'll store the metadatas in another file, because the output will probably be converted to PNG.
      char *path_bin = (char*)malloc(strlen(state->out_fn) + 5);
      strcpy(path_bin, state->out_fn);
      strcat(path_bin, ".bin");
      FILE *fout = fopen(path_bin, "wb");
      if (fout == NULL)
	perror(path_bin);
      fwrite(output + header.bfSize, output_size - header.bfSize, 1, fout);
      fclose(fout);
      free(path_bin);
    }

  if (state->out_mem)
    *state->out_mem = output;
  else
    free(output);

  if (state->out_size)
    *state->out_size = output_size;

  free(charDetails);
  free(rows);
  free(data);

  return 0;
}

void bmpfont_free(void *bmpfont)
{
  if (!bmpfont)
    return ;
  State *state = (State*)bmpfont;
  if (state->out_mem)
    free(*state->out_mem);
  free(state);
}

