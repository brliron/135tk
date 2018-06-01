#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <zlib.h>
#ifdef USTRING_WINDOWS
# include <Windows.h>
#else
# include <sys/stat.h>
# include <sys/types.h>
#endif
#include "TFPK.hpp"

TFPK::~TFPK()
{
  delete this->dirList;
  delete this->fnList;
  delete this->filesList;
}

TFPK0::TFPK0()
{
  // TODO: allocate everything when we support TFPK0
}

TFPK1::TFPK1()
{
  this->dirList = new DirList();
  this->fnList = new FnList1();
  this->fnList->readFromTextFile("fileslist.txt");
  this->fnList->readFromJsonFile("fileslist.js");
  this->filesList = new FilesList1();
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
  if (this->dirList->read(rsa, dirCount) == false)
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

bool TFPK0::check_version(uint8_t)
{
  printf("Touhou 13.5 archives aren't supported yet.\n");
  return false;
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

uint32_t FnList1::SpecialFNVHash(const char *path, uint32_t initHash)
{
  uint32_t hash; // eax@1
  uint32_t ch; // esi@2

  //int inMBCS = 0;
  for (hash = initHash; *path; hash = (hash ^ ch) * 0x1000193)
    {
      unsigned char c = *path++;
      ch = c;
      //if (inMBCS == 0 && ((c >= 0x81u && c <= 0x9Fu) || (unsigned char)(c + 32) <= 0x1Fu))
      //inMBCS = 2;
      if (/*inMBCS == 0*/(c & 0x80) == 0)
	{
	  ch = tolower(ch);
	  if (ch == '/')
	    ch = '\\';
	}
      //else
      //inMBCS--;
    }
  return hash * -1;
}

void FnList::add(UString::UString fn)
{
  (*this)[this->SpecialFNVHash(fn.c_str())] = fn;
}

bool FnList::readFromTextFile(UString::UString fn)
{
  printf("Reading %s... ", fn.c_str());
  fflush(stdout);

  File file(fn, File::READ);
  if (!file)
    {
      printf("%s\n", file.error());
      return false;
    }
  file.seek(0, File::Seek::END);
  size_t size = file.tell();
  file.seek(0, File::Seek::SET);

  char *data;
  data = new char[size + 1];
  data[size] = '\0';
  file.read(data, size);

  size_t i = 0;
  while (i < size) {
    size_t j = i;
    while (j < size && data[j] != '\n')
      j++;
    data[j] = '\0';
    if (j > 0 && data[j - 1] == '\r')
      data[j - 1] = '\0';
    const char *path = data + i;
    this->add(UString::UString(path, UString::SHIFT_JIS));
    i = j + 1;
  }

  delete[] data;
  printf("done.\n");
  return true;
}

bool FnList::readFromJsonFile(UString::UString fn)
{
  printf("Reading %s... ", fn.c_str());
  fflush(stdout);

  printf("FnList::readFromJsonfile isn't implemented yet.\n");
  return false;
}

bool FnList::readFromArchive(Rsa& rsa, uint32_t dirCount)
{
  if (dirCount == 0)
    return true;

#pragma pack(push, 1)
  struct {
    uint32_t compSize;
    uint32_t origSize;
    uint32_t blockCount;
  } fnHeader;
#pragma pack(pop)
  rsa.read((char*)&fnHeader, sizeof(fnHeader));

  const int blockSize = Rsa::KEY_BYTESIZE / 2;
  unsigned char *compressedFnList = new unsigned char[fnHeader.compSize + Rsa::KEY_BYTESIZE];
  for (size_t i = 0; i < fnHeader.blockCount; i++)
    rsa.read(compressedFnList + i * blockSize, blockSize);

  char *fnList = new char[fnHeader.origSize + 1];
  unsigned long destLen = fnHeader.origSize;
  int ret = uncompress((unsigned char*)fnList, &destLen, compressedFnList, fnHeader.compSize);
  if (ret != Z_OK)
    {
      if (ret == Z_MEM_ERROR)
	printf("Z_MEM_ERROR");
      else if (ret == Z_BUF_ERROR)
	printf("Z_BUF_ERROR");
      else if (ret == Z_DATA_ERROR)
	printf("Z_DATA_ERROR");
      delete[] compressedFnList;
      delete[] fnList;
      return false;
    }
  fnList[destLen] = '\0';

  size_t pos = 0;
  while (pos < fnHeader.origSize) {
    const char *str = fnList + pos;
    if (str[0] == '\0')
      break;

    printf("File list: %s\n", str);
    this->add(UString::UString(str, UString::SHIFT_JIS));
    pos += strlen(str);
  }

  // TODO: parse fn list
  //rsa.getFile().seek(fnHeader.blockCount * 64, File::Seek::CUR);

  delete[] compressedFnList;
  delete[] fnList;
  return true;
}

UString::UString FnList::hashToFn(uint32_t hash)
{
  auto elem = this->find(hash);
  if (elem != this->end())
    return elem->second;

  char fn[13];
  sprintf(fn, "unk_%08" PRIX32, hash);

  return UString::UString(fn);
}

bool FilesList1::read(Rsa& rsa, uint32_t fileCount, FnList& fnList)
{
#pragma pack(push, 1)
  struct {
    uint32_t FileSize;
    uint32_t Offset;
  } listItem;
#pragma pack(pop)
  uint32_t hash[2];

  FilesList_Entry entry;

  for (uint32_t i = 0; i < fileCount; i++) {
    rsa.read((char*)&listItem, sizeof(listItem));
    rsa.read((char*)hash, sizeof(hash));
    rsa.read((char*)entry.Key, sizeof(entry.Key));
    entry.FileSize = listItem.FileSize;
    entry.Offset = listItem.Offset;
    entry.NameHash = hash[0];
    // hash[1] seems ignored.

    entry.FileSize ^= entry.Key[0];
    entry.Offset   ^= entry.Key[1];
    entry.NameHash ^= entry.Key[2];
    for (int j = 0; j < 4; j++)
      entry.Key[j] *= -1;
    entry.FileName = fnList.hashToFn(entry.NameHash);

    this->push_back(entry);
  }
  return true;
}


bool TFPK::CreateDirectoryForPath(UString::UString fn)
{
  for (unsigned int i = 0; i < fn.length(); i++) {
    if (fn[i] == '\\' || fn[i] == '/') {
      UString::UString temp_fn = fn.substr(0, i);
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

void TFPK0::UncryptBlock(unsigned char*, size_t, uint32_t*)
{
  throw std::logic_error("TFPK0::UncryptBlock isn't implemented yet.");
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
  /*typedef bool (*compare_func)(const unsigned char *bytes, size_t size);
  std::map<compare_func, const char*> extensions = {
    { [](const unsigned char *bytes, size_t size) { return } }
    };*/
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

unsigned char *TFPK::extract_file(File& arc, UString::UString fn, size_t& size)
{
  auto it = std::find_if(this->filesList->begin(), this->filesList->end(),
		      [&fn](auto it) { return it.FileName == fn; });
  if (it != this->filesList->end())
    return this->extract_file(arc, *it, size);
  else
    return nullptr;
}

bool TFPK::extract_file(File& arc, UString::UString fn, UString::UString dest)
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

bool TFPK::extract_all(File& arc, UString::UString dest_dir)
{
  int i = 0;
  for (auto& it : *this->filesList) {
    printf("\r%d/%lu", i + 1, this->filesList->size());
    fflush(stdout);
    if (this->extract_file(arc, it.FileName,
			   dest_dir + "/" + UString::UString(it.FileName, UString::SHIFT_JIS)
			   ) == false)
      return false;
    i++;
  }
  printf("\n");
  return true;
}

bool TFPK::repack_all(File&, UString::UString)
{
  return false;
}

TFPK *TFPK::read(File& arc)
{
  char magic[4];
  arc.read(magic, 4);
  if (memcmp(magic, "TFPK", 4) != 0) {
    printf("Error: the given file isn't a TFPK archive.\n");
    return nullptr;
  }

  uint8_t version = arc.readByte();
  TFPK *tfpk;
  if (version == 0)
    tfpk = new TFPK0();
  else if (version == 1)
    tfpk = new TFPK1();
  else {
    printf("Error: this tool works only with Touhou 14.5 and Touhou 15.5 archives.\n");
    return nullptr;
  }

  arc.seek(0, File::Seek::SET);
  if (!tfpk->parse_header(arc)) {
    delete tfpk;
    return nullptr;
  }
  return tfpk;
}
