#ifndef PACKAGE_HPP
#define PACKAGE_HPP


#include <list>
#include <string>


typedef void (__cdecl *PackageFileWrittenCallBack)(const char* filename, int index, int total);


// creates .spk (.tar.gz) Sphere package
class CPackage
{
public:
  void AddFile(const char* filename);
  bool Write(const char* filename, PackageFileWrittenCallBack file_written = NULL);

private:
  std::list<std::string> m_files; // relative paths
};


#endif
