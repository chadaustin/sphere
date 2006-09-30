#include "Editor.hpp"
#include "DocumentWindow.hpp"
#include "EntityPersonDialog.hpp"
#include "FileDialogs.hpp"
#include "Scripting.hpp"
#include "../common/Map.hpp"
#include "resource.h"
BEGIN_MESSAGE_MAP(CEntityPersonDialog, CDialog)
  ON_COMMAND(IDC_BROWSE_SPRITESET, OnBrowseSpriteset)
  ON_COMMAND(IDC_CHECK_SYNTAX,     OnCheckSyntax)
  ON_COMMAND(IDC_GENERATE_NAME,    OnGenerateName)
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
  char old_directory[MAX_PATH] = {0};
  GetCurrentDirectory(MAX_PATH, old_directory);
  std::string directory = GetMainWindow()->GetDefaultFolder(WA_SPRITESET);
  SetCurrentDirectory(directory.c_str());
  CSpritesetFileDialog dialog(FDM_OPEN);
  if (dialog.DoModal() == IDOK) {
    SetDlgItemText(IDC_SPRITESET, dialog.GetFileName());
  }
  SetCurrentDirectory(old_directory);
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
CEntityPersonDialog::OnGenerateName()
{
  CString str;
  GetDlgItemText(IDC_SPRITESET, str);
  std::string filename = str;
  str.MakeLower();
  std::string lower_filename = str;
  GetDlgItemText(IDC_NAME, str);
  std::string current_name = str;
  if (filename.size() > 0 && lower_filename.size() > 0)
  {
    if (lower_filename.rfind(".rss") == lower_filename.size() - 4)
    {
      char gen_name[MAX_PATH + 80] = {0};
      memcpy(gen_name, filename.c_str(), filename.size() - 4);
      gen_name[filename.size() - 4] = '_';
      bool current_name_valid = false;
      bool is_unique = false;
      if (current_name.size() > strlen(gen_name)) {
        if (memcmp(current_name.c_str(), gen_name, strlen(gen_name)) == 0) {
          int id = 1;
          if (sscanf(current_name.c_str() + strlen(gen_name), "%d$", &id) == 1) {
            current_name_valid = true;
            is_unique = true;
          }
        }
      }
      if (current_name_valid) {
        bool found = false;
        for (int i = 0; i < m_Map->GetNumEntities(); i++) {
          sEntity& e = m_Map->GetEntity(i);
          if (e.GetEntityType() == sEntity::PERSON) {
            sPersonEntity& p = (sPersonEntity&)e;
            if (p.name == current_name) {
              if (found) {
                is_unique = false;
                break;
              }
              found = true;
            }
          }
        }
      }
      if (!is_unique) {
        int unique_id = 1;
        for (int i = 0; i < m_Map->GetNumEntities(); i++) {
          sEntity& e = m_Map->GetEntity(i);
          if (e.GetEntityType() == sEntity::PERSON) {
            sPersonEntity& p = (sPersonEntity&)e;
            if (p.name.size() > strlen(gen_name)) {
              if (memcmp(p.name.c_str(), gen_name, strlen(gen_name)) == 0) {
                int id = unique_id;
 
                if (sscanf(p.name.c_str() + strlen(gen_name), "%d$", &id) == 1) {
                  if (id >= unique_id) {
                    unique_id = id + 1;
                  }
                }
              }
            }
          }
        }
        sprintf (gen_name + strlen(gen_name), "%d", unique_id);
        str = gen_name;
        SetDlgItemText(IDC_NAME, str);
      }
    }
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
