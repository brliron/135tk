#ifndef FILESLIST_HPP_
# define FILESLIST_HPP_

# include <filesystem>
# include <functional>
# include <vector>
# include "FnList.hpp"
# include "Rsa.hpp"

struct FilesList_Entry
{
  uint32_t FileSize;
  uint32_t Offset;
  uint32_t NameHash;
  uint32_t Key[4];
  std::filesystem::path FileName;
};

class FilesList : public std::vector<FilesList_Entry>
{
protected:
#pragma pack(push, 1)
  struct ListItem
  {
    uint32_t FileSize;
    uint32_t Offset;
  };
#pragma pack(pop)

public:
  FilesList() {}
  virtual ~FilesList() {}
  virtual bool read(Rsa& rsa, uint32_t fileCount, FnList& fnList) = 0;
  virtual bool write(Rsa& rsa) const = 0;
  void set_content(const std::filesystem::path& dir, const std::vector<std::filesystem::path>& files, std::function<uint32_t (std::filesystem::path)> hash);
};

class FilesList0 : public FilesList
{
public:
  FilesList0() {}
  ~FilesList0() {}
  bool read(Rsa& rsa, uint32_t fileCount, FnList& fnList);
  bool write(Rsa& rsa) const;
};

class FilesList1 : public FilesList
{
public:
  FilesList1() {}
  ~FilesList1() {}
  bool read(Rsa& rsa, uint32_t fileCount, FnList& fnList);
  bool write(Rsa& rsa) const;
};

#endif /* !FILESLIST_HPP_ */
