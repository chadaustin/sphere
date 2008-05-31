#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include "win32_sphere_config.hpp"
#include "../../common/configfile.hpp"


////////////////////////////////////////////////////////////////////////////////
static void InitializePlayerConfig(PLAYERCONFIG* config, int player_index)
{
    if (!config)
        return;

    switch (player_index)
    {
        case 0:
            strcpy(config->key_up_str,    "KEY_UP");
            strcpy(config->key_down_str,  "KEY_DOWN");
            strcpy(config->key_left_str,  "KEY_LEFT");
            strcpy(config->key_right_str, "KEY_RIGHT");
            config->keyboard_input_allowed = true;
            config->joypad_input_allowed   = true;
            break;

        case 1:
            strcpy(config->key_up_str,    "KEY_W");
            strcpy(config->key_down_str,  "KEY_S");
            strcpy(config->key_left_str,  "KEY_A");
            strcpy(config->key_right_str, "KEY_D");
            config->keyboard_input_allowed = true;
            config->joypad_input_allowed   = true;
            break;

        case 2:
            strcpy(config->key_up_str,    "KEY_I");
            strcpy(config->key_down_str,  "KEY_K");
            strcpy(config->key_left_str,  "KEY_J");
            strcpy(config->key_right_str, "KEY_L");
            config->keyboard_input_allowed = true;
            config->joypad_input_allowed   = true;
            break;

        case 3:
            strcpy(config->key_up_str,    "KEY_NUM_8");
            strcpy(config->key_down_str,  "KEY_NUM_5");
            strcpy(config->key_left_str,  "KEY_NUM_4");
            strcpy(config->key_right_str, "KEY_NUM_6");
            config->keyboard_input_allowed = true;
            config->joypad_input_allowed   = true;
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
bool LoadSphereConfig(SPHERECONFIG* config, const char* filename)
{
    if (!config) return false;

    CConfigFile file;
    file.Load(filename);

    // load video settings
    config->videodriver = file.ReadString("Video", "Driver",     "");

    // load sound settings
    config->sound       = file.ReadInt   ("Audio", "Preference", SOUND_AUTODETECT);
    config->audiodriver = file.ReadString("Audio", "Driver",     "directsound");

    // load network settings
    config->allow_networking = file.ReadBool("Network", "AllowNetworking", true);

    // load video capture settings
    config->video_capture_mode      = file.ReadInt("VideoCapture", "Mode",      VIDEO_CAPTURE_SCREENSHOT_ONLY);
    config->video_capture_timer     = file.ReadInt("VideoCapture", "Timer",     1000);
    config->video_capture_framerate = file.ReadInt("VideoCapture", "FrameRate", 1);

    // validate video capture config settings
    if (config->video_capture_framerate <= 0)
        config->video_capture_framerate = 1;

    if (config->video_capture_mode != VIDEO_CAPTURE_SCREENSHOT_ONLY &&
        config->video_capture_mode != VIDEO_CAPTURE_UNTIL_OUTOFTIME &&
        config->video_capture_mode != VIDEO_CAPTURE_UNTIL_F12_KEYED)
    {
        config->video_capture_mode = VIDEO_CAPTURE_SCREENSHOT_ONLY;
    }

    if (config->video_capture_timer <= 0)
    {
        config->video_capture_timer = 1000;
    }

    for (int i = 0; i < 4; i++)
    {
        InitializePlayerConfig(&config->player_configurations[i], i);

        char section[100];
        sprintf (section, "Player%d", i + 1);

        std::string key_up    = file.ReadString(section, "Up",    config->player_configurations[i].key_up_str);
        std::string key_down  = file.ReadString(section, "Down",  config->player_configurations[i].key_down_str);
        std::string key_left  = file.ReadString(section, "Left",  config->player_configurations[i].key_left_str);
        std::string key_right = file.ReadString(section, "Right", config->player_configurations[i].key_right_str);

        config->player_configurations[i].keyboard_input_allowed = file.ReadBool(section, "AllowKeyboardInput", config->player_configurations[i].keyboard_input_allowed);
        config->player_configurations[i].joypad_input_allowed   = file.ReadBool(section, "AllowJoypadInput",   config->player_configurations[i].joypad_input_allowed);

        if (key_up.length() > 0 && key_up.length() < sizeof(config->player_configurations[i].key_up_str))
            strcpy(config->player_configurations[i].key_up_str, key_up.c_str());
        if (key_down.length() > 0 && key_down.length() < sizeof(config->player_configurations[i].key_down_str))
            strcpy(config->player_configurations[i].key_down_str, key_down.c_str());
        if (key_left.length() > 0 && key_left.length() < sizeof(config->player_configurations[i].key_left_str))
            strcpy(config->player_configurations[i].key_left_str, key_left.c_str());
        if (key_right.length() > 0 && key_right.length() < sizeof(config->player_configurations[i].key_right_str))
            strcpy(config->player_configurations[i].key_right_str, key_right.c_str());
    }

    SaveSphereConfig(config, filename);

    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool SaveSphereConfig(SPHERECONFIG* config, const char* filename)
{
    if (!config) return false;

    CConfigFile file;

    // save video settings
    file.WriteString("Video", "Driver",                 config->videodriver.c_str());

    // save sound settings
    file.WriteInt   ("Audio", "Preference",             config->sound);
    file.WriteString("Audio", "Driver",                 config->audiodriver.c_str());

    // save network settings
    file.WriteBool  ("Network", "AllowNetworking",      config->allow_networking);

    // save video capture settings
    file.WriteInt   ("VideoCapture", "Mode",            config->video_capture_mode);
    file.WriteInt   ("VideoCapture", "Timer",           config->video_capture_timer);
    file.WriteInt   ("VideoCapture", "FrameRate",       config->video_capture_framerate);

    // save input settings
    for (int i = 0; i < 4; i++)
    {
        char section[100];
        sprintf (section, "Player%d", i + 1);

        file.WriteString(section, "Up",                 config->player_configurations[i].key_up_str);
        file.WriteString(section, "Down",               config->player_configurations[i].key_down_str);
        file.WriteString(section, "Left",               config->player_configurations[i].key_left_str);
        file.WriteString(section, "Right",              config->player_configurations[i].key_right_str);
        file.WriteBool(section, "AllowKeyboardInput",   config->player_configurations[i].keyboard_input_allowed);
        file.WriteBool(section, "AllowJoypadInput",     config->player_configurations[i].joypad_input_allowed);
    }

    file.Save(filename);

    return true;
}

////////////////////////////////////////////////////////////////////////////////
