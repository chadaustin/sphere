#include "GameSettingsDialog.hpp"
#include "Project.hpp"
#include "resource.h"


////////////////////////////////////////////////////////////////////////////////

CGameSettingsDialog::CGameSettingsDialog(CProject* project)
: CDialog(IDD_GAME_SETTINGS)
, m_Project(project)
{
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CGameSettingsDialog::OnInitDialog()
{
  // set game title
  GetDlgItem(IDC_GAMETITLE)->SetFocus();
  SetDlgItemText(IDC_GAMETITLE, m_Project->GetGameTitle());
  SendDlgItemMessage(IDC_GAMETITLE, EM_SETSEL, 0, -1);

  // set game author and description
  SetDlgItemText(IDC_AUTHOR, m_Project->GetAuthor());
  SetDlgItemText(IDC_DESCRIPTION, m_Project->GetDescription());

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

////////////////////////////////////////////////////////////////////////////////

void
CGameSettingsDialog::OnOK()
{
  int screen_width  = GetDlgItemInt(IDC_SCREENWIDTH);
  int screen_height = GetDlgItemInt(IDC_SCREENHEIGHT);

  if (screen_width < 1 || screen_width > 2048)
  {
    MessageBox("Screen width must be between 1 and 2048", "Game Settings");
    return;
  }

  if (screen_height < 1 || screen_height > 1536)
  {
    MessageBox("Screen height must be between 1 and 1536", "Game Settings");
    return;
  }

  CString sGameTitle;
  GetDlgItemText(IDC_GAMETITLE, sGameTitle);
  m_Project->SetGameTitle(sGameTitle);

  CString author;
  GetDlgItemText(IDC_AUTHOR, author);
  m_Project->SetAuthor(author);
  
  CString description;
  GetDlgItemText(IDC_DESCRIPTION, description);
  m_Project->SetDescription(description);
  
  // if the project contains any scripts, set it to the one that's selected
  if (m_Project->GetItemCount(GT_SCRIPTS) > 0)
  {
    char script[MAX_PATH + 1024];
    int cur_sel = SendDlgItemMessage(IDC_SCRIPT, CB_GETCURSEL, 0, 0);
    SendDlgItemMessage(IDC_SCRIPT, CB_GETLBTEXT, cur_sel, (LPARAM)script);
    m_Project->SetGameScript(script);
  }

  m_Project->SetScreenWidth(screen_width);
  m_Project->SetScreenHeight(screen_height);

  m_Project->Save();

  CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////////
