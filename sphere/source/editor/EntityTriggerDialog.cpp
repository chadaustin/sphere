#include "EntityTriggerDialog.hpp"
#include "Scripting.hpp"
#include "resource.h"


BEGIN_MESSAGE_MAP(CEntityTriggerDialog, CDialog)

  ON_BN_CLICKED(IDC_CHECK_SYNTAX, OnCheckSyntax)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CEntityTriggerDialog::CEntityTriggerDialog(sTriggerEntity& trigger)
: CDialog(IDD_ENTITY_TRIGGER)
, m_Trigger(trigger)
{
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CEntityTriggerDialog::OnInitDialog()
{
  SetDlgItemText(IDC_SCRIPT, m_Trigger.script.c_str());
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void
CEntityTriggerDialog::OnOK()
{
  CString script;
  GetDlgItemText(IDC_SCRIPT, script);
  m_Trigger.script = script;
  
  CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CEntityTriggerDialog::OnCheckSyntax()
{
  CString script;
  GetDlgItemText(IDC_SCRIPT, script);

  // check for errors
  sCompileError error;
  if (VerifyScript(script, error)) {
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
