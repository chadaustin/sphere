#include "ZoneEditDialog.hpp"
#include "Scripting.hpp"
#include "resource.h"


BEGIN_MESSAGE_MAP(CZoneEditDialog, CDialog)

  ON_BN_CLICKED(IDC_CHECK_SYNTAX, OnCheckSyntax)

END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

static inline std::string itos(int i)
{
  char s[20];
  sprintf(s, "%d", i);
  return s;
}

////////////////////////////////////////////////////////////////////////////////

CZoneEditDialog::CZoneEditDialog(sMap::sZone& zone, sMap* map)
: CDialog(IDD_ZONE_EDIT)
, m_Zone(zone)
, m_Map(map)
{
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CZoneEditDialog::OnInitDialog()
{
  SetDlgItemText(IDC_SCRIPT, m_Zone.script.c_str());
  SetDlgItemInt(IDC_STEPS, m_Zone.reactivate_in_num_steps, FALSE);

  // add layer names in "layer_index - layer_name" style to dropdown layer list
  for (int i = 0; i < m_Map->GetNumLayers(); i++) {
    std::string layer_info = itos(i) + " - " + m_Map->GetLayer(i).GetName();
    SendDlgItemMessage(IDC_LAYER, CB_ADDSTRING, 0, (LPARAM)layer_info.c_str());
    if (i == m_Zone.layer)
      SendDlgItemMessage(IDC_LAYER, CB_SETCURSEL, m_Zone.layer);
  }
  
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
  
  m_Zone.layer = SendDlgItemMessage(IDC_LAYER, CB_GETCURSEL);

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
