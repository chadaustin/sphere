#ifndef FILE_TYPES_HPP
#define FILE_TYPES_HPP


#include <vector>
#include <string>


// supported file groups
enum
{
  GT_MAPS,
  GT_SPRITESETS,
  GT_SCRIPTS,
  GT_SOUNDS,
  GT_FONTS,
  GT_WINDOWSTYLES,
  GT_IMAGES,
  GT_ANIMATIONS,

  // special codes
  NUM_GROUP_TYPES,
  GT_UNKNOWN,

  GT_TILESETS = NUM_GROUP_TYPES,  // this one shouldn't be listed in projects
  GT_PACKAGES,
};


class CFileTypeLibrary
{
public:
  CFileTypeLibrary();

  const char* GetFileTypeLabel(int file_type);
  void        GetFileTypeExtensions(int file_type, std::vector<std::string>& extensions);
  const char* GetDefaultExtension(int file_type);
  int         GetNumSubTypes(int file_type);
  const char* GetSubTypeLabel(int file_type, int sub_type);
  void        GetSubTypeExtensions(int file_type, int sub_type, std::vector<std::string>& extensions); 

private:
  struct SubType
  {
    std::string label;
    std::vector<std::string> extensions;
  };

  struct FileType
  {
    std::string name;
    std::string default_extension;
    std::vector<SubType> sub_types;
  };

private:
  std::vector<FileType> m_FileTypes;
};


extern CFileTypeLibrary FTL;


#endif
