#ifndef PACKAGE_HPP
#define PACKAGE_HPP


#include <list>
#include <string>


// creates .spk (.tar.gz) Sphere package
typedef void (*PackageFileWrittenCallBack)(const char* filename, int index, int total);


class CPackage
{
public:
  void AddFile(const char* filename);
  bool Write(const char* filename, PackageFileWrittenCallBack file_written = NULL);

private:
  std::list<std::string> m_files; // relative paths
};


#endif
