#ifndef SOUND_WINDOW_HPP
#define SOUND_WINDOW_HPP


#include <afxcmn.h>
#include "DocumentWindow.hpp"
#include "Audio.hpp"


class CSoundWindow : public CDocumentWindow
{
public:
  CSoundWindow(const char* sound);
  ~CSoundWindow();

private:
  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnTimer(UINT timerID);
  afx_msg void OnVScroll(UINT code, UINT pos, CScrollBar *scroll_bar);

  afx_msg void OnSoundPlay();
  afx_msg void OnSoundStop();
  afx_msg void OnSoundRepeat();

private:
  CSound  m_Sound;
  CButton m_PlayButton;
  CButton m_StopButton;

  CSliderCtrl m_VolumeBar;
  CStatic m_VolumeBarGraphic;
  CBitmap m_VolumeBarBitmap;
  CStatic m_Blank;

  bool m_Repeat; 
  bool m_Playing;

  DECLARE_MESSAGE_MAP()
};


#endif
