#include "Audio.hpp"
// #include <assert.h>

static int s_AudioInitCount = 0;
static int s_MidiInitCount = 0;
static audiere::AudioDevicePtr s_AudioDevice;
static audiere::MIDIDevicePtr s_MidiDevice;

////////////////////////////////////////////////////////////////////////////////

static void InitializeAudio()
{
  if (s_AudioInitCount++ == 0) {
    s_AudioDevice = audiere::OpenDevice();
    if (!s_AudioDevice) {
      s_AudioDevice = audiere::OpenDevice("null");
    }
  }
}

static void InitializeMidi()
{
  if (s_MidiInitCount++ == 0) {
    s_MidiDevice = audiere::OpenMIDIDevice("");
    if (s_MidiDevice == NULL) {
      s_MidiDevice = audiere::OpenMIDIDevice("null");
    }
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

bool
CSound::Load(const char* filename)
{
  if (m_Sound || m_Midi) {
    Stop();
  }

  ///////

  if (!s_AudioDevice) {
    InitializeAudio();
    m_ClosedAudio = false;
  }
  else {
    if (s_AudioDevice.get() && s_AudioDevice.get()->getName() && strcmp("null", s_AudioDevice.get()->getName()) == 0) {
      CloseAudio();
      m_ClosedAudio = true;
      InitializeAudio();
      m_ClosedAudio = false;
    }
  }

  if (!s_MidiDevice) {
    InitializeMidi();
    m_ClosedMidi = false;
  }
  else {
    if (s_MidiDevice.get() && s_MidiDevice.get()->getName() && strcmp("null", s_MidiDevice.get()->getName()) == 0) {
      CloseMidi();
      m_ClosedMidi = true;
      InitializeMidi();
      m_ClosedMidi = false;
    }
  }

  ///////

  m_Filename = filename;
  m_Sound = audiere::OpenSound(s_AudioDevice.get(), filename, true);

  if (!m_Sound) {
    if (s_MidiDevice.get())
      m_Midi = s_MidiDevice.get()->openStream(filename);
  }

  if (!m_Sound && !m_Midi) {
    Stop();
  }

  return bool(m_Sound || m_Midi);
}

////////////////////////////////////////////////////////////////////////////////

void
CSound::Play()
{
  ///////

  if (!s_AudioDevice) {
    InitializeAudio();
    m_ClosedAudio = false;
  }
  else {
    if (s_AudioDevice.get() && s_AudioDevice.get()->getName() && strcmp("null", s_AudioDevice.get()->getName()) == 0) {
      CloseAudio();
      m_ClosedAudio = true;
      InitializeAudio();
      m_ClosedAudio = false;
    }
  }

  if (!s_MidiDevice) {
    InitializeMidi();
    m_ClosedMidi = false;
  }
  else {
    if (s_MidiDevice.get() && s_MidiDevice.get()->getName() && strcmp("null", s_MidiDevice.get()->getName()) == 0) {
      CloseMidi();
      m_ClosedMidi = true;
      InitializeMidi();
      m_ClosedMidi = false;
    }
  }

  ///////

  if (!IsPlaying()) {
    if (!m_Sound && !m_Midi && s_AudioDevice.get()) {
      m_Sound = audiere::OpenSound(s_AudioDevice.get(), m_Filename.c_str(), true);
    }
    if (!m_Sound && !m_Midi && s_MidiDevice.get()) {
      m_Midi = s_MidiDevice.get()->openStream(m_Filename.c_str());
    }

    if (m_Sound)
      m_Sound->play();

    if (m_Midi)
      m_Midi->play();
  }
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
  if (m_Midi) return true;
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
