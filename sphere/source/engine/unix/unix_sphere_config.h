#ifndef UNIX_SPHERE_CONFIG_H
#define UNIX_SPHERE_CONFIG_H


#include <string>


enum {
  SOUND_AUTODETECT,
  SOUND_ON,
  SOUND_OFF,
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
};


extern bool LoadSphereConfig(SPHERECONFIG* config, const char* filename);
extern bool SaveSphereConfig(SPHERECONFIG* config, const char* filename);


#endif
