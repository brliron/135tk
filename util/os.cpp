#include <algorithm>
#include <functional>
#include <vector>
#include "os.hpp"
#include "UString.hpp"
#ifdef OS_WINDOWS
# include <windows.h>
#else
# include <sys/stat.h>
# include <sys/types.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
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
bool OS::mkdir(UString& path)
{
  if (CreateDirectoryW(path.w_str().get(), NULL) != 0) {
    return true;
  }
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    return true;
  }
  return false;
}

UString OS::getSelfPath()
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

  std::replace(path.begin(), path.end(), '\\', '/');
  return path.data();
}

#else

bool OS::mkdir(UString& path)
{
  if (::mkdir(path.c_str(), 0777) != -1)
    return true;
  if (errno == EEXIST) {
    return true;
  }
  return false;
}

UString OS::getSelfPath()
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

#endif
