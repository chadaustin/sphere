#ifndef UNIX_SPHERE_CONFIG_H
#define UNIX_SPHERE_CONFIG_H


#include <string>


enum {
  SOUND_AUTODETECT,
  SOUND_ON,
  SOUND_OFF,
};

struct PLAYERCONFIG {
  char key_up_str[25];
  char key_down_str[25];
  char key_left_str[25];
  char key_right_str[25];
  bool keyboard_input_allowed;
  bool joypad_input_allowed;
};

struct SPHERECONFIG
{
  // video
  std::string videodriver;
  std::string scaling;
  int fullscreen;
  int showcursor;

  // audio
  int sound;

  // network
  bool allow_networking;

  // filter
  int r, g, b, a;
  int filter;

  // input
  PLAYERCONFIG player_configurations[4];
};


extern bool LoadSphereConfig(SPHERECONFIG* config, const char* filename);
extern bool SaveSphereConfig(SPHERECONFIG* config, const char* filename);


#endif
