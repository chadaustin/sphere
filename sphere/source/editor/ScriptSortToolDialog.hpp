#ifndef SCRIPT_SORT_TOOL_DIALOG_HPP
#define SCRIPT_SORT_TOOL_DIALOG_HPP

#include <afxwin.h>

///////////////////////////////////////////////////////////

class CScriptSortToolDialog : public CDialog
{
public:
  CScriptSortToolDialog();

  bool ShouldSortLines();
  bool ShouldReverseLines();
  bool ShouldRemoveDuplicateLines();

private:
  bool m_SortLines;
  bool m_ReverseLines;
  bool m_RemoveDuplicateLines;

private:
  void UpdateButtons();

private:
  afx_msg void OnSortOptionChanged();

private:
  BOOL OnInitDialog();
  void OnOK();

  DECLARE_MESSAGE_MAP()
};

///////////////////////////////////////////////////////////

#endif
