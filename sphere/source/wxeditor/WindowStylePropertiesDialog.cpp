#include "WindowStylePropertiesDialog.hpp"
//#include "resource.h"
//#include "IDs.hpp"

BEGIN_EVENT_TABLE(wWindowStylePropertiesDialog, wxDialog)
  EVT_BUTTON(wxID_OK, wWindowStylePropertiesDialog::OnOK)
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wWindowStylePropertiesDialog::wWindowStylePropertiesDialog(wxWindow *parent, sWindowStyle* ws)
: wxDialog()
, m_WindowStyle(ws)
{

  wxBoxSizer *mainsizer;
  wxBoxSizer *subsizer;
  wxGridSizer *gridsizer;

  mainsizer = new wxBoxSizer(wxVERTICAL);

    wxString modes[] = {"Tile", "Gradient"};
    mainsizer = new wxBoxSizer(wxHORIZONTAL);
    mainsizer->Add(m_BackgroundMode = new wxRadioBox(
          this, -1, "Background Modes", wxDefaultPosition, wxDefaultSize,
          2, modes));

    gridsizer = new wxGridSizer(2);
      subsizer = new wxBoxSizer(wxVERTICAL);
        subsizer->Add(new wxStaticText(this, -1, "Upper Left"));
        subsizer->Add(m_UpperLeftView = new wRGBAView(this, 0, 0, 64, 64, m_WindowStyle->GetBackgroundColor(sWindowStyle::BACKGROUND_UPPER_LEFT)));
      gridsizer->Add(subsizer);

      subsizer = new wxBoxSizer(wxVERTICAL);
        subsizer->Add(new wxStaticText(this, -1, "Upper Right"));
        subsizer->Add(m_UpperLeftView = new wRGBAView(this, 0, 0, 64, 64, m_WindowStyle->GetBackgroundColor(sWindowStyle::BACKGROUND_UPPER_RIGHT)));
      gridsizer->Add(subsizer);

      subsizer = new wxBoxSizer(wxVERTICAL);
        subsizer->Add(new wxStaticText(this, -1, "Lower Left"));
        subsizer->Add(m_UpperLeftView = new wRGBAView(this, 0, 0, 64, 64, m_WindowStyle->GetBackgroundColor(sWindowStyle::BACKGROUND_LOWER_LEFT)));
      gridsizer->Add(subsizer);

      subsizer = new wxBoxSizer(wxVERTICAL);
        subsizer->Add(new wxStaticText(this, -1, "Lower Right"));
        subsizer->Add(m_UpperLeftView = new wRGBAView(this, 0, 0, 64, 64, m_WindowStyle->GetBackgroundColor(sWindowStyle::BACKGROUND_LOWER_RIGHT)));
      gridsizer->Add(subsizer);
    mainsizer->Add(gridsizer);


    subsizer = new wxBoxSizer(wxHORIZONTAL);
      subsizer->Add(new wxButton(this, wxID_OK, "OK"));
      subsizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"));
    mainsizer->Add(subsizer);

  SetSizer(mainsizer);
  mainsizer->SetSizeHints(this);

  switch(m_WindowStyle->GetBackgroundMode()) {
  case sWindowStyle::TILED:     m_BackgroundMode->SetSelection(0); break;
  //case sWindowStyle::STRETCHED: m_BackgroundMode->SetSelection(?); break;
  case sWindowStyle::GRADIENT:  m_BackgroundMode->SetSelection(1); break;
  }

}

////////////////////////////////////////////////////////////////////////////////

wWindowStylePropertiesDialog::~wWindowStylePropertiesDialog()
{
}

/*
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
wWindowStylePropertiesDialog::InitializeView(wRGBAView& view, int corner, int control_id)
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
*/

////////////////////////////////////////////////////////////////////////////////

void
wWindowStylePropertiesDialog::OnOK(wxCommandEvent &event)
{
  switch(m_BackgroundMode->GetSelection()) {
  case 0: m_WindowStyle->SetBackgroundMode(sWindowStyle::TILED); break;
  //case ?: m_WindowStyle->SetBackgroundMode(sWindowStyle::STRETCHED); break;
  case 1: m_WindowStyle->SetBackgroundMode(sWindowStyle::GRADIENT); break;
  }

  m_WindowStyle->SetBackgroundColor(sWindowStyle::BACKGROUND_UPPER_LEFT,  m_UpperLeftView->GetColor());
  m_WindowStyle->SetBackgroundColor(sWindowStyle::BACKGROUND_UPPER_RIGHT, m_UpperRightView->GetColor());
  m_WindowStyle->SetBackgroundColor(sWindowStyle::BACKGROUND_LOWER_LEFT,  m_LowerLeftView->GetColor());
  m_WindowStyle->SetBackgroundColor(sWindowStyle::BACKGROUND_LOWER_RIGHT, m_LowerRightView->GetColor());

  wxDialog::OnOK(event);
}

////////////////////////////////////////////////////////////////////////////////
