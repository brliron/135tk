#include <algorithm>
#include <iostream>
#include <string.h>
#include <inttypes.h>
#include "OS.hpp"
#include "TFPK.hpp"

TFPK::~TFPK()
{}

// TODO: do not read files list from the constructor (we can't report errors).
TFPK0::TFPK0()
{
  std::filesystem::path basedir = OS::getSelfPath();
  basedir.remove_filename();

  this->fnList = std::make_unique<FnList0>();
  this->fnList->readFromTextFile(basedir / "fileslist.txt");
  this->fnList->readFromJsonFile(basedir / "fileslist.js");
  this->filesList = std::make_unique<FilesList0>();
}

TFPK1::TFPK1()
{
  std::filesystem::path basedir = OS::getSelfPath();
  basedir.remove_filename();

  this->fnList = std::make_unique<FnList1>();
  this->fnList->readFromTextFile(basedir / "fileslist.txt");
  this->fnList->readFromJsonFile(basedir / "fileslist.js");
  this->filesList = std::make_unique<FilesList1>();
}

uint8_t TFPK0::get_version()
{
  return 0;
}

uint8_t TFPK1::get_version()
{
  return 1;
}

bool TFPK::parse_header(std::ifstream& arc)
{
  std::cout << "Reading header... ";
  std::cout.flush();
  arc.seekg(0, std::ifstream::beg);

  char magic[4];
  arc.read(magic, 4);
  if (memcmp(magic, "TFPK", 4) != 0) {
    std::cerr << "Error: the given file isn't a TFPK archive." << std::endl;
    return false;
  }

  uint8_t version;
  arc.read((char*)&version, sizeof(uint8_t));
  if (version != this->get_version()) {
    std::cerr << "Version number must be " << this->get_version() << " for this archive." << std::endl;
    return false;
  }

  Rsa rsa(arc);
  uint32_t dirCount;
  if (rsa.read(&dirCount, sizeof(uint32_t)) == false) {
    std::cerr << "Error: unknown RSA key" << std::endl;
    return false;
  }
  if (this->dirList.read(rsa, dirCount) == false)
    return false;
  if (this->fnList->readFromArchive(rsa, dirCount) == false)
    return false;

  uint32_t fileCount;
  rsa.read(&fileCount, sizeof(uint32_t));
  if (this->filesList->read(rsa, fileCount, *this->fnList) == false)
    return false;

  this->dataOffset = arc.tellg();
  std::cerr << "done." << std::endl;
  return true;
}

bool TFPK::write_header(std::ofstream& arc)
{
  std::cout << "Writing header... ";
  std::cout.flush();
  arc.seekp(0, std::ofstream::beg);

  arc.write("TFPK", 4);

  uint8_t version = this->get_version();
  arc.write((char*)&version, sizeof(uint8_t));

  Rsa rsa(arc, this->get_version() == 0 ? Rsa::CryptMode::TH135 : Rsa::CryptMode::TH145);
  this->dirList.write(rsa);
  this->fnList->write(rsa);
  this->filesList->write(rsa);

  this->dataOffset = arc.tellp();
  std::cerr << "done." << std::endl;
  return true;
}

void DirList::set_content(const std::vector<std::filesystem::path>& files, std::function<uint32_t (std::filesystem::path)> hash)
{
  std::map<std::filesystem::path, uint32_t> dirs;

  for (auto file : files) {
    std::filesystem::path dir = file;
    dir.remove_filename();
    dirs[dir]++;
  }

  this->clear();
  for (auto dir : dirs) {
    this->push_back({ hash(dir.first), dir.second });
  }
}

bool DirList::read(Rsa& rsa, uint32_t dirCount)
{
  DirList_Entry entry;
  for (uint32_t i = 0; i < dirCount; i++) {
    rsa.read((char*)&entry, sizeof(entry));
    this->push_back(entry);
  }
  return true;
}

bool DirList::write(Rsa& rsa) const
{
  uint32_t dirCount = this->size();
  rsa.write(&dirCount, sizeof(uint32_t));

  for (const DirList_Entry& entry : *this) {
    rsa.write((const char*)&entry, sizeof(entry));
  }
  return true;
}


void TFPK0::UncryptBlock(std::vector<uint8_t>& data, uint32_t *Key)
{
  uint8_t *key = (uint8_t*)Key;

  for (size_t i = 0; i < data.size(); i++)
    data[i] ^= key[i % 16];
}

void TFPK0::CryptBlock(std::vector<uint8_t>& data, uint32_t *Key)
{
  return this->UncryptBlock(data, Key);
}

