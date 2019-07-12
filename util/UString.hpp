#ifndef USTRING_HPP_
# define USTRING_HPP_

// Unicode string designed to work on both Windows and Linux.
// The strings can either be used as UTF-8 or as wchar_t.
// For Linux, the conversion is done with iconv.
// For Windows, it uses MultiByteToWideChar and WideCharToMultiByte.

# include <string>
# include <memory>

# ifdef _WIN32
#  define USTRING_WINDOWS
# else
// We don't add a define, we will just test #ifndef USTRING_WINDOWS
#  include <iconv.h>
# endif

/*# ifdef USTRING_WINDOWS
  enum Encoding{
    UTF8 = CP_UTF8,
    SHIFT_JIS = 932,
    CONSOLE = CP_OEMCP
    // No value for wchar_t (UTF-16 or UTF-32), they will have their own functions.
  };*/

class UString : public std::string
{
public:
  enum Encoding {
    UTF8,
    WCHAR_T,
    SHIFT_JIS,
    CONSOLE,
  };

private:
  std::shared_ptr<wchar_t[]> wstring;

  // Convert a string from an encoding to another.
  // The returned value is allocated with new Dst[] and must be freed with delete[].
  template<typename Dst, typename Src>
  Dst *convert_string(const Src *src, Encoding src_enc, Encoding dst_enc);

public:
  UString();
  UString(const char *src, Encoding encoding = UTF8);
  UString(const wchar_t *src);
  UString(const std::string& src, Encoding encoding = UTF8);
  ~UString();

  // Returns a wide representation of the string.
  std::shared_ptr<wchar_t[]> w_str();

  UString& assign(const std::string& str, Encoding encoding = UTF8);
  UString& assign(const std::string& str, size_t subpos, size_t sublen, Encoding encoding = UTF8);
  UString& assign(const char* s, Encoding encoding = UTF8);
  UString& assign(const char* s, size_t n, Encoding encoding = UTF8);
  UString& assign(size_t n, char c, Encoding encoding = UTF8);
};

#endif /* !USTRING_HPP_ */
