#ifndef TFPK_HPP_
# define TFPK_HPP_

# include <filesystem>
# include <fstream>
# include <memory>
# include <vector>
# include "FnList.hpp"
# include "FilesList.hpp"
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



class TFPK
{
private:
  virtual bool check_version(uint8_t version) = 0;
  virtual void UncryptBlock(unsigned char *data, size_t size, uint32_t *Key) = 0;

  unsigned char *extract_file(std::ifstream& arc, FilesList_Entry& file, size_t& size);

public:
  DirList                    dirList;
  std::unique_ptr<FnList>    fnList;
  std::unique_ptr<FilesList> filesList;
  size_t                     dataOffset;

  TFPK() {}
  virtual ~TFPK();

  bool parse_header(std::ifstream& arc);
  unsigned char *extract_file(std::ifstream& arc, const std::filesystem::path& fn, size_t& size);
  bool extract_file(std::ifstream& arc, const std::filesystem::path& fn, std::filesystem::path dest);
  bool extract_all(std::ifstream& arc, const std::filesystem::path& dest_dir);
  bool repack_all(std::ofstream& arc, const std::filesystem::path& dest_dir);

  static std::unique_ptr<TFPK> read(std::ifstream& file);
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
