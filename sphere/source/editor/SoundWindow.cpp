#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include "SoundWindow.hpp"
#include "resource.h"


const int TIMER_UPDATE_SOUND_WINDOW = 987;
const int ID_MUSIC_VOLUMEBAR = 40102;


BEGIN_MESSAGE_MAP(CSoundWindow, CDocumentWindow)
  
  ON_WM_SIZE()
  ON_WM_TIMER()
  ON_WM_VSCROLL()

  ON_COMMAND(ID_SOUND_PLAY, OnSoundPlay)
  ON_COMMAND(ID_SOUND_STOP, OnSoundStop)
  ON_COMMAND(ID_SOUND_REPEAT, OnSoundRepeat)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CSoundWindow::CSoundWindow(const char* sound)
: CDocumentWindow(sound, IDR_SOUND, CSize(200, 120))
{
  m_Playing = false;
  m_Repeat = false;
  
  // load the sample
  if (!m_Sound.Load(sound))
  {
    // if it fails, show a message box and close the window
    MessageBox("Could not load sound file");
    delete this;
    return;
  }


  char szWindowTitle[MAX_PATH];
  strcpy(szWindowTitle, strrchr(sound, '\\') + 1);

  Create(AfxRegisterWndClass(0, NULL, NULL, AfxGetApp()->LoadIcon(IDI_SOUND)));


  CFont* pFont = CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT));
  
  // create the buttons
  m_PlayButton.Create("Play", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(), this, ID_SOUND_PLAY);
  m_PlayButton.SetFont(pFont);
  m_StopButton.Create("Stop", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(), this, ID_SOUND_STOP);
  m_StopButton.SetFont(pFont);

  // create the volume bar and its associated friends.
  m_VolumeBar.Create(WS_CHILD | WS_VISIBLE | TBS_VERT, CRect(), this, ID_MUSIC_VOLUMEBAR);
  m_VolumeBar.SetLineSize(20);
  m_VolumeBar.SetRange(0, 255, true);
  m_VolumeBar.SetPos(0);
  m_VolumeBarBitmap.LoadBitmap(IDB_VOLUMEBAR);
  m_VolumeBarGraphic.Create(NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SS_BITMAP, CRect(), this);
  m_VolumeBarGraphic.SetBitmap((HBITMAP)m_VolumeBarBitmap);
  m_Blank.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(), this);


  // make sure the buttons are in the right position
  RECT Rect;
  GetClientRect(&Rect);
  OnSize(0, Rect.right - Rect.left, Rect.bottom - Rect.top);

  SetTimer(TIMER_UPDATE_SOUND_WINDOW, 100, NULL);
  OnTimer(TIMER_UPDATE_SOUND_WINDOW);
}

////////////////////////////////////////////////////////////////////////////////

CSoundWindow::~CSoundWindow()
{
  m_VolumeBarBitmap.DeleteObject();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSoundWindow::OnSize(UINT type, int cx, int cy)
{
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

  CDocumentWindow::OnSize(type, cx, cy);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSoundWindow::OnTimer(UINT timerID)
{
  if (m_Sound.IsPlaying())
  {
    m_PlayButton.EnableWindow(FALSE);
    m_StopButton.EnableWindow(TRUE);
  }
  else
  {
    if (m_Playing && m_Repeat) {
      m_Sound.Play();
    }
    else {
      m_PlayButton.EnableWindow(TRUE);
      m_StopButton.EnableWindow(FALSE);
      m_Playing = false;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSoundWindow::OnVScroll(UINT code, UINT pos, CScrollBar *scroll_bar)
{
  //if (!m_VolumeBar.MouseDown)
    m_Sound.SetVolume(255 - m_VolumeBar.GetPos());
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSoundWindow::OnSoundPlay()
{
  // ignore this if the user managed to hit the play button while it was playing
  if (m_Sound.IsPlaying())
    return;

  m_Sound.Play();
  m_Playing = true;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSoundWindow::OnSoundStop()
{
  m_Sound.Stop();
  m_Playing = false;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSoundWindow::OnSoundRepeat()
{
  m_Repeat = !m_Repeat;
}

////////////////////////////////////////////////////////////////////////////////
