#include "WindowStylePropertiesDialog.hpp"
#include "resource.h"


////////////////////////////////////////////////////////////////////////////////

CWindowStylePropertiesDialog::CWindowStylePropertiesDialog(sWindowStyle* ws)
: CDialog(IDD_WINDOW_STYLE_PROPERTIES)
, m_WindowStyle(ws)
{
}

////////////////////////////////////////////////////////////////////////////////

CWindowStylePropertiesDialog::~CWindowStylePropertiesDialog()
{
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CWindowStylePropertiesDialog::OnInitDialog()
{
  // background mode
  switch (m_WindowStyle->GetBackgroundMode()) {
    case sWindowStyle::TILED:     CheckDlgButton(IDC_TILE_BACKGROUND,     BST_CHECKED); break;
    case sWindowStyle::STRETCHED: CheckDlgButton(IDC_STRETCH_BACKGROUND,  BST_CHECKED); break;
    case sWindowStyle::GRADIENT:  CheckDlgButton(IDC_GRADIENT_BACKGROUND, BST_CHECKED); break;
  }

  // background corner colors
  InitializeView(m_UpperLeftView,  sWindowStyle::BACKGROUND_UPPER_LEFT,  IDC_UPPER_LEFT);
  InitializeView(m_UpperRightView, sWindowStyle::BACKGROUND_UPPER_RIGHT, IDC_UPPER_RIGHT);
  InitializeView(m_LowerLeftView,  sWindowStyle::BACKGROUND_LOWER_LEFT,  IDC_LOWER_LEFT);
  InitializeView(m_LowerRightView, sWindowStyle::BACKGROUND_LOWER_RIGHT, IDC_LOWER_RIGHT);

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void
CWindowStylePropertiesDialog::InitializeView(CRGBAView& view, int corner, int control_id)
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
    m_WindowStyle->GetBackgroundColor(corner)
  );
}

////////////////////////////////////////////////////////////////////////////////

void
CWindowStylePropertiesDialog::OnOK()
{
  if (IsDlgButtonChecked(IDC_TILE_BACKGROUND) == BST_CHECKED) {
    m_WindowStyle->SetBackgroundMode(sWindowStyle::TILED);
  } else if (IsDlgButtonChecked(IDC_STRETCH_BACKGROUND) == BST_CHECKED) {
    m_WindowStyle->SetBackgroundMode(sWindowStyle::STRETCHED);
  } else if (IsDlgButtonChecked(IDC_GRADIENT_BACKGROUND) == BST_CHECKED) {
    m_WindowStyle->SetBackgroundMode(sWindowStyle::GRADIENT);
  }

  m_WindowStyle->SetBackgroundColor(sWindowStyle::BACKGROUND_UPPER_LEFT,  m_UpperLeftView.GetColor());
  m_WindowStyle->SetBackgroundColor(sWindowStyle::BACKGROUND_UPPER_RIGHT, m_UpperRightView.GetColor());
  m_WindowStyle->SetBackgroundColor(sWindowStyle::BACKGROUND_LOWER_LEFT,  m_LowerLeftView.GetColor());
  m_WindowStyle->SetBackgroundColor(sWindowStyle::BACKGROUND_LOWER_RIGHT, m_LowerRightView.GetColor());

  CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////////
