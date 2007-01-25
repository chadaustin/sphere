#include "GameSettingsDialog.hpp"
#include "Project.hpp"
#include "resource.h"

BEGIN_EVENT_TABLE(wGameSettingsDialog, wxDialog)
  EVT_BUTTON(wxID_OK, wGameSettingsDialog::OnOK)
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wGameSettingsDialog::wGameSettingsDialog(wxWindow *parent, wProject* project)
: wxDialog(parent, -1, "Game Settings", wxDefaultPosition, wxSize(208, 72), wxDIALOG_MODAL | wxCAPTION | wxSYSTEM_MENU)
, m_Project(project)
{
  wxBoxSizer *mainsizer = NULL;
  wxBoxSizer *subsizer = NULL;
  wxFlexGridSizer *gridsizer = NULL;

  mainsizer = new wxBoxSizer(wxVERTICAL);
  gridsizer = new wxFlexGridSizer(2);
  
  if (mainsizer && gridsizer) {
    gridsizer->Add(new wxStaticText(this, -1, "Title"), 0, wxALL, 5);
    gridsizer->Add(m_ProjectTitle = new wxTextCtrl(this, -1, m_Project->GetGameTitle()), 0, wxALL, 5);

    gridsizer->Add(new wxStaticText(this, -1, "Script"), 0, wxALL, 5);
    gridsizer->Add(m_StartScript = new wxComboBox(this, -1, "", wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY | wxCB_SORT), 0, wxALL, 5);
    
    gridsizer->Add(new wxStaticText(this, -1, "Screen Width"), 0, wxALL, 5);
    gridsizer->Add(m_ScreenWidth = new wxSpinCtrl(this, -1, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 2048, m_Project->GetScreenWidth()), 0, wxALL, 5);
    
    gridsizer->Add(new wxStaticText(this, -1, "Screen Height"), 0, wxALL, 5);
    gridsizer->Add(m_ScreenHeight = new wxSpinCtrl(this, -1, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 1536, m_Project->GetScreenHeight()), 0, wxALL, 5);

    mainsizer->Add(gridsizer);
  }

  subsizer = new wxBoxSizer(wxHORIZONTAL);
  if (subsizer) {
    subsizer->Add(new wxButton(this, wxID_OK, "OK"), 0, wxALL, 5);
    subsizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
  }

  if (mainsizer) {
    mainsizer->Add(subsizer, 0, wxALIGN_CENTER);
    SetSizer(mainsizer);
    mainsizer->SetSizeHints(this);
  }

  if (m_StartScript) {
    if (m_Project->GetItemCount(GT_SCRIPTS) > 0) {
      for (int i = 0; i < m_Project->GetItemCount(GT_SCRIPTS); i++) {
        m_StartScript->Append(m_Project->GetItem(GT_SCRIPTS, i));
        if (strcmp(m_Project->GetItem(GT_SCRIPTS, i), m_Project->GetGameScript()) == 0) {
          m_StartScript->SetValue(m_Project->GetGameScript());
        }
      }
    }

    m_StartScript->Enable(m_Project->GetItemCount(GT_SCRIPTS) > 0 ? TRUE : FALSE);
  }

  if (m_ProjectTitle) m_ProjectTitle->SetValue(m_Project->GetGameTitle());

  if (m_ScreenWidth) m_ScreenWidth->SetValue(m_Project->GetScreenWidth());
  if (m_ScreenHeight) m_ScreenHeight->SetValue(m_Project->GetScreenHeight());
}

/*
////////////////////////////////////////////////////////////////////////////////

BOOL
CGameSettingsDialog::OnInitDialog()
{
  // set game title
  GetDlgItem(IDC_GAMETITLE)->SetFocus();
  SetDlgItemText(IDC_GAMETITLE, m_Project->GetGameTitle());
  SendDlgItemMessage(IDC_GAMETITLE, EM_SETSEL, 0, -1);

  // fill script list
  if (m_Project->GetItemCount(GT_SCRIPTS) > 0)
  {
    // add the scripts from the project
    for (int i = 0; i < m_Project->GetItemCount(GT_SCRIPTS); i++)
      SendDlgItemMessage(IDC_SCRIPT, CB_ADDSTRING, 0, (LPARAM)m_Project->GetItem(GT_SCRIPTS, i));

    const char* game_script = m_Project->GetGameScript();
    SendDlgItemMessage(IDC_SCRIPT, CB_SELECTSTRING, -1, (LPARAM)game_script);
  }
  else
  {
    SendDlgItemMessage(IDC_SCRIPT, CB_ADDSTRING, 0, (LPARAM)"(none)");
    SendDlgItemMessage(IDC_SCRIPT, CB_SETCURSEL, 0, 0);
  }

  // set screen resolution
  SetDlgItemInt(IDC_SCREENWIDTH,  m_Project->GetScreenWidth());
  SetDlgItemInt(IDC_SCREENHEIGHT, m_Project->GetScreenHeight());

  return FALSE;
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wGameSettingsDialog::OnOK(wxCommandEvent &event)
{
  if (!m_ScreenWidth || !m_ScreenHeight || !m_ProjectTitle)
    return;

  int screen_width  = m_ScreenWidth->GetValue();
  int screen_height = m_ScreenHeight->GetValue();

  if (screen_width < 1 || screen_width > 2048)
  {
    ::wxMessageBox("Screen width must be between 1 and 2048", "Game Settings");
    return;
  }

  if (screen_height < 1 || screen_height > 1536)
  {
    ::wxMessageBox("Screen height must be between 1 and 1536", "Game Settings");
    return;
  }

  wxString sGameTitle = m_ProjectTitle->GetValue();

  m_Project->SetGameTitle(sGameTitle.c_str());
  
  // if the project contains any scripts, set it to the one that's selected
  if (m_Project->GetItemCount(GT_SCRIPTS) > 0)
  {
    m_Project->SetGameScript(m_StartScript->GetStringSelection().c_str());
  }

  m_Project->SetScreenWidth(screen_width);
  m_Project->SetScreenHeight(screen_height);

  m_Project->Save();

  wxDialog::OnOK(event);
}

////////////////////////////////////////////////////////////////////////////////
