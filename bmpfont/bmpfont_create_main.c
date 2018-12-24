#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmpfont_create.h"

// For debugging
//#define LATIN_ONLY

int options(void *bmpfont, int ac, char **av)
{
  // --exe defaults to bmpfont. I think it's a good value for libraries,
  // so I don't think I need to document it.
  // Create an issue or send a pull request if you don't agree.
  bmpfont_add_option(bmpfont, "--exe", av[0]);

  for (int i = 1; i < ac; i += 2)
    {
      if (strcmp(av[i], "--help") == 0)
	{
	  bmpfont_add_option(bmpfont, "--help", NULL);
	  return 0;
	}
      if (av[i][0] != '-' || av[i][1] != '-')
	{
	  fprintf(stderr, "%s: unrecognized option '%s'\n"
			  "Try '%s --help' for more information.",
		    av[0], av[i], av[0]);
	  return 0;
	}
      if (i + 1 >= ac)
	{
	  fprintf(stderr, "%s: option '%s' requires an argument\n"
			  "Try '%s --help' for more information.",
		    av[0], av[i], av[0]);
	  return 0;
	}
      if (bmpfont_add_option(bmpfont, av[i], av[i + 1]) == 0)
	return 0;
    }
  return 1;
}

int main(int ac, char **av)
{
  void *bmpfont = bmpfont_init();
  if (!options(bmpfont, ac, av))
    return 1;
#ifdef LATIN_ONLY
// Debug - generate only 256 characters.
  char chars_list[256];
  memset(chars_list, 1, 256);
  bmpfont_add_option_binary(bmpfont, "--chars-list", chars_list, 256);
#endif
  bmpfont_run(bmpfont);
  bmpfont_free(bmpfont);
  return 0;
}
