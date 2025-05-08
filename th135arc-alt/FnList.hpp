#ifndef FNLIST_HPP_
# define FNLIST_HPP_

# include <map>
# include <vector>
# include <filesystem>
#include <algorithm>
# include "OS.hpp"
# include "Rsa.hpp"

class FnList : public std::map<uint32_t, std::filesystem::path>
{
private:
  OS::SjisConverter converter;

protected:
  void add(const std::string& fn);
  virtual uint32_t SpecialFNVHash(const std::string& path, uint32_t initHash = 0x811C9DC5u) = 0;

public:
  FnList() {}
  virtual ~FnList() {}
  bool readFromTextFile(const std::filesystem::path& fn);
  bool readFromJsonFile(const std::filesystem::path& fn);
  bool readFromArchive(Rsa& rsa, uint32_t dirCount);
  bool set_content(const std::vector<std::filesystem::path>& files);
  bool write(Rsa& rsa);

  void add(const std::filesystem::path& fn);
  uint32_t SpecialFNVHash(const std::filesystem::path& path, uint32_t initHash = 0x811C9DC5u);
  std::filesystem::path hashToFn(uint32_t hash);
};

class FnList0 : public FnList
{
public:
  FnList0() {}
  ~FnList0() {}

  uint32_t SpecialFNVHash(const std::string& path, uint32_t initHash = 0x811C9DC5u);
};

class FnList1 : public FnList
{
public:
  FnList1() {}
  ~FnList1() {}

  uint32_t SpecialFNVHash(const std::string& path, uint32_t initHash = 0x811C9DC5u);
};

#endif /* !FNLIST_HPP_ */
