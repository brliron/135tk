#include <Windows.h>
#include <Gdiplus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmpfont_create.h"

/*
** GDI+ code adapted from https://www.codeproject.com/Articles/42529/Outline-Text
*/

typedef struct
{
  Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR                    gdiplusToken;
  Gdiplus::FontFamily*         font;

  HDC     hdc;
  HBITMAP hBmp;
  HGDIOBJ hOrigBmp;
  BYTE*   bmpData;
} GdiPlusGraphics;

void* graphics_init(int ac, char* const* av)
{
  (void)ac; (void)av; // Parameters support will be added later
  GdiPlusGraphics* obj = new GdiPlusGraphics;
  Gdiplus::GdiplusStartup(&obj->gdiplusToken, &obj->gdiplusStartupInput, NULL);

  obj->font = new Gdiplus::FontFamily(L"Arial");
  if (!obj->font->IsAvailable())
    {
      printf("Could not open font %s\n", "Arial");
      delete obj->font;
      Gdiplus::GdiplusShutdown(obj->gdiplusToken);
      delete obj;
      return NULL;
    }

  HDC hScreen = GetDC(NULL);
  obj->hdc  = CreateCompatibleDC(hScreen);
  obj->hBmp = CreateCompatibleBitmap(hScreen, 256, 256);
  ReleaseDC(NULL, hScreen);
  obj->hOrigBmp = SelectObject(obj->hdc, obj->hBmp);

  obj->bmpData = (BYTE*)malloc(256 * 256 * 4);

  return obj;
}

void graphics_free(void* obj_)
{
  GdiPlusGraphics* obj = (GdiPlusGraphics*)obj_;

  SelectObject(obj->hdc, obj->hOrigBmp);
  DeleteObject(obj->hBmp);
  DeleteDC(obj->hdc);

  delete obj->font;
  Gdiplus::GdiplusShutdown(obj->gdiplusToken);

  free(obj->bmpData);
  delete obj;
}

void graphics_put_char(void* obj_, WCHAR c, BYTE** dest, int* w, int* h)
{
  GdiPlusGraphics* obj = (GdiPlusGraphics*)obj_;

  {
    Gdiplus::Graphics graphics(obj->hdc);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    graphics.Clear(Gdiplus::Color(0, 0, 0));

    Gdiplus::GraphicsPath path;
    Gdiplus::StringFormat strFormat;
    path.AddString(&c, 1, obj->font, Gdiplus::FontStyleRegular, 20, Gdiplus::Point(0, 0), &strFormat);

    Gdiplus::Pen pen(Gdiplus::Color(150, 150, 150), 4);
    pen.SetLineJoin(Gdiplus::LineJoinRound);
    Gdiplus::SolidBrush brush(Gdiplus::Color(255, 255, 255));

    graphics.DrawPath(&pen, &path);
    graphics.FillPath(&brush, &path);

    Gdiplus::Rect rect;
    path.GetBounds(&rect, NULL, &pen);
    *w = rect.GetRight();
    *h = rect.GetBottom();
    if (*w < 0)
      *w = 0;
    if (*h < 0)
      *h = 0;
  }

  BITMAPINFO info;
  memset(&info, 0, sizeof(info));
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
  for (y = 0; y < *h; y++)
    memcpy(dest[y], &obj->bmpData[(255 - y) * 256 * 4], *w * 4);
}
