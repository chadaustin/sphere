#ifndef __SOUND_H
#define __SOUND_H


#include "types.h"


#ifndef SEFFECT
#define SEFFECT void*
#endif

#ifndef SEHANDLE
#define SEHANDLE void*
#endif

#ifndef SMODULE
#define SMODULE void*
#endif

#ifndef SMHANDLE
#define SMHANDLE void*
#endif

#ifndef DWORD
#define DWORD unsigned long
#endif

typedef struct
{
  SEFFECT effect;
  SEHANDLE handle;
  DWORD channel;
  dword rate;
  bool playing;
} SOUNDEFFECT;

typedef struct
{
  SMODULE  module;
  SMHANDLE handle;
  bool midi;       // whether or not file is a MIDI
  bool playing;
} MUSICMODULE;


#ifdef __cplusplus
extern "C" {
#endif


extern SOUNDEFFECT* LoadSoundEffect(const char* filename);
extern bool         DestroySoundEffect(SOUNDEFFECT* se);
extern bool         PlaySoundEffect(SOUNDEFFECT* se);
extern bool         StopSoundEffect(SOUNDEFFECT* se);

extern bool LoadMusicModule(MUSICMODULE* mm, const char* filename);
extern bool DestroyMusicModule(MUSICMODULE* mm);
extern bool StartMusicModule(MUSICMODULE* mm);
extern bool StopMusicModule(MUSICMODULE* mm);

extern bool SetMusicVolume(MUSICMODULE* mm, int volume);


#ifdef __cplusplus
}
#endif


#endif
