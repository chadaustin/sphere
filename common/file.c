#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "file.h"


////////////////////////////////////////////////////////////////////////////////

BOOL NewFileName(HWND parent, const char* title, const char* filter, char* filename)
{
  OPENFILENAME ofn;

  memset(&ofn, 0, sizeof(ofn));
  strcpy(filename, "");
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = parent;
  ofn.hInstance = (HINSTANCE)GetWindowLong(parent, GWL_HINSTANCE);
  ofn.lpstrFilter = filter;
  ofn.lpstrFile = filename;
  ofn.nMaxFile = 260;
  ofn.lpstrTitle = title;
  ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
  ofn.lpstrDefExt = filter;

  if (GetOpenFileName(&ofn) != TRUE)
    return FALSE;

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

BOOL OpenFileName(HWND parent, const char* title, const char* filter, char* filename)
{
  OPENFILENAME ofn;

  memset(&ofn, 0, sizeof(ofn));
  strcpy(filename, "");
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = parent;
  ofn.hInstance = (HINSTANCE)GetWindowLong(parent, GWL_HINSTANCE);
  ofn.lpstrFilter = filter;
  ofn.lpstrFile = filename;
  ofn.nMaxFile = 260;
  ofn.lpstrTitle = title;
  ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
  ofn.lpstrDefExt = filter;

  if (GetOpenFileName(&ofn) != TRUE)
    return FALSE;

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

BOOL SaveFileName(HWND parent, const char* title, const char* filter, char* filename)
{
  OPENFILENAME ofn;

  memset(&ofn, 0, sizeof(ofn));
  strcpy(filename, "");
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = parent;
  ofn.hInstance = (HINSTANCE)GetWindowLong(parent, GWL_HINSTANCE);
  ofn.lpstrFilter = filter;
  ofn.lpstrFile = filename;
  ofn.nMaxFile = 260;
  ofn.lpstrTitle = title;
  ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY;
  ofn.lpstrDefExt = filter;

  if (GetSaveFileName(&ofn) != TRUE)
    return FALSE;

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

long FileSize(const char* filename)
{
  long filesize;
  FILE* file = fopen(filename, "rb");
  if (file == NULL) return -1;
  fseek(file, 0, SEEK_END);
  filesize = ftell(file);
  fclose(file);
  return filesize;
}

////////////////////////////////////////////////////////////////////////////////

void GetProgramName(char* name)
{
  char commandline[512];
  char* lastbackslash;
  strcpy(commandline, GetCommandLine() + 1);  // Automatically gets rid of the first "
  *strchr(commandline, '"') = 0;  // Ends the string at the second "
  lastbackslash = strrchr(commandline, '\\');
  if (strchr(lastbackslash, ' '))  // Ends the string at the first space after the last slash, if there is one
    *strchr(lastbackslash, ' ') = 0;
  strcpy(name, commandline);
}

////////////////////////////////////////////////////////////////////////////////

void GetProgramDirectory(char* dir)
{
  char commandline[512];
  if (*GetCommandLine() == '"')
  {
    strcpy(commandline, GetCommandLine() + 1);
    *strchr(commandline, '"') = 0;
  }
  else
  {
    strcpy(commandline, GetCommandLine());
    *strchr(commandline, ' ') = 0;
  }

  *(strrchr(commandline, '\\') + 1) = 0;
  strcpy(dir, commandline);
}

////////////////////////////////////////////////////////////////////////////////

char** GetSubDirs(int* numdirs, const char* dir)
{
  char _c[512];
  char** c;
  int i;

  *numdirs = 0;

  strcpy(_c, dir);

  // if strings don't end with a backslash, make them
  if (_c[strlen(_c) - 1] != '\\')
    strcat(_c, "\\");

  // count backslashes
  {
    char* __c = _c;
    while (*__c)
    {
      if (*__c == '\\')
        (*numdirs)++;
      __c++;
    }
  }

  // allocate and fill string pointers
  c = (char**)(malloc(*numdirs * sizeof(char*)));

  {
    char *str = _c;
    for (i = 0; i < *numdirs; i++)
    {
      char* y = strchr(str, '\\');
      *y = 0;
      c[i] = (char*)malloc(strlen(str) + 1);
      strcpy(c[i], str);
      str += strlen(str) + 1;
    }
  }

  return c;
}

////////////////////////////////////////////////////////////////////////////////

const char* GetRelativePath(const char* destdir, const char* curpath)
{
  static char relpath[520];

  char cur[520], dest[520];
  int nc, nd;
  char** c, **d;
  int numcommon;
  int i;

  strcpy(cur, curpath);
  strcpy(dest, destdir);

  c = GetSubDirs(&nc, curpath);
  nc--;
  d = GetSubDirs(&nd, destdir);

  // find common path elements
  numcommon = 0;
  while (strcmp(c[numcommon], d[numcommon]) == 0)
  {
    numcommon++;
    if (numcommon >= nc || numcommon >= nd)
      break;
  }
  nc++;

  // build path string
  strcpy(relpath, "");
  for (i = 0; i < nd - numcommon; i++)
    strcat(relpath, "..\\");
  for (i = numcommon; i < nc; i++)
  {
    strcat(relpath, c[i]);
    strcat(relpath, "\\");
  }

  relpath[strlen(relpath) - 1] = 0;
  return relpath;
}

////////////////////////////////////////////////////////////////////////////////

const char* GetAbsolutePath(const char* absdir, const char* relpath)
{
  static char ret_absdir[520];

  int num_rel_sections;
  const char** rel_sections = GetSubDirs(&num_rel_sections, relpath);

  int num_abs_sections;
  const char** abs_sections = GetSubDirs(&num_abs_sections, absdir);

  int i;
  int back_up = 0;

  strcpy(ret_absdir, "");

  for (i = 0; i < num_rel_sections; i++)
    if (strcmp(rel_sections[i], "..") == 0)
      back_up++;

  for (i = 0; i < num_abs_sections - back_up; i++)
  {
    strcat(ret_absdir, abs_sections[i]);
    strcat(ret_absdir, "\\");
  }

  for (i = back_up; i < num_rel_sections; i++)
  {
    strcat(ret_absdir, rel_sections[i]);
    if (i != num_rel_sections - 1)
      strcat(ret_absdir, "\\");
  }

  return ret_absdir;
}

////////////////////////////////////////////////////////////////////////////////
