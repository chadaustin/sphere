#include "FontGradientDialog.hpp"
#include "resource.h"


////////////////////////////////////////////////////////////////////////////////

CFontGradientDialog::CFontGradientDialog()
: CDialog(IDD_FONT_GRADIENT)
, m_top_color(CreateRGBA(255, 255, 255, 255))
, m_bottom_color(CreateRGBA(255, 255, 255, 255))
{
}

////////////////////////////////////////////////////////////////////////////////

RGBA
CFontGradientDialog::GetTopColor()
{
  return m_top_color;
}

////////////////////////////////////////////////////////////////////////////////

RGBA
CFontGradientDialog::GetBottomColor()
{
  return m_bottom_color;
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CFontGradientDialog::OnInitDialog()
{
  InitializeView(m_top_view,    m_top_color,    IDC_TOP_COLOR);
  InitializeView(m_bottom_view, m_bottom_color, IDC_BOTTOM_COLOR);

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void
CFontGradientDialog::InitializeView(CRGBAView& view, RGBA color, int control_id)
{
  RECT rect;
  GetDlgItem(control_id)->GetWindowRect(&rect);
  ScreenToClient(&rect);
  view.Create(
    this,
    rect.left,
    rect.top,
    rect.right - rect.left,
    rect.bottom - rect.top,
    color
  );
}

////////////////////////////////////////////////////////////////////////////////

void
CFontGradientDialog::OnOK()
{
  m_top_color    = m_top_view.GetColor();
  m_bottom_color = m_bottom_view.GetColor();

  CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////////
