#include "TilesetSelectionDialog.hpp"
#include "FileDialogs.hpp"
#include "Project.hpp"
#include "resource.h"


BEGIN_MESSAGE_MAP(CTilesetSelectionDialog, CDialog)

  ON_COMMAND(IDC_TILESET_BROWSE, OnBrowseForTileset)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CTilesetSelectionDialog::CTilesetSelectionDialog()
: CDialog(IDD_TILESET_SELECTION)
, m_pSelectedTileset(NULL)
{
}

////////////////////////////////////////////////////////////////////////////////

CTilesetSelectionDialog::~CTilesetSelectionDialog()
{
  delete[] m_pSelectedTileset;
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CTilesetSelectionDialog::OnInitDialog()
{
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

const char*
CTilesetSelectionDialog::GetTilesetPath() const
{
  return m_pSelectedTileset;
}

////////////////////////////////////////////////////////////////////////////////

void
CTilesetSelectionDialog::OnOK()
{
  // free the old string if one exists
  delete[] m_pSelectedTileset;

  m_pSelectedTileset = new char[MAX_PATH];
  GetDlgItemText(IDC_TILESET, m_pSelectedTileset, MAX_PATH);

  if (strlen(m_pSelectedTileset) == 0)
    MessageBox("You must select a tileset");
  else
    CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetSelectionDialog::OnBrowseForTileset()
{
  CTilesetFileDialog dialog(FDM_OPEN);
  if (dialog.DoModal() == IDOK)
    SetDlgItemText(IDC_TILESET, dialog.GetPathName());
}

////////////////////////////////////////////////////////////////////////////////
