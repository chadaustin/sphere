#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#ifdef WIN32
#include <windows.h>
#endif

#include <vector>
#include <string>

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
std::vector<std::string> GetFolderList(const char* filter);

#endif // SYSTEM_HPP

