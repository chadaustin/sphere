#include "TilePropertiesDialog.hpp"
#include "TileObstructionDialog.hpp"
#include "../common/Tileset.hpp"
//#include "resource.h"
#include "IDs.hpp"

/*
BEGIN_MESSAGE_MAP(CTilePropertiesDialog, CDialog)

  ON_BN_CLICKED(IDC_ANIMATED, OnAnimatedClicked)
  
  ON_BN_CLICKED(IDC_NEXT,     OnNextTile)
  ON_BN_CLICKED(IDC_PREVIOUS, OnPrevTile)

  ON_BN_CLICKED(IDC_EDIT_OBSTRUCTIONS, OnEditObstructions)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wTilePropertiesDialog, wxDialog)
  EVT_BUTTON(wxID_OK, wTilePropertiesDialog::OnOK)
  EVT_CHECKBOX(wID_TILEPROPERTIES_ANIMATED, wTilePropertiesDialog::OnAnimatedClicked)

  EVT_BUTTON(wID_TILEPROPERTIES_NEXT, wTilePropertiesDialog::OnNextTile)
  EVT_BUTTON(wID_TILEPROPERTIES_PREVIOUS, wTilePropertiesDialog::OnPrevTile)
  EVT_BUTTON(wID_TILEPROPERTIES_EDIT_OBSTRUCTIONS, wTilePropertiesDialog::OnEditObstructions)

END_EVENT_TABLE()


////////////////////////////////////////////////////////////////////////////////

wTilePropertiesDialog::wTilePropertiesDialog(wxWindow *parent, sTileset* tileset, int tile)
: wxDialog(parent, -1, "Tile properties", wxDefaultPosition, wxSize(240, 80), 
           wxDIALOG_MODAL| wxCAPTION | wxTHICK_FRAME | wxSYSTEM_MENU, wxString("wTilePropertiesDialog"))
, m_Tileset(tileset)
, m_Tiles(NULL)
, m_Tile(tile)
{
  // dialog will modify these items
  // they will be set into tileset when OK is pressed
  m_Tiles = new sTile[m_Tileset->GetNumTiles()];
  for (int i = 0; i < m_Tileset->GetNumTiles(); i++)
    m_Tiles[i] = m_Tileset->GetTile(i);

  wxBoxSizer *subsizer = NULL;
  wxBoxSizer *mainsizer = new wxBoxSizer(wxVERTICAL);
  wxFlexGridSizer *gridsizer;

    mainsizer->Add(m_wAnimated = new wxCheckBox(this, wID_TILEPROPERTIES_ANIMATED, wxString("Animated")), 0, wxALL, 10);

    gridsizer = new wxFlexGridSizer(2);
      gridsizer->Add(new wxStaticText(this, -1, wxString("Next frame")), 0, wxALL, 10);
      gridsizer->Add(m_wNextFrame = new wxSpinCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, m_Tileset->GetNumTiles(), m_Tiles[m_Tile].GetNextTile()), 0, wxALL, 10);

      gridsizer->Add(new wxStaticText(this, -1, wxString("Next frame delay")), 0, wxALL, 10);
      gridsizer->Add(m_wNextDelay = new wxSpinCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 1024, m_Tiles[m_Tile].GetDelay()), 0, wxALL, 10);
    mainsizer->Add(gridsizer);
    subsizer = new wxBoxSizer(wxHORIZONTAL);
      subsizer->Add(new wxButton(this, wID_TILEPROPERTIES_EDIT_OBSTRUCTIONS, wxString("Edit Obstructions...")), 0, wxALL, 10);
      subsizer->Add(new wxButton(this, wID_TILEPROPERTIES_NEXT, wxString("Next >>")), 0, wxALL, 10);
      subsizer->Add(new wxButton(this, wID_TILEPROPERTIES_PREVIOUS, wxString("<< Prev")), 0, wxALL, 10);
    mainsizer->Add(subsizer);
    subsizer = new wxBoxSizer(wxHORIZONTAL);
      subsizer->Add(new wxButton(this, wxID_OK, "OK"), 0, wxALL, 10);
      subsizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 10);
    mainsizer->Add(subsizer);

  SetSizer(mainsizer);
  mainsizer->SetSizeHints(this);

  m_wNextFrame->SetValue(m_Tiles[m_Tile].GetNextTile());
  m_wNextDelay->SetValue(m_Tiles[m_Tile].GetDelay());
}

////////////////////////////////////////////////////////////////////////////////

wTilePropertiesDialog::~wTilePropertiesDialog()
{
  delete[] m_Tiles;
}

////////////////////////////////////////////////////////////////////////////////

bool
wTilePropertiesDialog::OnInitDialog(wxInitDialogEvent& event)
{
  UpdateDialog();
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void
wTilePropertiesDialog::OnOK(wxCommandEvent &event)
{
  if (StoreCurrentTile())
  {
    for (int i = 0; i < m_Tileset->GetNumTiles(); i++)
      m_Tileset->GetTile(i) = m_Tiles[i];

    wxDialog::OnOK(event);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wTilePropertiesDialog::OnAnimatedClicked(wxCommandEvent &event)
{
  if (m_wAnimated->GetValue())  // if it's checked
  {
    m_wNextFrame->Enable(TRUE);
    m_wNextDelay->Enable(TRUE);
  }
  else
  {
    m_wNextFrame->Enable(FALSE);
    m_wNextDelay->Enable(FALSE);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wTilePropertiesDialog::OnNextTile(wxCommandEvent &event)
{
  if (StoreCurrentTile())
  {
    m_Tile++;
    if (m_Tile > m_Tileset->GetNumTiles() - 1)
      m_Tile = 0;
    UpdateDialog();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wTilePropertiesDialog::OnPrevTile(wxCommandEvent &event)
{
  if (StoreCurrentTile())
  {
    m_Tile--;
    if (m_Tile < 0)
      m_Tile = m_Tileset->GetNumTiles() - 1;
    UpdateDialog();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wTilePropertiesDialog::OnEditObstructions(wxCommandEvent &event)
{
  wTileObstructionDialog dialog(this, m_Tiles + m_Tile);
  dialog.ShowModal();
}

////////////////////////////////////////////////////////////////////////////////

void
wTilePropertiesDialog::UpdateDialog()
{
  // put default values in
  sTile& tile = m_Tiles[m_Tile];

  // next tile
  m_wNextFrame->SetValue(tile.GetNextTile());

  // delay
  m_wNextDelay->SetValue(tile.GetDelay());

  // animation flag
  if (tile.IsAnimated())
  {
    m_wAnimated->SetValue(TRUE);
    m_wNextFrame->Enable(TRUE);
    m_wNextDelay->Enable(TRUE);
  }
  else
  {
    m_wAnimated->SetValue(FALSE);
    m_wNextFrame->Enable(FALSE);
    m_wNextDelay->Enable(FALSE);
  }

  wxString title;
  title.Format("Tile Properties - %d/%d", m_Tile, m_Tileset->GetNumTiles());
  SetTitle(title);
}

////////////////////////////////////////////////////////////////////////////////

bool
wTilePropertiesDialog::StoreCurrentTile()
{
  bool animated = m_wAnimated->GetValue();

  int nexttile = m_wNextFrame->GetValue();
  int delay    = m_wNextDelay->GetValue();

  if (animated == true &&
      (nexttile < 0 ||
       nexttile >= m_Tileset->GetNumTiles()))
  {
    char error_message[80];
    sprintf(error_message, "Next tile must be between 0 and %d", m_Tileset->GetNumTiles() - 1);
    ::wxMessageBox(error_message, "Next tile out of range", wxOK);
    return false;
  }

  if (animated && delay <= 0)
  {
    ::wxMessageBox("Delay must be greater than zero", "Delay too small", wxOK);
    return false;
  }

  // put default values in
  m_Tiles[m_Tile].SetAnimated(animated);
  m_Tiles[m_Tile].SetNextTile(nexttile);
  m_Tiles[m_Tile].SetDelay(delay);

  return true;
}

////////////////////////////////////////////////////////////////////////////////
