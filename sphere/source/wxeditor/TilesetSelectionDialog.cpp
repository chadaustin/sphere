#include "TilesetSelectionDialog.hpp"
#include "FileDialogs.hpp"
#include "Project.hpp"
//#include "resource.h"
#include "IDs.hpp"


/*
BEGIN_MESSAGE_MAP(CTilesetSelectionDialog, CDialog)

  ON_COMMAND(IDC_TILESET_BROWSE, OnBrowseForTileset)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wTilesetSelectionDialog, wxDialog)
  EVT_BUTTON(wxID_OK, wTilesetSelectionDialog::OnOK)
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wTilesetSelectionDialog::wTilesetSelectionDialog(wxWindow *parent)
: wxDialog(parent, -1, "Tileset Selection", wxDefaultPosition, wxSize(240, 80), 
           wxDIALOG_MODAL| wxCAPTION | wxTHICK_FRAME | wxSYSTEM_MENU)
{
  wxBoxSizer *mainsizer;
  wxBoxSizer *subsizer;
  mainsizer = new wxBoxSizer(wxVERTICAL);

    mainsizer->Add(new wxStaticText(this, -1, "Map has no tileset or and invalid tileset associated with it.  Choose one."), 0, wxALL, 10);

    subsizer = new wxBoxSizer(wxHORIZONTAL);
      subsizer->Add(m_TilesetCtrl = new wxTextCtrl(this, -1, ""), 0, wxALL, 10);
      subsizer->Add(new wxButton(this, wID_TILESETSELECTIONDIALOG_BROWSE, "..."), 0, wxALL, 10);
    mainsizer->Add(subsizer);

    subsizer = new wxBoxSizer(wxHORIZONTAL);
      subsizer->Add(new wxButton(this, wxID_OK, "OK"), 0, wxALL, 10);
      subsizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 10);
    mainsizer->Add(subsizer);

  SetSizer(mainsizer);

  mainsizer->SetSizeHints(this);
}

////////////////////////////////////////////////////////////////////////////////

wTilesetSelectionDialog::~wTilesetSelectionDialog()
{
}

/*
////////////////////////////////////////////////////////////////////////////////

BOOL
CTilesetSelectionDialog::OnInitDialog()
{
  return TRUE;
}
*/

////////////////////////////////////////////////////////////////////////////////

const char*
wTilesetSelectionDialog::GetTilesetPath() const
{
  return m_SelectedTileset.c_str();
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetSelectionDialog::OnOK(wxCommandEvent &event)
{

  m_SelectedTileset = m_TilesetCtrl->GetValue();

  if (m_SelectedTileset.length() == 0)
    ::wxMessageBox("You must select a tileset");
  else
    wxDialog::OnOK(event);
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetSelectionDialog::OnBrowseForTileset(wxCommandEvent &event)
{
  wTilesetFileDialog dialog(this, FDM_OPEN);
  if (dialog.ShowModal() == wxID_OK)
    m_TilesetCtrl->SetValue(dialog.GetPath());
}

////////////////////////////////////////////////////////////////////////////////
