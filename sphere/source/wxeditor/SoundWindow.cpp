#include "SoundWindow.hpp"
//#include "resource.h"
#include "IDs.hpp"


const int TIMER_UPDATE_SOUND_WINDOW = 987;
const int ID_MUSIC_VOLUMEBAR = 40102;


/*
BEGIN_MESSAGE_MAP(CSoundWindow, CDocumentWindow)
  
  ON_WM_SIZE()
  ON_WM_TIMER()
  ON_WM_VSCROLL()

  ON_COMMAND(ID_SOUND_PLAY, OnSoundPlay)
  ON_COMMAND(ID_SOUND_STOP, OnSoundStop)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wSoundWindow, wDocumentWindow)
  
  EVT_SIZE(wSoundWindow::OnSize)
  EVT_TIMER(wID_SOUND_TIMER, wSoundWindow::OnTimer)
  EVT_SCROLL(wSoundWindow::OnVScroll)

  EVT_BUTTON(wID_SOUND_PLAY, wSoundWindow::OnSoundPlay)
  EVT_BUTTON(wID_SOUND_STOP, wSoundWindow::OnSoundStop)

END_EVENT_TABLE()


////////////////////////////////////////////////////////////////////////////////

wSoundWindow::wSoundWindow(const char* sound)
: wDocumentWindow(sound, -1 /*todo:IDR_SOUND*/, wxSize(200, 120))
{
  // load the sample
  if (!m_Sound.Load(sound))
  {
    // if it fails, show a message box and close the window
    wxMessageBox("Could not load sound file");
    delete this;
    return;
  }


  //char szWindowTitle[MAX_PATH];
  //strcpy(szWindowTitle, strrchr(sound, '\\') + 1);


  
  // create the buttons
  m_PlayButton.Create(this, wID_SOUND_PLAY, "Play", wxPoint(0, 0), wxSize(100, 60));
  m_StopButton.Create(this, wID_SOUND_STOP, "Stop", wxPoint(100, 0), wxSize(100, 60));

  // create the volume bar and its associated friends.
  m_VolumeBar.Create(this, -1, 0, 0, 255, wxPoint(0, 60), wxSize(200, 60), wxSL_HORIZONTAL | wxSL_AUTOTICKS);
  //m_VolumeBarBitmap.LoadBitmap(IDB_VOLUMEBAR);
  //m_VolumeBarGraphic.Create(NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SS_BITMAP, CRect(), this);
  //m_VolumeBarGraphic.SetBitmap((HBITMAP)m_VolumeBarBitmap);
  //m_Blank.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(), this);

/*todo:
  // make sure the buttons are in the right position
  RECT Rect;
  GetClientRect(&Rect);
  OnSize(0, Rect.right - Rect.left, Rect.bottom - Rect.top);
*/
  m_Timer.SetOwner(this, wID_SOUND_TIMER);
  m_Timer.Start(100);
    
}

////////////////////////////////////////////////////////////////////////////////

wSoundWindow::~wSoundWindow()
{
  //m_VolumeBarBitmap.DeleteObject();
}

////////////////////////////////////////////////////////////////////////////////

void
wSoundWindow::OnSize(wxSizeEvent &event)
{
  int cx = GetClientSize().GetWidth();
  int cy = GetClientSize().GetHeight();
  m_PlayButton.SetSize(0, 0, cx / 2, cy - 32);
  m_StopButton.SetSize(cx / 2, 0, cx / 2, cy - 32);
  m_VolumeBar.SetSize(0, cy - 32, cx , 32);
/*
  if (m_PlayButton.m_hWnd != NULL)
    m_PlayButton.MoveWindow(CRect(0, 0, (cx-50) / 2, cy));

  if (m_StopButton.m_hWnd != NULL)
    m_StopButton.MoveWindow(CRect((cx-50) / 2, 0, cx-50, cy));
  
  if (m_VolumeBar.m_hWnd != NULL)
    m_VolumeBar.MoveWindow(CRect(cx-50, 0, cx-30, 90));
  if (m_VolumeBarGraphic.m_hWnd != NULL)
    m_VolumeBarGraphic.MoveWindow(CRect(cx-30, 0, cx, 90));
  if (m_Blank.m_hWnd != NULL)
    m_Blank.MoveWindow(CRect(cx-50, 90, cx, cy));
*/
  wDocumentWindow::OnSize(event);
}

////////////////////////////////////////////////////////////////////////////////

void
wSoundWindow::OnTimer(wxTimerEvent &event)
{
  if (m_Sound.IsPlaying())
  {
    m_PlayButton.Enable(FALSE);
    m_StopButton.Enable(TRUE);
  }
  else
  {
    m_PlayButton.Enable(TRUE);
    m_StopButton.Enable(FALSE);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wSoundWindow::OnVScroll(wxScrollEvent &event)
{
  //if (!m_VolumeBar.MouseDown)
    m_Sound.SetVolume(255 - m_VolumeBar.GetValue());
}

////////////////////////////////////////////////////////////////////////////////

void
wSoundWindow::OnSoundPlay(wxCommandEvent &event)
{
  // ignore this if the user managed to hit the play button while it was playing
  if (m_Sound.IsPlaying())
    return;

  m_Sound.Play();
}

////////////////////////////////////////////////////////////////////////////////

void
wSoundWindow::OnSoundStop(wxCommandEvent &event)
{
  m_Sound.Stop();
}

////////////////////////////////////////////////////////////////////////////////
