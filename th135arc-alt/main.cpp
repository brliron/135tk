#include <iostream>
#include "File.hpp"
#include "TFPK.hpp"

int main(int ac, char **av)
{
  if (ac != 3 || (av[1][0] != '/' && av[1][0] != '-') ||
      (av[1][1] != 'x' && av[1][1] != 'p'))
    {
      std::cout << "Usage: " << av[0] << " -x file.pak" << std::endl
		<< "Or:    " << av[0] << " -p dir" << std::endl;
      return 0;
    }

  UString fn(av[2], UString::CONSOLE);
  if (av[1][1] == 'x')
    {
      File file(fn, File::READ);
      if (!file)
	{
	  std::cerr << "Could not open " << fn << ": " << file.error() << std::endl;
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
      UString pak_fn = fn + ".pak";
      File file(pak_fn, File::WRITE | File::TRUNCATE);
      arc->repack_all(file, fn);
    }
  else
    std::cerr << "Error: unknown parameter " << av[1] << std::endl;

  return 0;
}
