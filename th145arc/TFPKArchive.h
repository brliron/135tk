#ifndef TFPKARCHIVE_H_
# define TFPKARCHIVE_H_
# include <Windows.h>

#pragma pack(push)
#pragma pack(1)

struct DIRLIST
{
	DWORD PathHash;
	DWORD FileCount;
	wchar_t Path[MAX_PATH];
};

struct FNHEADER
{
	DWORD CompSize;
	DWORD OrigSize;
	DWORD BlockCnt;
};

struct LISTHEADER
{
	DWORD FileCount;
	DWORD Unknown;
	DWORD Unknown1;
};

struct LISTITEM
{
	DWORD FileSize;
	DWORD Offset;
};

struct TFPKLIST
{
	DWORD Offset;
	DWORD FileSize;
	DWORD NameHash;
	DWORD Key[4]; // Xor
	wchar_t* Path;
	char* FileName;
}; // Size: 

#pragma pack(pop)

int ExtractAll(const char* ArchiveFileName,const char* OutputFolder);
int BuildArchive(const wchar_t* ArchiveFileName,const wchar_t* InputFolder);
int LoadFileNameList(const char* FileName);
#endif

