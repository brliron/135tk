#include <windows.h>
#include <gdiplus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "bmpfont_create.h"

/*
** GDI+ code adapted from https://www.codeproject.com/Articles/42529/Outline-Text
**
** Options for libraries:
** - graphics_consume_option_binary:
**      --font-memory: like --font-file, but loads a font file from memory.
**                     value points to the in-memory file, and
**                     value_size is the size of the in-memory file.
*/

class GdiPlusGraphics
{
private:
  static ULONG_PTR gdiplusToken;

public:
  static void initGdiplus();
  static void freeGdiplus();

  const char *font_name;
  Gdiplus::FontFamily *font;
  bool useFontCollection;
  Gdiplus::PrivateFontCollection fontCollection;

  HDC      hdc;
  HBITMAP  hBmp;
  HGDIOBJ  hOrigBmp;
  BYTE    *bmpData;

  unsigned int cp;
  int font_size;
  Gdiplus::Color bg;
  Gdiplus::Color outline;
  Gdiplus::Color text;
  int style;
  int outline_width;
  int margin;

  GdiPlusGraphics();
  ~GdiPlusGraphics();
};
ULONG_PTR GdiPlusGraphics::gdiplusToken;

GdiPlusGraphics::GdiPlusGraphics()
  : font(nullptr), useFontCollection(false), hdc(nullptr), hBmp(nullptr), hOrigBmp(nullptr),
    cp(CP_OEMCP), font_size(20), bg(0, 0, 0), outline(150, 150, 150), text(255, 255, 255),
    style(Gdiplus::FontStyleRegular), outline_width(4), margin(2)
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



std::wstring to_unicode(GdiPlusGraphics *obj, const char *src)
{
  wchar_t *buffer = new wchar_t[strlen(src) + 1];
  MultiByteToWideChar(obj->cp, 0, src, -1, buffer, strlen(src) + 1);
  std::wstring dst = buffer;
  delete[] buffer;
  return dst;
}



void help()
{
  printf("  --font-name font      Name of the font used to render the texts (required).\n"
	 "  --font-file file      Look for fonts in this file."
	 "                        This option can be supplied multiple times.\n"
	 "  --font-size size      Font size (default: 20).\n"
	 "  --bg-color R:G:B      Background color (default: 0:0:0).\n"
	 "  --outline-color R:G:B Outline color (default: 150:150:150).\n"
	 "  --outline-width n     Outline width (default: 4).\n"
	 "  --text-color R:G:B    Text color (default: 255:255:255).\n"
	 "  --margin n            Margin between characters (default: 2).\n"
	 "  --bold true|false     Put characters in bold (default: false).\n"
	 "\n"
	 "Note: only TTF fonts are supported.\n"
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

static int init_2(GdiPlusGraphics *obj);
int graphics_consume_option(void *obj_, const char *name, const char *value)
{
  GdiPlusGraphics *obj = (GdiPlusGraphics*)obj_;

  if (name == nullptr)
    return init_2(obj);
  else if (strcmp(name, "--font-name") == 0)
    obj->font_name = value;
  else if (strcmp(name, "--font-size") == 0)
    obj->font_size = atoi(value);
  else if (strcmp(name, "--font-file") == 0)
    {
      std::wstring font_file = to_unicode(obj, value);
      obj->fontCollection.AddFontFile(font_file.c_str());
      obj->useFontCollection = true;
    }
  else if (strcmp(name, "--bg-color") == 0)
    obj->bg = parse_color(value);
  else if (strcmp(name, "--outline-color") == 0)
    obj->outline = parse_color(value);
  else if (strcmp(name, "--outline-width") == 0)
    obj->outline_width = atoi(value);
  else if (strcmp(name, "--text-color") == 0)
    obj->text = parse_color(value);
  else if (strcmp(name, "--margin") == 0)
    obj->margin = atoi(value);
  else if (strcmp(name, "--bold") == 0)
    {
      if (strcmp(value, "true") == 0)
	obj->style |=  Gdiplus::FontStyleBold;
      else
	obj->style &= ~Gdiplus::FontStyleBold;
    }
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
  GdiPlusGraphics *obj = (GdiPlusGraphics*)obj_;

  if (strcmp(name, "--font-memory") == 0)
    {
      Gdiplus::Status status = obj->fontCollection.AddMemoryFont(value, value_size);
      if (status != Gdiplus::Ok)
	{
	  fprintf(stderr, "AddMemoryFont failed: %d\n", status);
	  return 0;
	}
      obj->useFontCollection = true;
    }
  else
    {
      fprintf(stderr, "Unrecognized option '%s'\n", name);
      return 0;
    }
  return 1;
}

void* graphics_init()
{
  GdiPlusGraphics::initGdiplus();
  GdiPlusGraphics* obj = new GdiPlusGraphics;
  return obj;
}

static int init_2(GdiPlusGraphics *obj)
{
  if (!obj->font_name)
    {
      printf("--font-name is required\n\n");
      return 0;
    }

  std::wstring w_font_name = to_unicode(obj, obj->font_name);
  if (!obj->useFontCollection)
    obj->font = new Gdiplus::FontFamily(w_font_name.c_str());
  else
    obj->font = new Gdiplus::FontFamily(w_font_name.c_str(), &obj->fontCollection);
  if (!obj->font->IsAvailable())
    {
      printf("Could not open font %s\n", obj->font_name);
      return 0;
    }

  return 1;
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
    path.AddString(&c, 1, obj->font, obj->style, (Gdiplus::REAL)obj->font_size, Gdiplus::Point(0, 0), &strFormat);

    Gdiplus::Pen pen(obj->outline, (Gdiplus::REAL)obj->outline_width);
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

  *w = rect.Width + obj->margin;
  *h = rect.GetBottom();
}
