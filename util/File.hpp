#ifndef FILE_HPP_
# define FILE_HPP_

# include "os.hpp"
# include "UString.hpp"
# ifdef OS_WINDOWS
#  include <windows.h>
# endif

class File
{
private:
# ifdef OS_WINDOWS
  HANDLE hFile = INVALID_HANDLE_VALUE;
  DWORD err = 0;
# else
  int fd = -1;
  int err = 0;
# endif

public:
  static const int READ = 1;
  static const int WRITE = 2;
  static const int TRUNCATE = 4;

  enum class Seek
    {
      SET,
      CUR,
      END
    };

  File();
  File(UString fn, int flags);
  File(const File&) = delete;
  ~File();
  File& operator=(const File&) = delete;

  bool open(UString fn, int flags);
  void close();
  int read(void *buffer, size_t size);
  int write(const void *buffer, size_t size);
  bool seek(ssize_t off, Seek pos);
  size_t tell();
  operator bool();
  std::string error();

  uint8_t readByte();
  uint32_t readDword();
};

#endif /* !FILE_HPP_ */
