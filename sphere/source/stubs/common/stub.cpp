#include <stdio.h>
#include "stub.hpp"


int Run(const char* executable, HINSTANCE instance, char* command_line)
{
  // get base Sphere path
  char sphere_path[MAX_PATH];
  GetModuleFileName(instance, sphere_path, MAX_PATH);
  *strrchr(sphere_path, '\\') = 0;

  // calculate path of real executable
  char executable_path[MAX_PATH * 2];
  strcpy(executable_path, sphere_path);
  strcat(executable_path, "\\bin\\");
  strcat(executable_path, executable);

  char true_command_line[8192];
  strcpy(true_command_line, executable);
  strcat(true_command_line, " ");
  strcat(true_command_line, command_line);

  STARTUPINFO si;
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);

  PROCESS_INFORMATION pi;
  BOOL result = CreateProcess(
    executable_path,
    true_command_line,
    NULL,
    NULL,
    FALSE,
    NORMAL_PRIORITY_CLASS,
    NULL,
    sphere_path,
    &si,
    &pi
  );

  if (result) {
    // we don't need the process handle
    CloseHandle(pi.hProcess);
  } else {
    char message[MAX_PATH * 2];
    sprintf(message, "Error:  Could not start '%s'", executable);
    MessageBox(NULL, message, "Sphere Stub", MB_OK);
  }

  return 0;
}
