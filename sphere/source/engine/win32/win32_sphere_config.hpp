#ifndef WIN32_SPHERE_CONFIG_H
#define WIN32_SPHERE_CONFIG_H


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

  // audio
  int sound;
};


extern bool LoadSphereConfig(SPHERECONFIG* config, const char* filename);
extern bool SaveSphereConfig(SPHERECONFIG* config, const char* filename);


#endif
