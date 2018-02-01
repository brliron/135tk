// th135arc.cpp : 定义控制台应用程序的入口点。
//

#if defined(WIN32) || defined(_WIN32)
# include <windows.h>
# include <locale.h>
#endif
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "TFPKArchive.h"
#include "tasofroCrypt.h"

void printUsage(char* myname)
{
	printf("Usage:\n"
		"  %s </p|/x> <Target>\n"
		"  eg. %s /x th155.pak\n"
		"        will eXtract all files in th155.pak to th155\\.\n"
		"      %s /p th155\n"
		"        will Pack all files in th155\\ into th155.pak.\n",
		myname, myname, myname);
	return;
}

int main(int argc, char* argv[])
{
#if defined(WIN32) || defined(_WIN32)
	setlocale(LC_ALL,"");
#endif
	printf("Archiver for Touhou 15.5(AoCF)\n"
	       "By brliron\n"
	       "Based on the archiver for Touhou 13.5(HM) by Riatre\n"
	       "\n"
	       );
	if(LoadFileNameList("fileslist.txt") == -1)
	{
		printf("[-] WARNING: fileslist.txt not found, path guessing disabled.\n\n");
	}
	if(argc < 3)
	{
		printUsage(argv[0]);
		return 0;
	}
	else
	{
		if(strcmp(argv[1],"/x") == 0 || strcmp(argv[1],"-x") == 0)
		{
			for(int i = 2;i < argc;i++)
			{
				char OutputDirectory[MAX_PATH] = {0};
				strcpy(OutputDirectory,argv[i]);
				char* ext = strrchr(OutputDirectory,'.');
				if(ext) *ext = 0;
				else strcat(OutputDirectory,"_eXtracted");

				printf("Extracting %s\n",argv[i]);
				ExtractAll(argv[i],OutputDirectory);
				printf("Finished.\n\n");
			}
		}
		else if(strcmp(argv[1],"/p") == 0 || strcmp(argv[1],"-p") == 0)
		{
			for(int i = 2;i < argc;i++)
			{
				WCHAR dir[strlen(argv[i]) + 1];
				WCHAR OutputFileName[MAX_PATH] = {0};
				for (int j = 0; j <= strlen(argv[i]); j++)
					dir[j] = argv[i][j];
				wcscpy(OutputFileName, dir);
				wcscat(OutputFileName, L".pak");

				wprintf(L"Packing %s\n", OutputFileName);
				BuildArchive(OutputFileName,dir);
				printf("Finished.\n\n");
			}
		}
		else printUsage(argv[0]);
	}
	return 0;
}
