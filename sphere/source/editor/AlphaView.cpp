#include "AlphaView.hpp"


static int s_AlphaViewID = 700;


BEGIN_MESSAGE_MAP(CAlphaView, CWnd)

  ON_WM_SIZE()
  ON_WM_VSCROLL()

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CAlphaView::CAlphaView()
: m_Created(false)
, m_Alpha(0)
{
}

////////////////////////////////////////////////////////////////////////////////

CAlphaView::~CAlphaView()
{
  m_AlphaSlider.DestroyWindow();
  m_AlphaStatic.DestroyWindow();
  DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CAlphaView::Create(IAlphaViewHandler* handler, CWnd* parent)
{
  m_Handler = handler;

  BOOL result = CWnd::Create(
    AfxRegisterWndClass(0, LoadCursor(NULL, IDC_ARROW), NULL, NULL),
    "",
    WS_CHILD | WS_VISIBLE,
    CRect(0, 0, 0, 0),
    parent,
    s_AlphaViewID++);

  m_AlphaSlider.Create(WS_CHILD | WS_VISIBLE | TBS_VERT, CRect(0, 0, 0, 0), this, 800);
  m_AlphaSlider.SetRange(0, 255);
  m_AlphaStatic.Create("", WS_CHILD | WS_VISIBLE | SS_CENTER, CRect(0, 0, 0, 0), this);

  m_Created = true;

  // make sure everything is in the correct place
  CRect rect;
  GetClientRect(rect);
  OnSize(0, rect.right, rect.bottom);

  UpdateSlider();

  return result;
}

////////////////////////////////////////////////////////////////////////////////

void
CAlphaView::SetAlpha(byte alpha)
{
  m_Alpha = 255 - alpha;
  UpdateSlider();
}

////////////////////////////////////////////////////////////////////////////////

byte
CAlphaView::GetAlpha() const
{
  return 255 - m_Alpha;
}

////////////////////////////////////////////////////////////////////////////////

void
CAlphaView::UpdateSlider()
{
  m_AlphaSlider.SetPos(m_Alpha);

  char str[80];
  sprintf(str, "%d", 255 - m_Alpha);
  m_AlphaStatic.SetWindowText(str);
}

////////////////////////////////////////////////////////////////////////////////

void
CAlphaView::OnSize(UINT type, int cx, int cy)
{
  const int STATIC_HEIGHT = 16;

  if (m_Created)
  {
    m_AlphaSlider.MoveWindow(0, 0, cx, cy - STATIC_HEIGHT);
    m_AlphaSlider.Invalidate();
    m_AlphaStatic.MoveWindow(0, cy - STATIC_HEIGHT, cx, STATIC_HEIGHT);
    m_AlphaStatic.Invalidate();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CAlphaView::OnVScroll(UINT code, UINT pos, CScrollBar* scrollbar)
{
  switch (code)
  {
    case SB_TOP:           m_Alpha = 0;   break;
    case SB_BOTTOM:        m_Alpha = 255; break;
    case SB_LINEDOWN:      m_Alpha++;     break;
    case SB_LINEUP:        m_Alpha--;     break;
    case SB_PAGEDOWN:      m_Alpha += 16; break;
    case SB_PAGEUP:        m_Alpha -= 16; break;
    case SB_THUMBPOSITION: m_Alpha = pos; break;
    case SB_THUMBTRACK:    m_Alpha = pos; break;
  }

  if (m_Alpha < 0)
    m_Alpha = 0;
  else if (m_Alpha > 255)
    m_Alpha = 255;

  UpdateSlider();
  m_Handler->AV_AlphaChanged(255 - m_Alpha);
}

////////////////////////////////////////////////////////////////////////////////
