#include "unix_filesystem.h"
#include <iostream>
#include <errno.h>


std::stack<string> directory_stack;

/*! \brief return the current working directory */
bool GetDirectory (string& directory) {
  char* dir;

  dir = getcwd(NULL, 0);
  if (dir == NULL)
    return false;
  directory = dir;
  free(dir);
  return true;
}

/*! \brief create a directory */
bool MakeDirectory (const char* directory) {
  /* we've got to assume a file access mode since sphere isn't aware of these */
  if ((mkdir(directory, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) < 0) && (errno != EEXIST))
    return false;
  return true;
}

/*! \brief enter into a directory

  This is more analogous to following a link than to cd'ing into a directory since you're able to leave the directory later (literally return the previous directory).  Internally a stack is used to implement this. */
bool EnterDirectory (const char* directory) {
  string current_directory;

  if (!GetDirectory(current_directory))
    return false;
  if (chdir(directory) < 0)
    return false;
  directory_stack.push(current_directory);
  return true;
}

/*! \brief exit the last entered directory
  \warning use only after entering a directory

  This function returns the program to the directory that it was in before the last call to EnterDirectory. */
bool LeaveDirectory () {
  string old_directory;

  if (directory_stack.empty())
    return false; /* can't return to a directory when we haven't entered one yet! */
  old_directory = directory_stack.top();
  if (chdir(old_directory.c_str()) < 0)
    return false;
  directory_stack.pop();
  return true;
}

/*! \brief return a list of directories matching mask

  The mask is used just like filename globbing in the shell. */
DIRECTORYLIST BeginDirectoryList (const char* mask) {
  DIRECTORYLIST dl = new DIRECTORYLISTimp;
  glob_t matches;
  int lcv;

  memset(&matches, 0, sizeof(matches));
  if (glob(mask, GLOB_ERR | GLOB_MARK, NULL, &matches) != 0) {
    delete dl;
    return NULL;
  }

  for (lcv = 0; lcv < matches.gl_pathc; lcv++) {
    /* we only want to show visible directories */
    if ((matches.gl_pathv[lcv][0] != '.') && (matches.gl_pathv[lcv][strlen(matches.gl_pathv[lcv]) - 1] == '/'))
      dl->directories.push_back(matches.gl_pathv[lcv]);
  }
  globfree(&matches);
  dl->current_directory = dl->directories.begin();
  return dl;
}

/*! \brief clean memory used by the directory list */
void EndDirectoryList (DIRECTORYLIST dl) {
  if (dl != NULL) {
    dl->directories.clear();
    delete dl;
  }
}

/*! \brief determine if the directory list is finished */
bool DirectoryListDone (DIRECTORYLIST dl) {
  if (dl == NULL)
    return true;
  return (dl->current_directory == dl->directories.end());
}

/*! \brief get the next directory from the listing

  Due to GNU's globbing, these are returned alphabetically */
void GetNextDirectory (DIRECTORYLIST dl, char directory[FILENAME_MAX]) {
  strcpy(directory, dl->current_directory->c_str());
  dl->current_directory++;
}

/*! \brief return a list of files matching mask

  The mask is used just like filename globbing in the shell. */
FILELIST BeginFileList (const char* mask) {
  FILELIST fl = new FILELISTimp;
  glob_t matches;
  int lcv;

  bzero(&matches, sizeof(glob_t));
  if (glob(mask, GLOB_ERR | GLOB_MARK, NULL, &matches) != 0)
    return fl;
  for (lcv = 0; lcv < matches.gl_pathc; lcv++) {
    /* we only want to show visible files */
    if ((matches.gl_pathv[lcv][0] != '.') && (matches.gl_pathv[lcv][strlen(matches.gl_pathv[lcv]) - 1] != '/'))
      fl->files.push_back(matches.gl_pathv[lcv]);
  }
  globfree(&matches);
  fl->current_file = fl->files.begin();
  return fl;
}

/*! \brief clean up the memory used by a file list */
void EndFileList (FILELIST fl) {
  if (fl != NULL) {
    fl->files.clear();
    delete fl;
  }
}

/*! \brief determine if the file list is finished */
bool FileListDone (FILELIST fl) {
  if (fl == NULL)
	 return true;
  return (fl->current_file == fl->files.end());
}

/*! \brief get the next directory from the listing

  Due to GNU's globbing, these are returned alphabetically */
void GetNextFile (FILELIST fl, char file[FILENAME_MAX]) {
  strcpy(file, fl->current_file->c_str());
  fl->current_file++;
}
