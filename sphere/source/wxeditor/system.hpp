#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <string>
#include <vector>

#ifdef WIN32
#include <windows.h>
#endif

#ifndef MAX_PATH
#define MAX_PATH 255
#endif

#ifndef WIN32

// non win32 definitions
bool SetCurrentDirectory(const char* directory);
bool CopyFile(const char* src, const char* dest, bool force);
bool GetCurrentDirectory(int max_path, char* directory);
bool CreateDirectory(const char* directory, const char* options);

#endif // WIN32

std::vector<std::string> GetFileList(const char* filter);

#endif // SYSTEM_HPP

