#include <stdio.h>
#include <stdlib.h>
#include "sound.h"
#include "internal.h"


/*
static bool SoundReady = false;

#define NUM_EFFECT_CHANNELS 4
static int CurrentChannel = 0;
static DWORD EffectChannels[NUM_EFFECT_CHANNELS];
*/

////////////////////////////////////////////////////////////////////////////////

bool InitSoundSystem(void)
{
  return true;
/*
  int outputmode;
  int i;

  SoundReady = true;

  if (!Config.sound && !Config.music)
    { SoundReady = false; return true; }

  if (!MIDASstartup())
    { SoundReady = false; return true; }

  if (Config.sixteenbit)
    if (Config.stereo)
      outputmode = MIDAS_MODE_16BIT_STEREO;
    else
      outputmode = MIDAS_MODE_16BIT_MONO;
  else
    if (Config.stereo)
      outputmode = MIDAS_MODE_8BIT_STEREO;
    else
      outputmode = MIDAS_MODE_8BIT_MONO;

  // set configuration
  MIDASsetOption(MIDAS_OPTION_MIXRATE,      Config.mixrate);
  MIDASsetOption(MIDAS_OPTION_OUTPUTMODE,   outputmode);
  MIDASsetOption(MIDAS_OPTION_MIXBUFLEN,    Config.bufferlength);
//  MIDASsetOption(MIDAS_OPTION_MIXBUFBLOCKS, 4);
  if (Config.directsound)
  {
    MIDASsetOption(MIDAS_OPTION_DSOUND_MODE, MIDAS_DSOUND_PRIMARY);
    MIDASsetOption(MIDAS_OPTION_DSOUND_HWND, (DWORD)SphereWindow);
    MIDASsetOption(MIDAS_OPTION_DSOUND_BUFLEN, Config.bufferlength);
  }
  MIDASsetOption(MIDAS_OPTION_MIXING_MODE,
    (Config.highquality ? MIDAS_MIX_HIGH_QUALITY : MIDAS_MIX_NORMAL_QUALITY));

  if (!MIDASinit())
    { SoundReady = false; return true; }

  if (!MIDASstartBackgroundPlay(0))
    { MIDASclose(); SoundReady = false; return true; }

  CurrentChannel = 0;
  for (i = 0; i < NUM_EFFECT_CHANNELS; i++)
    EffectChannels[i] = MIDASallocateChannel();

  MIDASopenChannels(64);
*/
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool CloseSoundSystem(void)
{
/*
  int i;

  if (!SoundReady) return true;

  MIDAScloseChannels();

  for (i = 0; i < NUM_EFFECT_CHANNELS; i++)
    MIDASfreeChannel(EffectChannels[i]);

  MIDASstopBackgroundPlay();
  MIDASclose();
*/
  return true;
}

////////////////////////////////////////////////////////////////////////////////
/*
static int GetWaveSampleRate(const char* filename)
{
  dword rate;
  FILE* file = fopen(filename, "rb");
  if (file == NULL) return -1;
  fseek(file, 24, SEEK_SET);  // this is a hack
  fread(&rate, 1, 4, file);
  fclose(file);
  return rate;
}
*/
////////////////////////////////////////////////////////////////////////////////

SOUNDEFFECT* LoadSoundEffect(const char* filename)
{
  SOUNDEFFECT* se;

//  if (!Config.sound) return (SOUNDEFFECT*)1;  // just so it isn't NULL
//  if (!SoundReady) return (SOUNDEFFECT*)1;

  se = (SOUNDEFFECT*)malloc(sizeof(SOUNDEFFECT));
//  se->effect = MIDASloadWaveSample((char*)filename, MIDAS_LOOP_NO);
//  if ((void*)se->effect == NULL)
//    return NULL;
//  se->rate = GetWaveSampleRate(filename);

  return se;
}

////////////////////////////////////////////////////////////////////////////////

bool DestroySoundEffect(SOUNDEFFECT* se)
{
//  if (!Config.sound) return true;
//  if (!SoundReady) return true;

//  StopSoundEffect(se);
//  MIDASfreeSample(se->effect);
  free(se);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool PlaySoundEffect(SOUNDEFFECT* se)
{
//  int channel = ++CurrentChannel % NUM_EFFECT_CHANNELS;
//  if (!Config.sound) return true;
//  if (!SoundReady) return true;
//  se->handle = MIDASplaySample(se->effect, channel, 1, se->rate, 255, MIDAS_PAN_MIDDLE);
//  se->playing = true;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool StopSoundEffect(SOUNDEFFECT* se)
{
//  if (se->playing)
//  {
//    MIDASstopSample(se->handle);
//    se->playing = false;
//  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool LoadMusicModule(MUSICMODULE* mm, const char* filename)
{
/*
  if (!Config.music) return true;
  if (!SoundReady) return true;

  // If file is a MIDI
  if (strcmp(filename + strlen(filename) - 4, ".mid") == 0 ||
      strcmp(filename + strlen(filename) - 5, ".midi") == 0)
  {
    char temp[520];
    char ret[520];
    sprintf(temp, "open \"%s\" type sequencer alias midifile", filename);
    if (mciSendString(temp, ret, 512, NULL) != 0) // if it failed
      return false;
    mm->midi = true;
  }
  else
  {
    mm->module = MIDASloadModule((char*)filename);
    if (mm->module == NULL)
      return false;
    mm->midi = false;
  }

  mm->playing = false;
*/
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DestroyMusicModule(MUSICMODULE* mm)
{
/*
  if (!Config.music) return true;
  if (!SoundReady) return true;

  if (mm->midi)
  {
    char ret[512];
    mciSendString("close midifile", ret, 500, NULL);
    mm->midi = false;
    mm->playing = false;
  }
  else
  {
    StopMusicModule(mm);
    MIDASfreeModule(mm->module);
  }
*/
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool StartMusicModule(MUSICMODULE* mm)
{
/*
  if (!Config.music) return true;
  if (!SoundReady) return true;

  if (mm->midi)
  {
    char ret[512];
    mciSendString("play midifile notify", ret, 500, SphereWindow);
  }
  else
    mm->handle = MIDASplayModule(mm->module, TRUE);
  mm->playing = true;
*/
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool StopMusicModule(MUSICMODULE* mm)
{
/*
  if (!SoundReady) return true;

  if (mm->playing)
  {
    if (mm->midi)
    {
      char ret[512];
      mciSendString("pause midifile", ret, 500, NULL);
      mciSendString("seek midifile to start", ret, 500, NULL);
    }
    else
      MIDASstopModule(mm->handle);
    mm->playing = false;
  }
*/
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool SetMusicVolume(MUSICMODULE* mm, int volume)
{
/*
  if (mm)
    return (MIDASsetMusicVolume(mm->handle, volume) ? true : false);
  else
    return false;
*/
  return true;
}

////////////////////////////////////////////////////////////////////////////////
