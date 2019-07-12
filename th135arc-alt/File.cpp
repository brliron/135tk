#include <stdexcept>
#include <string.h>
#ifndef USTRING_WINDOWS
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <unistd.h>
#endif
#include "File.hpp"
#include "Rsa.hpp"

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

#ifdef USTRING_WINDOWS
# error TODO
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

int File::write(void *buffer, size_t size)
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
  int ret = lseek(this->fd, off, whence) != -1;
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

const char *File::error()
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
