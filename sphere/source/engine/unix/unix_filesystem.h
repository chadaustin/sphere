#ifndef UNIX_FILESYSTEM_H
#define UNIX_FILESYSTEM_H

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <glob.h>
#include <stack>
#include <cstring>
#include <cstdio>
#include <string>
#include <list>

struct DIRECTORYLISTimp {
  std::list<std::string> directories;
  std::list<std::string>::iterator current_directory;
};

struct FILELISTimp {
  std::list<std::string> files;
  std::list<std::string>::iterator current_file;
};

typedef DIRECTORYLISTimp* DIRECTORYLIST;
typedef FILELISTimp* FILELIST;

bool GetDirectory(std::string& directory);
bool MakeDirectory(const char* directory);

bool EnterDirectory(const char* directory);
bool LeaveDirectory();

DIRECTORYLIST BeginDirectoryList(const char* mask);
void          EndDirectoryList(DIRECTORYLIST dl);
bool          DirectoryListDone(DIRECTORYLIST dl);
void          GetNextDirectory(DIRECTORYLIST dl, char directory[FILENAME_MAX]);

FILELIST BeginFileList(const char* mask);
void     EndFileList(FILELIST fl);
bool     FileListDone(FILELIST fl);
void     GetNextFile(FILELIST fl, char file[FILENAME_MAX]);

#endif /* UNIX_FILESYSTEM_H */
