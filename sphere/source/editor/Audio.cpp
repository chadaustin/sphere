#include "Audio.hpp"
// #include <assert.h>

static int s_AudioInitCount = 0;
static int s_MidiInitCount  = 0;
static audiere::AudioDevicePtr s_AudioDevice = NULL;
static audiere::MIDIDevicePtr  s_MidiDevice  = NULL;

////////////////////////////////////////////////////////////////////////////////

static bool IsMidi(const char* filename)
{
  struct Local {
    static inline bool extension_compare(const char* path, const char* extension) {
      int path_length = strlen(path);
      int ext_length  = strlen(extension);
      return (
        path_length >= ext_length &&
        strcmp(path + path_length - ext_length, extension) == 0
      );
    }
  };

  if (Local::extension_compare(filename, ".mid"))  return true;
  if (Local::extension_compare(filename, ".midi")) return true;
  if (Local::extension_compare(filename, ".rmi"))  return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////

static void InitializeAudio()
{
  if (s_AudioInitCount++ == 0) {
    s_AudioDevice = audiere::OpenDevice("winmm");
    if (!s_AudioDevice) {
      s_AudioDevice = audiere::OpenDevice("null");
    }
  }

  if (s_AudioDevice && s_AudioDevice.get()) {
    const char* device_name = s_AudioDevice.get()->getName();
  }
}

////////////////////////////////////////////////////////////////////////////////

static void InitializeMidi()
{
  if (s_MidiInitCount++ == 0) {
    s_MidiDevice = audiere::OpenMIDIDevice("");
    if (s_MidiDevice == NULL) {
      s_MidiDevice = audiere::OpenMIDIDevice("null");
    }
  }

  if (s_MidiDevice && s_MidiDevice.get()) {
    const char* device_name = s_MidiDevice.get()->getName();
  }
}

////////////////////////////////////////////////////////////////////////////////

static void CloseAudio()
{
  if (--s_AudioInitCount == 0) {
    s_AudioDevice = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////

static void CloseMidi()
{
  if (--s_MidiInitCount == 0) {
    s_MidiDevice = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////

CSound::CSound()
: m_Sound(NULL)
, m_Midi(NULL)
, m_ClosedAudio(false)
, m_ClosedMidi(false)
{
  InitializeAudio();
  InitializeMidi();
}

////////////////////////////////////////////////////////////////////////////////

CSound::~CSound()
{
  if (!m_ClosedAudio)
    CloseAudio();
  if (!m_ClosedMidi)
    CloseMidi();
}

////////////////////////////////////////////////////////////////////////////////

void
CSound::__GetDevice__()
{
  if (!s_MidiDevice) {
    InitializeMidi();
    m_ClosedMidi = false;
  }
  else {
    if (s_MidiDevice.get() && s_MidiDevice.get()->getName() != NULL) {
      const char* device_name = s_MidiDevice.get()->getName();
      if (strcmp("null", device_name) == 0) {
        CloseMidi();
        m_ClosedMidi = true;
        InitializeMidi();
        m_ClosedMidi = false;
      }
    }
  }

  if (!s_AudioDevice) {
    InitializeAudio();
    m_ClosedAudio = false;
  }
  else {
    if (s_AudioDevice.get() && s_AudioDevice.get()->getName() != NULL) {
      const char* device_name = s_AudioDevice.get()->getName();
      if (strcmp("null", device_name) == 0) {
        CloseAudio();
        m_ClosedAudio = true;
        InitializeAudio();
        m_ClosedAudio = false;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CSound::__GetSound__(const char* filename)
{
  if (!m_Sound && !m_Midi && s_AudioDevice.get()) {
    m_Sound = audiere::OpenSound(s_AudioDevice.get(), filename, true);
  }

  if (!m_Sound && !m_Midi && s_MidiDevice.get() && IsMidi(filename)) {
    /*
    audiere::File* file = audiere::OpenFile(filename, false);
    if (file) {
      m_Midi = s_MidiDevice.get()->openStream(file);
      file = NULL;
    }
    */

    m_Midi = s_MidiDevice.get()->openStream(filename);
  }
}

////////////////////////////////////////////////////////////////////////////////


bool
CSound::Load(const char* filename)
{
  if (m_Sound || m_Midi) {
    Stop();
  }

  __GetDevice__();
  __GetSound__(filename);

  m_Filename = filename;

  if (!m_Sound && !m_Midi) {
    Stop();
  }

  return bool(m_Sound || m_Midi);
}

////////////////////////////////////////////////////////////////////////////////

bool
CSound::Play()
{
  if (!IsPlaying()) {
    __GetDevice__();
    __GetSound__(m_Filename.c_str());

    if (m_Sound)
      m_Sound->play();

    if (m_Midi)
      m_Midi->play();
  }

  return (m_Sound || m_Midi);
}

////////////////////////////////////////////////////////////////////////////////

void
CSound::Stop()
{
  if (m_Sound) {
    m_Sound->stop();
    m_Sound->reset();
    m_Sound = 0;
  }

  if (m_Midi) {
    m_Midi->stop();
    m_Midi = 0;
  }

  if (!m_ClosedAudio) {
    m_ClosedAudio = true;
    CloseAudio();
  }

  if (!m_ClosedMidi) {
    m_ClosedMidi = true;
    CloseMidi();
  }
}

////////////////////////////////////////////////////////////////////////////////

int
CSound::GetVolume()
{
  if (m_Sound) return m_Sound->getVolume() * 255;
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

void
CSound::SetVolume(int Volume)
{
  if (m_Sound) {
    m_Sound->setVolume(Volume / 255.0f);
  }
}

////////////////////////////////////////////////////////////////////////////////

bool
CSound::IsPlaying() const
{
  if (m_Sound) return m_Sound->isPlaying();
  if (m_Midi)  return m_Midi->isPlaying();
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool
CSound::IsSeekable()  {
  if (m_Sound) return m_Sound->isSeekable();
  if (m_Midi)  return true;
  return false;
}

void
CSound::SetPosition(int pos) {
  if (m_Sound) m_Sound->setPosition(pos);
  if (m_Midi)  m_Midi->setPosition(pos);
}

int
CSound::GetPosition() {
  if (m_Sound) return m_Sound->getPosition();
  if (m_Midi)  return m_Midi->getPosition();
  return 0;
}

int
CSound::GetLength() {
  if (m_Sound) return m_Sound->getLength();
  if (m_Midi)  return m_Midi->getLength();
  return 0;
}

void
CSound::SetPitchShift(double pitch) {
  if (m_Sound) m_Sound->setPitchShift(pitch);
}

void
CSound::SetPan(double pan) {
  if (m_Sound) m_Sound->setPan(pan);
}

////////////////////////////////////////////////////////////////////////////////
