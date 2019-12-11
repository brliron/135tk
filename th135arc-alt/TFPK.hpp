#ifndef TFPK_HPP_
# define TFPK_HPP_

# include <filesystem>
# include <functional>
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
  void set_content(const std::vector<std::filesystem::path>& files, std::function<uint32_t (std::filesystem::path)> hash);
  bool read(Rsa& rsa, uint32_t dirCount);
  bool write(Rsa& rsa) const;
};



class TFPK
{
private:
  virtual uint8_t get_version() = 0;
  virtual void UncryptBlock(std::vector<uint8_t>& data, uint32_t *Key) = 0;
  virtual void CryptBlock(std::vector<uint8_t>& data, uint32_t *Key) = 0;

  const char *guess_extension(const std::vector<uint8_t>& data);
  std::vector<uint8_t> extract_file(std::ifstream& arc, FilesList_Entry& file);
  bool repack_file(std::ofstream& arc, FilesList_Entry& file, std::vector<uint8_t>&& data);

  size_t dataOffset;

public:
  DirList                    dirList;
  std::unique_ptr<FnList>    fnList;
  std::unique_ptr<FilesList> filesList;

  TFPK() {}
  virtual ~TFPK();

  bool parse_header(std::ifstream& arc);
  std::vector<uint8_t> extract_file(std::ifstream& arc, const std::filesystem::path& fn);
  bool extract_file(std::ifstream& arc, const std::filesystem::path& fn, std::filesystem::path dest);
  bool extract_all(std::ifstream& arc, const std::filesystem::path& dest_dir);

  bool write_header(std::ofstream& arc);
  bool repack_file(std::ofstream& arc, const std::filesystem::path& fn, std::vector<uint8_t>&& data);
  bool repack_file(std::ofstream& arc, const std::filesystem::path& src, std::filesystem::path fn);
  bool repack_all(std::ofstream& arc, const std::filesystem::path& dest_dir);

  std::vector<std::filesystem::path> list_files(const std::filesystem::path& dir);

  static std::unique_ptr<TFPK> read(std::ifstream& file);
};

class TFPK0 : public TFPK
{
private:
  uint8_t get_version();
  void UncryptBlock(std::vector<uint8_t>& data, uint32_t *Key);
  void CryptBlock(std::vector<uint8_t>& data, uint32_t *Key);

public:
  TFPK0();
  ~TFPK0() {}
};

class TFPK1 : public TFPK
{
private:
  uint8_t get_version();
  void UncryptBlock(std::vector<uint8_t>& data, uint32_t *Key);
  uint32_t CryptBlock(std::vector<uint8_t>& data, uint32_t* Key, uint32_t Aux);
  void CryptBlock(std::vector<uint8_t>& data, uint32_t *Key);

public:
  TFPK1();
  ~TFPK1(){}
};

#endif /* !TFPK_HPP_ */
