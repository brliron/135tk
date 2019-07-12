#include <map>
#include <mutex>
#include <vector>
#include <string.h>
#include "UString.hpp"

std::map<std::pair<UString::Encoding, UString::Encoding>, iconv_t> iconv_cache;
std::mutex iconv_cache_mutex;

static const char *encoding_to_name(UString::Encoding enc)
{
  switch (enc)
    {
    case UString::UTF8:
      return "utf-8";
    case UString::WCHAR_T:
      return "WCHAR_T";
    case UString::SHIFT_JIS:
      return "cp932";
    case UString::CONSOLE:
      return "utf-8"; // Why would you use something else on your Linux terminal?
    default:
      throw std::logic_error("Unknown encoding");
    }
}

static void init_cache()
{
  std::lock_guard<std::mutex> iconv_cache_guard(iconv_cache_mutex);

  if (!iconv_cache.empty())
    return ;
  auto add_to_cache = [](UString::Encoding src, UString::Encoding dst) {
    iconv_cache[std::pair<UString::Encoding, UString::Encoding>(src, dst)] =
      iconv_open(encoding_to_name(dst), encoding_to_name(src));
  };

  std::vector<UString::Encoding> encodings = {
    UString::UTF8,
    UString::WCHAR_T,
    UString::SHIFT_JIS,
    UString::CONSOLE,
  };
  for (UString::Encoding i : encodings) {
    for (UString::Encoding j : encodings) {
      if (i != j)
	add_to_cache(i, j);
    }
  }
}

template<typename T>
static size_t strlen_template(T str)
{
  int i;
  for (i = 0; str[i]; i++)
    {}
  return i;
}

template<typename Dst, typename Src>
Dst *UString::convert_string(const Src *src, UString::Encoding src_enc, UString::Encoding dst_enc)
{
  size_t src_len = strlen_template(src) + 1;

  if (src_enc == dst_enc) {
    if (!std::is_same<Src, Dst>::value)
      throw std::logic_error("Running UString::convert_string with same source/destination encodings but different types");
    Dst *dst = new Dst[src_len];
    memcpy(dst, src, src_len * sizeof(Src));
    return dst;
  }

  if (iconv_cache.empty())
    init_cache();

  size_t dst_len = src_len * 4;
  char *src_ = reinterpret_cast<char *>(const_cast<Src*>(src));
  Dst  *dst = new Dst[dst_len / sizeof(Dst)];
  char *dst_ = reinterpret_cast<char *>(dst);

  iconv(iconv_cache.at(std::pair<UString::Encoding, UString::Encoding>(src_enc, dst_enc)),
    &src_, &src_len, &dst_, &dst_len);
  return dst;
}

template char    *UString::convert_string<char,    char   >(const char    *src, UString::Encoding src_enc, UString::Encoding dst_enc);
template char    *UString::convert_string<char,    wchar_t>(const wchar_t *src, UString::Encoding src_enc, UString::Encoding dst_enc);
template wchar_t *UString::convert_string<wchar_t, char   >(const char    *src, UString::Encoding src_enc, UString::Encoding dst_enc);
