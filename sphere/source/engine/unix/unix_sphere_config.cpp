#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include "unix_sphere_config.h"
#include "../../common/configfile.hpp"


const char* DEFAULT_VIDEODRIVER = "";
const char* DEFAULT_RENDER      = "";

////////////////////////////////////////////////////////////////////////////////

bool LoadSphereConfig(SPHERECONFIG* config, const char* filename)
{
  CConfigFile file;
  file.Load(filename);
  config->videodriver = file.ReadString("Video" , "Driver",     DEFAULT_VIDEODRIVER);
  config->scaling     = file.ReadString("Video" , "Render",     DEFAULT_RENDER);
  config->fullscreen  = file.ReadInt   ("Video" , "Fullscreen", false);
  config->sound       = file.ReadInt   ("Audio" , "Preference", SOUND_AUTODETECT);
  config->r	      = file.ReadInt   ("Filter", "r"	      , 0);
  config->g	      = file.ReadInt   ("Filter", "g"	      , 0);
  config->b	      = file.ReadInt   ("Filter", "b"	      , 0);
  config->a	      = file.ReadInt   ("Filter", "a"	      , 0);
  config->filter      = file.ReadInt   ("Video" , "Filter"    , 0);
  config->showcursor  = file.ReadInt   ("Video" , "ShowCursor", 0);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool SaveSphereConfig(SPHERECONFIG* config, const char* filename)
{
  CConfigFile file;
  file.WriteString("Video" , "Driver",     config->videodriver.c_str());
  file.WriteString("Video" , "Render",     config->scaling.c_str());
  file.WriteInt   ("Video" , "Fullscreen", config->fullscreen);
  file.WriteInt   ("Audio" , "Preference", config->sound);
  file.WriteInt   ("Filter", "r"	 , config->r);
  file.WriteInt   ("Filter", "g"	 , config->g);
  file.WriteInt   ("Filter", "b"	 , config->b);
  file.WriteInt   ("Filter", "a"	 , config->a);
  file.WriteInt   ("Video", "Filter"	 , config->filter);
  file.WriteInt   ("Video", "ShowCursor" , config->showcursor);
  file.Save(filename);
  return true;
}

////////////////////////////////////////////////////////////////////////////////
