#ifndef ENTITY_TRIGGER_DIALOG_HPP
#define ENTITY_TRIGGER_DIALOG_HPP


#include <afxwin.h>
#include "../common/Entities.hpp"


class CEntityTriggerDialog : public CDialog
{
public:
  CEntityTriggerDialog(sTriggerEntity& trigger);

private:
  BOOL OnInitDialog();
  void OnOK();

  afx_msg void OnCheckSyntax();

private:
  sTriggerEntity& m_Trigger;

  DECLARE_MESSAGE_MAP()
};


#endif
