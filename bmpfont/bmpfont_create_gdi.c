#include <Windows.h>
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

void* graphics_init(const char* font)
{
  HFONT hFont = CreateFontA(32, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
			   ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
			   DEFAULT_PITCH | FF_DONTCARE, font);
  if (font == NULL)
    {
      printf("Could not open font %s\n", font);
      return NULL;
    }

  GdiGraphics* obj = malloc(sizeof(GdiGraphics));
  obj->hFont = hFont;

  HDC hScreen = GetDC(NULL);
  obj->hdc  = CreateCompatibleDC(hScreen);
  obj->hBmp = CreateCompatibleBitmap(hScreen, 256, 256);
  ReleaseDC(NULL, hScreen);

  obj->hOrigFont = SelectObject(obj->hdc, obj->hFont);
  obj->hOrigBmp = SelectObject(obj->hdc, obj->hBmp);
  SetTextColor(obj->hdc, RGB(255, 255, 255));
  SetBkColor(obj->hdc, RGB(0, 0, 0));

  obj->bmpData = malloc(256 * 256 * 4);

  return obj;
}

void graphics_free(void* obj_)
{
  GdiGraphics* obj = obj_;

  SelectObject(obj->hdc, obj->hOrigBmp);
  SelectObject(obj->hdc, obj->hOrigFont);
  DeleteObject(obj->hFont);
  DeleteObject(obj->hBmp);
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
