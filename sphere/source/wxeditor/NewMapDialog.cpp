#include "NewMapDialog.hpp"
#include "Project.hpp"
#include "FileDialogs.hpp"
//#include "resource.h"
#include "IDs.hpp"


/*
BEGIN_MESSAGE_MAP(CNewMapDialog, CDialog)

  ON_COMMAND(IDC_TILESET_BROWSE, OnTilesetBrowse)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wNewMapDialog, wxDialog)
  EVT_BUTTON(wxID_OK, wNewMapDialog::OnOK)
  EVT_BUTTON(wID_TILESETDIALOG_BROWSE, wNewMapDialog::OnTilesetBrowse)
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wNewMapDialog::wNewMapDialog(wxWindow *parent)
: wxDialog(parent, -1, "New Map", wxDefaultPosition, wxSize(208, 72), wxDIALOG_MODAL | wxCAPTION | wxSYSTEM_MENU)
, m_MapWidth(0)
, m_MapHeight(0)
{
  wxBoxSizer *mainsizer;
  wxFlexGridSizer *gridsizer;
  wxBoxSizer *subsizer;

  mainsizer = new wxBoxSizer(wxVERTICAL);
    gridsizer = new wxFlexGridSizer(2);

      gridsizer->Add(new wxStaticText(this, -1, "Map Width"), 0, wxALL, 10);
      gridsizer->Add(m_WidthCtrl = new wxSpinCtrl(this, -1, "64", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 1024, 64), 0, wxALL, 10);

      gridsizer->Add(new wxStaticText(this, -1, "Map Width"), 0, wxALL, 10);
      gridsizer->Add(m_HeightCtrl = new wxSpinCtrl(this, -1, "64", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 1024, 64), 0, wxALL, 10);

      gridsizer->Add(new wxStaticText(this, -1, "Tileset"), 0, wxALL, 10);
      subsizer = new wxBoxSizer(wxHORIZONTAL);
        subsizer->Add(m_TilesetCtrl = new wxTextCtrl(this, -1, ""), 0, wxALL, 10);
        subsizer->Add(new wxButton(this, wID_TILESETDIALOG_BROWSE, "..."), 0, wxALL, 10);
      gridsizer->Add(subsizer);
    mainsizer->Add(gridsizer);
    subsizer = new wxBoxSizer(wxHORIZONTAL);
      subsizer->Add(new wxButton(this, wxID_OK, "OK"), 0, wxALL, 10);
      subsizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 10);
    mainsizer->Add(subsizer);
  SetAutoLayout(TRUE);
  SetSizer(mainsizer);
  mainsizer->SetSizeHints(this);

}

////////////////////////////////////////////////////////////////////////////////

int
wNewMapDialog::GetMapWidth() const
{
  return m_MapWidth;
}

////////////////////////////////////////////////////////////////////////////////

int
wNewMapDialog::GetMapHeight() const
{
  return m_MapHeight;
}

////////////////////////////////////////////////////////////////////////////////

const char*
wNewMapDialog::GetTileset() const
{
  return m_Tileset.c_str();
}

/*
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
*/

////////////////////////////////////////////////////////////////////////////////

void
wNewMapDialog::OnOK(wxCommandEvent &event)
{
  int width  = m_WidthCtrl->GetValue();
  int height = m_HeightCtrl->GetValue();
  wxString tileset = m_TilesetCtrl->GetValue();

  if (width < 1 || height < 1)
  {
    ::wxMessageBox("Width and height must be at least 1.");
    return;
  }

  if (tileset.length() == 0)
  {
    if (::wxMessageBox("Are you sure you want to use an empty tileset?", "No tileset!", wxYES_NO) == wxNO)
      return;
  }

  m_MapWidth  = width;
  m_MapHeight = height;
  m_Tileset   = tileset.c_str();

  wxDialog::OnOK(event);
}

////////////////////////////////////////////////////////////////////////////////

void
wNewMapDialog::OnTilesetBrowse(wxCommandEvent &event)
{
  wTilesetFileDialog dialog(this, FDM_OPEN);
  if (dialog.ShowModal() == wxID_OK)
    //SetDlgItemText(IDC_TILESET, dialog.GetPathName());
    m_TilesetCtrl->SetValue(dialog.GetPath());
}

////////////////////////////////////////////////////////////////////////////////
