#include "ZoneEditDialog.hpp"
#include "Scripting.hpp"
//#include "resource.h"
#include "IDs.hpp"


/*
BEGIN_MESSAGE_MAP(CZoneEditDialog, CDialog)

  ON_BN_CLICKED(IDC_CHECK_SYNTAX, OnCheckSyntax)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wZoneEditDialog, wxDialog)
  EVT_BUTTON(wxID_OK, wZoneEditDialog::OnOK)

  EVT_BUTTON(wID_ZONEEDITDIALOG_CHECK_SYNTAX, wZoneEditDialog::OnCheckSyntax)

END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wZoneEditDialog::wZoneEditDialog(wxWindow *parent, sMap::sZone& zone)
: wxDialog(parent, -1, "Zone properties", wxDefaultPosition, wxSize(200, 152), wxDIALOG_MODAL | wxCAPTION | wxSYSTEM_MENU)
, m_Zone(zone)
{
  wxBoxSizer *subsizer = NULL;
  wxBoxSizer *mainsizer = new wxBoxSizer(wxVERTICAL);
  wxFlexGridSizer *gridsizer;

    gridsizer = new wxFlexGridSizer(2);
      gridsizer->Add(new wxStaticText(this, -1, "Steps", wxPoint(12, 12), wxSize(32, 8)), 0, wxALL, 10);
      gridsizer->Add(m_StepsCtrl = new wxSpinCtrl(this, -1, "", wxPoint (62, 10), wxSize(32, 12), wxSP_ARROW_KEYS, 1, 1024, m_Zone.reactivate_in_num_steps), 0, wxALL, 10);
      gridsizer->Add(new wxStaticText(this, -1, "Execute:", wxPoint(12, 36), wxSize(120, 8)), 0, wxALL, 10);
      gridsizer->Add(new wxButton(this, wID_ZONEEDITDIALOG_CHECK_SYNTAX, "Check Syntax", wxPoint(160, 32), wxSize(32, 16)), 0, wxALL, 10);
    mainsizer->Add(gridsizer);

    mainsizer->Add(m_ScriptBox = new wxTextCtrl(this, wID_ZONEEDITDIALOG_SCRIPT, m_Zone.script.c_str(), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_PROCESS_ENTER | wxTE_PROCESS_TAB), 0, wxALL, 10);

    subsizer = new wxBoxSizer(wxHORIZONTAL);
      subsizer->Add(new wxButton(this, wxID_OK, "OK"), 0, wxALL, 10);
      subsizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 10);
    mainsizer->Add(subsizer);

  SetSizer(mainsizer);
  mainsizer->SetSizeHints(this);

  m_StepsCtrl->SetValue(m_Zone.reactivate_in_num_steps);
}

/*
////////////////////////////////////////////////////////////////////////////////

BOOL
CZoneEditDialog::OnInitDialog()
{
  SetDlgItemText(IDC_SCRIPT, m_Zone.script.c_str());
  SetDlgItemInt(IDC_STEPS, m_Zone.reactivate_in_num_steps, FALSE);
  return TRUE;
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wZoneEditDialog::OnOK(wxCommandEvent &event)
{
  wxString script = m_ScriptBox->GetValue();

  m_Zone.reactivate_in_num_steps = m_StepsCtrl->GetValue();
  m_Zone.script = script;
  
  wxDialog::OnOK(event);
}

////////////////////////////////////////////////////////////////////////////////

void
wZoneEditDialog::OnCheckSyntax(wxCommandEvent &event)
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
      error.m_TokenStart + error.m_Token.length()
    );
    m_ScriptBox->SetFocus();
  }
}

////////////////////////////////////////////////////////////////////////////////
