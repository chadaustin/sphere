#ifndef WIN32_AUDIO_HPP
#define WIN32_AUDIO_HPP


#include <audiere.h>
#include <iostream>
#include "unix_sphere_config.h"

bool InitAudio(SPHERECONFIG* config);
void CloseAudio();

audiere::OutputStream* SA_OpenSound(audiere::File* file, bool streaming);

#endif
