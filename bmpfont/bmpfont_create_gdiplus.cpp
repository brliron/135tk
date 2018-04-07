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
  static ULONG_PTR gdiplusToken;

public:
  static void initGdiplus();
  static void freeGdiplus();

  Gdiplus::FontFamily* font;

  HDC     hdc;
  HBITMAP hBmp;
  HGDIOBJ hOrigBmp;
  BYTE*   bmpData;

  int font_size;
  Gdiplus::Color bg;
  Gdiplus::Color outline;
  Gdiplus::Color text;
  int outline_width;

  GdiPlusGraphics();
  ~GdiPlusGraphics();
};
ULONG_PTR GdiPlusGraphics::gdiplusToken;

GdiPlusGraphics::GdiPlusGraphics()
  : font(nullptr), hdc(nullptr), hBmp(nullptr), hOrigBmp(nullptr),
    font_size(20), bg(0, 0, 0), outline(150, 150, 150), text(255, 255, 255), outline_width(4)
{
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
}

void GdiPlusGraphics::initGdiplus()
{
  Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  Gdiplus::GdiplusStartup(&GdiPlusGraphics::gdiplusToken, &gdiplusStartupInput, NULL);
}

void GdiPlusGraphics::freeGdiplus()
{
  Gdiplus::GdiplusShutdown(GdiPlusGraphics::gdiplusToken);
}

void graphics_help()
{
  printf("  --font-name font      Name of the font used to render the texts (required).\n"
	 "  --font-size size      Font size (default: 20).\n"
	 "  --bg-color R:G:B      Background color (default: 0:0:0).\n"
	 "  --outline-color R:G:B Outline color (default: 150:150:150).\n"
	 "  --outline-width n     Outline width (default: 4).\n"
	 "  --text-color R:G:B    Text color (default: 255:255:255).\n"
	 "\n"
	 "Note: --font-file doesn't seem to work with this plugin, and only TTF fonts\n"
	 "are supported.\n"
	 );
}

Gdiplus::Color parse_color(const char* color)
{
  int r;
  int g;
  int b;

  r = strtol(color, (char**)&color, 0);
  if (*color) color++;
  g = strtol(color, (char**)&color, 0);
  if (*color) color++;
  b = strtol(color, (char**)&color, 0);
  return Gdiplus::Color(r, g, b);
}

enum {
  ARG_FONTNAME = 2,
  ARG_FONTSIZE,
  ARG_BG,
  ARG_OUTLINE,
  ARG_OUTLINEWIDTH,
  ARG_TEXT,
};
int options(int ac, char* const* av, char** font_name, GdiPlusGraphics* obj)
{
  struct option options[] = {
    { "font-name",      required_argument, NULL, ARG_FONTNAME },
    { "font-size",      required_argument, NULL, ARG_FONTSIZE },
    { "bg-color",       required_argument, NULL, ARG_BG },
    { "outline-color",  required_argument, NULL, ARG_OUTLINE },
    { "outline-width",  required_argument, NULL, ARG_OUTLINEWIDTH },
    { "text-color",     required_argument, NULL, ARG_TEXT },
    { NULL,             0,                 NULL, 0 },
  };
  *font_name = NULL;

  int idx;
  while (1)
    {
      idx = getopt_long(ac, av, "-:", options, NULL);
      switch (idx) {
      case ARG_FONTNAME:
	*font_name = optarg;
	break;

      case ARG_FONTSIZE:
	obj->font_size = atoi(optarg);
	break;

      case ARG_BG:
        obj->bg = parse_color(optarg);
	break;

      case ARG_OUTLINE:
        obj->outline = parse_color(optarg);
	break;

      case ARG_OUTLINEWIDTH:
        obj->outline_width = atoi(optarg);
	break;

      case ARG_TEXT:
	obj->text = parse_color(optarg);
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
  GdiPlusGraphics::initGdiplus();
  GdiPlusGraphics* obj = new GdiPlusGraphics;

  char* font_name;
  if (!options(ac, av, &font_name, obj))
    {
      delete obj;
      GdiPlusGraphics::freeGdiplus();
      return NULL;
    }

  WCHAR* w_font_name = new WCHAR[strlen(font_name) + 1];
  MultiByteToWideChar(CP_OEMCP, 0, font_name, -1, w_font_name, strlen(font_name) + 1);
  obj->font = new Gdiplus::FontFamily(w_font_name);
  free(w_font_name);
  if (!obj->font->IsAvailable())
    {
      printf("Could not open font %s\n", font_name);
      delete obj;
      GdiPlusGraphics::freeGdiplus();
      return NULL;
    }

  return obj;
}

void graphics_free(void* obj_)
{
  GdiPlusGraphics* obj = (GdiPlusGraphics*)obj_;
  delete obj;
  GdiPlusGraphics::freeGdiplus();
}

void graphics_put_char(void* obj_, WCHAR c, BYTE** dest, int* w, int* h)
{
  GdiPlusGraphics* obj = (GdiPlusGraphics*)obj_;
  Gdiplus::Rect rect;

  {
    Gdiplus::Graphics graphics(obj->hdc);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    graphics.Clear(obj->bg);

    Gdiplus::GraphicsPath path;
    Gdiplus::StringFormat strFormat;
    path.AddString(&c, 1, obj->font, Gdiplus::FontStyleRegular, obj->font_size, Gdiplus::Point(0, 0), &strFormat);

    Gdiplus::Pen pen(obj->outline, obj->outline_width);
    pen.SetLineJoin(Gdiplus::LineJoinRound);
    Gdiplus::SolidBrush brush(obj->text);

    graphics.DrawPath(&pen, &path);
    graphics.FillPath(&brush, &path);

    path.GetBounds(&rect, NULL, &pen);
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

  // Keep the rect inside the bitmap
  if (rect.X < 0)
    rect.X = 0;
  if (rect.Y < 0)
    rect.Y = 0;
  // Fix left bound
  for (int i = rect.X; i < rect.GetRight(); i++)
    {
      int j;
      for (j = rect.Y; j < rect.GetBottom(); j++)
	if (obj->bmpData[(255 - j) * 256 * 4 + i * 4 + 0] != 0 ||
	    obj->bmpData[(255 - j) * 256 * 4 + i * 4 + 1] != 0 ||
	    obj->bmpData[(255 - j) * 256 * 4 + i * 4 + 2] != 0)
	  break;
      if (j != rect.GetBottom())
	break;
      rect.X++;
      rect.Width--;
    }
  // Fix right bound
  for (int i = rect.X; i < rect.GetRight(); i++)
    {
      int j;
      for (j = rect.Y; j < rect.GetBottom(); j++)
	if (obj->bmpData[(255 - j) * 256 * 4 + i * 4 + 0] != 0 ||
	    obj->bmpData[(255 - j) * 256 * 4 + i * 4 + 1] != 0 ||
	    obj->bmpData[(255 - j) * 256 * 4 + i * 4 + 2] != 0)
	  break;
      if (j == rect.GetBottom())
	{
	  rect.Width = i - rect.X;
	  break;
	}
    }

  for (int y = 0; y < rect.GetBottom(); y++)
    memcpy(dest[y], &obj->bmpData[(255 - y) * 256 * 4 + rect.X * 4], rect.Width * 4);

  *w = rect.Width + 2;
  *h = rect.GetBottom();
}
