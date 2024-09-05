#include <Windows.h>
#include <Shlwapi.h>
#include <stdio.h>

// basedir is NOT const. It will be modified, but it will be returned to its original state after the call.
void find_images(LPWSTR basedir, void (*callback)(LPCWSTR path))
{
  LPWSTR file = basedir + wcslen(basedir);
  wcscpy(file, L"\\*");

  WIN32_FIND_DATAW ffd;
  HANDLE hFind = FindFirstFileW(basedir, &ffd);
  if (hFind == INVALID_HANDLE_VALUE)
    {
      wprintf(L"FindFristfile failed.\n");
      *file = L'\0';
      return ;
    }

  do
    {
      wcscpy(file + 1, ffd.cFileName);
      if (wcscmp(ffd.cFileName, L".") == 0 || wcscmp(ffd.cFileName, L"..") == 0)
        {} // Do nothing
      else if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        find_images(basedir, callback);
      else
	{
	  LPCWSTR ext = PathFindExtension(basedir);
	  if (ext && (wcscmp(ext, L".png") == 0 || wcscmp(ext, L".bmp") == 0)) {
	    wprintf(L"%S\n", basedir);
	    callback(basedir);
	  }
	}
    } while (FindNextFileW(hFind, &ffd));

  *file = L'\0';
  FindClose(hFind);
}
