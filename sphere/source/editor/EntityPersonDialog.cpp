#include "EntityPersonDialog.hpp"
#include "FileDialogs.hpp"
#include "Scripting.hpp"
#include "resource.h"
#include "../common/Map.hpp"

BEGIN_MESSAGE_MAP(CEntityPersonDialog, CDialog)

  ON_COMMAND(IDC_BROWSE_SPRITESET, OnBrowseSpriteset)
  ON_COMMAND(IDC_CHECK_SYNTAX,     OnCheckSyntax)

  ON_CBN_SELCHANGE(IDC_SCRIPT_TYPE, OnScriptChanged)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

static inline std::string itos(int i)
{
  char s[20];
  sprintf(s, "%d", i);
  return s;
}

////////////////////////////////////////////////////////////////////////////////

CEntityPersonDialog::CEntityPersonDialog(sPersonEntity& person, sMap* map)
: CDialog(IDD_ENTITY_PERSON)
, m_Person(person)
, m_CurrentScript(0)
, m_Map(map)
{
}

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

  // add layer names in "layer_index - layer_name" style to dropdown layer list
  for (int i = 0; i < m_Map->GetNumLayers(); i++) {
    std::string layer_info = itos(i) + " - " + m_Map->GetLayer(i).GetName();
    if (m_Map->GetStartLayer() == i) {
      layer_info += " - ST";
    }

    SendDlgItemMessage(IDC_LAYER, CB_ADDSTRING, 0, (LPARAM)layer_info.c_str());
    if (i == m_Person.layer)
      SendDlgItemMessage(IDC_LAYER, CB_SETCURSEL, m_Person.layer);
  }

  // give spriteset edit box focus
  GetDlgItem(IDC_NAME)->SetFocus();
  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

void
CEntityPersonDialog::OnOK()
{
  StoreScript();

  // store the data back into the entity
  CString str;

  GetDlgItemText(IDC_NAME, str);
  m_Person.name = str;

  GetDlgItemText(IDC_SPRITESET, str);
  m_Person.spriteset = str;

  m_Person.script_create            = m_Scripts[0];
  m_Person.script_destroy           = m_Scripts[1];
  m_Person.script_activate_touch    = m_Scripts[2];
  m_Person.script_activate_talk     = m_Scripts[3];
  m_Person.script_generate_commands = m_Scripts[4];

  m_Person.layer = SendDlgItemMessage(IDC_LAYER, CB_GETCURSEL);

  CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////////

void
CEntityPersonDialog::OnBrowseSpriteset()
{
  CSpritesetFileDialog dialog(FDM_OPEN);
  if (dialog.DoModal() == IDOK) {
    SetDlgItemText(IDC_SPRITESET, dialog.GetFileName());
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CEntityPersonDialog::OnCheckSyntax()
{
  CString script;
  GetDlgItemText(IDC_SCRIPT, script);

  // check for errors
  sCompileError error;
  if (sScripting::VerifyScript(script, error)) {

    MessageBox("No syntax errors", "Check Syntax");

  } else {

    // show error
    MessageBox(("Script error:\n" + error.m_Message).c_str(), "Check Syntax");

    // select text
    SendDlgItemMessage(
      IDC_SCRIPT,
      EM_SETSEL,
      error.m_TokenStart,
      error.m_TokenStart + error.m_Token.length() + 1
    );
    GetDlgItem(IDC_SCRIPT)->SetFocus();

  }
}

////////////////////////////////////////////////////////////////////////////////

void
CEntityPersonDialog::OnScriptChanged()
{
  StoreScript();
  m_CurrentScript = SendDlgItemMessage(IDC_SCRIPT_TYPE, CB_GETCURSEL);
  SetScript();
}

////////////////////////////////////////////////////////////////////////////////

void
CEntityPersonDialog::SetScript()
{
  SetDlgItemText(IDC_SCRIPT, m_Scripts[m_CurrentScript]);
}

////////////////////////////////////////////////////////////////////////////////

void
CEntityPersonDialog::StoreScript()
{
  GetDlgItemText(IDC_SCRIPT, m_Scripts[m_CurrentScript]);
}

////////////////////////////////////////////////////////////////////////////////
