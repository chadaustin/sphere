#ifndef LINE_SORTER_DIALOG_HPP
#define LINE_SORTER_DIALOG_HPP

#include <afxwin.h>

///////////////////////////////////////////////////////////

class CLineSorterDialog : public CDialog
{
public:
  CLineSorterDialog();

  bool ShouldSortLines();
  bool ShouldReverseLines();
  bool ShouldRemoveDuplicateLines();
  bool ShouldIgnoreCase();

private:
  bool m_SortLines;
  bool m_ReverseLines;
  bool m_RemoveDuplicateLines;
  bool m_IgnoreCase;

private:
  void UpdateButtons();

private:
  afx_msg void OnOptionChanged();

private:
  BOOL OnInitDialog();
  void OnOK();

  DECLARE_MESSAGE_MAP()
};

///////////////////////////////////////////////////////////

#endif
