#ifdef WIN32
#include <windows.h>
#else

#ifndef MAX_PATH
#define MAX_PATH 255
#endif

void SetCurrentDirectory(const char* directory) {

}

bool CopyFile(const char* src, const char* dest, bool force) {
 return false; 
}

bool GetCurrentDirectory(int max_path, char* directory) {
  return false;
}

bool CreateDirectory(const char* directory, const char* options) {
 return false;
}

#endif
