#ifndef ENTITY_PERSON_DIALOG_HPP
#define ENTITY_PERSON_DIALOG_HPP


#include <afxwin.h>
#include "../common/Entities.hpp"


class CEntityPersonDialog : public CDialog
{
public:
  CEntityPersonDialog(sPersonEntity& person);

private:
  BOOL OnInitDialog();
  void OnOK();

  afx_msg void OnBrowseSpriteset();
  afx_msg void OnCheckSyntax();
  afx_msg void OnScriptChanged();

  void SetScript();
  void StoreScript();

private:
  sPersonEntity& m_Person;
  
  int     m_CurrentScript;
  CString m_Scripts[5];

  DECLARE_MESSAGE_MAP()
};


#endif
