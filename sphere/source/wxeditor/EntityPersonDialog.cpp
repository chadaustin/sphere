#include "EntityPersonDialog.hpp"
#include "FileDialogs.hpp"
#include "Scripting.hpp"
//#include "resource.h"
#include "IDs.hpp"


/*
BEGIN_MESSAGE_MAP(CEntityPersonDialog, CDialog)

  ON_COMMAND(IDC_BROWSE_SPRITESET, OnBrowseSpriteset)
  ON_COMMAND(IDC_CHECK_SYNTAX,     OnCheckSyntax)

  ON_CBN_SELCHANGE(IDC_SCRIPT_TYPE, OnScriptChanged)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wEntityPersonDialog, wxDialog)
  EVT_BUTTON(wxID_OK, wEntityPersonDialog::OnOK)
  EVT_BUTTON(wID_ENTITYPERSONDIALOG_BROWSE_SPRITESET, wEntityPersonDialog::OnBrowseSpriteset)
  EVT_BUTTON(wID_ENTITYPERSONDIALOG_CHECK_SYNTAX, wEntityPersonDialog::OnCheckSyntax)
  EVT_RADIOBOX(wID_ENTITYPERSONDIALOG_RADIOBOX,   wEntityPersonDialog::OnScriptChanged)
END_EVENT_TABLE()


////////////////////////////////////////////////////////////////////////////////

wEntityPersonDialog::wEntityPersonDialog(wxWindow *parent, sPersonEntity& person)
: wxDialog(parent, -1, "Person Properties", wxDefaultPosition, wxSize(208, 72), wxDIALOG_MODAL | wxCAPTION | wxSYSTEM_MENU)
, m_Person(person)
, m_CurrentScript(0)
{
  m_CurrentScript = 0;
  m_Scripts[0] = m_Person.script_create.c_str();
  m_Scripts[1] = m_Person.script_destroy.c_str();
  m_Scripts[2] = m_Person.script_activate_touch.c_str();
  m_Scripts[3] = m_Person.script_activate_talk.c_str();
  m_Scripts[4] = m_Person.script_generate_commands.c_str();

  wxBoxSizer *mainsizer;
  wxBoxSizer *subsizer;
  wxFlexGridSizer *gridsizer;

  mainsizer = new wxBoxSizer(wxVERTICAL);

  gridsizer = new wxFlexGridSizer(2);
    gridsizer->Add(new wxStaticText(this, -1, "Name"), 0, wxALL, 5);
    gridsizer->Add(m_PersonNameCtrl = new wxTextCtrl(this, -1, ""), 0, wxALL, 5);
    gridsizer->Add(new wxStaticText(this, -1, "Spriteset"), 0, wxALL, 5);
    subsizer = new wxBoxSizer(wxHORIZONTAL);
      subsizer->Add(m_SpritesetCtrl = new wxTextCtrl(this, -1, ""), 0, wxALL, 5);
      subsizer->Add(new wxButton(this, wID_ENTITYPERSONDIALOG_BROWSE_SPRITESET, "..."), 0, wxALL, 5);
    gridsizer->Add(subsizer);
  mainsizer->Add(gridsizer);

  mainsizer->Add(new wxButton(this, wID_ENTITYPERSONDIALOG_CHECK_SYNTAX, "Check Syntax"), 0, wxALL | wxALIGN_RIGHT, 5);

  wxString choices[] = {"On Create", "On Destroy", "On Activate(touch)", "On Activate(talk)", "On Generate Commands"};
  subsizer = new wxBoxSizer(wxHORIZONTAL);
  subsizer->Add(m_ScriptRadioBox = new wxRadioBox(
        this, wID_ENTITYPERSONDIALOG_RADIOBOX, "Scripts", wxDefaultPosition, wxDefaultSize,
        5, choices, 1), 0, wxALL, 5);
  subsizer->Add(m_ScriptTextBox = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxSize(374, 128), wxTE_MULTILINE | wxTE_PROCESS_TAB), 1, wxALL | wxGROW, 5);
  mainsizer->Add(subsizer);

  subsizer = new wxBoxSizer(wxHORIZONTAL);
    subsizer->Add(new wxButton(this, wxID_OK, "OK"), 0, wxALL, 5);
    subsizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
  mainsizer->Add(subsizer);

  SetSizer(mainsizer);
  mainsizer->SetSizeHints(this);

  m_ScriptRadioBox->SetSelection(0);
  SetScript();

  m_PersonNameCtrl->SetValue(m_Person.name.c_str());
  m_SpritesetCtrl->SetValue(m_Person.spriteset.c_str());
}

/*
////////////////////////////////////////////////////////////////////////////////

BOOL
CEntityPersonDialog::OnInitDialog()
{
  // initialize controls
  
  SendDlgItemMessage(IDC_SCRIPT_TYPE, CB_ADDSTRING, 0, (LPARAM)"On Create");
  SendDlgItemMessage(IDC_SCRIPT_TYPE, CB_ADDSTRING, 0, (LPARAM)"On Destroy");
  SendDlgItemMessage(IDC_SCRIPT_TYPE, CB_ADDSTRING, 0, (LPARAM)"On Activate (Touch)");
  SendDlgItemMessage(IDC_SCRIPT_TYPE, CB_ADDSTRING, 0, (LPARAM)"On Activate (Talk)");
  SendDlgItemMessage(IDC_SCRIPT_TYPE, CB_ADDSTRING, 0, (LPARAM)"On Generate Commands");
  
  m_CurrentScript = 0;
  m_Scripts[0] = m_Person.script_create.c_str();
  m_Scripts[1] = m_Person.script_destroy.c_str();
  m_Scripts[2] = m_Person.script_activate_touch.c_str();
  m_Scripts[3] = m_Person.script_activate_talk.c_str();
  m_Scripts[4] = m_Person.script_generate_commands.c_str();


  // put in the initial values
  
  SetDlgItemText(IDC_NAME,      m_Person.name.c_str());
  SetDlgItemText(IDC_SPRITESET, m_Person.spriteset.c_str());
  SendDlgItemMessage(IDC_SCRIPT_TYPE, CB_SETCURSEL, 0);
  SetScript();

  // give spriteset edit box focus
  GetDlgItem(IDC_NAME)->SetFocus();
  return FALSE;
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wEntityPersonDialog::OnOK(wxCommandEvent &event)
{
  StoreScript();

  // store the data back into the entity
  m_Person.name = m_PersonNameCtrl->GetValue();

  m_Person.spriteset = m_SpritesetCtrl->GetValue();

  m_Person.script_create            = m_Scripts[0];
  m_Person.script_destroy           = m_Scripts[1];
  m_Person.script_activate_touch    = m_Scripts[2];
  m_Person.script_activate_talk     = m_Scripts[3];
  m_Person.script_generate_commands = m_Scripts[4];

  wxDialog::OnOK(event);
}

////////////////////////////////////////////////////////////////////////////////

void
wEntityPersonDialog::OnBrowseSpriteset(wxCommandEvent &event)
{
  wSpritesetFileDialog dialog(this, FDM_OPEN);
  if (dialog.ShowModal() == wxID_OK) {
    m_SpritesetCtrl->SetValue(dialog.GetFilename());
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wEntityPersonDialog::OnCheckSyntax(wxCommandEvent &event)
{
  wxString script = m_ScriptTextBox->GetValue();

  // check for errors
  sCompileError error;
  if (VerifyScript(script, error)) {

    ::wxMessageBox("No syntax errors", "Check Syntax");

  } else {

    // show error
    ::wxMessageBox(("Script error:\n" + error.m_Message).c_str(), "Check Syntax");

    // select text
    m_ScriptTextBox->SetSelection(
      error.m_TokenStart,
      error.m_TokenStart + error.m_Token.length()
    );
    m_ScriptTextBox->SetFocus();

  }
}

////////////////////////////////////////////////////////////////////////////////

void
wEntityPersonDialog::OnScriptChanged(wxCommandEvent &event)
{
  StoreScript();
  m_CurrentScript = m_ScriptRadioBox->GetSelection();
  SetScript();
}

////////////////////////////////////////////////////////////////////////////////

void
wEntityPersonDialog::SetScript()
{
  m_ScriptTextBox->SetValue(m_Scripts[m_CurrentScript]);
}

////////////////////////////////////////////////////////////////////////////////

void
wEntityPersonDialog::StoreScript()
{
  m_Scripts[m_CurrentScript] = m_ScriptTextBox->GetValue();
}

////////////////////////////////////////////////////////////////////////////////
