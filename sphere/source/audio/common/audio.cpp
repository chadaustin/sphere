#include <windows.h>
#include "audio.hpp"


HINSTANCE DriverInstance;


////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE instance, DWORD, LPVOID)
{
  DriverInstance = instance;
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void GetDriverConfigFile(char* config_file)
{
  GetModuleFileName(DriverInstance, config_file, MAX_PATH);
  
  // convert driver filename into configuration filename
  if (strrchr(config_file, '.') > strrchr(config_file, '\\'))
    strcpy(strrchr(config_file, '.'), ".cfg");
  else
    strcat(config_file, ".cfg");
}

////////////////////////////////////////////////////////////////////////////////
