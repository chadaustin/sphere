#include <afxdlgs.h>
#include "ColorView.hpp"
#include "resource.h"


BEGIN_MESSAGE_MAP(CColorView, CWnd)

  ON_WM_PAINT()
  ON_WM_LBUTTONDOWN()

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CColorView::CColorView()
: m_Color(CreateRGB(0, 0, 0))
{
}

////////////////////////////////////////////////////////////////////////////////

CColorView::~CColorView()
{
  DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CColorView::Create(IColorViewHandler* handler, CWnd* parent, RGB color)
{
  m_Handler = handler;
  m_Color = color;

  return CWnd::Create(
    AfxRegisterWndClass(0, LoadCursor(NULL, IDC_ARROW), NULL, NULL),
    "CColorView",
    WS_CHILD | WS_VISIBLE,
    CRect(0, 0, 0, 0),
    parent,
    0);
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CColorView::Create(IColorViewHandler* handler, CWnd* parent)
{
  RGB color = CreateRGB(255, 255, 255);
  return Create(handler, parent, color);
}

////////////////////////////////////////////////////////////////////////////////

void
CColorView::SetColor(RGB color)
{
  m_Color = color;
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

RGB
CColorView::GetColor() const
{
  return m_Color;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CColorView::OnPaint()
{
  CPaintDC dc(this);
  CBrush* brush = new CBrush(RGB(m_Color.red, m_Color.green, m_Color.blue));
  if (brush) {
    // fill client rectangle with current color
    RECT ClientRect;
    GetClientRect(&ClientRect);
    dc.FillRect(&ClientRect, brush);

    brush->DeleteObject();
    delete brush;
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CColorView::OnLButtonDown(UINT flags, CPoint point)
{
  CColorDialog ColorDialog(
    RGB(m_Color.red, m_Color.green, m_Color.blue),
    CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT);
  if (ColorDialog.DoModal() == IDOK)
  {
    m_Color.red   = GetRValue(ColorDialog.GetColor());
    m_Color.green = GetGValue(ColorDialog.GetColor());
    m_Color.blue  = GetBValue(ColorDialog.GetColor());
    Invalidate();

    m_Handler->CV_ColorChanged(m_Color);
  }
}

////////////////////////////////////////////////////////////////////////////////
