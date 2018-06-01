#include <string.h>
#include "UString.hpp"

UString::UString::Converter::Converter()
  : from_enc(""), from_cd((iconv_t)-1), to_enc(""), to_cd((iconv_t)-1)
{}

UString::UString::Converter::~Converter()
{
  if (from_cd != (iconv_t)-1)
    iconv_close(from_cd);
  if (to_cd != (iconv_t)-1)
    iconv_close(to_cd);
}

// Don't copy the cache from src, start with a new cache.
UString::UString::Converter::Converter(const UString::Converter&)
  : from_enc(""), from_cd((iconv_t)-1), to_enc(""), to_cd((iconv_t)-1)
{}

UString::UString::Converter& UString::UString::Converter::operator=(const UString::Converter&)
{
  return *this;
}

void UString::UString::Converter::init_cd_from(Encoding enc)
{
  if (strcmp(enc, this->from_enc) == 0)
    return ;
  if (from_cd != (iconv_t)-1)
    iconv_close(from_cd);
  this->from_cd = iconv_open(enc, "WCHAR_T");
  this->from_enc = enc;
}

void UString::UString::Converter::init_cd_to(Encoding enc)
{
  if (strcmp(enc, this->to_enc) == 0)
    return ;
  if (to_cd != (iconv_t)-1)
    iconv_close(to_cd);
  this->to_cd = iconv_open("WCHAR_T", enc);
  this->to_enc = enc;
}

char *UString::UString::Converter::from_wide_string(const wchar_t *src, Encoding encoding)
{
  this->init_cd_from(encoding);

  size_t src_len = 0;
  while (src[src_len])
    src_len++;

  size_t dst_len = src_len;
  char *dst = new char[dst_len + 1];
  char *dst_ptr = dst;

  src_len *= sizeof(wchar_t);
  iconv(this->from_cd, (char**)&src, &src_len, &dst_ptr, &dst_len);
  *dst_ptr = 0;

  return dst;
}

wchar_t *UString::UString::Converter::to_wide_string(const char *src, Encoding encoding)
{
  this->init_cd_to(encoding);

  size_t src_len = strlen(src);
  size_t dst_len = src_len;
  // In UTF-16, a code point can have either 2 or 4 bytes.
  if (sizeof(wchar_t) == 2)
    dst_len *= 2;

  wchar_t *dst = new wchar_t[dst_len + 1];
  wchar_t *dst_ptr = dst;

  dst_len *= sizeof(wchar_t);
  iconv(this->to_cd, const_cast<char**>(&src), &src_len, (char**)&dst_ptr, &dst_len);
  *dst_ptr = 0;

  return dst;
}
