#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include "unix_sphere_config.h"
#include "../../common/configfile.hpp"


const char* DEFAULT_VIDEODRIVER = "";


////////////////////////////////////////////////////////////////////////////////

bool LoadSphereConfig(SPHERECONFIG* config, const char* filename)
{
  CConfigFile file;
  file.Load(filename);
  config->videodriver = file.ReadString("Video", "Driver",     DEFAULT_VIDEODRIVER);
  config->sound       = file.ReadInt   ("Audio", "Preference", SOUND_AUTODETECT);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool SaveSphereConfig(SPHERECONFIG* config, const char* filename)
{
  CConfigFile file;
  file.WriteString("Video", "Driver",     config->videodriver.c_str());
  file.WriteInt   ("Audio", "Preference", config->sound);
  file.Save(filename);
  return true;
}

////////////////////////////////////////////////////////////////////////////////
