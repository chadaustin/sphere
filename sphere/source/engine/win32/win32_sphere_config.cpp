#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include "win32_sphere_config.hpp"
#include "../../common/configfile.hpp"


const char* DEFAULT_VIDEODRIVER = "standard32.dll";
const char* DEFAULT_AUDIODRIVER = "audio_directsound.dll";


////////////////////////////////////////////////////////////////////////////////

bool LoadSphereConfig(SPHERECONFIG* config, const char* filename)
{
  CConfigFile file;
  file.Load(filename);

  // video
  config->videodriver = file.ReadString("Video", "Driver", DEFAULT_VIDEODRIVER);

  // input
  config->joystick = file.ReadBool("Input", "Joystick", false);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool SaveSphereConfig(SPHERECONFIG* config, const char* filename)
{
  CConfigFile file;

  // video
  file.WriteString("Video", "Driver", config->videodriver.c_str());

  // input
  file.WriteBool("Input", "Joystick", config->joystick);

  file.Save(filename);

  return true;
}

////////////////////////////////////////////////////////////////////////////////
