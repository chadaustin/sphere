#include "NumberDialog.hpp"
#include "resource.h"


////////////////////////////////////////////////////////////////////////////////

CNumberDialog::CNumberDialog(const char* title, const char* text, int value, int min, int max)
: CDialog(IDD_NUMBER)
, m_Title(title)
, m_Text(text)

, m_Value(value)

, m_Min(min)
, m_Max(max)
{
}

////////////////////////////////////////////////////////////////////////////////

int
CNumberDialog::GetValue() const
{
  return m_Value;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg BOOL
CNumberDialog::OnInitDialog()
{
  CDialog::OnInitDialog();

  // get pointers to all dialog controls
  m_Static = (CStatic*)GetDlgItem(IDC_NUMBERTEXT);
  m_NumberEdit = (CEdit*)GetDlgItem(IDC_NUMBER);

  // initialize the dialog box
  SetWindowText(m_Title.c_str());
  m_Static->SetWindowText(m_Text.c_str());

  m_NumberEdit->SetFocus();

  char number[80];
  sprintf(number, "%d", m_Value);
  m_NumberEdit->SetWindowText(number);

  m_NumberEdit->SetSel(0, -1);

  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CNumberDialog::OnOK()
{
  CString text;
  m_NumberEdit->GetWindowText(text);
  int value = atoi(text);

  int prefix = 0;
  int digits = 0;

  // count up how many digits there are in the string and if there's + or - prefix
  for (int i = 0; i < strlen(text); i++) {
    if (isdigit(text[i])) {
      digits++;
    }
    else
    if (i == 0) {
      if (text[i] == '+' || text[i] == '-') {
        prefix = 1;
      }
    }
  }

  if (digits == 0 || strlen(text) - digits - prefix != 0) {
    MessageBox("Invalid number format", m_Title.c_str());
  }
  else
  if (value < m_Min || value > m_Max)
  {
    char string[80];
    sprintf(string, "Value must be between %d and %d", m_Min, m_Max);
    MessageBox(string, m_Title.c_str());
  }
  else
  {
    m_Value = value;
    CDialog::OnOK();
  }
}

////////////////////////////////////////////////////////////////////////////////
