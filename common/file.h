#ifndef __XFILE_H__
#define __XFILE_H__


#include "begin_c_prototypes.h"

  #define FileExists(filename) (FileSize(filename) != -1)

  extern BOOL NewFileName(HWND parent, const char* title, const char* filter, char* filename);
  extern BOOL OpenFileName(HWND parent, const char* title, const char* filter, char* filename);
  extern BOOL SaveFileName(HWND parent, const char* title, const char* filter, char* filename);

  extern long FileSize(const char* filename);

  extern void GetProgramName(char* name);
  extern void GetProgramDirectory(char* dir);
  //extern const char* GetRelativeDirectory(const char* cur, const char* dest);
  extern const char* GetRelativePath(const char* destdir, const char* curpath);
  extern const char* GetAbsolutePath(const char* absdir, const char* relpath);

#include "end_c_prototypes.h"


#endif
