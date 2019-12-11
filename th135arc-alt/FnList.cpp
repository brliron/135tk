#include <iostream>
#include <string.h>
#include <inttypes.h>
#include <zlib.h>
#include "OS.hpp"
#include "TFPK.hpp"

static const int RSA_BLOCK_SIZE = 32;

uint32_t FnList0::SpecialFNVHash(const std::string& path, uint32_t initHash)
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

uint32_t FnList1::SpecialFNVHash(const std::string& path, uint32_t initHash)
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

uint32_t FnList::SpecialFNVHash(const std::filesystem::path& path, uint32_t initHash)
{
  return this->SpecialFNVHash(this->converter.toSjis(path), initHash);
}

void FnList::add(const std::string& fn)
{
  (*this)[this->SpecialFNVHash(fn)] = this->converter.fromSjis(fn);
}

void FnList::add(const std::filesystem::path& fn)
{
  (*this)[this->SpecialFNVHash(this->converter.toSjis(fn))] = fn;
}

bool FnList::set_content(const std::vector<std::filesystem::path>& files)
{
  for (const auto& file : files) {
    this->add(file);
  }
  return true;
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
      std::string line;

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

#pragma pack(push, 1)
struct FnHeader
{
  uint32_t compSize;
  uint32_t origSize;
  uint32_t blockCount;
};
#pragma pack(pop)

bool FnList::readFromArchive(Rsa& rsa, uint32_t dirCount)
{
  if (dirCount == 0)
    return true;

  FnHeader fnHeader;
  rsa.read((char*)&fnHeader, sizeof(fnHeader));

  auto compressedFnList = std::make_unique<unsigned char[]>(fnHeader.blockCount * RSA_BLOCK_SIZE);
  rsa.read(compressedFnList.get(), fnHeader.blockCount * RSA_BLOCK_SIZE);

  auto fnList = std::make_unique<char[]>(fnHeader.origSize + 1);
  unsigned long destLen = fnHeader.origSize;
  int ret = uncompress((unsigned char*)fnList.get(), &destLen, compressedFnList.get(), fnHeader.compSize);
  if (ret != Z_OK)
    {
      if (ret == Z_MEM_ERROR)
	std::cerr << "Z_MEM_ERROR";
      else if (ret == Z_BUF_ERROR)
	std::cerr << "Z_BUF_ERROR";
      else if (ret == Z_DATA_ERROR)
	std::cerr << "Z_DATA_ERROR";
      return false;
    }
  fnList[destLen] = '\0';

  size_t pos = 0;
  while (pos < fnHeader.origSize) {
    const char *str = fnList.get() + pos;
    if (str[0] == '\0')
      break;

    this->add(std::string(str));
    pos += strlen(str) + 1;
  }

  return true;
}

bool FnList::write(Rsa& rsa)
{
  std::vector<std::string> sjis_filenames(this->size());
  std::transform(this->begin(), this->end(), sjis_filenames.begin(), [this](const auto& it) {
    return this->converter.toSjis(it.second);
  });

  std::vector<char> uncompressedList;
  for (const auto& it : sjis_filenames) {
    uncompressedList.insert(uncompressedList.end(), it.begin(), it.end());
    uncompressedList.push_back('\0');
  }

  // rsa.write will be called with a size multiple of RSA_BLOCK_SIZE, so it may read a bit after the compressed data
  unsigned long compressedLen = compressBound(uncompressedList.size()) + RSA_BLOCK_SIZE;
  auto compressedList = std::make_unique<unsigned char[]>(compressedLen);
  int ret = compress(compressedList.get(), &compressedLen, (unsigned char*)uncompressedList.data(), uncompressedList.size());
  if (ret != Z_OK)
    {
      if (ret == Z_MEM_ERROR)
	std::cerr << "Z_MEM_ERROR";
      else if (ret == Z_BUF_ERROR)
	std::cerr << "Z_BUF_ERROR";
      else if (ret == Z_DATA_ERROR)
	std::cerr << "Z_DATA_ERROR";
      return false;
    }

  FnHeader fnHeader;
  fnHeader.compSize = compressedLen;
  fnHeader.origSize = uncompressedList.size();
  fnHeader.blockCount = compressedLen / RSA_BLOCK_SIZE;
  if (compressedLen % RSA_BLOCK_SIZE != 0)
    fnHeader.blockCount++;

  rsa.write((char*)&fnHeader, sizeof(fnHeader));
  rsa.write(compressedList.get(), fnHeader.blockCount * RSA_BLOCK_SIZE);
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
