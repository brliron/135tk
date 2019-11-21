#ifndef OS_HPP_
# define OS_HPP_

# ifdef _WIN32
#  define OS_WINDOWS
# else
// We don't add a define, we will just test #ifndef OS_WINDOWS
# endif

class UString;

namespace OS
{
  bool mkdir(UString& path);
  UString getSelfPath();
}

#endif /* !OS_H_ */
