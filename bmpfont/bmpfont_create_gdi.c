#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmpfont_create.h"

/*
** Options for libraries:
** - graphics_consume_option:
**     --cp codepage: codepage to use when processing strings (defaults to CP_OEMCP).
**                    It may or may not apply to options passed before it. In doubt, 
**                    pass it before any text-based option.
**
** - graphics_consume_option_binary:
**      --font-memory: like --font-file, but loads a font file from memory.
**                     value points to the in-memory file, and
**                     value_size is the size of the in-memory file.
*/

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
  // Options parsing
  unsigned int    cp;
  const char     *font_name;
  int             font_size;
  const char    **font_files;
  int             font_files_count;
  HANDLE         *font_mem;
  int             font_mem_count;

  DCWrapper       text;
  DCWrapper       outline;
  DCWrapper       target;
  int             outline_width;
  BYTE*           bmpData;
} GdiGraphics;



void init_DCWrapper(DCWrapper *dc, HFONT hFont)
{
  HDC hScreen   = GetDC(NULL);
  dc->hdc      = CreateCompatibleDC(hScreen);
  dc->hBmp     = CreateCompatibleBitmap(hScreen, 256, 256);
  ReleaseDC(NULL, hScreen);

  dc->hFont = hFont;
  SetBkColor(dc->hdc, RGB(0, 0, 0));
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



wchar_t *to_unicode(GdiGraphics *obj, const char *src)
{
  wchar_t *dst = malloc((strlen(src) + 1) * sizeof(wchar_t));
  MultiByteToWideChar(obj->cp, 0, src, -1, dst, strlen(src) + 1);
  return dst;
}



void* graphics_init()
{
  GdiGraphics* obj = malloc(sizeof(GdiGraphics));
  memset(obj, 0, sizeof(GdiGraphics));
  obj->cp = CP_OEMCP;
  obj->font_size = 32;
  obj->outline_width = 2;
  obj->text.textColor = RGB(255, 255, 255);
  obj->outline.textColor = RGB(150, 150, 150);
  obj->bmpData = malloc(256 * 256 * 4);
  return obj;
}

static void help()
{
  printf("  --font-name font      Name of the font used to render the texts (required).\n"
	 "  --font-file file      Font file to load before looking for a font.\n"
	 "  --font-size size      Font size (default: 32).\n"
	 "  --outline-color R:G:B Outline color (default: 150:150:150).\n"
	 "  --outline-width n     Outline size (default: 2, use 0 to remove outline).\n"
	 "  --text-color R:G:B    Text color (default: 255:255:255).\n"
	 );
}

static DWORD parse_color(const char *color)
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

static int init_2(GdiGraphics *obj);
int graphics_consume_option(void *obj_, const char *name, const char *value)
{
  GdiGraphics *obj = obj_;

  if (name == NULL)
    return init_2(obj);
  else if (strcmp(name, "--font-name") == 0)
    obj->font_name = value;
  else if (strcmp(name, "--font-size") == 0)
    obj->font_size = atoi(value);
  else if (strcmp(name, "--font-file") == 0)
    {
      LPWSTR wValue = to_unicode(obj, value);
      if (AddFontResourceExW(wValue, FR_PRIVATE, 0) == 0)
	{
	  fprintf(stderr, "Error: could not add font from %s\n", value);
	  free(wValue);
	  return 0;
	}
      free(wValue);
      obj->font_files_count++;
      obj->font_files = realloc((void*)obj->font_files, obj->font_files_count * sizeof(char*));
      obj->font_files[obj->font_files_count - 1] = value;
    }
  else if (strcmp(name, "--outline-color") == 0)
    obj->outline.textColor = parse_color(value);
  else if (strcmp(name, "--outline-width") == 0)
    obj->outline_width = atoi(value);
  else if (strcmp(name, "--text-color") == 0)
    obj->text.textColor = parse_color(value);
  else if (strcmp(name, "--cp") == 0)
    obj->cp = atoi(value);
  else if (strcmp(name, "--help") == 0)
    help();
  else
    {
      fprintf(stderr, "Unrecognized option '%s'\n", name);
      return 0;
    }
  return 1;
}

int graphics_consume_option_binary(void *obj_, const char *name, void *value, size_t value_size)
{
  GdiGraphics *obj = obj_;

  if (strcmp(name, "--font-memory") == 0)
    {
      DWORD nb_fonts;
      HANDLE hFont = AddFontMemResourceEx(value, value_size, 0, &nb_fonts);
      if (hFont == 0)
	{
	  fprintf(stderr, "Error: could not add font from memory\n");
	  return 0;
	}
      obj->font_mem_count++;
      obj->font_mem = realloc(obj->font_mem, obj->font_mem_count * sizeof(HANDLE));
      obj->font_mem[obj->font_mem_count - 1] = hFont;
    }
  else
    {
      fprintf(stderr, "Unrecognized binary option '%s'\n", name);
      return 0;
    }
  return 1;
}

static int init_2(GdiGraphics *obj)
{
  if (!obj->font_name)
    {
      printf("--font-name is required\n\n");
      return 0;
    }

  LPWSTR font_name   = to_unicode(obj, obj->font_name);
  HFONT hTextFont    = CreateFontW(obj->font_size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
				   ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
				   DEFAULT_PITCH | FF_DONTCARE, font_name);
  HFONT hOutlineFont = CreateFontW(obj->font_size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
				   ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
				   DEFAULT_PITCH | FF_DONTCARE, font_name);
  free(font_name);
  if (hTextFont == NULL || hOutlineFont == NULL)
    {
      printf("Could not open font %s\n", obj->font_name);
      return 0;
    }

  init_DCWrapper(&obj->text, hTextFont);
  init_DCWrapper(&obj->outline, hOutlineFont);
  init_DCWrapper(&obj->target, NULL);
  return 1;
}

void graphics_free(void *obj_)
{
  GdiGraphics *obj = obj_;

  if (!obj)
    return ;

  free_DCWrapper(&obj->text);
  free_DCWrapper(&obj->outline);
  free_DCWrapper(&obj->target);

  for (int i = 0; i < obj->font_files_count; i++)
    {
      LPWSTR wFontFile = to_unicode(obj, obj->font_files[i]);
      RemoveFontResourceExW(wFontFile, FR_PRIVATE, 0);
      free(wFontFile);
    }
  free((void*)obj->font_files);
  for (int i = 0; i < obj->font_mem_count; i++)
    RemoveFontMemResourceEx(obj->font_mem[i]);
  free(obj->font_mem);

  free(obj->bmpData);
  free(obj);
}


void graphics_put_char(void *obj_, WCHAR c, BYTE **dest, int *w, int *h)
{
  GdiGraphics *obj = obj_;

  DCWrapper *dc;
  RECT rect;

  if (obj->outline_width)
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
		 rect.right  - rect.left + obj->outline_width * 2,
		 rect.bottom - rect.top  + obj->outline_width * 2,
		 obj->outline.hdc, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
		 SRCCOPY);

      // Finally, blit the text.
      StretchBlt(obj->target.hdc,
		 rect.left + obj->outline_width, rect.top + obj->outline_width,
		 rect.right - rect.left, rect.bottom - rect.top,
		 obj->text.hdc, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
		 SRCPAINT);

      rect.right  += obj->outline_width * 2;
      rect.bottom += obj->outline_width * 2;
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
