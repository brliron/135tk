#include <stdexcept>
#include <sstream>
#include <string.h>
#include "os.hpp"
#ifndef OS_WINDOWS
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <unistd.h>
#endif
#include "File.hpp"

File::File()
{}

File::File(UString fn, int flags)
{
  this->open(fn, flags);
}

File::~File()
{
  this->close();
}

#ifdef OS_WINDOWS
bool File::open(UString fn, int flags)
{
  if (this->hFile == INVALID_HANDLE_VALUE)
    this->close();

  DWORD dwDesiredAccess = (flags & READ ? GENERIC_READ : 0) | (flags & WRITE ? GENERIC_WRITE : 0);
  DWORD dwCreationDisposition = 0;
  if ((flags & (WRITE | TRUNCATE)) == (WRITE | TRUNCATE))
    dwCreationDisposition = CREATE_ALWAYS;
  else if (flags & WRITE)
    dwCreationDisposition = OPEN_ALWAYS;
  else if (flags & READ)
    dwCreationDisposition = OPEN_EXISTING;

  this->hFile = CreateFileW(fn.w_str().get(), dwDesiredAccess, FILE_SHARE_READ, nullptr, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, nullptr);
  this->err = GetLastError();
  return this->hFile != INVALID_HANDLE_VALUE;
}

void File::close()
{
  if (this->hFile != INVALID_HANDLE_VALUE)
    CloseHandle(this->hFile);
  this->hFile = INVALID_HANDLE_VALUE;
}

int File::read(void *buffer, size_t size)
{
  DWORD nRead;
  BOOL ret = ReadFile(this->hFile, buffer, size, &nRead, nullptr);
  if (ret == FALSE) {
    this->err = GetLastError();
    return false;
  }
  return nRead;
}

int File::write(const void *buffer, size_t size)
{
  DWORD nWritten;
  BOOL ret = WriteFile(this->hFile, buffer, size, &nWritten, nullptr);
  if (ret == FALSE) {
    this->err = GetLastError();
    return false;
  }
  return nWritten;
}

bool File::seek(ssize_t off, Seek pos)
{
  DWORD dwMoveMethod;
  switch (pos)
    {
    case Seek::SET:
      dwMoveMethod = FILE_BEGIN;
      break;
    case Seek::CUR:
      dwMoveMethod = FILE_CURRENT;
      break;
    case Seek::END:
      dwMoveMethod = FILE_END;
      break;
    default:
      throw std::logic_error("File::seek: wrong value for parameter pos");
    }
  bool ret = SetFilePointer(this->hFile, off, nullptr, dwMoveMethod) != INVALID_SET_FILE_POINTER;
  this->err = GetLastError();
  return ret;
}

size_t File::tell()
{
  DWORD ret = SetFilePointer(this->hFile, 0, nullptr, FILE_CURRENT);
  this->err = GetLastError();
  return ret;
}

File::operator bool()
{
  return this->hFile != INVALID_HANDLE_VALUE;
}

std::string File::error()
{
  std::ostringstream ss;
  ss << "Windows error code " << this->err;
  return ss.str();
}
#else
bool File::open(UString fn, int flags)
{
  if (this->fd != -1)
    this->close();

  int open_flags = 0;
  if (flags & (READ | WRITE))
    open_flags = O_RDWR;
  else if (flags & READ)
    open_flags = O_RDONLY;
  else if (flags & WRITE)
    open_flags = O_WRONLY;

  if (flags & WRITE)
    open_flags |= O_CREAT;
  if (flags & TRUNCATE)
    open_flags |= O_TRUNC;

  if (flags & WRITE)
    this->fd = ::open(fn.c_str(), open_flags, 0644);
  else
    this->fd = ::open(fn.c_str(), open_flags);

  this->err = errno;
  return this->fd != -1;
}

void File::close()
{
  if (this->fd != -1)
    ::close(this->fd);
  this->fd = -1;
}

int File::read(void *buffer, size_t size)
{
  int ret = ::read(this->fd, buffer, size);
  this->err = errno;
  return ret;
}

int File::write(const void *buffer, size_t size)
{
  int ret = ::write(this->fd, buffer, size);
  this->err = errno;
  return ret;
}

bool File::seek(ssize_t off, Seek pos)
{
  int whence;
  switch (pos)
    {
    case Seek::SET:
      whence = SEEK_SET;
      break;
    case Seek::CUR:
      whence = SEEK_CUR;
      break;
    case Seek::END:
      whence = SEEK_END;
      break;
    default:
      throw std::logic_error("File::seek: wrong value for parameter pos");
    }
  bool ret = lseek(this->fd, off, whence) != -1;
  this->err = errno;
  return ret;
}

size_t File::tell()
{
  int ret = lseek(this->fd, 0, SEEK_CUR);
  this->err = errno;
  return ret;
}

File::operator bool()
{
  return this->fd != -1;
}

std::string File::error()
{
  return strerror(this->err);
}
#endif

uint8_t File::readByte()
{
  uint8_t n;
  if (this->read(&n, 1) != 1)
    throw std::runtime_error("File::readByte() failed");
  return n;
}

uint32_t File::readDword()
{
  uint32_t n;
  if (this->read(&n, 4) != 4)
    throw std::runtime_error("File::readDword() failed");
  return n;
}
