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

private:
  audiere::OutputStreamPtr m_Sound;
};


#endif