void TFPK1::UncryptBlock(std::vector<uint8_t>& data, uint32_t *Key)
{
  uint8_t *key = (uint8_t*)Key;
  uint8_t  aux[4];
  for (int i = 0; i < 4; i++)
    aux[i] = key[i];

  for (size_t i = 0; i < data.size(); i++)
    {
      uint8_t tmp = data[i];
      data[i] = data[i] ^ key[i % 16] ^ aux[i % 4];
      aux[i % 4] = tmp;
    }
}

uint32_t TFPK1::CryptBlock(std::vector<uint8_t>& data, uint32_t* Key, uint32_t Aux)
{
  uint8_t* key = (uint8_t*)Key;
  uint8_t* aux = (uint8_t*)&Aux;

  for (signed int i = data.size() - 1; i >= 0; i--)
    {
      uint8_t unencByte = data[i];
      uint8_t encByte = aux[i % 4];
      data[i] = encByte;
      aux[i % 4] = unencByte ^ encByte ^ key[i % 16];
    }

  return Aux;
}

void TFPK1::CryptBlock(std::vector<uint8_t>& data, uint32_t* Key)
{
  uint32_t Aux;
  std::vector<uint8_t> tempCopy = data;
  Aux = this->CryptBlock(tempCopy, Key, Key[0]); // This call seems to give the correct Aux value.

  CryptBlock(data, Key, Aux);
}

const char *TFPK::guess_extension(const std::vector<uint8_t>& data)
{
  struct Magic
  {
    size_t size;
    const uint8_t *magic;
    const char *ext;
    Magic(size_t size, const char *magic, const char *ext)
      : size(size), magic((const uint8_t*)magic), ext(ext) {}
  };
  std::vector<Magic> magics = {
    { 73, "#========================================================================", ".pl" },
    { 6, "\xFA\xFARIQS", ".nut" },
    { 4, "TFBM",    ".png" },
    { 4, "\x89PNG", ".png" },
    { 4, "TFCS",    ".csv" },
    { 4, "DDS ",    ".dds" },
    { 4, "OggS",    ".ogg" },
    { 4, "eft$",    ".eft" },
    { 4, "TFWA",    ".wav" },
    { 4, "TFPA",    ".bmp" },
    { 4, "IBMB",    ".bmb" },
    { 4, "ACT1",    ".act" },
    { 2, "MZ",      ".dll" },
    { 2, "BM",      ".bmp" },
    { 1, "\x11",    ".pat" },
    { 1, "{",       ".pat" },
  };

  uint32_t expected_nhtex_header[12] = {
    0x20, 0, 0x10, 0,
    0x20, 0, (uint32_t)(data.size() - 0x30), 0,
    0, 0, 0, 0
  };
  if (data.size() >= 0x34 && std::equal(data.begin(), data.begin() + 0x30, (uint8_t*)expected_nhtex_header) &&
      (std::equal(data.begin() + 0x30, data.begin() + 0x34, "\x89PNG") || std::equal(data.begin() + 0x30, data.begin() + 0x34, "DDS ")))
    return ".nhtex";

  if (data.size() >= 12 && std::equal(data.begin(), data.begin() + 4, "RIFF") && std::equal(data.begin() + 8, data.begin() + 12, "SFPL"))
    return ".sfl";

  for (auto& it : magics)
    if (data.size() >= it.size && std::equal(data.begin(), data.begin() + it.size, it.magic))
      return it.ext;

  return "";
}

std::vector<uint8_t> TFPK::extract_file(std::ifstream& arc, FilesList_Entry& file)
{
  arc.seekg(this->dataOffset + file.Offset, std::ifstream::beg);

  std::vector<uint8_t> data(file.FileSize);
  arc.read((char*)data.data(), file.FileSize);
  this->UncryptBlock(data, file.Key);

  return data;
}

std::vector<uint8_t> TFPK::extract_file(std::ifstream& arc, const std::filesystem::path& fn)
{
  auto it = std::find_if(this->filesList->begin(), this->filesList->end(),
		      [&fn](auto it) { return it.FileName == fn; });
  if (it != this->filesList->end())
    return this->extract_file(arc, *it);
  else
    return std::vector<uint8_t>();
}

bool TFPK::extract_file(std::ifstream& arc, const std::filesystem::path& fn, std::filesystem::path dest)
{
  std::vector<uint8_t> data = this->extract_file(arc, fn);
  if (data.empty()) {
    return false;
  }

  if (dest.filename().string().compare(0, 4, "unk_") == 0)
    dest.replace_extension(this->guess_extension(data));

  std::filesystem::path dir = dest;
  dir.remove_filename();
  std::filesystem::create_directories(dir);

  std::ofstream out(dest, std::ofstream::trunc | std::ofstream::binary);
  out.write((char*)data.data(), data.size());
  return true;
}

