#include "RGBAView.hpp"


////////////////////////////////////////////////////////////////////////////////

wRGBAView::wRGBAView(wxWindow *parent, int x, int y, int w, int h, RGBA color)
: wxWindow(parent, -1, wxPoint(x, y), wxSize(w, h))
{
  m_Color = color;

  m_ColorView = new wColorView(this, this);
  m_AlphaView = new wAlphaView(this, this);

  m_ColorView->SetSize(0, 0, w - 24, h);
  m_AlphaView->SetSize(w - 24, 0, 24, h);

  SetColor(color);
}

////////////////////////////////////////////////////////////////////////////////

wRGBAView::~wRGBAView()
{
}

/*
////////////////////////////////////////////////////////////////////////////////

BOOL
CRGBAView::Create(CWnd* parent, int x, int y, int w, int h, RGBA color)
{
  m_Color = color;

  m_ColorView.Create(this, parent, CreateRGB(m_Color.red, m_Color.green, m_Color.blue));
  m_AlphaView.Create(this, parent);
  m_AlphaView.SetAlpha(m_Color.alpha);

  m_ColorView.MoveWindow(x, y, w - 32, h);
  m_AlphaView.MoveWindow(x + w - 32, y, 32, h);

  return TRUE;
}
*/

////////////////////////////////////////////////////////////////////////////////

RGBA
wRGBAView::GetColor()
{
  return m_Color;
}

////////////////////////////////////////////////////////////////////////////////

void
wRGBAView::SetColor(RGBA color)
{
  m_Color = color;
  m_ColorView->SetColor(CreateRGB(m_Color.red, m_Color.green, m_Color.blue));
  m_AlphaView->SetAlpha(m_Color.alpha);
}

////////////////////////////////////////////////////////////////////////////////

void
wRGBAView::CV_ColorChanged(RGB color)
{
  m_Color.red   = color.red;
  m_Color.green = color.green;
  m_Color.blue  = color.blue;
}

////////////////////////////////////////////////////////////////////////////////

void
wRGBAView::AV_AlphaChanged(byte alpha)
{
  m_Color.alpha = alpha;
}

////////////////////////////////////////////////////////////////////////////////
