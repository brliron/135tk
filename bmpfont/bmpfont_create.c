#include <Windows.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmpfont_create.h"

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
} State;

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
static OutputType output_type = INVALID_FORMAT;

// Biggest character used. Use 65535 for all supported characters, 127 for ASCII only.
//#define BIGGEST_CHAR 127
#define BIGGEST_CHAR 65535

static void* (*graphics_init_func)(int ac, char* const* av)                             = NULL;
static void  (*graphics_free_func)(void* obj)                                           = NULL;
static void  (*graphics_put_char_func)(void* obj, WCHAR c, BYTE** dest, int* w, int* h) = NULL;
static void  (*graphics_help_func)()                                                    = NULL;

int put_char(void* obj, WCHAR c, BYTE** dest, State* state, CharDetail* charDetail)
{
  int w;
  int h;
  BYTE*  buffer =      (BYTE*) malloc(256 * 256 * 4);
  BYTE** buffer_rows = (BYTE**)malloc(256 * sizeof(BYTE*));
  memset(buffer, 0, 256 * 256 * 4);
  int i;
  for (i = 0; i < 256; i++)
    buffer_rows[i] = &buffer[256 * 4 * i];

  graphics_put_char_func(obj, c, buffer_rows, &w, &h);
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
      switch (output_type)
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

void usage(const char* exe)
{
  printf("Usage: %s option...\n"
	 "  --help            Display this help message.\n"
	 "  --format format   unpacked_rgba, packed_rgba, unpacked_grayscale\n"
         "                    or packed_grayscale.\n"
	 "                    unpacked_rgba and unpacked_grayscale can be opened\n"
	 "                    with an image editor.\n"
	 "                    packed_rgba is the format used by the game.\n"
	 "                    packed_grayscale is the format expected by thcrap.\n"
	 "  --out output_file Output file name (required).\n"
	 "  --png             Convert the output to PNG (not implemented).\n"
	 "  --plugin plugin   Plugin to be used to render the texts (required).\n"
	 "  --font-file file  Font file to load before looking for a font.\n"
	 , exe);
  if (graphics_help_func)
    {
      printf("\nPlugin-specific options:\n");
      graphics_help_func();
    }
  else if (!graphics_init_func)
    printf("For plugin-specific options, use --plugin [plugin] --help\n");
}

int init_plugin(const char* fn)
{
  HMODULE hMod;

  hMod = LoadLibrary(fn);
  if (!hMod)
    {
      printf("Error: can't open plugin %s\n", fn);
      return 0;
    }

  graphics_init_func     = (void*(*)(int, char* const*))              GetProcAddress(hMod, "graphics_init");
  graphics_free_func     = (void (*)(void*))                          GetProcAddress(hMod, "graphics_free");
  graphics_help_func     = (void(*)())                                GetProcAddress(hMod, "graphics_help");
  graphics_put_char_func = (void(*)(void*, WCHAR, BYTE**, int*, int*))GetProcAddress(hMod, "graphics_put_char");

  if (!graphics_init_func || !graphics_free_func || !graphics_put_char_func)
    {
      printf("Error: the %s plugin doesn's define all the functions needed by this program.\n", fn);
      graphics_init_func     = NULL;
      graphics_free_func     = NULL;
      graphics_help_func     = NULL;
      graphics_put_char_func = NULL;
      FreeLibrary(hMod);
      return 0;
    }
  return 1;
}

enum {
  ARG_HELP = 2,
  ARG_FORMAT,
  ARG_OUT,
  ARG_PNG,
  ARG_PLUGIN,
  ARG_FONTFILE,
};
int options(int ac, char* const* av, char** out, int* png, char** font_file)
{
  struct option options[] = {
    { "help",      no_argument,       NULL, ARG_HELP },
    { "format",    required_argument, NULL, ARG_FORMAT },
    { "out",       required_argument, NULL, ARG_OUT },
    { "png",       no_argument,       NULL, ARG_PNG },
    { "plugin",    required_argument, NULL, ARG_PLUGIN },
    { "font-file", required_argument, NULL, ARG_FONTFILE },
    { NULL,        0,                 NULL, 0 },
  };
  int help = 0;
  *out = NULL;
  *png = 0;
  *font_file = NULL;

  int idx;
  while (1)
    {
      idx = getopt_long(ac, av, "-:", options, NULL);
      switch (idx) {
      case ARG_HELP:
	help = 1;
	break;

      case ARG_FORMAT:
	if (strcmp(optarg, "unpacked_rgba") == 0)
	  output_type = UNPACKED_RGBA;
	else if (strcmp(optarg, "packed_rgba") == 0)
	  output_type = PACKED_RGBA;
	else if (strcmp(optarg, "unpacked_grayscale") == 0)
	  output_type = UNPACKED_GRAYSCALE;
	else if (strcmp(optarg, "packed_grayscale") == 0)
	  output_type = PACKED_GRAYSCALE;
	else
	  {
	    printf("Error: unknown format %s\n", optarg);
	    return 0;
	  }
	break;

      case ARG_OUT:
	*out = optarg;
	break;

      case ARG_PNG:
	*png = 1;
	break;

      case ARG_FONTFILE:
	*font_file = optarg;
	break;

      case ARG_PLUGIN:
	if (!init_plugin(optarg))
	  return 0;
	break;

      case '?':
	break;

      case ':':
	printf("Missing argument for one of the options\n");
	return 0;

      case -1:
	if (help)
	  {
	    usage(av[0]);
	    return 0;
	  }
	if (output_type == INVALID_FORMAT)
	  printf("--format is required\n\n");
	if (!*out)
	  printf("--out is required\n\n");
	if (!graphics_init_func)
	  printf("--plugin is required\n\n");
	if (output_type == INVALID_FORMAT || !*out || !graphics_init_func)
	  {
	    usage(av[0]);
	    return 0;
	  }

        optind = 1;
	return 1;
      }
    }
}

int main(int ac, char* const* av)
{
  // OutputType outputType; // global
  char* out_fn;
  int png;
  char* font_file;
  (void)png; // not implemented

  if (!options(ac, av, &out_fn, &png, &font_file))
    return 0;

  if (font_file)
    {
      if (AddFontResourceEx(font_file, FR_PRIVATE, 0) == 0)
	printf("Warning: 0 fonts were added from %s\n", font_file);
    }

  void *obj = graphics_init_func(ac, av);
  if (!obj)
    return 1;

  State state;
  state.x = 0;
  state.y = 0;
  state.channel = 0;
  state.w = 2048;
  state.h = 1024;
  state.char_w = 0;
  state.line_h = 0;
  BYTE*  data = (BYTE*) malloc(state.w * state.h * 4);
  BYTE** rows = (BYTE**)malloc(state.h * sizeof(BYTE*));
  memset(data, 0, state.w * state.h * 4);
  unsigned int i;
  for (i = 0; i < state.h; i++)
    rows[i] = data + i * state.w * 4;
  CharDetail* charDetails = (CharDetail*)malloc(65536 * sizeof(CharDetail));

  uint16_t c;
  for (c = L' '; c < BIGGEST_CHAR; c++)
    {
      if (put_char(obj, c, rows, &state, &charDetails[c]) == 0)
	{
	  // TODO: clear (x;y) -> (x+char_w;y+line_h)
	  if (state.y + state.line_h + 256 > state.h)
	    {
	      state.h += 1024;
	      data = (BYTE*) realloc(data, state.w * state.h * 4);
	      rows = (BYTE**)realloc(rows, state.h * sizeof(BYTE*));
	      for (i = 0; i < state.h; i++)
		rows[i] = data + i * state.w * 4;
	    }
	  state.y += state.line_h;
	  state.x = 0;
	  state.line_h = 0;

	  // Redraw all the characters in the current cell
	  state.char_w = 0;
	  while (state.channel > 0)
	    {
	      c--;
	      state.channel--;
	    }
	  c--;
	  continue;
	}

      if (output_type != UNPACKED_RGBA && output_type != UNPACKED_GRAYSCALE)
	state.channel++;
      else
	state.channel += 4;
      if (state.channel == 4) {
	state.x += state.char_w;
	state.char_w = 0;
	state.channel = 0;
      }
    }

  state.h = state.y + state.line_h;
  graphics_free_func(obj);

  if (output_type == PACKED_GRAYSCALE)
    state.w *= 4;
  BITMAPFILEHEADER header;
  BITMAPINFOHEADER info;
  header.bfType = 'B' | ('M' << 8);
  if (output_type != UNPACKED_GRAYSCALE && output_type != PACKED_GRAYSCALE)
    header.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + state.w * state.h * 4;
  else
    header.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 256 * 4 + state.w * state.h;
  header.bfReserved1 = 0;
  header.bfReserved2 = 0;
  if (output_type != UNPACKED_GRAYSCALE && output_type != PACKED_GRAYSCALE)
    header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
  else
    header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 256 * 4;

  info.biSize = sizeof(BITMAPINFOHEADER);
  info.biWidth = state.w;
  info.biHeight = state.h;
  info.biPlanes = 1;
  if (output_type != UNPACKED_GRAYSCALE && output_type != PACKED_GRAYSCALE)
    info.biBitCount = 32;
  else
    info.biBitCount = 8;
  info.biCompression = BI_RGB;
  info.biSizeImage = 0;
  info.biXPelsPerMeter = 0;
  info.biYPelsPerMeter = 0;
  info.biClrUsed = 0;
  info.biClrImportant = 0;

  FILE *fout = fopen(out_fn, "wb");
  if (!fout)
    {
      perror(out_fn);
      return 1;
    }

  // Header
  fwrite(&header, sizeof(header), 1, fout);
  fwrite(&info,   sizeof(info),   1, fout);

  // Palette
  if (output_type == UNPACKED_GRAYSCALE || output_type == PACKED_GRAYSCALE)
    {
      uint8_t c = 0;
      const uint8_t zero = 0;
      do
	{
	  fwrite(&c, 1, 1, fout);
	  fwrite(&c, 1, 1, fout);
	  fwrite(&c, 1, 1, fout);
	  fwrite(&zero, 1, 1, fout);
	  c++;
	} while (c != 0);
    }

  // Bitmap
  int line;
  for (line = state.h - 1; line >= 0; line--)
    fwrite(rows[line], state.w * info.biBitCount / 8, 1, fout);

  // Metadatas
  if (output_type == PACKED_GRAYSCALE)
    {
      // We'll store the metadatas in another file, because the output will probably be converted to PNG.
      char *path_bin = (char*)malloc(strlen(out_fn) + 5);
      strcpy(path_bin, out_fn);
      strcat(path_bin, ".bin");
      fclose(fout);
      fout = fopen(path_bin, "wb");
      if (fout == NULL)
	perror(path_bin);
      free(path_bin);
    }
  if (output_type == PACKED_RGBA || output_type == PACKED_GRAYSCALE)
    {
      uint16_t unk = 0x0215; // I don't know what is that, so I take the bytes in spell_font.bmp for now.
      uint16_t nb_chars = BIGGEST_CHAR -  L' ';
      fwrite(&unk, 2, 1, fout);
      fwrite(&nb_chars, 2, 1, fout);
      for (c = L' '; c < BIGGEST_CHAR; c++)
	fwrite(&c, 2, 1, fout);
      for (c = L' '; c < BIGGEST_CHAR; c++)
	fwrite(&charDetails[c], sizeof(CharDetail), 1, fout);
    }
  /*
  ** JSON output is too big (8 MB for 65536 characters), so we won't use it.
  ** If someone ever needs it someday, I keep the source code here.
  else if (output_type == PACKED_GRAYSCALE)
    {
      json_t* object = json_object();
      wchar_t wc;
      for (wc = L' '; wc < BIGGEST_CHAR; wc++)
	{
	  char utf8[5];
	  WideCharToMultiByte(CP_UTF8, 0, &wc, 1, utf8, 5, NULL, NULL);
	  json_t* elem = json_object();
	  json_object_set(object, utf8, elem);

	  json_object_set_new(elem, "x",        json_integer(charDetails[wc].x));
	  json_object_set_new(elem, "y",        json_integer(charDetails[wc].y));
	  json_object_set_new(elem, "width",    json_integer(charDetails[wc].width));
	  json_object_set_new(elem, "height",   json_integer(charDetails[wc].height));
	  json_object_set_new(elem, "y_offset", json_integer(charDetails[wc].y_offset));
	  json_object_set_new(elem, "channel",  json_integer(charDetails[wc].channel));

	  json_decref(elem);
	}

      char *path_jdiff = (char*)malloc(strlen(out_fn) + 7);
      strcpy(path_jdiff, out_fn);
      strcat(path_jdiff, ".jdiff");
      json_dump_file(object, path_jdiff, JSON_INDENT(2));
      json_decref(object);
      free(path_jdiff);
    }
  */
  fclose(fout);

  free(charDetails);
  free(rows);
  free(data);
  if (ac >= 5)
    RemoveFontResourceEx(font_file, FR_PRIVATE, 0);

  return 0;
}
