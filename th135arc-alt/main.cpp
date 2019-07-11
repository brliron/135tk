#include <stdio.h>
#include "File.hpp"
#include "TFPK.hpp"

int main(int ac, char **av)
{
  if (ac != 3 || (av[1][0] != '/' && av[1][0] != '-') ||
      (av[1][1] != 'x' && av[1][1] != 'p'))
    {
      printf("Usage: %s /x file.pak\n"
	     "Or:    %s /p dir\n",
	     av[0], av[0]);
      return 0;
    }

  UString::UString fn(av[2], UString::CONSOLE);
  if (av[1][1] == 'x')
    {
      File file(fn, File::READ);
      if (!file)
	{
	  printf("Could not open %s: %s\n", fn.c_str(), file.error());
	  return 1;
	}
      std::unique_ptr<TFPK> arc = TFPK::read(file);
      if (!arc)
	return 1;
      size_t ext = fn.rfind(".");
      if (ext != std::string::npos)
        fn.erase(ext);
      else
	fn += "_extracted";
      arc->extract_all(file, fn);
    }
  else if (av[1][1] == 'p')
    {
      std::unique_ptr<TFPK> arc = std::make_unique<TFPK1>();
      UString::UString pak_fn = fn + ".pak";
      File file(pak_fn, File::WRITE | File::TRUNCATE);
      arc->repack_all(file, fn);
    }
  else
    printf("Error: unknown parameter %s\n", av[1]);

  return 0;
}
