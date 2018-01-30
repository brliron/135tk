#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unordered_map>
#include <list>
#include <forward_list>
#include <random>
#include <ctype.h>

#include "tasofroCrypt.h"
#include "TFPKArchive.h"

BOOL CreateDirectoryForPath(wchar_t* Path)
{
  	BOOL result = FALSE;
	unsigned int length = wcslen(Path);
	for(unsigned int i = 0; i < length; i++)
	{
		if(Path[i] == L'\\' || Path[i] == L'/')
		{
			Path[i] = 0;
			result = CreateDirectoryW(Path,NULL);
			Path[i] = L'\\';
		}
	}
	return result;
}

// Normalized Hash
DWORD SpecialFNVHash(char *begin, char *end, DWORD initHash=0x811C9DC5u)
{
	DWORD hash; // eax@1
	DWORD ch; // esi@2
	
	int inMBCS = 0;
	for ( hash = initHash; begin != end; hash = (hash ^ ch) * 0x1000193 )
	{
		ch = *begin++;
		if(!inMBCS && ( (unsigned char)ch >= 0x81u && (unsigned char)ch <= 0x9Fu || (unsigned char)ch+32 <= 0x1Fu )) inMBCS = 2;
		if(!inMBCS)
		{
			ch = tolower(ch);  // bad ass style but WORKS PERFECTLY!
			if(ch == '/') ch = '\\';
		}
		else inMBCS--;
	}
	return hash * -1;
}

std::unordered_map<DWORD,std::string> fileHashToName;
int LoadFileNameList(const char* FileName)
{
	FILE* fp = fopen(FileName,"rt");
	if(!fp) return -1;
	char FilePath[MAX_PATH] = {0};
	while(fgets(FilePath,MAX_PATH,fp))
	{
		int tlen = strlen(FilePath);
		while(tlen && FilePath[tlen-1] == '\n') FilePath[--tlen] = 0;
		DWORD thash = SpecialFNVHash(FilePath,FilePath+tlen);
		fileHashToName[thash] = FilePath;
	}
	fclose(fp);
	return 0;
}

void	UncryptBlock(BYTE* Data,DWORD FileSize,DWORD* Key)
{
  BYTE* key = (BYTE*)Key;
  BYTE	aux[4];
  for (int i = 0; i < 4; i++)
    aux[i] = key[i];

  for (DWORD i = 0; i < FileSize; i++)
    {
      BYTE tmp = Data[i];
      Data[i] = Data[i] ^ key[i % 16] ^ aux[i % 4];
      aux[i % 4] = tmp;
    }
}

DWORD	CryptBlock(BYTE* Data, DWORD FileSize, DWORD* Key, DWORD Aux)
{
  BYTE*	key = (BYTE*)Key;
  BYTE*	aux = (BYTE*)&Aux;

  for (int i = FileSize - 1; i >= 0; i--)
    {
      BYTE	unencByte = Data[i];
      BYTE	encByte = aux[i % 4];
      Data[i] = aux[i % 4];
      aux[i % 4] = unencByte ^ encByte ^ key[i % 16];
    }
  return Aux;
}

DWORD	CryptBlock(BYTE* Data, DWORD FileSize, DWORD* Key)
{
  DWORD	Aux;
  BYTE* tempCopy = new BYTE[FileSize];
  memcpy(tempCopy, Data, FileSize);
  Aux = CryptBlock(tempCopy, FileSize, Key, Key[0]); // This call seems to give the correct Aux value.
  delete[] tempCopy;

  return CryptBlock(Data, FileSize, Key, Aux);
}

struct MapStruct
{
  HANDLE hFile;
  HANDLE hMapping;
  BYTE* pFile;
};

BYTE*	map(const char* FileName, size_t size, MapStruct& mapStruct)
{
  if (size == 0)
    {
      mapStruct.hFile = CreateFileA(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_RANDOM_ACCESS,NULL);
      mapStruct.hMapping = CreateFileMapping(mapStruct.hFile,NULL,PAGE_READONLY,0,GetFileSize(mapStruct.hFile,NULL),NULL);
      mapStruct.pFile = (BYTE*)MapViewOfFile(mapStruct.hMapping,FILE_MAP_READ,0,0,0);
    }
  else
    {
      mapStruct.hFile = CreateFileA(FileName,GENERIC_READ|GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_FLAG_RANDOM_ACCESS,NULL);
      mapStruct.hMapping = CreateFileMapping(mapStruct.hFile,NULL,PAGE_READWRITE,0,size,NULL);
      mapStruct.pFile = (BYTE*)MapViewOfFile(mapStruct.hMapping,FILE_MAP_READ | FILE_MAP_WRITE,0,0,0);
    }
  if (mapStruct.hFile == INVALID_HANDLE_VALUE || mapStruct.hMapping == INVALID_HANDLE_VALUE || !mapStruct.pFile)
    return 0;
  return mapStruct.pFile;
}

void	unmap(MapStruct& mapStruct)
{
  UnmapViewOfFile(mapStruct.pFile);
  CloseHandle(mapStruct.hMapping);
  CloseHandle(mapStruct.hFile);
}

