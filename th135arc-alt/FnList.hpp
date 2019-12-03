#ifndef FNLIST_HPP_
# define FNLIST_HPP_

# include <map>
# include <filesystem>
# include "OS.hpp"
# include "Rsa.hpp"

class FnList : public std::map<uint32_t, std::filesystem::path>
{
public:
  FnList() {}
  virtual ~FnList() {}
  void add(const OS::sjisstring& fn);
  bool readFromTextFile(const std::filesystem::path& fn);
  bool readFromJsonFile(const std::filesystem::path& fn);
  bool readFromArchive(Rsa& rsa, uint32_t dirCount);
  std::filesystem::path hashToFn(uint32_t hash);

  virtual uint32_t SpecialFNVHash(const OS::sjisstring& path, uint32_t initHash = 0x811C9DC5u) = 0;
};

class FnList0 : public FnList
{
public:
  FnList0() {}
  ~FnList0() {}

  uint32_t SpecialFNVHash(const OS::sjisstring& path, uint32_t initHash = 0x811C9DC5u);
};

class FnList1 : public FnList
{
public:
  FnList1() {}
  ~FnList1() {}

  uint32_t SpecialFNVHash(const OS::sjisstring& path, uint32_t initHash = 0x811C9DC5u);
};

#endif /* !FNLIST_HPP_ */
