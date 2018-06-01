#ifndef FILE_HPP_
# define FILE_HPP_

# include "UString.hpp"

class File
{
private:
# ifdef USTRING_WINDOWS
#  include <Windows.h>
  HANDLE hFile;
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
  File(UString::UString fn, int flags);
  File(const File&) = delete;
  ~File();
  File& operator=(const File&) = delete;

  bool open(UString::UString fn, int flags);
  void close();
  int read(void *buffer, size_t size);
  int write(void *buffer, size_t size);
  bool seek(ssize_t off, Seek pos);
  size_t tell();
  operator bool();
  const char *error();

  uint8_t readByte();
  uint32_t readDword();
};

#endif /* !FILE_HPP_ */
