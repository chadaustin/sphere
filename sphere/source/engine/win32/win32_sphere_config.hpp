#ifndef WIN32_SPHERE_CONFIG_H
#define WIN32_SPHERE_CONFIG_H


#include <string>


struct SPHERECONFIG
{
  // video
  std::string videodriver;

  // audio
  std::string audiodriver;

  // input
  bool joystick;
};


extern bool LoadSphereConfig(SPHERECONFIG* config, const char* filename);
extern bool SaveSphereConfig(SPHERECONFIG* config, const char* filename);


#endif
