#include "NewMapDialog.hpp"
#include "Project.hpp"
#include "FileDialogs.hpp"
#include "resource.h"


BEGIN_MESSAGE_MAP(CNewMapDialog, CDialog)

  ON_COMMAND(IDC_TILESET_BROWSE, OnTilesetBrowse)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CNewMapDialog::CNewMapDialog()
: CDialog(IDD_NEW_MAP)
, m_MapWidth(0)
, m_MapHeight(0)
{
}

////////////////////////////////////////////////////////////////////////////////

int
CNewMapDialog::GetMapWidth() const
{
  return m_MapWidth;
}

////////////////////////////////////////////////////////////////////////////////

int
CNewMapDialog::GetMapHeight() const
{
  return m_MapHeight;
}

////////////////////////////////////////////////////////////////////////////////

const char*
CNewMapDialog::GetTileset() const
{
  return m_Tileset.c_str();
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CNewMapDialog::OnInitDialog()
{
  CDialog::OnInitDialog();

  // put default values into the edit boxes
  SetDlgItemInt(IDC_WIDTH, 64);
  SetDlgItemInt(IDC_HEIGHT, 64);

  // set the focus and selection and tell the dialog not to set the focus
  GetDlgItem(IDC_WIDTH)->SetFocus();
  ((CEdit*)GetDlgItem(IDC_WIDTH))->SetSel(0, -1);
  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

void
CNewMapDialog::OnOK()
{
  int width  = GetDlgItemInt(IDC_WIDTH);
  int height = GetDlgItemInt(IDC_HEIGHT);
  char tileset[MAX_PATH];
  GetDlgItemText(IDC_TILESET, tileset, MAX_PATH);

  if (width < 1 || height < 1)
  {
    MessageBox("Width and height must be at least 1.");
    return;
  }

  if (strlen(tileset) == 0)
  {
    if (MessageBox("Are you sure you want to use an empty tileset?", NULL, MB_YESNO) == IDNO)
      return;
  }

  m_MapWidth  = width;
  m_MapHeight = height;
  m_Tileset   = tileset;

  CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CNewMapDialog::OnTilesetBrowse()
{
  CTilesetFileDialog dialog(FDM_OPEN);
  if (dialog.DoModal() == IDOK)
    SetDlgItemText(IDC_TILESET, dialog.GetPathName());
}

////////////////////////////////////////////////////////////////////////////////