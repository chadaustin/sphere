#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include <windows.h>
#include <stdio.h>
#include <list>
#include "win32_audio.hpp"
#include "win32_internal.hpp"
#include "win32_sphere_config.hpp"


static audiere::AudioDevicePtr s_AudioDevice = NULL;
#ifdef WIN32
static audiere::MIDIDevicePtr s_MidiDevice = NULL;
#endif

////////////////////////////////////////////////////////////////////////////////

bool InitAudio(HWND window, SPHERECONFIG* config)
{
  switch (config->sound) {
    case SOUND_AUTODETECT:
      s_AudioDevice = audiere::OpenDevice("winmm");
      if (!s_AudioDevice) {
        s_AudioDevice = audiere::OpenDevice("null");
      }

#ifdef WIN32
      s_MidiDevice = audiere::OpenMIDIDevice("");
      if (!s_MidiDevice) {
        s_MidiDevice = audiere::OpenMIDIDevice("null");
      }

      return bool(s_AudioDevice && s_MidiDevice);
#else
      return bool(s_AudioDevice);
#endif

    case SOUND_ON:
      s_AudioDevice = audiere::OpenDevice("winmm");
#ifdef WIN32
      s_MidiDevice  = audiere::OpenMIDIDevice("");

      return bool(s_AudioDevice && s_MidiDevice);
#else
      return bool(s_AudioDevice);
#endif

    case SOUND_OFF:
      s_AudioDevice = audiere::OpenDevice("null");
#ifdef WIN32
      s_MidiDevice  = audiere::OpenMIDIDevice("null");

      return bool(s_AudioDevice && s_MidiDevice);
#else
      return bool(s_AudioDevice);
#endif

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

#ifdef WIN32
audiere::MIDIStream* SA_OpenMIDI(const char* filename)
{
  if (!s_MidiDevice.get())
    return NULL;
  return s_MidiDevice.get()->openStream(filename);
}

audiere::MIDIStream* SA_OpenMIDI(audiere::File* file)
{
  if (!s_MidiDevice.get())
    return NULL;
  return s_MidiDevice.get()->openStream(file);
}
#endif

////////////////////////////////////////////////////////////////////////////////
