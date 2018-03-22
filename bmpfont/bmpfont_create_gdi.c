#include <Windows.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmpfont_create.h"

typedef struct DCWrapper
{
  HDC     hdc;
  HBITMAP hBmp;
  HBITMAP hOldBmp;
  HFONT   hFont;
  HFONT   hOldFont;
  DWORD   textColor;
} DCWrapper;

typedef struct
{
  DCWrapper text;
  DCWrapper outline;
  DCWrapper target;
  int       outline_size;
  BYTE*     bmpData;
} GdiGraphics;

void graphics_help()
{
  printf("  --font-name font      Name of the font used to render the texts (required).\n"
	 "  --font-size size      Font size (default: 32).\n"
	 "  --outline-color R:G:B Outline color (default: 150:150:150).\n"
	 "  --outline-size n      Outline size (default: 2, use 0 to remove outline).\n"
	 "  --text-color R:G:B    Text color (default: 255:255:255).\n"
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
  ARG_OUTLINE,
  ARG_OUTLINESIZE,
  ARG_TEXT,
};
int options(int ac, char* const* av, char** font_name, int* font_size, GdiGraphics *obj)
{
  struct option options[] = {
    { "font-name",         required_argument, NULL, ARG_FONTNAME },
    { "font-size",         required_argument, NULL, ARG_FONTSIZE },
    { "outline-color",     required_argument, NULL, ARG_OUTLINE },
    { "outline-size", required_argument, NULL, ARG_OUTLINESIZE },
    { "text-color",        required_argument, NULL, ARG_TEXT },
    { NULL,                0,                 NULL, 0 },
  };
  *font_name = NULL;
  *font_size = 32;
  obj->outline_size = 2;
  obj->text.textColor = RGB(255, 255, 255);
  obj->outline.textColor = RGB(150, 150, 150);

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

      case ARG_OUTLINE:
	obj->outline.textColor = parse_color(optarg);
	break;

      case ARG_OUTLINESIZE:
	obj->outline_size = atoi(optarg);
	break;

      case ARG_TEXT:
	obj->text.textColor = parse_color(optarg);
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

void init_DCWrapper(DCWrapper* dc, HFONT hFont)
{
  HDC hScreen   = GetDC(NULL);
  dc->hdc      = CreateCompatibleDC(hScreen);
  dc->hBmp     = CreateCompatibleBitmap(hScreen, 256, 256);
  ReleaseDC(NULL, hScreen);

  dc->hFont = hFont;
  SetBkColor(dc->hdc, RGB(0, 0, 0));
}

void* graphics_init(int ac, char* const* av)
{
  GdiGraphics* obj = malloc(sizeof(GdiGraphics));
  memset(obj, 0, sizeof(GdiGraphics));

  char *font_name;
  int font_size;
  if (!options(ac, av, &font_name, &font_size, obj))
    {
      graphics_free(obj);
      return NULL;
    }

  HFONT hTextFont    = CreateFontA(font_size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
				   ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
				   DEFAULT_PITCH | FF_DONTCARE, font_name);
  HFONT hOutlineFont = CreateFontA(font_size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
				   ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
				   DEFAULT_PITCH | FF_DONTCARE, font_name);
  if (hTextFont == NULL || hOutlineFont == NULL)
    {
      printf("Could not open font %s\n", font_name);
      graphics_free(obj);
      return NULL;
    }

  init_DCWrapper(&obj->text, hTextFont);
  init_DCWrapper(&obj->outline, hOutlineFont);
  init_DCWrapper(&obj->target, NULL);
  obj->bmpData = malloc(256 * 256 * 4);
  return obj;
}

void enter_DCWrapper(DCWrapper *dc)
{
  dc->hOldBmp  = SelectObject(dc->hdc, dc->hBmp);
  if (dc->hFont)
    dc->hOldFont = SelectObject(dc->hdc, dc->hFont);
  SetTextColor(dc->hdc, dc->textColor);
}

void leave_DCWrapper(DCWrapper *dc)
{
  if (dc->hOldBmp)
    {
      SelectObject(dc->hdc, dc->hBmp);
      dc->hOldBmp = NULL;
    }
  if (dc->hOldFont)
    {
      SelectObject(dc->hdc, dc->hFont);
      dc->hOldFont = NULL;
    }
}

void free_DCWrapper(DCWrapper *dc)
{
  leave_DCWrapper(dc);
  if (dc->hBmp)
    DeleteObject(dc->hBmp);
  if (dc->hFont)
    DeleteObject(dc->hFont);
  if (dc->hdc)
    DeleteDC(dc->hdc);
}

void graphics_free(void* obj_)
{
  GdiGraphics* obj = obj_;

  if (!obj)
    return ;

  free_DCWrapper(&obj->text);
  free_DCWrapper(&obj->outline);
  free_DCWrapper(&obj->target);

  free(obj->bmpData);
  free(obj);
}


void graphics_put_char(void* obj_, WCHAR c, BYTE** dest, int* w, int* h)
{
  GdiGraphics* obj = obj_;

  DCWrapper* dc;
  RECT rect;

  if (obj->outline_size)
    {
      enter_DCWrapper(&obj->outline);
      enter_DCWrapper(&obj->text);
      enter_DCWrapper(&obj->target);

      // First, draw the outline.
      memset(&rect, 0, sizeof(rect));
      DrawTextW(obj->outline.hdc, &c, 1, &rect, DT_CALCRECT);
      DrawTextW(obj->outline.hdc, &c, 1, &rect, 0);

      // Then, draw the text.
      DrawTextW(obj->text.hdc, &c, 1, &rect, 0);

      // Next, blit the outline.
      StretchBlt(obj->target.hdc,
		 rect.left, rect.top,
		 rect.right  - rect.left + obj->outline_size * 2,
		 rect.bottom - rect.top  + obj->outline_size * 2,
		 obj->outline.hdc, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
		 SRCCOPY);

      // Finally, blit the text.
      StretchBlt(obj->target.hdc,
		 rect.left + obj->outline_size, rect.top + obj->outline_size,
		 rect.right - rect.left, rect.bottom - rect.top,
		 obj->text.hdc, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
		 SRCPAINT);

      rect.right  += obj->outline_size * 2;
      rect.bottom += obj->outline_size * 2;
      leave_DCWrapper(&obj->outline);
      leave_DCWrapper(&obj->text);
      dc = &obj->target;
    }
  else
    {
      enter_DCWrapper(&obj->text);

      memset(&rect, 0, sizeof(rect));
      DrawTextW(obj->text.hdc, &c, 1, &rect, DT_CALCRECT);
      DrawTextW(obj->text.hdc, &c, 1, &rect, 0);

      dc = &obj->text;
    }

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
  GetDIBits(dc->hdc, dc->hBmp, 0, 256, obj->bmpData, &info, DIB_RGB_COLORS);

  int y;
  for (y = 0; y < rect.bottom; y++)
    memcpy(dest[y], &obj->bmpData[(255 - y) * 256 * 4], rect.right * 4);

  leave_DCWrapper(dc);
  *w = rect.right;
  *h = rect.bottom;
  if (c == L' ')
    *w -= 8;
}
