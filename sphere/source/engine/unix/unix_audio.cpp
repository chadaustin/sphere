#include "unix_audio.h"


static audiere::AudioDevicePtr s_AudioDevice;


////////////////////////////////////////////////////////////////////////////////

bool InitAudio()
{
  s_AudioDevice = audiere::OpenDevice();
  if (!s_AudioDevice) {
    fprintf(stderr, "Using null device instead...\n");
    s_AudioDevice = audiere::OpenDevice("null");
  }
  return bool(s_AudioDevice);
}

////////////////////////////////////////////////////////////////////////////////

void CloseAudio()
{
  s_AudioDevice = 0;
}

////////////////////////////////////////////////////////////////////////////////

audiere::AudioDevice* SA_GetAudioDevice()
{
  return s_AudioDevice.get();
}

////////////////////////////////////////////////////////////////////////////////

audiere::OutputStream* SA_OpenSound(audiere::File* file, bool streaming)
{
  return audiere::OpenSound(s_AudioDevice.get(), file, streaming);
}

////////////////////////////////////////////////////////////////////////////////
