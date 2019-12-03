#include <algorithm>
#include <functional>
#include <mutex>
#include <vector>
#include "OS.hpp"
#ifdef OS_WINDOWS
# include <windows.h>
#else
# include <sys/stat.h>
# include <sys/types.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
# include <iconv.h>
#endif

// Calls a function repetively with a growing buffer.
// The function should return true if it succeeded,
// or false if it needs a bigger buffer.
// Returns the buffer.
template<typename T>
static std::vector<T> grow_buffer(std::function< bool(std::vector<T>&) > fn)
{
  std::vector<T> buffer;
  size_t size = 64;

  while (true) {
    size *= 2;
    buffer.resize(size);
    if (fn(buffer)) {
      return buffer;
    }
  }
}

#ifdef OS_WINDOWS

std::filesystem::path OS::getSelfPath()
{
  std::vector<wchar_t> path = grow_buffer<wchar_t>([] (std::vector<wchar_t>& path) {
    SetLastError(0);
    DWORD ret = GetModuleFileNameW(nullptr, path.data(), path.size());
    if (ret == path.size() && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
      return false;
    }
    if (ret == 0) {
      throw std::runtime_error("GetModuleFileNameW() failed");
    }
    return true;
  });

  return path.data();
}

std::filesystem::path OS::sjisToPath(const OS::sjisstring& src)
{
  size_t dst_len = MultiByteToWideChar(932, 0, src.c_str(), -1, nullptr, 0);
  auto dst = std::make_unique<wchar_t[]>(dst_len);
  MultiByteToWideChar(932, 0, src.c_str(), -1, dst.get(), dst_len);
  return dst.get();
}

#else

std::filesystem::path OS::getSelfPath()
{
  std::vector<char> path = grow_buffer<char>([] (std::vector<char>& path) {
    ssize_t ret = readlink("/proc/self/exe", path.data(), path.size() - 1);
    if (ret == -1) {
      throw std::runtime_error(std::string("readlink() failed: ") + strerror(errno));
    }
    if (ret == static_cast<ssize_t>(path.size()) - 1) {
      return false;
    }
    path[ret] = '\0';
    return true;
  });

  return path.data();
}

static std::mutex iconv_cache_mutex;
static iconv_t& iconv_get_cached()
{
  std::lock_guard<std::mutex> iconv_cache_guard(iconv_cache_mutex);
  static iconv_t *iconv_cd;

  if (!iconv_cd)
    {
      iconv_cd = new iconv_t[1];
      *iconv_cd = iconv_open("utf-8", "cp932");
    }

  return *iconv_cd;
}

std::filesystem::path OS::sjisToPath(const OS::sjisstring& src)
{
  size_t src_len = src.length() + 1;

  iconv_t& cd = iconv_get_cached();

  size_t dst_len = src_len * 4;
  char *src_ = const_cast<char*>(src.c_str());
  auto dst = std::make_unique<char[]>(dst_len);
  char *dst_ = dst.get();

  iconv(cd, &src_, &src_len, &dst_, &dst_len);
  return (char8_t*)dst.get();
}

#endif
