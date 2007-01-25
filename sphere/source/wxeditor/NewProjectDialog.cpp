#include "NewProjectDialog.hpp"
#include "resource.h"

BEGIN_EVENT_TABLE(wNewProjectDialog, wxDialog)
  EVT_BUTTON(wxID_OK, wNewProjectDialog::OnOK)
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wNewProjectDialog::wNewProjectDialog(wxWindow* parent) :
  wxDialog(parent, -1, "New Project", wxDefaultPosition, wxSize(256, 128), wxDIALOG_MODAL | wxCAPTION | wxSYSTEM_MENU)
, m_ProjectNameCtrl(NULL)
, m_GameTitleCtrl(NULL)
{
  wxBoxSizer *mainsizer = NULL;
  wxFlexGridSizer *gridsizer = NULL;
  wxBoxSizer *subsizer = NULL;

  mainsizer = new wxBoxSizer(wxVERTICAL);
  gridsizer = new wxFlexGridSizer(2);
  
  if (mainsizer && gridsizer) {
    gridsizer->Add(new wxStaticText(this, -1, "Project Name"), 0, wxALL, 10);
    gridsizer->Add(m_ProjectNameCtrl = new wxTextCtrl(this, -1, ""), 1, wxALL | wxGROW, 10);
    gridsizer->Add(new wxStaticText(this, -1, "Game Title"), 1, wxALL, 10);
    gridsizer->Add(m_GameTitleCtrl = new wxTextCtrl(this, -1, ""), 0, wxALL | wxGROW, 10);

    mainsizer->Add(gridsizer);
    subsizer = new wxBoxSizer(wxHORIZONTAL);
    if (subsizer) {
      subsizer->Add(new wxButton(this, wxID_OK, "OK"), 0, wxALL, 10); 
      subsizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 10);
    }

    mainsizer->Add(subsizer);

    SetAutoLayout(TRUE);
    SetSizer(mainsizer);
    mainsizer->SetSizeHints(this);
  }   
}

////////////////////////////////////////////////////////////////////////////////

wNewProjectDialog::~wNewProjectDialog()
{
}

////////////////////////////////////////////////////////////////////////////////

const char*
wNewProjectDialog::GetProjectName()
{
  return m_ProjectName.c_str();
}

////////////////////////////////////////////////////////////////////////////////

const char*
wNewProjectDialog::GetGameTitle()
{
  return m_GameTitle.c_str();
}

////////////////////////////////////////////////////////////////////////////////

void
wNewProjectDialog::OnOK(wxCommandEvent &event)
{
  if (!m_ProjectNameCtrl || !m_GameTitleCtrl)
    return;

  m_ProjectName = m_ProjectNameCtrl->GetValue();
  m_GameTitle = m_GameTitleCtrl->GetValue();

  if (m_ProjectName.length() == 0)
  {
    ::wxMessageBox("Please enter a project name", "New Project");
    return;
  }

  if (m_GameTitle.length() == 0)
  {
    ::wxMessageBox("Please enter a game title", "New Project");
    return;
  }

  wxDialog::OnOK(event);
}

/*
////////////////////////////////////////////////////////////////////////////////

afx_msg BOOL
CNewProjectDialog::OnInitDialog()
{
  CDialog::OnInitDialog();

  GetDlgItem(IDC_PROJECTNAME)->SetFocus();

  return FALSE;
}
*/

////////////////////////////////////////////////////////////////////////////////
