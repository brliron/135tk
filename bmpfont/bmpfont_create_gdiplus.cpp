#include <Windows.h>
#include <getopt.h>
#include <Gdiplus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmpfont_create.h"

/*
** GDI+ code adapted from https://www.codeproject.com/Articles/42529/Outline-Text
*/

class GdiPlusGraphics
{
private:
  Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR                    gdiplusToken;

public:
  Gdiplus::FontFamily*         font;

  HDC     hdc;
  HBITMAP hBmp;
  HGDIOBJ hOrigBmp;
  BYTE*   bmpData;

  GdiPlusGraphics();
  ~GdiPlusGraphics();
};

GdiPlusGraphics::GdiPlusGraphics()
  : font(nullptr), hdc(nullptr), hBmp(nullptr), hOrigBmp(nullptr)
{
  Gdiplus::GdiplusStartup(&this->gdiplusToken, &this->gdiplusStartupInput, NULL);

  HDC hScreen = GetDC(NULL);
  this->hdc  = CreateCompatibleDC(hScreen);
  this->hBmp = CreateCompatibleBitmap(hScreen, 256, 256);
  ReleaseDC(NULL, hScreen);
  this->hOrigBmp = SelectObject(this->hdc, this->hBmp);

  this->bmpData = new BYTE[256 * 256 * 4];
}

GdiPlusGraphics::~GdiPlusGraphics()
{
  delete this->font;

  SelectObject(this->hdc, this->hOrigBmp);
  DeleteObject(this->hBmp);
  DeleteDC(this->hdc);

  delete[] this->bmpData;
  Gdiplus::GdiplusShutdown(this->gdiplusToken);
}

void* graphics_init(int ac, char* const* av)
{
  (void)ac; (void)av; // Parameters support will be added later
  GdiPlusGraphics* obj = new GdiPlusGraphics;

  obj->font = new Gdiplus::FontFamily(L"Arial");
  if (!obj->font->IsAvailable())
    {
      printf("Could not open font %s\n", "Arial");
      delete obj;
      return NULL;
    }

  return obj;
}

void graphics_free(void* obj_)
{
  GdiPlusGraphics* obj = (GdiPlusGraphics*)obj_;
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
