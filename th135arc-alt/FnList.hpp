#ifndef FNLIST_HPP_
# define FNLIST_HPP_

# include <map>
# include "UString.hpp"
# include "Rsa.hpp"

class FnList : public std::map<uint32_t, UString::UString>
{
public:
  FnList() {}
  virtual ~FnList() {}
  void add(UString::UString fn);
  bool readFromTextFile(UString::UString fn);
  bool readFromJsonFile(UString::UString fn);
  bool readFromArchive(Rsa& rsa, uint32_t dirCount);
  UString::UString hashToFn(uint32_t hash);

  virtual uint32_t SpecialFNVHash(const char *path, uint32_t initHash = 0x811C9DC5u) = 0;
};

class FnList0 : public FnList
{
public:
  FnList0() {}
  ~FnList0() {}

  uint32_t SpecialFNVHash(const char *path, uint32_t initHash = 0x811C9DC5u);
};

class FnList1 : public FnList
{
public:
  FnList1() {}
  ~FnList1() {}

  uint32_t SpecialFNVHash(const char *path, uint32_t initHash = 0x811C9DC5u);
};

#endif /* !FNLIST_HPP_ */
