#ifndef TFPKARCHIVE_H_
# define TFPKARCHIVE_H_
# if 1 || defined(WIN32) || defined(_WIN32)
#  include <Windows.h>
# else
typedef	unsigned int	DWORD;
typedef	unsigned char	BYTE;
typedef	unsigned char	BOOL;
#  include <linux/limits.h>
#  define MAX_PATH	PATH_MAX
#  define TRUE		1
#  define FALSE		0
# endif

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

