#ifndef AUDIO_HPP
#define AUDIO_HPP


#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include <audiere.h>


class CSound
{
public:
  CSound();
  ~CSound();

  bool Load(const char* filename);
  void Play();
  void Stop();

  int  GetVolume();
  void SetVolume(int volume);

  bool IsPlaying() const;

  bool IsSeekable() {
    if (!m_Sound) return false;
    return m_Sound->isSeekable();
  }

  void SetPosition(int pos) {
    if (!m_Sound) return;
    m_Sound->setPosition(pos);
  }

  int GetPosition() {
    if (!m_Sound) return 0;
    return m_Sound->getPosition();
  }

  int GetLength() {
    if (!m_Sound) return 0;
    return m_Sound->getLength();
  }

  void SetPitchShift(double pitch) {
    if (!m_Sound) return;
    m_Sound->setPitchShift(pitch);
  }

  void SetPan(double pan) {
    if (!m_Sound) return;
    m_Sound->setPan(pan);
  }

private:
  audiere::OutputStreamPtr m_Sound;
  bool m_ClosedAudio;
  std::string m_Filename;
};


#endif
