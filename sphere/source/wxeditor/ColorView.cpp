//#include <afxdlgs.h>
#include <wx/dcclient.h>
#include "ColorView.hpp"
#include "resource.h"
/*
BEGIN_MESSAGE_MAP(CColorView, CWnd)

  ON_WM_PAINT()
  ON_WM_LBUTTONDOWN()

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wColorView, wxWindow)
  EVT_PAINT(wColorView::OnPaint)
  EVT_LEFT_DOWN(wColorView::OnLButtonDown)
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wColorView::wColorView(wxWindow *parent, IColorViewHandler* handler)
: m_Color(CreateRGB(0, 0, 0))
, m_ColorData(wxColourData())
{
  m_Handler = handler;
  Create(parent, -1);
}

/*
////////////////////////////////////////////////////////////////////////////////

wColorView::~wColorView()
{
  DestroyWindow();
}
*/
/*
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
*/
////////////////////////////////////////////////////////////////////////////////

void
wColorView::SetColor(RGB color)
{
  m_Color = color;
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

RGB
wColorView::GetColor() const
{
  return m_Color;
}

////////////////////////////////////////////////////////////////////////////////

void
wColorView::OnPaint(wxPaintEvent& event)
{
  int cx;
  int cy;
  this->GetSize(&cx, &cy);
  wxPaintDC dc(this);
  dc.SetBrush(wxBrush(wxColour(m_Color.red, m_Color.green, m_Color.blue), wxSOLID));
  dc.SetPen(wxPen(wxColour(m_Color.red, m_Color.green, m_Color.blue), 1, wxSOLID));
  dc.DrawRectangle(0, 0, cx, cy);

  dc.SetBrush(wxNullBrush);
  dc.SetPen(wxNullPen);
/*
  CPaintDC dc(this);
  CBrush* brush = new CBrush(RGB(m_Color.red, m_Color.green, m_Color.blue));

  // fill client rectangle with current color
  RECT ClientRect;
  GetClientRect(&ClientRect);
  dc.FillRect(&ClientRect, brush);

  brush->DeleteObject();
  delete brush;
*/
}

////////////////////////////////////////////////////////////////////////////////

void
wColorView::OnLButtonDown(wxMouseEvent& event)
{
  wxColourDialog dialog(this, &m_ColorData);
  m_ColorData.SetColour(wxColour(m_Color.red, m_Color.green, m_Color.blue));

  if (dialog.ShowModal() == wxID_OK)
  {
    wxColourData retData = dialog.GetColourData();
    wxColour col = retData.GetColour();
    m_Color.red = col.Red();
    m_Color.green = col.Green();
    m_Color.blue = col.Blue();
    this->Refresh();
    m_Handler->CV_ColorChanged(m_Color);
  }
  
/*
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
*/
}

////////////////////////////////////////////////////////////////////////////////
