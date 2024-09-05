#include <stdio.h>
#include "TFBMTool.h"

int wmain(int ac, wchar_t **av)
{
  if (ac < 3 || (av[1][0] != L'/' && av[1][0] != L'-') || (av[1][1] != L'x' && av[1][1] != L'p')) {
    wprintf(L"Usage: %S (/x|/p) in.[bmp|png] [palette.bmp]\n"
	    "palette.bmp is used if (and only if) the input file is a 8-bit file with a palette.\n"
	    "For /x, palette.bmp is required for 8-bit files, and is used as the palette for the input file.\n"
	    "For /p, palette.bmp is optional, and if specified, will contain the file's palette\n"
		"in the TFPA format.", av[0]);
    return 0;
  }

  if (av[1][1] == L'x')
    convert_TFBM_to_PNG(av[2], av[3], av[2]);
  else
    convert_PNG_to_TFBM(av[2], av[2], av[3]);
  return 0;
}
