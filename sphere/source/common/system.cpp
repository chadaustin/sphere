
// disable 'identifier too long' warning
#pragma warning(disable : 4786)

///////////////////////////////////////////////////////////

#ifdef WIN32
#include <windows.h>
#endif

#include <vector>
#include <string>

#ifndef WIN32
#include <glob.h>
#include <unistd.h>
#endif

///////////////////////////////////////////////////////////

#ifndef WIN32
bool SetCurrentDirectory(const char* directory)
{
  return (chdir(directory) >= 0);
}

bool CopyFile(const char* src, const char* dest, bool force) {
 return false; 
}

bool GetCurrentDirectory(int max_path, char* directory) {
  directory = getcwd(NULL, 0);
  if (directory == NULL)
    return false;
  return true;
}

bool CreateDirectory(const char* directory, const char* options) {
  return false;
}
#endif

///////////////////////////////////////////////////////////

std::vector<std::string> GetFileList(const char* filter)
{
  std::vector<std::string> file_list;
#ifdef WIN32
  WIN32_FIND_DATA ffd;
  HANDLE h = FindFirstFile(filter, &ffd);
  if (h != INVALID_HANDLE_VALUE) {

    do {
      if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        file_list.push_back(ffd.cFileName);

    } while (FindNextFile(h, &ffd));
    FindClose(h);
  }
#else // linux
  glob_t matches;
  memset(&matches, 0, sizeof(matches));

  if (glob(filter, GLOB_ERR | GLOB_MARK, NULL, &matches) == 0)
  {
    for (int lcv = 0; lcv < matches.gl_pathc; lcv++) {
      file_list.push_back(matches.gl_pathv[lcv]);
    }
    globfree(&matches);
  }
#endif
  return file_list;
}

///////////////////////////////////////////////////////////

std::vector<std::string> GetFolderList(const char* filter)
{
  std::vector<std::string> folder_list;
#ifdef WIN32
  WIN32_FIND_DATA ffd;
  HANDLE h = FindFirstFile(filter, &ffd);
  if (h != INVALID_HANDLE_VALUE) {

    do {
      if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        folder_list.push_back(ffd.cFileName);
			}
    } while (FindNextFile(h, &ffd));
    FindClose(h);
  }
#endif
  return folder_list;
}

///////////////////////////////////////////////////////////

std::string GetFilePath(const char* filename) {
  std::string path = filename;
  if (path.rfind("/") == -1) {
    path = "";
  }
  else {
    path = path.substr(0, path.rfind("/"));
  }

  return path;
}

///////////////////////////////////////////////////////////

std::string GetFileTitle(const char* filename) {
  std::string title = filename;
  if (strrchr(filename, '/') != NULL)
    title = strrchr(filename, '/') + 1;
  return title;
}

///////////////////////////////////////////////////////////

bool IsDirectory(const char* filename) {
  std::string title = GetFileTitle(filename);
  std::string path = GetFilePath(filename);

  std::vector<std::string> folderlist = GetFolderList(path == "" ? "*" : (path + "/" + "*").c_str());

  for (int i = 0; i < int(folderlist.size()); i++) {
    if (title == folderlist[i]) {
      return true;
    }
  }

  return false;
}

///////////////////////////////////////////////////////////

bool IsFile(const char* filename) {
  std::string title = GetFileTitle(filename);
  std::string path = GetFilePath(filename);

  std::vector<std::string> filelist = GetFileList(path == "" ? "*" : (path + "/" + "*").c_str());

  for (int i = 0; i < int(filelist.size()); i++) {
    if (title == filelist[i]) {
      return true;
    }
  }

  return false;
}

///////////////////////////////////////////////////////////

