#ifndef FILE_SYSTEM_HPP
#define FILE_SYSTEM_HPP


#include <list>
#include <string>


extern bool PathExists(const char* pathname);
extern bool FileExists(const char* filename);
extern int  FileSize(const char* filename);
extern bool EnumerateFiles(const char* filter, const char* directory, std::list<std::string>& files);


#endif
