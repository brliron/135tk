#ifndef FILESLIST_HPP_
# define FILESLIST_HPP_

# include <filesystem>
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
public:
  FilesList() {}
  virtual ~FilesList() {}
  virtual bool read(Rsa& rsa, uint32_t fileCount, FnList& fnList) = 0;
};

class FilesList0 : public FilesList
{
public:
  FilesList0() {}
  ~FilesList0() {}
  bool read(Rsa& rsa, uint32_t fileCount, FnList& fnList);
};

class FilesList1 : public FilesList
{
public:
  FilesList1() {}
  ~FilesList1() {}
  bool read(Rsa& rsa, uint32_t fileCount, FnList& fnList);
};

#endif /* !FILESLIST_HPP_ */
