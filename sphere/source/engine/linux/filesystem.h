#ifndef __FILESYSTEM_H
#define __FILESYSTEM_H


#ifdef __cplusplus
extern "C" {
#endif


#include "types.h"


typedef struct
{
  void* _data;
} DIRECTORYLIST;


extern bool EnterDirectory(const char* directory);
extern bool LeaveDirectory(void);


extern void BeginDirectoryList(DIRECTORYLIST* dl, const char* mask);
extern void EndDirectoryList(DIRECTORYLIST* dl);
extern bool DirectoryListDone(DIRECTORYLIST* dl);
extern void GetNextDirectory(DIRECTORYLIST* dl, char* directory);


#ifdef __cplusplus
}
#endif


#endif
