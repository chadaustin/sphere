#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include "unix_sphere_config.h"
#include "../../common/configfile.hpp"


const char* DEFAULT_VIDEODRIVER = "";
const char* DEFAULT_RENDER      = "";

////////////////////////////////////////////////////////////////////////////////

static
void InitializePlayerConfig(PLAYERCONFIG* config, int player_index)
{
  if (!config) return;

  switch (player_index)
  {
    case 0:
      config->key_up_str                = "KEY_UP";
      config->key_down_str              = "KEY_DOWN";
      config->key_left_str              = "KEY_LEFT";
      config->key_right_str             = "KEY_RIGHT";
      config->keyboard_input_allowed    = true;
      config->joypad_input_allowed      = true;
    break;

    case 1:
      config->key_up_str                = "KEY_W";
      config->key_down_str              = "KEY_S";
      config->key_left_str              = "KEY_A";
      config->key_right_str             = "KEY_D";
      config->keyboard_input_allowed    = true;
      config->joypad_input_allowed      = true;
    break;

    case 2:
      config->key_up_str                = "KEY_I";
      config->key_down_str              = "KEY_K";
      config->key_left_str              = "KEY_J";
      config->key_right_str             = "KEY_L";
      config->keyboard_input_allowed    = true;
      config->joypad_input_allowed      = true;
    break;

    case 3:
      config->key_up_str                = "KEY_NUM_8";
      config->key_down_str              = "KEY_NUM_2";
      config->key_left_str              = "KEY_NUM_4";
      config->key_right_str             = "KEY_NUM_6";
      config->keyboard_input_allowed    = true;
      config->joypad_input_allowed      = true;
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool LoadSphereConfig(SPHERECONFIG* config, const char* filename)
{
  CConfigFile file;
  file.Load(filename);

  config->videodriver      = file.ReadString("Video" ,  "Driver",       DEFAULT_VIDEODRIVER);
  config->bitdepth         = file.ReadInt   ("Video",   "BitDepth",     32);
  config->fullscreen       = file.ReadBool  ("Video",   "Fullscreen",   false);
  config->vsync            = file.ReadBool  ("Video",   "VSync",        true);
  config->scale            = file.ReadBool  ("Video",   "Scale",        false);
  config->filter           = file.ReadInt   ("Video",   "Filter",       0);

  config->sound            = file.ReadInt   ("Audio" ,  "Preference", SOUND_AUTODETECT);
  config->allow_networking = file.ReadBool  ("Network", "Allowed",    true);

  const char identify[] = "1234";

  for(int x = 0; x < 4; x++)
  {
    std::string current("Player");
    std::string key_up_str;
    std::string key_down_str;
    std::string key_left_str;
    std::string key_right_str;

    current += identify[x];

    // Set default configuration to be overridden.
    InitializePlayerConfig(&config->player_configurations[x], x);

    if ((key_up_str = file.ReadString(current.c_str(), "KeyUp", "")) != "")
        config->player_configurations[x].key_up_str = key_up_str;

    if ((key_down_str = file.ReadString(current.c_str(), "KeyDown", "")) != "")
        config->player_configurations[x].key_down_str = key_down_str;

    if ((key_left_str = file.ReadString(current.c_str(), "KeyLeft", "")) != "")
        config->player_configurations[x].key_left_str = key_left_str;

    if ((key_right_str = file.ReadString(current.c_str(), "KeyRight", "")) != "")
        config->player_configurations[x].key_right_str = key_right_str;

    config->player_configurations[x].keyboard_input_allowed = file.ReadInt(current.c_str(), "KeyboardAllowed", 1);
    config->player_configurations[x].joypad_input_allowed   = file.ReadInt(current.c_str(), "JoypadAllowed", 1);
  }

  /*
  reference...
  char key_up_str[25];
  char key_down_str[25];
  char key_left_str[25];
  char key_right_str[25];
  bool keyboard_input_allowed;
  bool joypad_input_allowed;
  */
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool SaveSphereConfig(SPHERECONFIG* config, const char* filename)
{
  CConfigFile file;
  file.WriteString("Video" ,  "Driver",      config->videodriver.c_str());
  file.WriteInt   ("Video" ,  "BitDepth",    config->bitdepth);
  file.WriteBool  ("Video",   "Fullscreen",  config->fullscreen);
  file.WriteBool  ("Video",   "VSync",       config->vsync);
  file.WriteBool  ("Video",   "Scale",       config->scale);
  file.WriteInt   ("Video" ,  "Filter",      config->filter);

  file.WriteInt   ("Audio" ,  "Preference", config->sound);
  file.WriteBool  ("Network", "Allowed",    config->allow_networking);

  const char identify[] = "1234";

  for(int x = 0; x < 4; x++)
  {
    std::string current("Player");
    current += identify[x];

    file.WriteString(current.c_str(), "KeyUp",           config->player_configurations[x].key_up_str.c_str());
    file.WriteString(current.c_str(), "KeyLeft",         config->player_configurations[x].key_left_str.c_str());
    file.WriteString(current.c_str(), "KeyRight",        config->player_configurations[x].key_right_str.c_str());
    file.WriteString(current.c_str(), "KeyDown",         config->player_configurations[x].key_down_str.c_str());
    file.WriteInt   (current.c_str(), "KeyboardAllowed", config->player_configurations[x].keyboard_input_allowed);
    file.WriteInt   (current.c_str(), "JoypadAllowed",   config->player_configurations[x].joypad_input_allowed);
  }

  file.Save(filename);
  return true;
}

////////////////////////////////////////////////////////////////////////////////
