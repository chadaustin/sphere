#include <stdio.h>
#include <audiere.h>
#include "Configuration.hpp"
#include "Keys.hpp"
#include "Audio.hpp"


static int s_InitCount = 0;
static ADR_CONTEXT s_Context;


////////////////////////////////////////////////////////////////////////////////

static void InitializeAudio()
{
  if (s_InitCount++ == 0) {
    ADR_CONTEXT_ATTR attr = AdrCreateContextAttr();
    AdrContextAttrSetOutputDevice(attr, "autodetect");
    
    s_Context = AdrCreateContext(attr);
    if (!s_Context) {
      AdrContextAttrSetOutputDevice(attr, "null");
      s_Context = AdrCreateContext(attr);
    }

    AdrDestroyContextAttr(attr);
  }
}

////////////////////////////////////////////////////////////////////////////////

static void CloseAudio()
{
  if (--s_InitCount == 0) {
    if (s_Context) {
      AdrDestroyContext(s_Context);
      s_Context = NULL;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

CSound::CSound()
: m_Sound(NULL)
{
  InitializeAudio();
}

////////////////////////////////////////////////////////////////////////////////

CSound::~CSound()
{
  if (m_Sound) {
    AdrCloseStream(m_Sound);
    m_Sound = NULL;
  }

  CloseAudio();
}

////////////////////////////////////////////////////////////////////////////////

bool
CSound::Load(const char* filename)
{
  if (m_Sound) {
    AdrCloseStream(m_Sound);
    m_Sound = NULL;
  }

  if (s_Context) {
    m_Sound = AdrOpenStream(s_Context, filename);
    return (m_Sound != NULL);
  } else {
    m_Sound = NULL;
    return NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CSound::Play()
{
  if (m_Sound) {
    AdrPlayStream(m_Sound);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CSound::Stop()
{
  if (m_Sound) {
    AdrPauseStream(m_Sound);
    AdrResetStream(m_Sound);
  }
}

////////////////////////////////////////////////////////////////////////////////

int
CSound::GetVolume()
{
  if (m_Sound) {
    return AdrGetStreamVolume(m_Sound);
  } else {
    return 0;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CSound::SetVolume(int Volume)
{
  if (m_Sound) {
    AdrSetStreamVolume(m_Sound, Volume);
  }
}

////////////////////////////////////////////////////////////////////////////////

bool
CSound::IsPlaying() const
{
  if (m_Sound) {
    return (AdrIsStreamPlaying(m_Sound) == ADR_TRUE);
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////
