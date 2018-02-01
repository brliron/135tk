#include <Windows.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmpfont_create.h"

typedef struct
{
  HDC     hdc;
  HFONT   hFont;
  HBITMAP hBmp;
  HFONT   hOrigFont;
  HBITMAP hOrigBmp;
  BYTE*   bmpData;
} GdiGraphics;

void graphics_help()
{
  printf("  --font-name font  Name of the font used to render the texts (required).\n"
	 "  --font-size size  Font size (default: 32).\n"
	 "  --fg-color R:G:B  Foreground color (default: 255:255:255).\n"
	 "  --bg-color R:G:B  Background color (default: 0:0:0).\n"
	 );
}

DWORD parse_color(const char* color)
{
  int r;
  int g;
  int b;

  r = strtol(color, (char**)&color, 0);
  if (*color) color++;
  g = strtol(color, (char**)&color, 0);
  if (*color) color++;
  b = strtol(color, (char**)&color, 0);
  return RGB(r, g, b);
}

enum {
  ARG_FONTNAME = 2,
  ARG_FONTSIZE,
  ARG_FG,
  ARG_BG,
};
int options(int ac, char* const* av, char** font_name, int* font_size, HDC hdc)
{
  struct option options[] = {
    { "font-name", required_argument, NULL, ARG_FONTNAME },
    { "font-size", required_argument, NULL, ARG_FONTSIZE },
    { "fg-color",  required_argument, NULL, ARG_FG },
    { "bg-color",  required_argument, NULL, ARG_BG },
    { NULL,        0,                 NULL, 0 },
  };
  *font_name = NULL;
  *font_size = 32;
  SetTextColor(hdc, RGB(255, 255, 255));
  SetBkColor(hdc, RGB(0, 0, 0));

  int idx;
  while (1)
    {
      idx = getopt_long(ac, av, "-:", options, NULL);
      switch (idx) {
      case ARG_FONTNAME:
	*font_name = optarg;
	break;

      case ARG_FONTSIZE:
	*font_size = atoi(optarg);
	break;

      case ARG_FG:
	SetTextColor(hdc, parse_color(optarg));
	break;

      case ARG_BG:
	SetBkColor(hdc, parse_color(optarg));
	break;

      case '?':
	break;

      case ':':
	printf("Missing argument for one of the options\n");
	return 0;

      case -1:
	if (!*font_name)
	  {
	    printf("--font-name is required\n\n");
	    return 0;
	  }
	return 1;
      }
    }
}

void* graphics_init(int ac, char* const* av)
{
  GdiGraphics* obj = malloc(sizeof(GdiGraphics));
  memset(obj, 0, sizeof(GdiGraphics));

  HDC hScreen   = GetDC(NULL);
  obj->hdc      = CreateCompatibleDC(hScreen);
  obj->hBmp     = CreateCompatibleBitmap(hScreen, 256, 256);
  obj->hOrigBmp = SelectObject(obj->hdc, obj->hBmp);
  ReleaseDC(NULL, hScreen);

  char *font_name;
  int font_size;
  if (!options(ac, av, &font_name, &font_size, obj->hdc))
    {
      graphics_free(obj);
      return NULL;
    }

  obj->hFont = CreateFontA(font_size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
			   ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
			   DEFAULT_PITCH | FF_DONTCARE, font_name);
  if (obj->hFont == NULL)
    {
      printf("Could not open font %s\n", font_name);
      graphics_free(obj);
      return NULL;
    }
  obj->hOrigFont = SelectObject(obj->hdc, obj->hFont);

  obj->bmpData = malloc(256 * 256 * 4);
  return obj;
}

void graphics_free(void* obj_)
{
  GdiGraphics* obj = obj_;

  if (!obj)
    return ;

  if (obj->hBmp)
    {
      SelectObject(obj->hdc, obj->hOrigBmp);
      DeleteObject(obj->hBmp);
    }
  if (obj->hFont)
    {
      SelectObject(obj->hdc, obj->hOrigFont);
      DeleteObject(obj->hFont);
    }
  if (obj->hdc)
    DeleteDC(obj->hdc);

  free(obj->bmpData);
  free(obj);
}

void graphics_put_char(void* obj_, WCHAR c, BYTE** dest, int* w, int* h)
{
  GdiGraphics* obj = obj_;

  RECT rect;
  rect.left   = 0;
  rect.top    = 0;
  rect.right  = 0;
  rect.bottom = 0;
  DrawTextW(obj->hdc, &c, 1, &rect, DT_CALCRECT);
  FillRect(obj->hdc, &rect, (HBRUSH)(COLOR_SCROLLBAR+1));
  DrawTextW(obj->hdc, &c, 1, &rect, 0);

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
  GetDIBits(obj->hdc, obj->hBmp, 0, 256, obj->bmpData, &info, DIB_RGB_COLORS);

  int y;
  for (y = 0; y < rect.bottom; y++)
    memcpy(dest[y], &obj->bmpData[(255 - y) * 256 * 4], rect.right * 4);

  *w = rect.right;
  *h = rect.bottom;
}
