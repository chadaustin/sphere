#ifndef SOUND_WINDOW_HPP
#define SOUND_WINDOW_HPP


#include <afxcmn.h>
#include "DocumentWindow.hpp"
#include "Audio.hpp"
#include "../common/DefaultFileSystem.hpp" 


class CPlaylist {
public:
  CPlaylist();
  ~CPlaylist();

private:
  std::vector<std::string> m_Filenames;

public:
  const char* GetFile(int index) const;
  int GetNumFiles() const;

  bool AppendFile(const char* filename);
  bool LoadFromFile(const char* filename, IFileSystem& fs = g_DefaultFileSystem);
};


class CSoundWindow : public CDocumentWindow
{
public:
  CSoundWindow(const char* sound);
  ~CSoundWindow();

private:
  void LoadSound(const char* sound);

private:
  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnTimer(UINT timerID);
  afx_msg void OnVScroll(UINT code, UINT pos, CScrollBar *scroll_bar);
  afx_msg void OnHScroll(UINT code, UINT pos, CScrollBar *scroll_bar);

  afx_msg void OnSoundPlay();
  afx_msg void OnSoundPause();
  afx_msg void OnSoundStop();
  afx_msg void OnSoundRepeat();

  afx_msg void OnNextSound();
  afx_msg void OnPrevSound();

  afx_msg void OnUpdatePlayCommand(CCmdUI* cmdui);
  afx_msg void OnUpdatePauseCommand(CCmdUI* cmdui);
  afx_msg void OnUpdateStopCommand(CCmdUI* cmdui);
  afx_msg void OnUpdateRepeatCommand(CCmdUI* cmdui);

  afx_msg BOOL OnNeedText(UINT id, NMHDR* nmhdr, LRESULT* result);

  afx_msg void OnDropFiles(HDROP drop_info);

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
  
  CPlaylist m_Playlist;
  int m_CurrentSound;

  DECLARE_MESSAGE_MAP()
};


#endif
