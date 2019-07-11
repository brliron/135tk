#include "FnList.hpp"
#include "FilesList.hpp"

bool FilesList0::read(Rsa& rsa, uint32_t fileCount, FnList& fnList)
{
#pragma pack(push, 1)
  struct {
    uint32_t FileSize;
    uint32_t Offset;
  } listItem;
#pragma pack(pop)
  uint32_t hash;

  FilesList_Entry entry;

  for (uint32_t i = 0; i < fileCount; i++) {
    rsa.read((char*)&listItem, sizeof(listItem));
    rsa.read((char*)&hash, sizeof(hash));
    rsa.read((char*)entry.Key, sizeof(entry.Key));
    entry.FileSize = listItem.FileSize;
    entry.Offset = listItem.Offset;
    entry.NameHash = hash;

    entry.FileName = fnList.hashToFn(entry.NameHash);

    this->push_back(entry);
  }
  return true;
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
