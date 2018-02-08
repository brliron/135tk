#include <stdio.h>
#include "TFBMTool.h"

int main(int ac, char **av)
{
  if (ac < 2) {
    printf("Usage: %s in.[bmp|png] [palette.bmp]\n", av[0]);
    return 0;
  }

  convert_TFBM_to_PNG(av[1], av[2], av[1]);
  return 0;
}