int ExtractAll(const char* ArchiveFileName,const char* OutputFolder)
{
	MapStruct mapStruct;
	BYTE* pPackage = map(ArchiveFileName, 0, mapStruct);
	if(pPackage == NULL)
		return 0;

	DWORD Magic = *(DWORD*)pPackage;
	if(Magic != 'KPFT')
	{
		printf("Error: the given file isn't a TFPK archive.\n");
		return 0;
	}
	if (pPackage[4] != 1)
	{
		printf("Error: this tool works only with Touhou 14.5 archives.\n");
		return 0;
	}

	DWORD cur = 5;
	DWORD DirCount = 0;
	Decrypt6432(pPackage+cur,(BYTE*)&DirCount,sizeof(DWORD));
	cur += KEY_BYTESIZE;

	// Ignore the dirlist if there is one
	cur += DirCount * KEY_BYTESIZE; 

	if (DirCount > 0)
	{
		FNHEADER fnh;
		Decrypt6432(pPackage+cur,(BYTE*)&fnh,sizeof(FNHEADER));
		cur += KEY_BYTESIZE;

		cur += fnh.BlockCnt * KEY_BYTESIZE; // Ignore the compressed files names list
	}

	LISTHEADER lh;
	memset(&lh,0,sizeof(LISTHEADER));
	Decrypt6432(pPackage+cur,(BYTE*)&lh,4);
	cur += KEY_BYTESIZE;

	TFPKLIST* FileList = new TFPKLIST[lh.FileCount];
	memset(FileList,0,sizeof(TFPKLIST)*lh.FileCount);
	for(DWORD i = 0;i < lh.FileCount;i++)
	{
		LISTITEM li;
		Decrypt6432(pPackage+cur,(BYTE*)&li,sizeof(LISTITEM));
		cur += KEY_BYTESIZE;
		DWORD hash[2];
		Decrypt6432(pPackage+cur,(BYTE*)hash,sizeof(DWORD) * 2);
		FileList[i].NameHash = hash[0];
		// hash[1] seems ignored.
		cur += KEY_BYTESIZE;
		Decrypt6432(pPackage+cur,(BYTE*)FileList[i].Key,sizeof(DWORD)*4);
		cur += KEY_BYTESIZE;

		FileList[i].Offset = li.Offset;
		FileList[i].FileSize = li.FileSize;

		FileList[i].FileSize ^= FileList[i].Key[0];
		FileList[i].Offset ^= FileList[i].Key[1];
		FileList[i].NameHash ^= FileList[i].Key[2];
		for (int j = 0; j < 4; j++)
			FileList[i].Key[j] *= -1; // GCC doesn't emit a warning for this ? Ok, fine.
		try
		{
			FileList[i].FileName = (char*)fileHashToName.at(FileList[i].NameHash).c_str();
		}
		catch (...)
		{
			char* path = new char[13];
			sprintf(path, "unk_%08lX", FileList[i].NameHash);
			FileList[i].FileName = path;
		}
	}

	DWORD FileBeginOffset = cur;

	for(DWORD i = 0;i < lh.FileCount;i++)
	{
		BYTE* Data = new BYTE[FileList[i].FileSize+100];
		memcpy(Data,pPackage+FileBeginOffset+FileList[i].Offset,FileList[i].FileSize);
		UncryptBlock(Data,FileList[i].FileSize,FileList[i].Key);

		char PathName[MAX_PATH] = {0};
		sprintf(PathName,"%s\\%s",OutputFolder,FileList[i].FileName);
		wchar_t	unicodeName[MAX_PATH];
		if (MultiByteToWideChar(932, MB_ERR_INVALID_CHARS, PathName, -1, unicodeName, MAX_PATH) == 0)
		{
			printf("Shift-JIS to Unicode conversion of %s failed (error %lu). Fallback to ASCII to Unicode conversion.\n", PathName, GetLastError());
			for (int i = 0; i == 0 || PathName[i - 1]; i++)
				unicodeName[i] = PathName[i];
		}
		if (strncmp(FileList[i].FileName, "unk_", 4) == 0)
		{
			if (*(DWORD*)Data == 'MBFT' || *(DWORD*)Data == 'GNP\x89')
				wcscat(unicodeName, L".png");
			if (*(DWORD*)Data == 'SCFT')
				wcscat(unicodeName, L".csv");
			if (*(DWORD*)Data == ' SDD')
				wcscat(unicodeName, L".dds");
		}
		CreateDirectoryForPath(unicodeName);
		FILE* fp;
		fp = _wfopen(unicodeName, L"wb");
		fwrite(Data, FileList[i].FileSize, 1, fp);
		delete[] Data;
		fclose(fp);

		printf("%lu%%\r", i * 100 / lh.FileCount);
	}

	delete[] FileList;

	unmap(mapStruct);

	return 1;
}


