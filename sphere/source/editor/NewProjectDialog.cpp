#include "NewProjectDialog.hpp"
#include "resource.h"


////////////////////////////////////////////////////////////////////////////////

CNewProjectDialog::CNewProjectDialog(CWnd* pParent) :
  CDialog(IDD_NEW_PROJECT, pParent)
{
  strcpy(m_szProjectName, "");
  strcpy(m_szGameTitle, "");
}

////////////////////////////////////////////////////////////////////////////////

CNewProjectDialog::~CNewProjectDialog()
{
}

////////////////////////////////////////////////////////////////////////////////

const char*
CNewProjectDialog::GetProjectName()
{
  return m_szProjectName;
}

////////////////////////////////////////////////////////////////////////////////

const char*
CNewProjectDialog::GetGameTitle()
{
  return m_szGameTitle;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CNewProjectDialog::OnOK()
{
  GetDlgItemText(IDC_PROJECTNAME, m_szProjectName, 32);
  GetDlgItemText(IDC_GAMETITLE,   m_szGameTitle, 32);

  if (strlen(m_szProjectName) == 0)
  {
    MessageBox("Please enter a project name", "New Project");
    return;
  }

  if (strlen(m_szGameTitle) == 0)
  {
    MessageBox("Please enter a game title", "New Project");
    return;
  }

  CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg BOOL
CNewProjectDialog::OnInitDialog()
{
  CDialog::OnInitDialog();

  GetDlgItem(IDC_PROJECTNAME)->SetFocus();

  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
