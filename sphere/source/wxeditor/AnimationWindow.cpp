#include "AnimationWindow.hpp"
#include "DIBSection.hpp"
#include "../common/AnimationFactory.hpp"
//#include "resource.h"
#include "IDs.hpp"


//const int ANIMATION_TIMER = 9000;


/*
BEGIN_MESSAGE_MAP(CAnimationWindow, CDocumentWindow)

  ON_WM_DESTROY()
  ON_WM_PAINT()
  ON_WM_TIMER()

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wAnimationWindow, wDocumentWindow)
  EVT_PAINT(wAnimationWindow::OnPaint)
  EVT_TIMER(wID_ANIMATION_TIMER, wAnimationWindow::OnTimer)
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wAnimationWindow::wAnimationWindow(const char* animation)
: wDocumentWindow(animation, -1 /*IDR_ANIMATION*/)
, m_Animation(NULL)
, m_BlitFrame(NULL)
{
  m_Animation = LoadAnimation(animation);
  if (!m_Animation) {
    //AfxGetApp()->m_pMainWnd->MessageBox("Error: Could not load animation");
    ::wxMessageBox("Error: Could not load animation");
    delete this;
    return;
  }

  m_BlitFrame = new wDIBSection(m_Animation->GetWidth(), m_Animation->GetHeight());
  m_ImageFrame = new CImage32(m_Animation->GetWidth(), m_Animation->GetHeight());
  //m_Animation->ReadNextFrame((RGBA*)m_BlitFrame->GetPixels());

  RGBA *img = m_ImageFrame->GetPixels();
  RGB *blt = (RGB *)m_BlitFrame->GetPixels();
  int pixels = m_Animation->GetWidth() * m_Animation->GetHeight();
  int i;
  m_Animation->ReadNextFrame(img);
  for(i = 0; i < pixels; i++){
    blt[i].red = img[i].red;
    blt[i].green = img[i].green;
    blt[i].blue = img[i].blue;
  }

  //Create(AfxRegisterWndClass(0, NULL, NULL, AfxGetApp()->LoadIcon(IDI_ANIMATION)));

  m_Delay = m_Animation->GetDelay();
  m_Timer = new wxTimer(this, wID_ANIMATION_TIMER);
  //SetTimer(ANIMATION_TIMER, m_Delay, NULL);
  m_Timer->Start(m_Delay);

  return;
}

////////////////////////////////////////////////////////////////////////////////

wAnimationWindow::~wAnimationWindow()
{
  delete m_BlitFrame;
  //m_Animation->Destroy();
}

/*
////////////////////////////////////////////////////////////////////////////////

afx_msg void
CAnimationWindow::OnDestroy()
{
  KillTimer(m_Timer);
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wAnimationWindow::OnPaint(wxPaintEvent &event)
{
  wxPaintDC dc(this);

  // get client rectangle
  wxSize ClientSize = GetClientSize();

  int offsetx = (ClientSize.GetWidth()  - m_Animation->GetWidth())  / 2;
  int offsety = (ClientSize.GetHeight() - m_Animation->GetHeight()) / 2;
  
  // draw the frame
  //dc.BitBlt(offsetx, offsety, m_Animation->GetWidth(), m_Animation->GetHeight(),
  //          CDC::FromHandle(m_BlitFrame->GetDC()), 0, 0, SRCCOPY);
  dc.DrawBitmap(m_BlitFrame->GetImage()->ConvertToBitmap(), offsetx, offsety);

  

  //RECT Rect;  
  //CBrush* pBrush = CBrush::FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH));

  dc.SetBrush(*wxBLACK_BRUSH);
  dc.SetPen(*wxBLACK_PEN);
  // top
  dc.DrawRectangle(0, 0, ClientSize.GetWidth(), offsety);

  // bottom
  dc.DrawRectangle(0, offsety + m_Animation->GetHeight(), ClientSize.GetWidth(), ClientSize.GetHeight() - offsety - m_Animation->GetHeight());

  // left
  dc.DrawRectangle(0, offsety, offsetx, m_Animation->GetHeight());

  // right
  dc.DrawRectangle(offsetx + m_Animation->GetWidth(), offsety, ClientSize.GetWidth() - offsetx - m_Animation->GetWidth(), m_Animation->GetHeight());

  dc.SetBrush(wxNullBrush);
  dc.SetPen(wxNullPen);
}

////////////////////////////////////////////////////////////////////////////////

void
wAnimationWindow::OnTimer(wxTimerEvent &event)
{
  RGBA *img = m_ImageFrame->GetPixels();
  RGB *blt = (RGB *)m_BlitFrame->GetPixels();
  int pixels = m_Animation->GetWidth() * m_Animation->GetHeight();
  int i;
  m_Animation->ReadNextFrame(img);
  for(i = 0; i < pixels; i++){
    blt[i].red = img[i].red;
    blt[i].green = img[i].green;
    blt[i].blue = img[i].blue;
  }
  // if delay changed, update the animation timer
  int new_delay = m_Animation->GetDelay();
  if (m_Delay != new_delay) {
    m_Delay = new_delay;

    m_Timer->Start(m_Delay);
  }

  Refresh();
}

////////////////////////////////////////////////////////////////////////////////
