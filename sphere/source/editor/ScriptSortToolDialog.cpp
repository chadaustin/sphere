#include "ScriptSortToolDialog.hpp"
#include "resource.h"

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CScriptSortToolDialog, CDialog)


  ON_BN_CLICKED(IDC_SORT_TOOL_SORTLINES, OnSortOptionChanged)
  ON_BN_CLICKED(IDC_SORT_TOOL_REVERSELINES, OnSortOptionChanged)
  ON_BN_CLICKED(IDC_SORT_TOOL_DELETEDUPLICATES, OnSortOptionChanged)
  ON_BN_CLICKED(IDC_SORT_TOOL_IGNORECASE, OnSortOptionChanged)

END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

CScriptSortToolDialog::CScriptSortToolDialog()
: CDialog(IDD_SCRIPT_SORT)
{
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CScriptSortToolDialog::OnInitDialog()
{
  m_SortLines = true;
  m_ReverseLines = false;
  m_RemoveDuplicateLines = false;
  m_IgnoreCase = false;

  UpdateButtons();
  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptSortToolDialog::OnOK()
{
  CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptSortToolDialog::UpdateButtons()
{
  BOOL enabled = TRUE;

  CheckDlgButton(IDC_SORT_TOOL_SORTLINES, m_SortLines ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(IDC_SORT_TOOL_REVERSELINES, m_ReverseLines ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(IDC_SORT_TOOL_DELETEDUPLICATES, m_RemoveDuplicateLines ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(IDC_SORT_TOOL_IGNORECASE, m_IgnoreCase ? BST_CHECKED : BST_UNCHECKED);


  if (IsDlgButtonChecked(IDC_SORT_TOOL_SORTLINES) != BST_CHECKED
   && IsDlgButtonChecked(IDC_SORT_TOOL_REVERSELINES) != BST_CHECKED
   && IsDlgButtonChecked(IDC_SORT_TOOL_DELETEDUPLICATES) != BST_CHECKED) {
    enabled = FALSE;
  }

  if (GetDlgItem(IDOK)) {
    GetDlgItem(IDOK)->EnableWindow(enabled); 
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptSortToolDialog::OnSortOptionChanged()
{
  m_SortLines = (IsDlgButtonChecked(IDC_SORT_TOOL_SORTLINES) == BST_CHECKED);
  m_ReverseLines = (IsDlgButtonChecked(IDC_SORT_TOOL_REVERSELINES) == BST_CHECKED);
  m_RemoveDuplicateLines = (IsDlgButtonChecked(IDC_SORT_TOOL_DELETEDUPLICATES) == BST_CHECKED);
  m_IgnoreCase = (IsDlgButtonChecked(IDC_SORT_TOOL_IGNORECASE) == BST_CHECKED);
  UpdateButtons();
}

////////////////////////////////////////////////////////////////////////////////

bool
CScriptSortToolDialog::ShouldSortLines() {
  return m_SortLines;
}

bool
CScriptSortToolDialog::ShouldReverseLines() {
  return m_ReverseLines;
}

bool
CScriptSortToolDialog::ShouldRemoveDuplicateLines() {
  return m_RemoveDuplicateLines;
}

bool
CScriptSortToolDialog::ShouldIgnoreCase() {
  return m_IgnoreCase;
}

////////////////////////////////////////////////////////////////////////////////

