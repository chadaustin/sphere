#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include <windows.h>
#include <stdio.h>
#include <list>
#include "win32_audio.hpp"
#include "win32_internal.hpp"
#include "win32_sphere_config.hpp"


static audiere::AudioDevicePtr s_AudioDevice;


////////////////////////////////////////////////////////////////////////////////

bool InitAudio(HWND window, SPHERECONFIG* config)
{
  switch (config->sound) {
    case SOUND_AUTODETECT:
      s_AudioDevice = audiere::OpenDevice();
      if (!s_AudioDevice) {
        s_AudioDevice = audiere::OpenDevice("null");
      }
      return bool(s_AudioDevice);

    case SOUND_ON:
      s_AudioDevice = audiere::OpenDevice();
      return bool(s_AudioDevice);

    case SOUND_OFF:
      s_AudioDevice = audiere::OpenDevice("null");
      return bool(s_AudioDevice);

    default:
      return false;
  }
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
