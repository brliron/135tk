#include <string.h>
#include <inttypes.h>
#include <zlib.h>
#include "TFPK.hpp"

uint32_t FnList0::SpecialFNVHash(const char *path, uint32_t initHash)
{
  uint32_t hash; // eax@1
  uint32_t ch; // esi@2

  for (hash = initHash; *path; hash = ch ^ 0x1000193 * hash)
    {
      unsigned char c = *path++;
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

uint32_t FnList1::SpecialFNVHash(const char *path, uint32_t initHash)
{
  uint32_t hash; // eax@1
  uint32_t ch; // esi@2

  for (hash = initHash; *path; hash = (hash ^ ch) * 0x1000193)
    {
      unsigned char c = *path++;
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

    this->add(UString::UString(str, UString::SHIFT_JIS));
    pos += strlen(str) + 1;
  }

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
