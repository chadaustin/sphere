#pragma warning(disable : 4786)  // identifier too long


#include <windows.h>
#include <stdio.h>
#include "FileSystem.hpp"


////////////////////////////////////////////////////////////////////////////////

bool PathExists(const char* szPath)
{
  char szOldDirectory[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, szOldDirectory);

  BOOL bRetVal = SetCurrentDirectory(szPath);

  SetCurrentDirectory(szOldDirectory);
  return (bRetVal ? true : false);
}

////////////////////////////////////////////////////////////////////////////////

bool FileExists(const char* szFile)
{
  HANDLE hFile = CreateFile(
    szFile, 
    0,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL,
    OPEN_EXISTING,
    0,
    NULL);
  if (hFile == INVALID_HANDLE_VALUE)
    return false;

  CloseHandle(hFile);
  return true;    
}

////////////////////////////////////////////////////////////////////////////////

int FileSize(const char* filename)
{
  FILE* file = fopen(filename, "rb");
  if (file == NULL)
    return -1;
  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  fclose(file);
  return size;
}

////////////////////////////////////////////////////////////////////////////////

bool EnumerateFiles(const char* filter, const char* directory, std::list<std::string>& files)
{
  // store current directory
  char old_directory[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, old_directory);
  SetCurrentDirectory(directory);

  // add files in this directory
  WIN32_FIND_DATA ffd;
  HANDLE search = FindFirstFile(filter, &ffd);
  if (search != INVALID_HANDLE_VALUE) {
    do {
      if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {

        std::string root_path = directory;
        root_path += ffd.cFileName;
        files.push_back(root_path);

      }
    } while (FindNextFile(search, &ffd));
    FindClose(search);
  }

  // now add subdirectories
  search = FindFirstFile("*", &ffd);
  if (search != INVALID_HANDLE_VALUE) {
    do {
      if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
          strcmp(ffd.cFileName, ".") != 0 &&
          strcmp(ffd.cFileName, "..") != 0) {

        std::string directory = ffd.cFileName;
        directory += "/";
        EnumerateFiles(filter, directory.c_str(), files);

      }
    } while (FindNextFile(search, &ffd));
    FindClose(search);
  }

  // restore directory
  SetCurrentDirectory(old_directory);
  return true;
}

////////////////////////////////////////////////////////////////////////////////
