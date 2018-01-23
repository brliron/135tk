#include <Windows.h>
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

// Disable packing 4 characters per pixels.
//#define DISABLE_PACKING

int put_char(void* obj, WCHAR c, BYTE **dest, State* state, CharDetail* charDetail)
{
  int w;
  int h;
  BYTE*  buffer =      (BYTE*) malloc(256 * 256 * 4);
  BYTE** buffer_rows = (BYTE**)malloc(256 * sizeof(BYTE*));
  memset(buffer, 0, 256 * 256 * 4);
  int i;
  for (i = 0; i < 256; i++)
    buffer_rows[i] = &buffer[256 * 4 * i];

  graphics_put_char(obj, c, buffer_rows, &w, &h);

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
#ifndef DISABLE_PACKING
      dest[y2][x2] = buffer_rows[y][x * 4];
#else
      dest[y2][x2 + 0] = buffer_rows[y][x * 4 + 0];
      dest[y2][x2 + 1] = buffer_rows[y][x * 4 + 1];
      dest[y2][x2 + 2] = buffer_rows[y][x * 4 + 2];
      dest[y2][x2 + 3] = buffer_rows[y][x * 4 + 3];
#endif
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

int main(int ac, const char** av)
{
  if (ac < 3)
    {
      printf("Usage: %s out.bmp font_name [in.ttf]\n", av[0]);
      return 0;
    }

  if (ac >= 4)
    {
      if (AddFontResourceEx(av[3], FR_PRIVATE, 0) == 0)
	printf("Warning: 0 fonts were added from %s\n", av[3]);
    }

  void *obj = graphics_init(av[2]);
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
  for (c = L' '; c != /*0*/127; c++)
    {
      if (put_char(obj, c, rows, &state, &charDetails[c]) == 0)
	{
	  // TODO: clear (x;y) -> (x+char_w;y+line_h)
	  if (state.y + state.line_h > state.h)
	    break; // TODO: expand the bitmap
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

#ifndef DISABLE_PACKING
      state.channel++;
#else
      state.channel += 4;
#endif
      if (state.channel == 4) {
	state.x += state.char_w;
	state.char_w = 0;
	state.channel = 0;
      }
    }

  state.h = state.y + state.line_h;
  graphics_free(obj);

  BITMAPFILEHEADER header;
  BITMAPINFOHEADER info;
  header.bfType = 'B' | ('M' << 8);
  header.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + state.w * state.h * 4;
  header.bfReserved1 = 0;
  header.bfReserved2 = 0;
  header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

  info.biSize = sizeof(BITMAPINFOHEADER);
  info.biWidth = state.w;
  info.biHeight = state.h;
  info.biPlanes = 1;
  info.biBitCount = 32;
  info.biCompression = BI_RGB;
  info.biSizeImage = 0;
  info.biXPelsPerMeter = 0;
  info.biYPelsPerMeter = 0;
  info.biClrUsed = 0;
  info.biClrImportant = 0;

  FILE *fout = fopen(av[1], "wb");
  if (!fout)
    {
      perror(av[1]);
      return 1;
    }
  fwrite(&header, sizeof(header), 1, fout);
  fwrite(&info,   sizeof(info),   1, fout);
  int line;
  for (line = state.h - 1; line >= 0; line--)
    fwrite(rows[line], state.w * 4, 1, fout);
  uint16_t unk = 0x0215; // I don't know what is that, so I take the bytes in spell_font.bmp for now.
  uint16_t nb_chars = 127 -  L' ';
  fwrite(&unk, 2, 1, fout);
  fwrite(&nb_chars, 2, 1, fout);
  for (c = L' '; c != /*0*/127; c++)
    fwrite(&c, 2, 1, fout);
  for (c = L' '; c != /*0*/127; c++)
    fwrite(&charDetails[c], sizeof(CharDetail), 1, fout);
  fclose(fout);

  free(charDetails);
  free(rows);
  free(data);
  if (ac >= 4)
    RemoveFontResourceEx(av[3], FR_PRIVATE, 0);

  return 0;
}
