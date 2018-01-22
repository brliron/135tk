#include <Windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  int nb_channels;
  unsigned int char_w;
  unsigned int line_h;
} State;

int put_char(HDC hdc, HBITMAP bmp, WCHAR c, BYTE **rows, State* state)
{
  RECT rect;
  rect.left   = 0;
  rect.top    = 0;
  rect.right  = 0;
  rect.bottom = 0;
  DrawTextW(hdc, &c, 1, &rect, DT_CALCRECT);
  if (state->x + rect.right >= state->w)
    return 0;
  if (rect.right > (int)state->char_w)
    state->char_w = rect.right;
  if (rect.bottom > (int)state->line_h)
    state->line_h = rect.bottom;

  FillRect(hdc, &rect, (HBRUSH)(COLOR_SCROLLBAR+1));
  DrawTextW(hdc, &c, 1, &rect, 0);

  BITMAPINFO info = { 0 };
  info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth = 256;
  info.bmiHeader.biHeight = 256;
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biCompression = BI_RGB;
  info.bmiHeader.biSizeImage = 0;
  info.bmiHeader.biXPelsPerMeter = 0;
  info.bmiHeader.biYPelsPerMeter = 0;
  info.bmiHeader.biClrUsed = 0;
  info.bmiHeader.biClrImportant = 0;
  char *data = malloc(256 * 256 * 4);
  GetDIBits(hdc, bmp, 0, 256, data, &info, DIB_RGB_COLORS);
  int x;
  int y;
  for (x = 0; x < rect.right; x++)
    for (y = 0; y < rect.bottom; y++) {
      int x1 = x * 4;
      int y1 = 255 - y;
      int x2 = (state->x + x) * state->nb_channels + state->channel;
      int y2 = state->y + y;
      rows[y2][x2+0] = data[y1 * 4 * 256 + x1 + 0];
      rows[y2][x2+1] = data[y1 * 4 * 256 + x1 + 1];
      rows[y2][x2+2] = data[y1 * 4 * 256 + x1 + 2];
      rows[y2][x2+3] = data[y1 * 4 * 256 + x1 + 3];
    }

  free(data);
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

  HFONT font = CreateFontA(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
			   ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
			   DEFAULT_PITCH | FF_DONTCARE, av[2]);
  if (font == NULL)
    {
      printf("Could not open font %s\n", av[2]);
      return 1;
    }

  State state;
  state.x = 0;
  state.y = 0;
  state.channel = 0;
  state.w = 2048;
  state.h = 1024;
  state.nb_channels = 4;
  state.char_w = 0;
  state.line_h = 0;
  BYTE *data = malloc(state.w * state.h * state.nb_channels);
  BYTE** rows = malloc(state.h * sizeof(BYTE*));
  memset(data, 0xFF, state.w * state.h * state.nb_channels);
  unsigned int i;
  for (i = 0; i < state.h; i++)
    rows[i] = data + i * state.w * state.nb_channels;

  HDC hScreen = GetDC(NULL);
  HDC hdc = CreateCompatibleDC(hScreen);
  HBITMAP hBmp = CreateCompatibleBitmap(hScreen, 256, 256);
  HFONT hOrigFont = SelectObject(hdc, font);
  HBITMAP hOrigBmp = SelectObject(hdc, hBmp);
  SetTextColor(hdc, RGB(255, 255, 255));
  SetBkColor(hdc, RGB(0, 0, 0));

  uint16_t c;
  for (c = L'!'; c != /*0*/127; c++)
    {
      if (put_char(hdc, hBmp, c, rows, &state) == 0)
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

      state.channel += 4;
      if (state.channel == state.nb_channels) {
	state.x += state.char_w;
	state.char_w = 0;
	state.channel = 0;
      }
    }

  SelectObject(hdc, hOrigBmp);
  SelectObject(hdc, hOrigFont);
  DeleteObject(hBmp);
  DeleteDC(hdc);
  ReleaseDC(NULL, hScreen);
  DeleteObject(font);

  BITMAPFILEHEADER header;
  BITMAPINFOHEADER info;
  header.bfType = 'B' | ('M' << 8);
  header.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + state.w * state.h * state.nb_channels;
  header.bfReserved1 = 0;
  header.bfReserved2 = 0;
  header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

  info.biSize = sizeof(BITMAPINFOHEADER);
  info.biWidth = state.w;
  info.biHeight = -state.h; // TODO: reverse the bitmap
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
  fwrite(data,    state.w * state.h * state.nb_channels, 1, fout);
  fclose(fout);

  // At the end
  if (ac >= 4)
    RemoveFontResourceEx(av[3], FR_PRIVATE, 0);

  return 0;
}
