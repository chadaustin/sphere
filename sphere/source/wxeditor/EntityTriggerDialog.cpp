#include "EntityTriggerDialog.hpp"
#include "Scripting.hpp"
//#include "resource.h"
#include "IDs.hpp"

/*
BEGIN_MESSAGE_MAP(wEntityTriggerDialog, wxDialog)

  ON_BN_CLICKED(wID_MISC_CHECK_SYNTAX, OnCheckSyntax)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wEntityTriggerDialog, wxDialog)
  EVT_BUTTON(wxID_OK, wEntityTriggerDialog::OnOK)

  EVT_BUTTON(wID_TRIGGERDIALOG_CHECK_SYNTAX, wEntityTriggerDialog::OnCheckSyntax)

END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wEntityTriggerDialog::wEntityTriggerDialog(wxWindow *parent, sTriggerEntity& trigger)
: wxDialog(parent, -1, "Trigger script", wxDefaultPosition, wxSize(200, 120), wxDIALOG_MODAL | wxCAPTION | wxSYSTEM_MENU)
, m_Trigger(trigger)
{
  wxBoxSizer *subsizer = NULL;
  wxBoxSizer *mainsizer = new wxBoxSizer(wxVERTICAL);

    subsizer = new wxBoxSizer(wxHORIZONTAL);
      subsizer->Add(new wxStaticText(this, -1, "Execute:"), 0, wxALL, 10);
      subsizer->Add(new wxButton(this, wID_TRIGGERDIALOG_CHECK_SYNTAX, "Check Syntax"), 0, wxALL, 10);
    mainsizer->Add(subsizer);

    mainsizer->Add(m_ScriptBox = new wxTextCtrl(this, wID_TRIGGERDIALOG_SCRIPT, m_Trigger.script.c_str(), wxDefaultPosition, wxSize(374, 128), wxTE_MULTILINE | wxTE_PROCESS_TAB), 0, wxALL, 10);

    subsizer = new wxBoxSizer(wxHORIZONTAL);
      subsizer->Add(new wxButton(this, wxID_OK, "OK"), 0, wxALL, 10);
      subsizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 10);
    mainsizer->Add(subsizer);

  SetSizer(mainsizer);
  mainsizer->SetSizeHints(this);

}

/*
////////////////////////////////////////////////////////////////////////////////

BOOL
CEntityTriggerDialog::OnInitDialog()
{
  SetDlgItemText(IDC_SCRIPT, m_Trigger.script.c_str());
  return TRUE;
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wEntityTriggerDialog::OnOK(wxCommandEvent &event)
{
  m_Trigger.script = m_ScriptBox->GetValue();
  
  wxDialog::OnOK(event);
}

////////////////////////////////////////////////////////////////////////////////

void
wEntityTriggerDialog::OnCheckSyntax(wxCommandEvent &event)
{
  wxString script = m_ScriptBox->GetValue();

  // check for errors
  sCompileError error;
  if (VerifyScript(script.c_str(), error)) {
    ::wxMessageBox("No syntax errors", "Check Syntax");
  } else {
    // show error
    ::wxMessageBox(("Script error:\n" + error.m_Message).c_str(), "Check Syntax");

    // select text
    m_ScriptBox->SetSelection(
      error.m_TokenStart,
      error.m_TokenStart + error.m_Token.length() + 1
    );
    m_ScriptBox->SetFocus();
  }
}

////////////////////////////////////////////////////////////////////////////////
