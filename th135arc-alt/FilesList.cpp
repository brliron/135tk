#include "FnList.hpp"
#include "FilesList.hpp"

void FilesList::set_content(const std::filesystem::path& dir, const std::vector<std::filesystem::path>& files, std::function<uint32_t (std::filesystem::path)> hash)
{
  uint32_t offset = 0;

  this->clear();
  for (auto& file : files) {
    std::filesystem::directory_entry direntry(dir / file);
    this->push_back({
      (uint32_t)direntry.file_size(),
      offset,
      hash(file),
      { 0, 0, 0, 0 },
      file
    });
    offset += direntry.file_size();
  }
}


bool FilesList0::read(Rsa& rsa, uint32_t fileCount, FnList& fnList)
{
  FilesList::ListItem listItem;
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

bool FilesList0::write(Rsa& rsa) const
{
  uint32_t fileCount = this->size();
  rsa.write(&fileCount, sizeof(uint32_t));

  for (const FilesList_Entry& entry : *this) {
    FilesList::ListItem listItem = {
      entry.FileSize,
      entry.Offset
    };

    rsa.write((char*)&listItem, sizeof(listItem));
    rsa.write((char*)&entry.NameHash, sizeof(entry.NameHash));
    rsa.write((char*)entry.Key, sizeof(entry.Key));
  }
  return true;
}


bool FilesList1::read(Rsa& rsa, uint32_t fileCount, FnList& fnList)
{
  FilesList::ListItem listItem;
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

bool FilesList1::write(Rsa& rsa) const
{
  uint32_t fileCount = this->size();
  rsa.write(&fileCount, sizeof(uint32_t));

  // Copy the entry object to apply the XOR
  for (FilesList_Entry entry : *this) {
    for (int j = 0; j < 4; j++)
      entry.Key[j] *= -1;

    entry.FileSize ^= entry.Key[0];
    entry.Offset   ^= entry.Key[1];
    entry.NameHash ^= entry.Key[2];

    FilesList::ListItem listItem = {
      entry.FileSize,
      entry.Offset
    };
    uint32_t hash[2] = {
      entry.NameHash,
      0 // Unused
    };

    rsa.write((char*)&listItem, sizeof(listItem));
    rsa.write((char*)&hash, sizeof(hash));
    rsa.write((char*)entry.Key, sizeof(entry.Key));
  }
  return true;
}
