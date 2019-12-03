#ifndef OS_HPP_
# define OS_HPP_

# ifdef _WIN32
#  define OS_WINDOWS
# else
// We don't add a define, we will just test #ifndef OS_WINDOWS
# endif

#include <filesystem>
#include <string>

namespace OS
{
  class sjisstring : public std::basic_string<char>
  {
  public:
    sjisstring() {}
    sjisstring(const sjisstring& s) : basic_string(s) {}
    sjisstring(const char *s) : basic_string(s) {}
  };

  std::filesystem::path getSelfPath();
  std::filesystem::path sjisToPath(const OS::sjisstring& str);
}

#endif /* !OS_H_ */
