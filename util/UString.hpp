#ifndef USTRING_HPP_
# define USTRING_HPP_

// Unicode string designed to work on both Windows and Linux.
// The strings can either be used as UTF-8 or as wchar_t.
// For Linux, the conversion is done with iconv.
// For Windows, it uses MultiByteToWideChar and WideCharToMultiByte.

# include <string>

# if 0
#  define USTRING_WINDOWS
# else
// We don't add a define, we will just test #ifndef USTRING_WINDOWS
#  include <iconv.h>
# endif

namespace UString
{
# ifdef USTRING_WINDOWS
  enum Encoding{
    UTF8 = CP_UTF8,
    SHIFT_JIS = 932,
    CONSOLE = CP_OEMCP
    // No value for wchar_t (UTF-16 or UTF-32), they will have their own functions.
  };
# else
  typedef const char* Encoding;
  static const Encoding UTF8      = "utf-8";
  static const Encoding SHIFT_JIS = "cp932";
  static const Encoding CONSOLE   = "utf-8"; // Why would you use something else on your Linux terminal?
  // No value for wchar_t (UTF-16 or UTF-32), they will have their own functions.
# endif

  class UString : public std::string
  {
  private:
    class Converter
    {
    private:
#ifndef USTRING_WINDOWS
      Encoding from_enc;
      iconv_t  from_cd;
      Encoding to_enc;
      iconv_t  to_cd;
#endif

      void init_cd_from(Encoding enc);
      void init_cd_to(Encoding enc);
    public:
      Converter();
      Converter(const Converter& src);
      ~Converter();
      Converter& operator=(const Converter& src);
      // src must be null-terminated
      char *from_wide_string(const wchar_t *src, Encoding encoding);
      wchar_t *to_wide_string(const char *src, Encoding encoding);
    };

    Converter converter;
    wchar_t   *wstring;

  public:
    UString();
    UString(const char *src, Encoding encoding = UTF8);
    UString(const wchar_t *src);
    UString(const std::string& src, Encoding encoding = UTF8);
    ~UString();

    // Returns a wide representation of the string.
    // This representation is valid while the string object doesn't change.
    const wchar_t *w_str();

    UString& assign(const std::string& str, Encoding encoding = UTF8);
    UString& assign(const std::string& str, size_t subpos, size_t sublen, Encoding encoding = UTF8);
    UString& assign(const char* s, Encoding encoding = UTF8);
    UString& assign(const char* s, size_t n, Encoding encoding = UTF8);
    UString& assign(size_t n, char c, Encoding encoding = UTF8);
  };
}

#endif /* !USTRING_HPP_ */
