#include "ZoneEditDialog.hpp"
#include "Scripting.hpp"
#include "resource.h"


BEGIN_MESSAGE_MAP(CZoneEditDialog, CDialog)

  ON_BN_CLICKED(IDC_CHECK_SYNTAX, OnCheckSyntax)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CZoneEditDialog::CZoneEditDialog(sMap::sZone& zone)
: CDialog(IDD_ZONE_EDIT)
, m_Zone(zone)
{
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CZoneEditDialog::OnInitDialog()
{
  SetDlgItemText(IDC_SCRIPT, m_Zone.script.c_str());
  SetDlgItemInt(IDC_STEPS, m_Zone.reactivate_in_num_steps, FALSE);
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void
CZoneEditDialog::OnOK()
{
  CString script;
  GetDlgItemText(IDC_SCRIPT, script);
  m_Zone.reactivate_in_num_steps = GetDlgItemInt(IDC_STEPS, NULL, FALSE);
  m_Zone.script = script;
  
  CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CZoneEditDialog::OnCheckSyntax()
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
