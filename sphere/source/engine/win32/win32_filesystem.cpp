#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include <vector>
#include <string>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include "win32_filesystem.hpp"


struct DIRECTORYLISTimp
{
  int    numdirectories;
  char** directories;

  int    currentdirectory;
};


struct FILELISTimp
{
  std::vector<std::string> files;
  int current_file;
};



static int DirectoryStackSize;
static char** DirectoryStack;


static char OriginalDirectory[520];
static char BaseDirectory[520];


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
  char olddirectory[MAX_PATH];

  // get the old directory
  GetCurrentDirectory(MAX_PATH, olddirectory);
  
  // push it on the stack
  DirectoryStack = (char**)realloc(DirectoryStack, sizeof(char*) * (DirectoryStackSize + 1));
  DirectoryStack[DirectoryStackSize] = (char*)malloc(strlen(olddirectory) + 1);
  strcpy(DirectoryStack[DirectoryStackSize], olddirectory);

  DirectoryStackSize++;

  // set the new directory
  return (SetCurrentDirectory(directory) != 0);
}

////////////////////////////////////////////////////////////////////////////////

bool LeaveDirectory()
{
  char olddirectory[MAX_PATH];

  // get the previous directory off of the stack
  strcpy(olddirectory, DirectoryStack[DirectoryStackSize - 1]);

  // pop the topmost entry
  DirectoryStackSize--;
  free(DirectoryStack[DirectoryStackSize]);
  DirectoryStack = (char**)realloc(DirectoryStack, sizeof(char*) * DirectoryStackSize);

  // set it
  if (SetCurrentDirectory(olddirectory))
    return true;
  else
    return false;
}

////////////////////////////////////////////////////////////////////////////////

DIRECTORYLIST BeginDirectoryList(const char* mask)
{
  DIRECTORYLISTimp* dl = (DIRECTORYLISTimp*)malloc(sizeof(DIRECTORYLISTimp));
  WIN32_FIND_DATA ffd;
  HANDLE fh;

  // obviously, there are no directories at first
  dl->numdirectories = 0;
  dl->directories = NULL;

  dl->currentdirectory = 0;

  fh = FindFirstFile(mask, &ffd);
  if (fh == INVALID_HANDLE_VALUE)  // if there are no directories, we're done searching
    return dl;

  // build the directory list
  do
  {
    // it has to be a directory and it can't start with a period
    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
        ffd.cFileName[0] != '.')
    {

      dl->directories = (char**)realloc(dl->directories, (sizeof(char*) * (dl->numdirectories + 1)));
      dl->directories[dl->numdirectories] = (char*)malloc(strlen(ffd.cFileName) + 1);
      strcpy(dl->directories[dl->numdirectories], ffd.cFileName);
      dl->numdirectories++;

    }

  } while (FindNextFile(fh, &ffd));

  FindClose(fh);

  return dl;
}

////////////////////////////////////////////////////////////////////////////////

void EndDirectoryList(DIRECTORYLIST dl)
{
  for (int i = 0; i < dl->numdirectories; i++)
    free(dl->directories[i]);
  free(dl->directories);

  free(dl);
}

////////////////////////////////////////////////////////////////////////////////

bool DirectoryListDone(DIRECTORYLIST dl)
{
  return !(dl->currentdirectory < dl->numdirectories);
}

////////////////////////////////////////////////////////////////////////////////

void GetNextDirectory(DIRECTORYLIST dl, char directory[FILENAME_MAX])
{
  strcpy(directory, dl->directories[dl->currentdirectory]);
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
