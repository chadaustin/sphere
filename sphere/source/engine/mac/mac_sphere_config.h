#ifndef MAC_SPHERE_CONFIG_H
#define MAC_SPHERE_CONFIG_H

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

    int keyboard_input_allowed;
    int joypad_input_allowed;
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
