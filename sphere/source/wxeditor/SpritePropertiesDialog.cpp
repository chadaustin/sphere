#include "SpritePropertiesDialog.hpp"
#include "../common/Spriteset.hpp"
//#include "resource.h"

BEGIN_EVENT_TABLE(wSpritePropertiesDialog, wxDialog)
  EVT_BUTTON(wxID_OK, OnOK)
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wSpritePropertiesDialog::wSpritePropertiesDialog(wxWindow *parent, sSpriteset* spriteset, int direction, int frame)
: wxDialog(parent, -1, "Sprite properties", wxDefaultPosition, wxSize(200, 64), wxDIALOG_MODAL | wxCAPTION | wxSYSTEM_MENU)
, m_Spriteset(spriteset)
, m_Direction(direction)
, m_Frame(frame)
{
  wxBoxSizer *mainsizer;
  wxBoxSizer *subsizer;
  mainsizer = new wxBoxSizer(wxVERTICAL);

    subsizer = new wxBoxSizer(wxHORIZONTAL);
      subsizer->Add(new wxStaticText(this, -1, "Delay"), 0, wxALL, 10);
      subsizer->Add(m_Delay = new wxSpinCtrl(this, -1, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 1024, m_Spriteset->GetFrameDelay(m_Direction, m_Frame)), 0, wxALL, 10);
    mainsizer->Add(subsizer);

    subsizer = new wxBoxSizer(wxHORIZONTAL);
      subsizer->Add(new wxButton(this, wxID_OK, "OK"), 0, wxALL, 10);
      subsizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 10);
    mainsizer->Add(subsizer);

  SetSizer(mainsizer);

  mainsizer->SetSizeHints(this);

  m_Delay->SetValue(m_Spriteset->GetFrameDelay(m_Direction, m_Frame));
}

////////////////////////////////////////////////////////////////////////////////

wSpritePropertiesDialog::~wSpritePropertiesDialog()
{
}

/*
////////////////////////////////////////////////////////////////////////////////

BOOL
CSpritePropertiesDialog::OnInitDialog()
{
  SetDlgItemInt(IDC_DELAY, m_Spriteset->GetFrameDelay(m_Direction, m_Frame));

  CEdit* edit = (CEdit*)GetDlgItem(IDC_DELAY);
  edit->SetSel(0, -1);
  edit->SetFocus();

  return FALSE;
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wSpritePropertiesDialog::OnOK(wxCommandEvent &event)
{
  int delay = m_Delay->GetValue();
  m_Spriteset->SetFrameDelay(m_Direction, m_Frame, delay);
  wxDialog::OnOK(event);
}

////////////////////////////////////////////////////////////////////////////////