bool TFPK::extract_all(std::ifstream& arc, const std::filesystem::path& dest_dir)
{
  int i = 0;
  for (auto& it : *this->filesList) {
    std::cout << "\r" << i + 1 << "/" << this->filesList->size();
    std::cout.flush();
    if (this->extract_file(arc, it.FileName, dest_dir / it.FileName) == false)
      return false;
    i++;
  }
  std::cout << std::endl;
  return true;
}

bool TFPK::repack_file(std::ofstream& arc, FilesList_Entry& file, std::vector<uint8_t>&& data)
{
  if (data.size() != file.FileSize) {
    return false;
  }

  ssize_t offset = this->dataOffset + file.Offset;
  arc.seekp(0, std::ifstream::end);
  if (arc.tellp() < offset) {
    std::vector<char> fill_data(offset - arc.tellp());
    arc.write(fill_data.data(), fill_data.size());
  }

  arc.seekp(offset, std::ifstream::beg);

  std::vector<uint8_t> local_data = data;
  this->CryptBlock(local_data, file.Key);
  arc.write((char*)local_data.data(), local_data.size());

  return true;
}

bool TFPK::repack_file(std::ofstream& arc, const std::filesystem::path& fn, std::vector<uint8_t>&& data)
{
  auto it = std::find_if(this->filesList->begin(), this->filesList->end(),
		      [&fn](auto it) { return it.FileName == fn; });
  if (it != this->filesList->end())
    return this->repack_file(arc, *it, std::move(data));
  else
    return false;
}

bool TFPK::repack_file(std::ofstream& arc, const std::filesystem::path& src, std::filesystem::path fn)
{
  std::ifstream in(src, std::ofstream::binary);
  if (!in) {
    std::cerr << "Could not open " << src << std::endl;
    return false;
  }

  size_t size;
  in.seekg(0, std::ifstream::end);
  size = in.tellg();
  in.seekg(0, std::ifstream::beg);

  std::vector<uint8_t> data;
  data.resize(size);
  in.read((char*)data.data(), data.size());

  return this->repack_file(arc, fn, std::move(data));
}

std::vector<std::filesystem::path> TFPK::list_files(const std::filesystem::path& dir)
{
  std::vector<std::filesystem::path> files;

  for (auto& it : std::filesystem::recursive_directory_iterator(dir)) {
    if (!it.is_directory()) {
      files.push_back(it.path().lexically_relative(dir));
    }
  }
  return files;
}

bool TFPK::repack_all(std::ofstream& arc, const std::filesystem::path& src_dir)
{
  std::vector<std::filesystem::path> files = this->list_files(src_dir);

  auto hash_function = [this](const std::filesystem::path& path) { return this->fnList->SpecialFNVHash(path); };
  this->dirList.set_content(files, hash_function);
  this->fnList->set_content(files);
  this->filesList->set_content(src_dir, files, hash_function);
  this->write_header(arc);

  // Browse in the filesList header allows us to create the file sequentially.
  int i = 0;
  for (auto& it : *this->filesList) {
    std::cout << "\r" << i + 1 << "/" << this->filesList->size();
    std::cout.flush();
    if (!this->repack_file(arc, src_dir / it.FileName, it.FileName))
      return false;
    i++;
  }
  std::cout << std::endl;

  return true;
}

std::unique_ptr<TFPK> TFPK::read(std::ifstream& arc)
{
  char magic[4];
  arc.read(magic, 4);
  if (memcmp(magic, "TFPK", 4) != 0) {
    std::cerr << "Error: the given file isn't a TFPK archive." << std::endl;
    return nullptr;
  }

  uint8_t version;
  arc.read((char*)&version, sizeof(uint8_t));
  std::unique_ptr<TFPK> tfpk;
  if (version == 0)
    tfpk = std::make_unique<TFPK0>();
  else if (version == 1)
    tfpk = std::make_unique<TFPK1>();
  else {
    std::cerr << "Error: this tool works only with the archives from the following games:" << std::endl
	      << "  Touhou 13.5" << std::endl
	      << "  Touhou 14.5" << std::endl
	      << "  Touhou 15.5" << std::endl;
    return nullptr;
  }

  if (!tfpk->parse_header(arc))
    return nullptr;

  return tfpk;
}
