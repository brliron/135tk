#include <windows.h>
#include <stdexcept>
#include "UString.hpp"

static DWORD encoding_to_cp(UString::Encoding enc)
{
  switch (enc)
    {
    case UString::UTF8:
      return CP_UTF8;
    case UString::WCHAR_T:
      return 0xFFFFFFFF;
    case UString::SHIFT_JIS:
      return 932;
    case UString::CONSOLE:
      return CP_OEMCP;
    default:
      throw std::logic_error("Unknown encoding");
    }
}

template<>
wchar_t *UString::convert_string(const char *src, UString::Encoding src_enc, UString::Encoding dst_enc)
{
  if (src_enc == UString::WCHAR_T || dst_enc != UString::WCHAR_T)
    throw std::logic_error("UString::convert_string: mismatch between encodings and template types");

  size_t dst_len = MultiByteToWideChar(encoding_to_cp(src_enc), 0, src, -1, nullptr, 0);
  wchar_t *dst = new wchar_t[dst_len];
  MultiByteToWideChar(encoding_to_cp(src_enc), 0, src, -1, dst, dst_len);
  return dst;
}

template<>
char *UString::convert_string(const wchar_t *src, UString::Encoding src_enc, UString::Encoding dst_enc)
{
  if (src_enc != UString::WCHAR_T || dst_enc == UString::WCHAR_T)
    throw std::logic_error("UString::convert_string: mismatch between encodings and template types");

  size_t dst_len = WideCharToMultiByte(encoding_to_cp(dst_enc), 0, src, -1, nullptr, 0, nullptr, nullptr);
  char *dst = new char[dst_len];
  WideCharToMultiByte(encoding_to_cp(dst_enc), 0, src, -1, dst, dst_len, nullptr, nullptr);
  return dst;
}

template<>
char *UString::convert_string(const char *src, UString::Encoding src_enc, UString::Encoding dst_enc)
{
  // Windows can't convert a multi-byte string to another, so we go through a wide string.
  wchar_t *tmp = convert_string<wchar_t, char   >(src, src_enc,          UString::WCHAR_T);
  char    *dst = convert_string<char,    wchar_t>(tmp, UString::WCHAR_T, dst_enc);
  delete[] tmp;
  return dst;
}
