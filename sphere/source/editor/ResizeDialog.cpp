#include "ResizeDialog.hpp"
#include "resource.h"


////////////////////////////////////////////////////////////////////////////////

CResizeDialog::CResizeDialog(const char* caption, int default_width, int default_height)
: CDialog(IDD_RESIZE)
, m_Caption(caption)
, m_Width(default_width)
, m_Height(default_height)
, m_MinWidth(0)
, m_MaxWidth(65535)
, m_MinHeight(0)
, m_MaxHeight(65536)
{
}

////////////////////////////////////////////////////////////////////////////////

void
CResizeDialog::SetRange(int min_width, int max_width, int min_height, int max_height)
{
  m_MinWidth  = min_width;
  m_MaxWidth  = max_width;
  m_MinHeight = min_height;
  m_MaxHeight = max_height;
}

////////////////////////////////////////////////////////////////////////////////

int
CResizeDialog::GetWidth() const
{
  return m_Width;
}

////////////////////////////////////////////////////////////////////////////////

int
CResizeDialog::GetHeight() const
{
  return m_Height;
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CResizeDialog::OnInitDialog()
{
  SetWindowText(m_Caption.c_str());
  SetDlgItemInt(IDC_WIDTH,  m_Width);
  SetDlgItemInt(IDC_HEIGHT, m_Height);
  GetDlgItem(IDC_WIDTH)->SetFocus();
  ((CEdit*)GetDlgItem(IDC_WIDTH))->SetSel(0, -1);
 
  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

void
CResizeDialog::OnOK()
{
  int w = GetDlgItemInt(IDC_WIDTH);
  int h = GetDlgItemInt(IDC_HEIGHT);

  if (w < m_MinWidth || w > m_MaxWidth ||
      h < m_MinHeight || h > m_MaxHeight)
  {
    char message[520];
    sprintf(message,
      "Width must be between %d and %d.\n"
      "Height must be between %d and %d.",
      m_MinWidth, m_MaxWidth,
      m_MinHeight, m_MaxHeight);
    MessageBox(message, "Resize");
    return;
  }

  m_Width  = GetDlgItemInt(IDC_WIDTH);
  m_Height = GetDlgItemInt(IDC_HEIGHT);
  CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////////
