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
  afx_msg void OnHScroll(UINT code, UINT pos, CScrollBar *scroll_bar);

  afx_msg void OnSoundPlay();
  afx_msg void OnSoundStop();
  afx_msg void OnSoundRepeat();

  afx_msg void OnUpdatePlayCommand(CCmdUI* cmdui);
  afx_msg void OnUpdateStopCommand(CCmdUI* cmdui);
  afx_msg void OnUpdateRepeatCommand(CCmdUI* cmdui);

private:
  CSound  m_Sound;
  CButton m_PlayButton;
  CButton m_StopButton;

  CSliderCtrl m_VolumeBar;
  CSliderCtrl m_PositionBar;
  bool m_PositionDown;

  CSliderCtrl m_PanBar;
  CSliderCtrl m_PitchBar;

  CStatic m_VolumeBarGraphic;
  CBitmap m_VolumeBarBitmap;
  CStatic m_Blank;

  bool m_Repeat; 
  bool m_Playing;

  DECLARE_MESSAGE_MAP()
};


#endif
