#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include "win32_sphere_config.hpp"
#include "../../common/configfile.hpp"


const char* DEFAULT_VIDEODRIVER = "standard32.dll";


////////////////////////////////////////////////////////////////////////////////

bool LoadSphereConfig(SPHERECONFIG* config, const char* filename)
{
  if (!config) return false;

  CConfigFile file;
  file.Load(filename);
  config->videodriver = file.ReadString("Video", "Driver",     DEFAULT_VIDEODRIVER);
  config->sound       = file.ReadInt   ("Audio", "Preference", SOUND_AUTODETECT);
  
  // load video capture settings
  config->video_capture_mode      = file.ReadInt("VideoCapture", "Mode",      VIDEO_CAPTURE_SCREENSHOT_ONLY);
  config->video_capture_timer     = file.ReadInt("VideoCapture", "Timer",     1000);
  config->video_capture_framerate = file.ReadInt("VideoCapture", "FrameRate", 1);

  // validate video capture config settings
  if (config->video_capture_framerate <= 0)
    config->video_capture_framerate = 1;

  if (config->video_capture_mode != VIDEO_CAPTURE_SCREENSHOT_ONLY
   && config->video_capture_mode != VIDEO_CAPTURE_UNTIL_OUTOFTIME
   && config->video_capture_mode != VIDEO_CAPTURE_UNTIL_F12_KEYED) {
    config->video_capture_mode = VIDEO_CAPTURE_SCREENSHOT_ONLY;
  }

  if (config->video_capture_timer <= 0) {
    config->video_capture_timer = 1000;
  }

  SaveSphereConfig(config, filename);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool SaveSphereConfig(SPHERECONFIG* config, const char* filename)
{
  if (!config) return false;

  CConfigFile file;
  file.WriteString("Video", "Driver",     config->videodriver.c_str());
  file.WriteInt   ("Audio", "Preference", config->sound);

  file.WriteInt  ("VideoCapture", "Mode", config->video_capture_mode);
  file.WriteInt  ("VideoCapture", "Timer", config->video_capture_timer);
  file.WriteInt  ("VideoCapture", "FrameRate", config->video_capture_framerate);
  
  file.Save(filename);
  return true;
}

////////////////////////////////////////////////////////////////////////////////
