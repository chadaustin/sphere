#include "LineSorterDialog.hpp"
#include "resource.h"

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CLineSorterDialog, CDialog)

  ON_BN_CLICKED(IDC_LINESORT_SORTLINES,        OnOptionChanged)
  ON_BN_CLICKED(IDC_LINESORT_REVERSELINES,     OnOptionChanged)
  ON_BN_CLICKED(IDC_LINESORT_DELETEDUPLICATES, OnOptionChanged)
  ON_BN_CLICKED(IDC_LINESORT_IGNORECASE,       OnOptionChanged)

END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

CLineSorterDialog::CLineSorterDialog()
: CDialog(IDD_LINE_SORTER)
{
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CLineSorterDialog::OnInitDialog()
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
CLineSorterDialog::OnOK()
{
  CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////////

void
CLineSorterDialog::UpdateButtons()
{
  BOOL enabled = TRUE;

  CheckDlgButton(IDC_LINESORT_SORTLINES,        m_SortLines    ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(IDC_LINESORT_REVERSELINES,     m_ReverseLines ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(IDC_LINESORT_DELETEDUPLICATES, m_RemoveDuplicateLines ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(IDC_LINESORT_IGNORECASE,       m_IgnoreCase   ? BST_CHECKED : BST_UNCHECKED);


  if (IsDlgButtonChecked(IDC_LINESORT_SORTLINES) != BST_CHECKED
   && IsDlgButtonChecked(IDC_LINESORT_REVERSELINES) != BST_CHECKED
   && IsDlgButtonChecked(IDC_LINESORT_DELETEDUPLICATES) != BST_CHECKED) {
    enabled = FALSE;
  }

  if (GetDlgItem(IDOK)) {
    GetDlgItem(IDOK)->EnableWindow(enabled); 
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CLineSorterDialog::OnOptionChanged()
{
  m_SortLines    = (IsDlgButtonChecked(IDC_LINESORT_SORTLINES)    == BST_CHECKED);
  m_ReverseLines = (IsDlgButtonChecked(IDC_LINESORT_REVERSELINES) == BST_CHECKED);
  m_RemoveDuplicateLines = (IsDlgButtonChecked(IDC_LINESORT_DELETEDUPLICATES) == BST_CHECKED);
  m_IgnoreCase   = (IsDlgButtonChecked(IDC_LINESORT_IGNORECASE)   == BST_CHECKED);
  UpdateButtons();
}

////////////////////////////////////////////////////////////////////////////////

bool
CLineSorterDialog::ShouldSortLines() {
  return m_SortLines;
}

bool
CLineSorterDialog::ShouldReverseLines() {
  return m_ReverseLines;
}

bool
CLineSorterDialog::ShouldRemoveDuplicateLines() {
  return m_RemoveDuplicateLines;
}

bool
CLineSorterDialog::ShouldIgnoreCase() {
  return m_IgnoreCase;
}

////////////////////////////////////////////////////////////////////////////////

