#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include "filesystem.h"


typedef struct
{
  int    numdirectories;
  char** directories;

  int    currentdirectory;
} INTERNAL_DIRECTORYLIST;


static int DirectoryStackSize;
static char** DirectoryStack;


////////////////////////////////////////////////////////////////////////////////

bool EnterDirectory(const char* directory)
{
  char olddirectory[512];
  int numbytes;

  // get current directory
  getcwd(olddirectory, 500);

  // push it on the stack
  numbytes = sizeof(char*) * (DirectoryStackSize + 1);
  DirectoryStack = (char**)realloc(DirectoryStack, numbytes);
  DirectoryStack[DirectoryStackSize] = (char*)malloc(strlen(olddirectory) + 1);
  strcpy(DirectoryStack[DirectoryStackSize], olddirectory);

  DirectoryStackSize++;

  // set the new directory
  if (chdir(directory))
    return true;
  else
    return false;
}

////////////////////////////////////////////////////////////////////////////////

bool LeaveDirectory(void)
{
  // get the previous directory off of the stack
  chdir(DirectoryStack[DirectoryStackSize - 1]);

  // pop the topmost entry
  DirectoryStackSize--;
  free(DirectoryStack[DirectoryStackSize]);
  DirectoryStack = (char**)realloc(DirectoryStack, sizeof(char*) * DirectoryStackSize);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void BeginDirectoryList(DIRECTORYLIST* dl, const char* mask)
{
  INTERNAL_DIRECTORYLIST* idl = (INTERNAL_DIRECTORYLIST*)malloc(sizeof(INTERNAL_DIRECTORYLIST));
  DIR* dir;
  struct dirent* d;

  dl->_data = idl;

  // obviously, there are no directories at first
  idl->numdirectories = 0;
  idl->directories = NULL;
  idl->currentdirectory = 0;

  // open directory stream
  dir = opendir(".");
  if (dir == NULL)
    return;

  // build the directory list
  while ((d = readdir(dir)))
  {
    // can't start with a period
    if (d->d_name[0] != 0 &&
        d->d_name[0] != '.')
    {
      idl->directories = (char**)realloc(idl->directories, (sizeof(char*) * (idl->numdirectories + 1)));
      idl->directories[idl->numdirectories] = (char*)malloc(strlen(d->d_name) + 1);
      strcpy(idl->directories[idl->numdirectories], d->d_name);
      idl->numdirectories++;
    }
  }

  closedir(dir);
}

////////////////////////////////////////////////////////////////////////////////

void EndDirectoryList(DIRECTORYLIST* dl)
{
  INTERNAL_DIRECTORYLIST* idl = dl->_data;
  int i;

  idl->currentdirectory = 0;

  for (i = 0; i < idl->numdirectories; i++)
    free(idl->directories[i]);
  free(idl->directories);

  idl->numdirectories = 0;
  idl->directories = NULL;
  
  dl->_data = NULL;
}

////////////////////////////////////////////////////////////////////////////////

bool DirectoryListDone(DIRECTORYLIST* dl)
{
  INTERNAL_DIRECTORYLIST* idl = dl->_data;
  return !(idl->currentdirectory < idl->numdirectories);
}

////////////////////////////////////////////////////////////////////////////////

void GetNextDirectory(DIRECTORYLIST* dl, char* directory)
{
  INTERNAL_DIRECTORYLIST* idl = dl->_data;
  strcpy(directory, idl->directories[idl->currentdirectory]);
  idl->currentdirectory++;
}

////////////////////////////////////////////////////////////////////////////////
