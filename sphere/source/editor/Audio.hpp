#ifndef AUDIO_HPP
#define AUDIO_HPP


#include <audiere.h>


bool InitializeAudio();
void CloseAudio();


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
  ADR_STREAM m_Sound;
};


#endif
