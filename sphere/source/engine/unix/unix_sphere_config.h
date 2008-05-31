#ifndef UNIX_SPHERE_CONFIG_H
#define UNIX_SPHERE_CONFIG_H

#include <string>

enum
{
    SOUND_AUTODETECT,
    SOUND_ON,
    SOUND_OFF,
};

struct PLAYERCONFIG
{
    std::string key_up_str;
    std::string key_down_str;
    std::string key_left_str;
    std::string key_right_str;

    bool keyboard_input_allowed;
    bool joypad_input_allowed;
};

struct SPHERECONFIG
{
    // video
    std::string videodriver;

    // audio
    int sound;
    std::string audiodriver;

    // network
    bool allow_networking;

    // input
    PLAYERCONFIG player_configurations[4];
};

extern bool LoadSphereConfig(SPHERECONFIG* config, const char* filename);
extern bool SaveSphereConfig(SPHERECONFIG* config, const char* filename);


#endif
