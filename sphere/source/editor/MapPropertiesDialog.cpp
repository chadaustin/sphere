#include "Editor.hpp"
#include "DocumentWindow.hpp"
#include "MapPropertiesDialog.hpp"
#include "Scripting.hpp"
#include "../common/Map.hpp"
#include "FileDialogs.hpp"
#include "resource.h"
BEGIN_MESSAGE_MAP(CMapPropertiesDialog, CDialog)
  ON_BN_CLICKED(IDC_BROWSE_MUSIC, OnBrowseBackgroundMusic)
  ON_BN_CLICKED(IDC_BROWSE_TILESET, OnBrowseTileset)
  ON_BN_CLICKED(IDC_CHECK_SYNTAX, OnCheckSyntax)
  ON_BN_CLICKED(IDC_ENTRY,        OnClickEntry)
  ON_BN_CLICKED(IDC_EXIT,         OnClickExit)
  ON_BN_CLICKED(IDC_NORTH,        OnClickNorth)
  ON_BN_CLICKED(IDC_EAST,         OnClickEast)
  ON_BN_CLICKED(IDC_SOUTH,        OnClickSouth)
  ON_BN_CLICKED(IDC_WEST,         OnClickWest)
END_MESSAGE_MAP()
////////////////////////////////////////////////////////////////////////////////
CMapPropertiesDialog::CMapPropertiesDialog(sMap* map, const char* document_path)
: CDialog(IDD_MAP_PROPERTIES)
, m_Map(map)
, m_CurrentScript(ENTRY)
, m_DocumentPath(document_path)
{
  m_EntryScript = m_Map->GetEntryScript();
  m_ExitScript  = m_Map->GetExitScript();
  m_NorthScript = m_Map->GetEdgeScript(sMap::NORTH);
  m_EastScript  = m_Map->GetEdgeScript(sMap::EAST);
  m_SouthScript = m_Map->GetEdgeScript(sMap::SOUTH);
  m_WestScript  = m_Map->GetEdgeScript(sMap::WEST);
}
////////////////////////////////////////////////////////////////////////////////
BOOL
CMapPropertiesDialog::OnInitDialog()
{
  if (m_DocumentPath) {
    SetDlgItemText(IDC_MAPPATH, m_DocumentPath);
  } else {
    SetDlgItemText(IDC_MAPPATH, "");
  }
  // set music file
  SetDlgItemText(IDC_MUSIC,   m_Map->GetMusicFile());
  SetDlgItemText(IDC_TILESET, m_Map->GetTilesetFile());
  // set script file
  CheckDlgButton(IDC_ENTRY, BST_CHECKED);
  // check "repeating" button
  CheckDlgButton(IDC_REPEATING, (m_Map->IsRepeating() ? BST_CHECKED : BST_UNCHECKED));
  // put script in edit control
  LoadNewScript();
  return TRUE;
}
////////////////////////////////////////////////////////////////////////////////
void
CMapPropertiesDialog::OnOK()
{
  // get music file
  CString str;
  GetDlgItemText(IDC_MUSIC, str);
  m_Map->SetMusicFile(str);
  GetDlgItemText(IDC_TILESET, str);
  m_Map->SetTilesetFile(str);
  m_Map->SetRepeating(IsDlgButtonChecked(IDC_REPEATING) == BST_CHECKED);
  StoreCurrentScript();
  m_Map->SetEntryScript(            m_EntryScript);
  m_Map->SetExitScript(             m_ExitScript);
  m_Map->SetEdgeScript(sMap::NORTH, m_NorthScript);
  m_Map->SetEdgeScript(sMap::EAST,  m_EastScript);
  m_Map->SetEdgeScript(sMap::SOUTH, m_SouthScript);
  m_Map->SetEdgeScript(sMap::WEST,  m_WestScript);
  CDialog::OnOK();
}
////////////////////////////////////////////////////////////////////////////////
afx_msg void
CMapPropertiesDialog::OnBrowseBackgroundMusic()
{
  char old_directory[MAX_PATH] = {0};
  GetCurrentDirectory(MAX_PATH, old_directory);
  std::string directory = GetMainWindow()->GetDefaultFolder(WA_SOUND);
  SetCurrentDirectory(directory.c_str());
  CSoundFileDialog dialog(FDM_OPEN);
  if (dialog.DoModal() == IDOK) {
    SetDlgItemText(IDC_MUSIC, dialog.GetFileName());
  }
  SetCurrentDirectory(old_directory);
}
////////////////////////////////////////////////////////////////////////////////
afx_msg void
CMapPropertiesDialog::OnBrowseTileset()
{
  char old_directory[MAX_PATH] = {0};
  GetCurrentDirectory(MAX_PATH, old_directory);
  std::string directory = GetMainWindow()->GetDefaultFolder(WA_MAP);
  SetCurrentDirectory(directory.c_str());
  CTilesetFileDialog dialog(FDM_OPEN);
  if (dialog.DoModal() == IDOK) {
    SetDlgItemText(IDC_TILESET, dialog.GetFileName());
  }
  SetCurrentDirectory(old_directory);
}
////////////////////////////////////////////////////////////////////////////////
afx_msg void
CMapPropertiesDialog::OnCheckSyntax()
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
afx_msg void
CMapPropertiesDialog::OnClickEntry()
{
  StoreCurrentScript();
  m_CurrentScript = ENTRY;
  LoadNewScript();
}
////////////////////////////////////////////////////////////////////////////////
afx_msg void
CMapPropertiesDialog::OnClickExit()
{
  StoreCurrentScript();
  m_CurrentScript = EXIT;
  LoadNewScript();
}
////////////////////////////////////////////////////////////////////////////////
afx_msg void
CMapPropertiesDialog::OnClickNorth()
{
  StoreCurrentScript();
  m_CurrentScript = NORTH;
  LoadNewScript();
}
////////////////////////////////////////////////////////////////////////////////
afx_msg void
CMapPropertiesDialog::OnClickEast()
{
  StoreCurrentScript();
  m_CurrentScript = EAST;
  LoadNewScript();
}
////////////////////////////////////////////////////////////////////////////////
afx_msg void
CMapPropertiesDialog::OnClickSouth()
{
  StoreCurrentScript();
  m_CurrentScript = SOUTH;
  LoadNewScript();
}
////////////////////////////////////////////////////////////////////////////////
afx_msg void
CMapPropertiesDialog::OnClickWest()
{
  StoreCurrentScript();
  m_CurrentScript = WEST;
  LoadNewScript();
}
////////////////////////////////////////////////////////////////////////////////
void
CMapPropertiesDialog::StoreCurrentScript()
{
  CString* string = NULL;
  
  switch (m_CurrentScript) {
    case ENTRY: string = &m_EntryScript; break;
    case EXIT:  string = &m_ExitScript;  break;
    case NORTH: string = &m_NorthScript; break;
    case EAST:  string = &m_EastScript;  break;
    case SOUTH: string = &m_SouthScript; break;
    case WEST:  string = &m_WestScript;  break;
    default:    return;
  }
  GetDlgItemText(IDC_SCRIPT, *string);
}
////////////////////////////////////////////////////////////////////////////////
void
CMapPropertiesDialog::LoadNewScript()
{
  CString* string = NULL;
  switch (m_CurrentScript) {
    case ENTRY: string = &m_EntryScript; break;
    case EXIT:  string = &m_ExitScript;  break;
    case NORTH: string = &m_NorthScript; break;
    case EAST:  string = &m_EastScript;  break;
    case SOUTH: string = &m_SouthScript; break;
    case WEST:  string = &m_WestScript;  break;
    default:    return;
  }
  SetDlgItemText(IDC_SCRIPT, *string);
}
////////////////////////////////////////////////////////////////////////////////
