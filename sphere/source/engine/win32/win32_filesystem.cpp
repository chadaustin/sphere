#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include <stack>
#include <string>
#include <vector>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include "win32_filesystem.hpp"


struct DIRECTORYLISTimp
{
  std::vector<std::string> directories;
  int currentdirectory;
};


struct FILELISTimp
{
  std::vector<std::string> files;
  int current_file;
};



std::stack<std::string> DirectoryStack;


////////////////////////////////////////////////////////////////////////////////

bool GetDirectory(std::string& directory)
{
  char path[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, path);
  directory = path;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool MakeDirectory(const char* directory)
{
  return (CreateDirectory(directory, NULL) != FALSE);
}

////////////////////////////////////////////////////////////////////////////////

bool EnterDirectory(const char* directory)
{
  // get the old directory and remember it
  char old_directory[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, old_directory);
  DirectoryStack.push(old_directory);

  // set the new directory
  return (SetCurrentDirectory(directory) != 0);
}

////////////////////////////////////////////////////////////////////////////////

bool LeaveDirectory()
{
  std::string old = DirectoryStack.top();
  DirectoryStack.pop();
  return (SetCurrentDirectory(old.c_str()) != 0);
}

////////////////////////////////////////////////////////////////////////////////

DIRECTORYLIST BeginDirectoryList(const char* mask)
{
  DIRECTORYLISTimp* dl = new DIRECTORYLISTimp;
  dl->currentdirectory = 0;

  WIN32_FIND_DATA ffd;
  HANDLE fh = FindFirstFile(mask, &ffd);
  
  // if there are no directories, we're done searching
  if (fh == INVALID_HANDLE_VALUE) {
    return dl;
  }

  // build the directory list
  do {
    // it has to be a directory and it can't start with a period
    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
        ffd.cFileName[0] != '.')
    {
      dl->directories.push_back(ffd.cFileName);
    }
  } while (FindNextFile(fh, &ffd));

  FindClose(fh);
  return dl;
}

////////////////////////////////////////////////////////////////////////////////

void EndDirectoryList(DIRECTORYLIST dl)
{
  delete dl;
}

////////////////////////////////////////////////////////////////////////////////

bool DirectoryListDone(DIRECTORYLIST dl)
{
  return (dl->currentdirectory >= dl->directories.size());
}

////////////////////////////////////////////////////////////////////////////////

void GetNextDirectory(DIRECTORYLIST dl, char directory[FILENAME_MAX])
{
  strcpy(directory, dl->directories[dl->currentdirectory].c_str());
  dl->currentdirectory++;
}

////////////////////////////////////////////////////////////////////////////////

FILELIST BeginFileList(const char* mask)
{
  FILELISTimp* fl = new FILELISTimp;

  // obviously, there are no directories at first
  fl->current_file = 0;

  WIN32_FIND_DATA ffd;
  HANDLE fh = FindFirstFile(mask, &ffd);
  if (fh == INVALID_HANDLE_VALUE) { // if there are no files, we're done searching
    return fl;
  }

  // build the file list
  do {
    // it can't be a directory
    if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

      fl->files.push_back(ffd.cFileName);

    }

  } while (FindNextFile(fh, &ffd));

  FindClose(fh);
  return fl;
}

////////////////////////////////////////////////////////////////////////////////

void EndFileList(FILELIST fl)
{
  delete fl;
}

////////////////////////////////////////////////////////////////////////////////

bool FileListDone(FILELIST fl)
{
  return !(fl->current_file < fl->files.size());
}

////////////////////////////////////////////////////////////////////////////////

void GetNextFile(FILELIST fl, char directory[FILENAME_MAX])
{
  strcpy(directory, fl->files[fl->current_file].c_str());
  fl->current_file++;
}

////////////////////////////////////////////////////////////////////////////////
