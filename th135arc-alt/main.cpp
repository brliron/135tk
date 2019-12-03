#include <iostream>
#include <fstream>
#include "TFPK.hpp"

#ifdef OS_WINDOWS
int wmain(int ac, wchar_t **av)
#else
int main(int ac, char **av)
#endif
{
  if (ac != 3 || (av[1][0] != '/' && av[1][0] != '-') ||
      (av[1][1] != 'x' && av[1][1] != 'p'))
    {
      std::cout << "Usage: " << av[0] << " -x file.pak" << std::endl
		<< "Or:    " << av[0] << " -p dir" << std::endl;
      return 0;
    }

  std::filesystem::path fn = av[2];
  if (av[1][1] == 'x')
    {
      std::ifstream file(fn, std::ifstream::binary);
      if (!file)
	{
	  std::cerr << "Could not open " << fn << std::endl;
	  return 1;
	}
      std::unique_ptr<TFPK> arc = TFPK::read(file);
      if (!arc)
	return 1;
      if (fn.has_extension())
	fn.replace_extension();
      else
	fn += "_extracted";
      arc->extract_all(file, fn);
    }
  else if (av[1][1] == 'p')
    {
      std::unique_ptr<TFPK> arc = std::make_unique<TFPK1>();
      std::filesystem::path pak_fn = std::filesystem::path(fn).concat(".pak");
      std::ofstream file(pak_fn, std::ofstream::trunc | std::ofstream::binary);
      arc->repack_all(file, fn);
    }
  else
    std::cerr << "Error: unknown parameter " << av[1] << std::endl;

  return 0;
}