int BuildList(const wchar_t* BasePath, const wchar_t* Path, std::list<TFPKLIST>& FileList)
{
	wchar_t FindPath[MAX_PATH] = {0};
	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA FindData;

	swprintf(FindPath,L"%s\\%s\\*.*", BasePath, Path);
	hFind = FindFirstFile(FindPath,&FindData);
	if(hFind  == INVALID_HANDLE_VALUE) return -1;

	int FileCount = 0;
	do 
	{
		if(wcscmp(FindData.cFileName,L".") == 0 || wcscmp(FindData.cFileName,L"..") == 0) continue;

		if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			wchar_t NPath[MAX_PATH] = {0};
			if (Path[0] == L'\0')
				swprintf(NPath,L"%s",FindData.cFileName);
			else
				swprintf(NPath,L"%s\\%s",Path,FindData.cFileName);
			BuildList(BasePath,NPath,FileList);
		}
		else
		{
			FileCount++;
			wchar_t* PathName = new wchar_t[MAX_PATH];
			swprintf(PathName, L"%s\\%s\\%s", BasePath, Path, FindData.cFileName);

			char* FName = new char[MAX_PATH];
			DWORD hash = 0;
			if (Path[0] == L'\0')
			{
				if (wcsncmp(FindData.cFileName, L"unk_", 4) != 0)
					sprintf(FName, "%S", FindData.cFileName);
				else
					swscanf(FindData.cFileName, L"unk_%08X", &hash);
			}
			else
				sprintf(FName, "%S\\%S", Path, FindData.cFileName);
			int len = strlen(FName);
			TFPKLIST item;
			memset(&item,0,sizeof(TFPKLIST));
			item.Path = PathName;
			item.FileName = FName;
			if (hash != 0)
				item.NameHash = hash;
			else
				item.NameHash = SpecialFNVHash(FName,FName+len);
			FileList.push_front(item);
		}
	} while(FindNextFile(hFind,&FindData));

	return 0;
}

int BuildArchive(const wchar_t* ArchiveFileName,const wchar_t* InputFolder)
{
	std::list<TFPKLIST> FileList;
	wprintf(L"Scanning files...");
	BuildList(InputFolder, L"", FileList);
	wprintf(L"Done.\nCalculating necessary metainfo...");

	DWORD Offset = 0;
	LISTHEADER lh;
	memset(&lh,0,sizeof(LISTHEADER));
	for(auto& it: FileList)
	{
		lh.FileCount++;
		for(int i = 0;i < 4;i++) it.Key[i] = 0;
		it.Offset = Offset;

		HANDLE hFile = CreateFile(it.Path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
		it.FileSize = GetFileSize(hFile,NULL);
		CloseHandle(hFile);
		Offset += it.FileSize;
	}

	DWORD PackageSize = 5 + 1*KEY_BYTESIZE + (1+lh.FileCount*3)*KEY_BYTESIZE + Offset;
	wprintf(L"Done!\nWill generate a package file with %d files.\n",lh.FileCount);
	wprintf(L"Generating encrypted file list...");

	HANDLE hOutFile = CreateFile(ArchiveFileName,GENERIC_WRITE | GENERIC_READ,FILE_SHARE_WRITE | FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
	HANDLE hFileMapping = CreateFileMapping(hOutFile,NULL,PAGE_READWRITE,0,PackageSize,NULL);
	BYTE* pPackage = (BYTE*)MapViewOfFile(hFileMapping,FILE_MAP_READ | FILE_MAP_WRITE,0,0,PackageSize);

	*(DWORD*) pPackage = 'KPFT'; // Magic
	pPackage[4] = 1;
	DWORD cur = 5;
	DWORD DirCount = 0;
	Encrypt3264((BYTE*)&DirCount,pPackage+cur,sizeof(DWORD)); cur += KEY_BYTESIZE;
	Encrypt3264((BYTE*)&lh,pPackage+cur,4); cur += KEY_BYTESIZE;
	for(auto& it: FileList)
	{
		LISTITEM li;
		li.Offset = it.Offset; li.FileSize = it.FileSize;
		Encrypt3264((BYTE*)&li,pPackage+cur,sizeof(LISTITEM)); cur += KEY_BYTESIZE;

		DWORD hash[2]; hash[0] = it.NameHash; hash[1] = 0;
		Encrypt3264((BYTE*)hash,pPackage+cur,sizeof(DWORD)*2); cur += KEY_BYTESIZE;
		Encrypt3264((BYTE*)&it.Key,pPackage+cur,sizeof(it.Key)); cur += KEY_BYTESIZE;
	}
	wprintf(L"Done.\nCopying files...\n");
	int for_i = 0, for_max = FileList.size();
	for(auto& it: FileList)
	{
		DWORD ReadBytes = 0;
		HANDLE hIn = CreateFile(it.Path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
		ReadFile(hIn,pPackage+cur,it.FileSize,&ReadBytes,NULL);
		CloseHandle(hIn);

		CryptBlock(pPackage+cur,it.FileSize,it.Key);
		cur += it.FileSize;
		printf("%d%%\r", for_i * 100 / for_max);
		for_i++;
	}

	UnmapViewOfFile(pPackage);
	CloseHandle(hFileMapping);
	CloseHandle(hOutFile);
	return 1;
}
