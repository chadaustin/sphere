#ifndef SOUND_WINDOW_HPP
#define SOUND_WINDOW_HPP


//#include <afxcmn.h>
#include <wx/wx.h>
#include "DocumentWindow.hpp"
#include "Audio.hpp"


class wSoundWindow : public wDocumentWindow
{
public:
  wSoundWindow(const char* sound);
  ~wSoundWindow();

private:
  void OnSize(wxSizeEvent &event);
  void OnTimer(wxTimerEvent &evnt);
  void OnVScroll(wxScrollEvent &event);

  void OnSoundPlay(wxCommandEvent &event);
  void OnSoundStop(wxCommandEvent &event);

private:
  CSound  m_Sound;
  wxTimer m_Timer;
  wxButton m_PlayButton;
  wxButton m_StopButton;

  wxSlider m_VolumeBar;
//  wxStatic m_VolumeBarGraphic;
//  wxBitmap m_VolumeBarBitmap;
//  wxStatic m_Blank;

private:
//  DECLARE_CLASS(wSoundWindow)
  DECLARE_EVENT_TABLE()
};


#endif
