#ifdef WIN32
#include <windows.h>
#endif

#include <string>
#include <vector>

#ifndef WIN32
#include <glob.h>
#include <unistd.h>

#ifndef MAX_PATH
#define MAX_PATH 255
#endif

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
