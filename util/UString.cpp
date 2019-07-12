#include <string.h>
#include "UString.hpp"

UString::UString()
  : wstring(nullptr)
{}

UString::UString(const char *src, Encoding encoding)
  : std::string(src), wstring(nullptr)
{
  this->assign(src, encoding);
}

UString::UString(const wchar_t *src)
{
  char *str_utf8 = this->convert_string<char, wchar_t>(src, WCHAR_T, UTF8);
  this->std::string::assign(str_utf8);
  delete[] str_utf8;
}

UString::UString(const std::string& src, Encoding encoding)
  : std::string(src), wstring(nullptr)
{
  this->assign(src, encoding);
}

UString::UString::~UString()
{}

std::shared_ptr<wchar_t[]> UString::w_str()
{
  this->wstring.reset(this->convert_string<wchar_t, char>(this->c_str(), UTF8, WCHAR_T));
  return this->wstring;
}

UString& UString::assign(const std::string& str, Encoding encoding)
{ return this->assign(str.c_str(), encoding); }
UString& UString::assign(const std::string& str, size_t subpos, size_t sublen, Encoding encoding)
{
  if (subpos > str.length())
    return *this; // TODO: throw an exception
  return this->assign(str.c_str() + subpos, sublen, encoding);
}

UString& UString::assign(const char* s, size_t n, Encoding encoding)
{
  if (s[n - 1] == '\0')
    return this->assign(s, encoding);

  std::string tmp_s(s, n);
  this->assign(tmp_s, encoding);
  return *this;
}

UString& UString::assign(const char* s, Encoding encoding)
{
  char *str_utf8 = this->convert_string<char, char>(s, encoding, UTF8);
  this->std::string::assign(str_utf8);
  delete[] str_utf8;
  return *this;
}

UString& UString::assign(size_t n, char c, Encoding encoding)
{
  char str[2];
  str[0] = c;
  str[1] = '\0';
  char *str_utf8 = this->convert_string<char, char>(str, encoding, UTF8);
  this->clear();
  for (size_t i = 0; i < n; i++)
    this->std::string::append(str_utf8);
  delete[] str_utf8;
  return *this;
}
