#ifndef WIN32_SPHERE_CONFIG_H
#define WIN32_SPHERE_CONFIG_H


#include <string>


enum {
  SOUND_AUTODETECT,
  SOUND_ON,
  SOUND_OFF,
};

enum {
  VIDEO_CAPTURE_SCREENSHOT_ONLY,
  VIDEO_CAPTURE_UNTIL_OUTOFTIME,
  VIDEO_CAPTURE_UNTIL_F12_KEYED,
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
  int video_capture_mode;
  int video_capture_timer;
  int video_capture_framerate;

  // audio
  int sound;

  // network
  bool allow_networking;

  // input
  PLAYERCONFIG player_configurations[4];
};


extern bool LoadSphereConfig(SPHERECONFIG* config, const char* filename);
extern bool SaveSphereConfig(SPHERECONFIG* config, const char* filename);


#endif
