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

  switch (player_index) {
    case 0:
      config->key_up_str = "KEY_UP";
      config->key_down_str = "KEY_DOWN";
      config->key_left_str = "KEY_LEFT";
      config->key_right_str = "KEY_RIGHT";
      config->keyboard_input_allowed = true;
      config->joypad_input_allowed = true;
    break;

    case 1:
      config->key_up_str = "KEY_W";
      config->key_down_str = "KEY_S";
      config->key_left_str = "KEY_A";
      config->key_right_str = "KEY_F";
      config->keyboard_input_allowed = true;
      config->joypad_input_allowed = true;
    break;

    case 2:
      config->key_up_str = "KEY_I";
      config->key_down_str = "KEY_K";
      config->key_left_str = "KEY_J";
      config->key_right_str = "KEY_L";
      config->keyboard_input_allowed = true;
      config->joypad_input_allowed = true;
    break;

    case 3:
      config->key_up_str = "KEY_8";
      config->key_down_str = "KEY_5";
      config->key_left_str = "KEY_4";
      config->key_right_str = "KEY_6";
      config->keyboard_input_allowed = true;
      config->joypad_input_allowed = true;
    break;
  }
}

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
  config->allow_networking = file.ReadBool ("Network", "Allowed", true);

  const char identify[] = "1234";

  for(int x = 0; x < 4; x++) {
    std::string current("Player");
    current += identify[x];
    config->player_configurations[x].key_up_str = file.ReadString(current.c_str(), "KeyUp", "");
    config->player_configurations[x].key_down_str = file.ReadString(current.c_str(), "KeyDown", "");
    config->player_configurations[x].key_left_str = file.ReadString(current.c_str(), "KeyLeft", "");
    config->player_configurations[x].key_right_str = file.ReadString(current.c_str(), "KeyRight", "");
    config->player_configurations[x].keyboard_input_allowed = file.ReadInt(current.c_str(), "KeyboardAllowed", 1);
    config->player_configurations[x].joypad_input_allowed = file.ReadInt(current.c_str(), "JoypadAllowed", 1);
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
  file.WriteBool  ("Network", "Allowed", config->allow_networking);

  const char identify[] = "1234";

  for(int x = 0; x < 4; x++) {
    std::string current("Player");
    current += identify[x];
    file.WriteString(current.c_str(), "KeyUp", config->player_configurations[x].key_up_str.c_str());
    file.WriteString(current.c_str(), "KeyLeft", config->player_configurations[x].key_left_str.c_str());
    file.WriteString(current.c_str(), "KeyRight", config->player_configurations[x].key_right_str.c_str());
    file.WriteString(current.c_str(), "KeyDown", config->player_configurations[x].key_down_str.c_str());
    file.WriteInt(current.c_str(), "KeyboardAllowed", config->player_configurations[x].keyboard_input_allowed);
    file.WriteInt(current.c_str(), "JoypadAllowed", config->player_configurations[x].joypad_input_allowed);
  }

  file.Save(filename);
  return true;
}

////////////////////////////////////////////////////////////////////////////////
