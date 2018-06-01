#include <string.h>
#include "UString.hpp"

UString::UString::UString()
  : wstring(nullptr)
{}

UString::UString::UString(const char *src, Encoding encoding)
  : std::string(src), wstring(nullptr)
{
  this->assign(src, encoding);
}

UString::UString::UString(const wchar_t *src)
{
  char *str_utf8 = this->converter.from_wide_string(src, UTF8);
  this->std::string::assign(str_utf8);
  delete[] str_utf8;
}

UString::UString::UString(const std::string& src, Encoding encoding)
  : std::string(src), wstring(nullptr)
{
  this->assign(src, encoding);
}

UString::UString::~UString()
{
  if (wstring)
    delete[] wstring;
}

UString::UString& UString::UString::assign(const std::string& str, Encoding encoding)
{ return this->assign(str.c_str(), encoding); }
UString::UString& UString::UString::assign(const std::string& str, size_t subpos, size_t sublen, Encoding encoding)
{
  if (subpos > str.length())
    return *this; // TODO: throw an exception
  return this->assign(str.c_str() + subpos, sublen, encoding);
}

UString::UString& UString::UString::assign(const char* s, size_t n, Encoding encoding)
{
  if (s[n - 1] == '\0')
    return this->assign(s, encoding);

  char *str = new char[n + 1];
  memcpy(str, s, n);
  str[n] = '\0';
  this->assign(str, encoding);
  delete[] str;
  return *this;
}

UString::UString& UString::UString::assign(const char* s, Encoding encoding)
{
  wchar_t *str_w = this->converter.to_wide_string(s, encoding);
  char *str_utf8 = this->converter.from_wide_string(str_w, UTF8);
  this->std::string::assign(str_utf8);
  delete[] str_w;
  delete[] str_utf8;
  return *this;
}

UString::UString& UString::UString::assign(size_t n, char c, Encoding encoding)
{
  char str[2];
  str[0] = c;
  str[1] = '\0';
  wchar_t *str_w = this->converter.to_wide_string(str, encoding);
  char *str_utf8 = this->converter.from_wide_string(str_w, UTF8);
  this->clear();
  for (size_t i = 0; i < n; i++)
    this->std::string::append(str_utf8);
  delete[] str_w;
  delete[] str_utf8;
  return *this;
}
