#include "Audio.hpp"
// #include <assert.h>

static int s_InitCount = 0;
static audiere::AudioDevicePtr s_Device;


////////////////////////////////////////////////////////////////////////////////

static void InitializeAudio()
{
  if (s_InitCount++ == 0) {
    s_Device = audiere::OpenDevice();
    if (!s_Device) {
      s_Device = audiere::OpenDevice("null");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

static void CloseAudio()
{
  // assert(s_InitCount >= 1);
  if (--s_InitCount == 0) {
    s_Device = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////

CSound::CSound()
: m_Sound(NULL)
, m_ClosedAudio(false)
{
  InitializeAudio();
}

////////////////////////////////////////////////////////////////////////////////

CSound::~CSound()
{
  if (!m_ClosedAudio)
    CloseAudio();
}

////////////////////////////////////////////////////////////////////////////////

bool
CSound::Load(const char* filename)
{
  m_Filename = filename;
  m_Sound = audiere::OpenSound(s_Device.get(), filename, true);
  return bool(m_Sound);
}

////////////////////////////////////////////////////////////////////////////////

void
CSound::Play()
{
  if (!s_Device) {
    InitializeAudio();
    m_ClosedAudio = false;
  }
  else {
    if (strcmp("null", s_Device.get()->getName()) == 0) {
      CloseAudio();
      m_ClosedAudio = true;
      InitializeAudio();
      m_ClosedAudio = false;
    }
  }

  if (!IsPlaying()) {
    if (!m_Sound) {
      m_Sound = audiere::OpenSound(s_Device.get(), m_Filename.c_str(), true);
    }
    if (m_Sound)
      m_Sound->play();
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

  if (!m_ClosedAudio) {
    m_ClosedAudio = true;
    CloseAudio();
  }
}

////////////////////////////////////////////////////////////////////////////////

int
CSound::GetVolume()
{
  if (m_Sound) {
    return m_Sound->getVolume() * 255;
  } else {
    return 0;
  }
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
  if (m_Sound) {
    return m_Sound->isPlaying();
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////
