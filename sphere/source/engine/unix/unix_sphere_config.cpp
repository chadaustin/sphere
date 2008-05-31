#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include "unix_sphere_config.h"
#include "../../common/configfile.hpp"


////////////////////////////////////////////////////////////////////////////////
static void InitializePlayerConfig(PLAYERCONFIG* config, int player_index)
{
    if (!config)
        return;

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

    // load video settings
    config->videodriver      = file.ReadString("Video" ,  "Driver",          "");

    // load sound settings
    config->sound            = file.ReadInt   ("Audio" ,  "Preference",      SOUND_AUTODETECT);
    config->audiodriver      = file.ReadString("Audio",   "Driver",          "");

    // load network settings
    config->allow_networking = file.ReadBool  ("Network", "AllowNetworking", true);

    // load input settings
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

        if ((key_up_str = file.ReadString(current.c_str(), "Up", "")) != "")
            config->player_configurations[x].key_up_str = key_up_str;

        if ((key_down_str = file.ReadString(current.c_str(), "Down", "")) != "")
            config->player_configurations[x].key_down_str = key_down_str;

        if ((key_left_str = file.ReadString(current.c_str(), "Left", "")) != "")
            config->player_configurations[x].key_left_str = key_left_str;

        if ((key_right_str = file.ReadString(current.c_str(), "Right", "")) != "")
            config->player_configurations[x].key_right_str = key_right_str;

        config->player_configurations[x].keyboard_input_allowed = file.ReadBool(current.c_str(), "AllowKeyboardInput", true);
        config->player_configurations[x].joypad_input_allowed   = file.ReadBool(current.c_str(), "AllowJoypadInput",   true);
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool SaveSphereConfig(SPHERECONFIG* config, const char* filename)
{
    CConfigFile file;

    // save video settings
    file.WriteString("Video" ,  "Driver",      config->videodriver.c_str());

    // save sound settings
    file.WriteInt   ("Audio" ,  "Preference",  config->sound);
    file.WriteString("Audio",   "Driver",      config->audiodriver.c_str());

    // save network settings
    file.WriteBool  ("Network", "Allowed",     config->allow_networking);

    // save input settings
    const char identify[] = "1234";
    for(int x = 0; x < 4; x++)
    {
        std::string current("Player");
        current += identify[x];

        file.WriteString(current.c_str(), "Up",                 config->player_configurations[x].key_up_str.c_str());
        file.WriteString(current.c_str(), "Left",               config->player_configurations[x].key_left_str.c_str());
        file.WriteString(current.c_str(), "Right",              config->player_configurations[x].key_right_str.c_str());
        file.WriteString(current.c_str(), "Down",               config->player_configurations[x].key_down_str.c_str());
        file.WriteBool  (current.c_str(), "AllowKeyboardInput", config->player_configurations[x].keyboard_input_allowed);
        file.WriteBool  (current.c_str(), "AllowJoypadInput",   config->player_configurations[x].joypad_input_allowed);
    }

    file.Save(filename);

    return true;
}

////////////////////////////////////////////////////////////////////////////////
