#include "RGBAView.hpp"


////////////////////////////////////////////////////////////////////////////////

CRGBAView::CRGBAView()
{
}

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

////////////////////////////////////////////////////////////////////////////////

RGBA
CRGBAView::GetColor()
{
  return m_Color;
}

////////////////////////////////////////////////////////////////////////////////

void
CRGBAView::CV_ColorChanged(RGB color)
{
  m_Color.red   = color.red;
  m_Color.green = color.green;
  m_Color.blue  = color.blue;
}

////////////////////////////////////////////////////////////////////////////////

void
CRGBAView::AV_AlphaChanged(byte alpha)
{
  m_Color.alpha = alpha;
}

////////////////////////////////////////////////////////////////////////////////
