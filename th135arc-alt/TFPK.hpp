#ifndef TFPK_HPP_
# define TFPK_HPP_

# include <vector>
# include <map>
# include "UString.hpp"
# include "File.hpp"
# include "Rsa.hpp"

#pragma pack(push, 1)
struct DirList_Entry
{
  uint32_t PathHash;
  uint32_t FileCount;
};
#pragma pack(pop)
class DirList : public std::vector<DirList_Entry>
{
public:
  DirList() {}
  ~DirList() {}
  bool read(Rsa& rsa, uint32_t dirCount);
};


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


struct FilesList_Entry
{
  uint32_t FileSize;
  uint32_t Offset;
  uint32_t NameHash;
  uint32_t Key[4];
  UString::UString FileName;
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


class TFPK
{
private:
  virtual bool check_version(uint8_t version) = 0;
  virtual void UncryptBlock(unsigned char *data, size_t size, uint32_t *Key) = 0;

  bool CreateDirectoryForPath(UString::UString fn);
  unsigned char *extract_file(File& arc, FilesList_Entry& file, size_t& size);

public:
  DirList   *dirList;
  FnList    *fnList;
  FilesList *filesList;
  size_t     dataOffset;

  TFPK() {}
  virtual ~TFPK();

  bool parse_header(File& arc);
  unsigned char *extract_file(File& arc, UString::UString fn, size_t& size);
  bool extract_file(File& arc, UString::UString fn, UString::UString dest);
  bool extract_all(File& arc, UString::UString dest_dir);
  bool repack_all(File& arc, UString::UString dest_dir);

  static TFPK *read(File& file);
};

class TFPK0 : public TFPK
{
private:
  bool check_version(uint8_t version);
  void UncryptBlock(unsigned char *data, size_t size, uint32_t *Key);

public:
  TFPK0();
  ~TFPK0() {}
};

class TFPK1 : public TFPK
{
private:
  bool check_version(uint8_t version);
  void UncryptBlock(unsigned char *data, size_t size, uint32_t *Key);

public:
  TFPK1();
  ~TFPK1(){}
};

#endif /* !TFPK_HPP_ */
