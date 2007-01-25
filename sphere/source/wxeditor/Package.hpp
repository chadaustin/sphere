#ifndef PACKAGE_HPP
#define PACKAGE_HPP


#include <list>
#include <string>


// creates .spk (.tar.gz) Sphere package


class CPackage
{
public:
  void AddFile(const char* filename);
  bool Write(const char* filename);

private:
  std::list<std::string> m_files; // relative paths
};


#endif
