#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#ifdef USTRING_WINDOWS
# include <windows.h>
#else
# include <sys/stat.h>
# include <sys/types.h>
#endif
#include "TFPK.hpp"

TFPK::~TFPK()
{}

// TODO: do not read files list from the constructor (we can't report errors).
TFPK0::TFPK0()
{
  this->fnList = std::make_unique<FnList0>();
  this->fnList->readFromTextFile("fileslist.txt");
  this->fnList->readFromJsonFile("fileslist.js");
  this->filesList = std::make_unique<FilesList0>();
}

TFPK1::TFPK1()
{
  this->fnList = std::make_unique<FnList1>();
  this->fnList->readFromTextFile("fileslist.txt");
  this->fnList->readFromJsonFile("fileslist.js");
  this->filesList = std::make_unique<FilesList1>();
}

bool TFPK::parse_header(File& arc)
{
  printf("Reading header... ");
  char magic[4];
  arc.read(magic, 4);
  if (memcmp(magic, "TFPK", 4) != 0) {
    printf("Error: the given file isn't a TFPK archive.\n");
    return false;
  }

  uint8_t version = arc.readByte();
  if (this->check_version(version) == false)
    return false;

  Rsa rsa(arc);
  uint32_t dirCount;
  if (rsa.read(&dirCount, sizeof(uint32_t)) == false) {
    printf("Error: unknown RSA key!\n");
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

  this->dataOffset = arc.tell();
  printf("done.\n");
  return true;
}

bool TFPK0::check_version(uint8_t version)
{
  if (version != 0) {
    printf("Version number must be 0 for TFPK0 archives.\n");
    return false;
  }
  return true;
}

bool TFPK1::check_version(uint8_t version)
{
  if (version != 1) {
    printf("Version number must be 1 for TFPK1 archives.\n");
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


bool TFPK::CreateDirectoryForPath(UString fn)
{
  for (unsigned int i = 0; i < fn.length(); i++) {
    if (fn[i] == '\\' || fn[i] == '/') {
      UString temp_fn = fn.substr(0, i);
#ifdef USTRING_WINDOWS
      if (CreateDirectoryW(temp_fn.w_str(), NULL) == 0 && GetLastError() != ERROR_ALREADY_EXISTS)
	return false;
#else
      if (mkdir(temp_fn.c_str(), 0777) == -1 && errno != EEXIST)
	return false;
#endif
    }
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

unsigned char *TFPK::extract_file(File& arc, FilesList_Entry& file, size_t& size)
{
  arc.seek(this->dataOffset + file.Offset, File::Seek::SET);

  unsigned char *data = new unsigned char[file.FileSize];
  arc.read(data, file.FileSize);
  this->UncryptBlock(data, file.FileSize, file.Key);

  size = file.FileSize;
  return data;
}

unsigned char *TFPK::extract_file(File& arc, UString fn, size_t& size)
{
  auto it = std::find_if(this->filesList->begin(), this->filesList->end(),
		      [&fn](auto it) { return it.FileName == fn; });
  if (it != this->filesList->end())
    return this->extract_file(arc, *it, size);
  else
    return nullptr;
}

bool TFPK::extract_file(File& arc, UString fn, UString dest)
{
  size_t size;
  unsigned char *data = this->extract_file(arc, fn, size);

  if (strncmp(fn.c_str(), "unk_", 4) == 0)
    dest += guess_extension(data, size);

  std::for_each(dest.begin(), dest.end(), [](char& c) {
      if (c == '\\')
	c = '/';
    } );
  CreateDirectoryForPath(dest);
  File out(dest, File::WRITE | File::TRUNCATE);
  out.write(data, size);
  delete[] data;
  return true;
}

bool TFPK::extract_all(File& arc, UString dest_dir)
{
  int i = 0;
  for (auto& it : *this->filesList) {
    printf("\r%d/%lu", i + 1, this->filesList->size());
    fflush(stdout);
    if (this->extract_file(arc, it.FileName,
			   dest_dir + "/" + UString(it.FileName, UString::SHIFT_JIS)
			   ) == false)
      return false;
    i++;
  }
  printf("\n");
  return true;
}

bool TFPK::repack_all(File&, UString)
{
  return false;
}

std::unique_ptr<TFPK> TFPK::read(File& arc)
{
  char magic[4];
  arc.read(magic, 4);
  if (memcmp(magic, "TFPK", 4) != 0) {
    printf("Error: the given file isn't a TFPK archive.\n");
    return nullptr;
  }

  uint8_t version = arc.readByte();
  std::unique_ptr<TFPK> tfpk;
  if (version == 0)
    tfpk = std::make_unique<TFPK0>();
  else if (version == 1)
    tfpk = std::make_unique<TFPK1>();
  else {
    printf("Error: this tool works only with the archives from the following games:\n"
	   "  Touhou 13.5\n"
	   "  Touhou 14.5\n"
	   "  Touhou 15.5\n"
	  );
    return nullptr;
  }

  arc.seek(0, File::Seek::SET);
  if (!tfpk->parse_header(arc))
    return nullptr;

  return tfpk;
}
