#include "TFBMTool.h"

static void callback(LPCWSTR path)
{
  WCHAR palette[MAX_PATH];
  wcscpy(palette, path);
  LPWSTR palette_fn = wcsrchr(palette, L'\\');
  if (!palette_fn)
    palette_fn = wcsrchr(palette, L'/');
  if (palette_fn)
    palette_fn++;
  else
    palette_fn = palette;
  wcscpy(palette_fn, L"palette000.bmp");
  convert_TFBM_to_PNG(path, palette, path);
}

int wmain()
{
  WCHAR dir[MAX_PATH] = L".";
  find_images(dir, callback);
  return 0;
}
