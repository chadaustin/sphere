#ifndef MAC_AUDIO_HPP
#define MAC_AUDIO_HPP

#include <audiere.h>
#include <iostream>

#include "mac_sphere_config.h"

bool InitAudio(SPHERECONFIG* config);
void CloseAudio();

audiere::OutputStream* SA_OpenSound(audiere::File* file, bool streaming);
audiere::SoundEffect*  SA_OpenSoundEffect(audiere::File* file, audiere::SoundEffectType type);

#endif
