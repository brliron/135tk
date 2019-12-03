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

bool TFPK::parse_header(std::ifstream& arc)
{
  std::cout << "Reading header... ";
  std::cout.flush();
  char magic[4];
  arc.read(magic, 4);
  if (memcmp(magic, "TFPK", 4) != 0) {
    std::cerr << "Error: the given file isn't a TFPK archive." << std::endl;
    return false;
  }

  uint8_t version;
  arc.read((char*)&version, sizeof(uint8_t));
  if (this->check_version(version) == false)
    return false;

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

bool TFPK0::check_version(uint8_t version)
{
  if (version != 0) {
    std::cerr << "Version number must be 0 for TFPK0 archives." << std::endl;
    return false;
  }
  return true;
}

bool TFPK1::check_version(uint8_t version)
{
  if (version != 1) {
    std::cerr << "Version number must be 1 for TFPK1 archives." << std::endl;
    return false;
  }
  return true;
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


void TFPK0::UncryptBlock(unsigned char *data, size_t size, uint32_t *Key)
{
  uint8_t *key = (uint8_t*)Key;

  for (size_t i = 0; i < size; i++)
    data[i] ^= key[i % 16];
}

void TFPK1::UncryptBlock(unsigned char *data, size_t size, uint32_t *Key)
{
  uint8_t *key = (uint8_t*)Key;
  uint8_t  aux[4];
  for (int i = 0; i < 4; i++)
    aux[i] = key[i];

  for (uint32_t i = 0; i < size; i++)
    {
      uint8_t tmp = data[i];
      data[i] = data[i] ^ key[i % 16] ^ aux[i % 4];
      aux[i % 4] = tmp;
    }
}

const char *guess_extension(const unsigned char *bytes, size_t size)
{
  struct Magic
  {
    size_t size;
    const char *magic;
    const char *ext;
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
    0x20, 0, (uint32_t)(size - 0x30), 0,
    0, 0, 0, 0
  };
  if (size >= 52 && memcmp(bytes, expected_nhtex_header, 0x30) == 0 &&
      (memcmp(bytes + 0x30, "\x89PNG", 4) == 0 || memcmp(bytes + 0x30, "DDS ", 4) == 0))
    return ".nhtex";

  if (size >= 12 && memcmp(bytes, "RIFF", 4) == 0 && memcmp(bytes + 8, "SFPL", 4) == 0)
    return ".sfl";

  for (auto& it : magics)
    if (size >= it.size && memcmp(bytes, it.magic, it.size) == 0)
      return it.ext;

  return "";
}

unsigned char *TFPK::extract_file(std::ifstream& arc, FilesList_Entry& file, size_t& size)
{
  arc.seekg(this->dataOffset + file.Offset, std::ifstream::beg);

  unsigned char *data = new unsigned char[file.FileSize];
  arc.read((char*)data, file.FileSize);
  this->UncryptBlock(data, file.FileSize, file.Key);

  size = file.FileSize;
  return data;
}

unsigned char *TFPK::extract_file(std::ifstream& arc, const std::filesystem::path& fn, size_t& size)
{
  auto it = std::find_if(this->filesList->begin(), this->filesList->end(),
		      [&fn](auto it) { return it.FileName == fn; });
  if (it != this->filesList->end())
    return this->extract_file(arc, *it, size);
  else
    return nullptr;
}

bool TFPK::extract_file(std::ifstream& arc, const std::filesystem::path& fn, std::filesystem::path dest)
{
  size_t size;
  unsigned char *data = this->extract_file(arc, fn, size);

  if (dest.filename().string().compare(0, 4, "unk_") == 0)
    dest.replace_extension(guess_extension(data, size));

  std::filesystem::path dir = dest;
  dir.remove_filename();
  std::filesystem::create_directories(dir);

  std::ofstream out(dest, std::ofstream::trunc | std::ofstream::binary);
  out.write((char*)data, size);
  delete[] data;
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

bool TFPK::repack_all(std::ofstream&, const std::filesystem::path&)
{
  return false;
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

  arc.seekg(0, std::ifstream::beg);
  if (!tfpk->parse_header(arc))
    return nullptr;

  return tfpk;
}
