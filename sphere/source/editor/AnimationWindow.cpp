#include "AnimationWindow.hpp"
#include "DIBSection.hpp"
#include "../common/AnimationFactory.hpp"
#include "resource.h"


const int ANIMATION_TIMER = 9000;


BEGIN_MESSAGE_MAP(CAnimationWindow, CDocumentWindow)

  ON_WM_DESTROY()
  ON_WM_PAINT()
  ON_WM_TIMER()

  ON_COMMAND(ID_ANIMATION_PLAY, OnPlay)
  ON_COMMAND(ID_ANIMATION_STOP, OnStop)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CAnimationWindow::CAnimationWindow(const char* animation)
: CDocumentWindow(animation, IDR_ANIMATION)
, m_BlitFrame(NULL)
, m_Animation(NULL)
{
  m_Animation = LoadAnimation(animation);
  if (!m_Animation) {
    AfxGetApp()->m_pMainWnd->MessageBox("Error: Could not load animation");
    delete this;
    return;
  }

  m_BlitFrame = new CDIBSection(m_Animation->GetWidth(), m_Animation->GetHeight(), 32);
  m_Animation->ReadNextFrame((BGRA*)m_BlitFrame->GetPixels());

  Create(AfxRegisterWndClass(0, NULL, NULL, AfxGetApp()->LoadIcon(IDI_ANIMATION)));

  //m_Delay = m_Animation->GetDelay();
  //m_Timer = SetTimer(ANIMATION_TIMER, m_Delay, NULL);

  return;
}

////////////////////////////////////////////////////////////////////////////////

CAnimationWindow::~CAnimationWindow()
{
  delete m_BlitFrame;
  delete m_Animation;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CAnimationWindow::OnDestroy()
{
  KillTimer(m_Timer);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CAnimationWindow::OnPaint()
{
  CPaintDC dc(this);

  // get client rectangle
  RECT ClientRect;
  GetClientRect(&ClientRect);

  int offsetx = (ClientRect.right  - m_Animation->GetWidth())  / 2;
  int offsety = (ClientRect.bottom - m_Animation->GetHeight()) / 2;
  
  // draw the frame
  dc.BitBlt(offsetx, offsety, m_Animation->GetWidth(), m_Animation->GetHeight(),
            CDC::FromHandle(m_BlitFrame->GetDC()), 0, 0, SRCCOPY);

  RECT Rect;  
  CBrush* pBrush = CBrush::FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH));

  // top
  SetRect(&Rect, 0, 0, ClientRect.right, offsety);
  dc.FillRect(&Rect, pBrush);

  // bottom
  SetRect(&Rect, 0, offsety + m_Animation->GetHeight(), ClientRect.right, ClientRect.bottom);
  dc.FillRect(&Rect, pBrush);

  // left
  SetRect(&Rect, 0, offsety, offsetx, offsety + m_Animation->GetHeight());
  dc.FillRect(&Rect, pBrush);

  // right
  SetRect(&Rect, offsetx + m_Animation->GetWidth(), offsety, ClientRect.right, offsety + m_Animation->GetHeight());
  dc.FillRect(&Rect, pBrush);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CAnimationWindow::OnTimer(UINT event)
{
  m_Animation->ReadNextFrame((BGRA*)m_BlitFrame->GetPixels());

  // if delay changed, update the animation timer
  int new_delay = m_Animation->GetDelay();
  if (m_Delay != new_delay) {
    m_Delay = new_delay;

    KillTimer(m_Timer);
    m_Timer = SetTimer(ANIMATION_TIMER, m_Delay, NULL);
  }

  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CAnimationWindow::OnPlay()
{
  m_Delay = m_Animation->GetDelay();
  KillTimer(m_Timer);
  m_Timer = SetTimer(ANIMATION_TIMER, m_Delay, NULL);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CAnimationWindow::OnStop()
{
  KillTimer(m_Timer);
}

////////////////////////////////////////////////////////////////////////////////