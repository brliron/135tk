#include <iostream>
#include <string.h>
#include <inttypes.h>
#include <zlib.h>
#include "OS.hpp"
#include "TFPK.hpp"

uint32_t FnList0::SpecialFNVHash(const OS::sjisstring& path, uint32_t initHash)
{
  const char *cpath = path.c_str();
  uint32_t hash; // eax@1
  uint32_t ch; // esi@2

  for (hash = initHash; *cpath; hash = ch ^ 0x1000193 * hash)
    {
      unsigned char c = *cpath++;
      ch = c;
      if ((c & 0x80) == 0)
	{
	  ch = tolower(ch);
	  if (ch == '/')
	    ch = '\\';
	}
    }
  return hash;
}

uint32_t FnList1::SpecialFNVHash(const OS::sjisstring& path, uint32_t initHash)
{
  const char *cpath = path.c_str();
  uint32_t hash; // eax@1
  uint32_t ch; // esi@2

  for (hash = initHash; *cpath; hash = (hash ^ ch) * 0x1000193)
    {
      unsigned char c = *cpath++;
      ch = c;
      if ((c & 0x80) == 0)
	{
	  ch = tolower(ch);
	  if (ch == '/')
	    ch = '\\';
	}
    }
  return hash * -1;
}

void FnList::add(const OS::sjisstring& fn)
{
  (*this)[this->SpecialFNVHash(fn)] = OS::sjisToPath(fn);
}

bool FnList::readFromTextFile(const std::filesystem::path& fn)
{
  std::cout << "Reading " << fn << "... ";
  std::cout.flush();

  std::ifstream file(fn);
  if (!file)
    {
      std::cerr << "Could not open " << fn << std::endl;
      return false;
    }

  while (file)
    {
      OS::sjisstring line;

      std::getline(file, line);
      if (line.size() > 0 && line[line.size() - 1] == '\r')
	line.resize(line.size() - 1);

      if (!line.empty())
	{
	  std::replace(line.begin(), line.end(), '\\', '/');
	  this->add(line);
	}
    }

  std::cout << "done." << std::endl;
  return true;
}

bool FnList::readFromJsonFile(const std::filesystem::path& fn)
{
  std::cout << "Reading " <<  fn << "... ";
  std::cout.flush();

  std::cerr << "FnList::readFromJsonfile isn't implemented yet." << std::endl;
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
	std::cerr << "Z_MEM_ERROR";
      else if (ret == Z_BUF_ERROR)
	std::cerr << "Z_BUF_ERROR";
      else if (ret == Z_DATA_ERROR)
	std::cerr << "Z_DATA_ERROR";
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

    this->add(OS::sjisstring(str));
    pos += strlen(str) + 1;
  }

  delete[] compressedFnList;
  delete[] fnList;
  return true;
}

std::filesystem::path FnList::hashToFn(uint32_t hash)
{
  auto elem = this->find(hash);
  if (elem != this->end())
    return elem->second;

  char fn[13];
  sprintf(fn, "unk_%08" PRIX32, hash);

  return fn;
}
