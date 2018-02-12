#include <stdio.h>
#include "TFBMTool.h"

int wmain(int ac, wchar_t **av)
{
  if (ac < 3 || (av[1][0] != L'/' && av[1][0] != L'-') || (av[1][1] != L'x' && av[1][1] != L'p')) {
    wprintf(L"Usage: %s (/x|/p) in.[bmp|png] [palette.bmp]\n"
	    "palette.bmp is used for /x if (and only if) the input file\n"
	    "is a 8-bit file with a palette.\n"
	    "palette.bmp is ignored for /p.\n", av[0]);
    return 0;
  }

  if (av[1][1] == L'x')
    convert_TFBM_to_PNG(av[2], av[3], av[2]);
  else
    convert_PNG_to_TFBM(av[2], av[2]);
  return 0;
}
