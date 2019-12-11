#ifndef OS_HPP_
# define OS_HPP_

# ifdef _WIN32
#  define OS_WINDOWS
# else
// We don't add a define, we will just test #ifndef OS_WINDOWS
#  include <iconv.h>
# endif

#include <filesystem>
#include <string>

namespace OS
{
  class SjisConverter
  {
  private:
#ifndef OS_WINDOWS
    iconv_t from;
    iconv_t to;
#endif

  public:
    SjisConverter();
    ~SjisConverter();
    std::filesystem::path fromSjis(const std::string& str);
    std::string toSjis(const std::filesystem::path& path);
  };

  std::filesystem::path getSelfPath();
}

#endif /* !OS_H_ */
