#include "TileObstructionDialog.hpp"
#include "resource.h"
#include "IDs.hpp"

/*
BEGIN_MESSAGE_MAP(CTileObstructionDialog, CDialog)

  ON_WM_SIZE()

  ON_COMMAND(IDC_PRESETS, OnPresets)
  
  ON_COMMAND(ID_OBSTRUCTIONPRESETS_UNBLOCKED,  OnPresetUnblocked)
  ON_COMMAND(ID_OBSTRUCTIONPRESETS_BLOCKED,    OnPresetBlocked)
  ON_COMMAND(ID_OBSTRUCTIONPRESETS_UPPERRIGHT, OnPresetUpperRight)
  ON_COMMAND(ID_OBSTRUCTIONPRESETS_LOWERRIGHT, OnPresetLowerRight)
  ON_COMMAND(ID_OBSTRUCTIONPRESETS_LOWERLEFT,  OnPresetLowerLeft)
  ON_COMMAND(ID_OBSTRUCTIONPRESETS_UPPERLEFT,  OnPresetUpperLeft)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wTileObstructionDialog, wxDialog)
  EVT_BUTTON(wxID_OK, wTileObstructionDialog::OnOK)
  EVT_SIZE(wTileObstructionDialog::OnSize)

  EVT_BUTTON(wID_TILEOBSTRUCTION_PRESETS, wTileObstructionDialog::OnPresets)

  EVT_MENU(wID_TILEOBSTRUCTION_PRESET_UNBLOCKED,       wTileObstructionDialog::OnPresetUnblocked)
  EVT_MENU(wID_TILEOBSTRUCTION_PRESET_BLOCKED,         wTileObstructionDialog::OnPresetBlocked)
  EVT_MENU(wID_TILEOBSTRUCTION_PRESET_UPPER_LEFT,      wTileObstructionDialog::OnPresetUpperLeft)
  EVT_MENU(wID_TILEOBSTRUCTION_PRESET_UPPER_RIGHT,     wTileObstructionDialog::OnPresetUpperRight)
  EVT_MENU(wID_TILEOBSTRUCTION_PRESET_LOWER_LEFT,      wTileObstructionDialog::OnPresetLowerLeft)
  EVT_MENU(wID_TILEOBSTRUCTION_PRESET_LOWER_RIGHT,     wTileObstructionDialog::OnPresetLowerRight)

END_EVENT_TABLE()


////////////////////////////////////////////////////////////////////////////////

wTileObstructionDialog::wTileObstructionDialog(wxWindow *parent, sTile* tile)
: wxDialog(parent, -1, "Tile Obstruction", wxDefaultPosition, wxSize(320,240), wxDIALOG_MODAL | wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU, "wTileObstructionDialog")
, m_tile(tile)
, m_edit_tile(*tile) 
{
  m_obstruction_view = new wTileObstructionView(this, &m_edit_tile);
  m_OK = new wxButton(this, wxOK, wxString("OK"));
  m_Cancel = new wxButton(this, wxCANCEL, wxString("Cancel"));
  m_Presets = new wxButton(this, wID_TILEOBSTRUCTION_PRESETS, wxString("Presets"));
  //m_edit_tile = *tile;
  m_PopupMenu = new wxMenu("TileObstruction");
  m_PopupMenu->Append(wID_TILEOBSTRUCTION_PRESET_UNBLOCKED,    "Unblocked",      "Clear tile of obstructions");
  m_PopupMenu->Append(wID_TILEOBSTRUCTION_PRESET_BLOCKED,      "Blocked",        "Block entire tile");
  m_PopupMenu->Append(wID_TILEOBSTRUCTION_PRESET_UPPER_LEFT,   "Upper Left",     "Block upper left quarter");
  m_PopupMenu->Append(wID_TILEOBSTRUCTION_PRESET_UPPER_RIGHT,  "Upper Right",    "Block upper right quarter");
  m_PopupMenu->Append(wID_TILEOBSTRUCTION_PRESET_LOWER_LEFT,   "Lower Left",     "Block lower left quarter");
  m_PopupMenu->Append(wID_TILEOBSTRUCTION_PRESET_LOWER_RIGHT,  "Lower Right",    "Block lower right quarter");
}

////////////////////////////////////////////////////////////////////////////////

wTileObstructionDialog::~wTileObstructionDialog()
{
  delete m_PopupMenu;
}

////////////////////////////////////////////////////////////////////////////////

bool
wTileObstructionDialog::OnInitDialog(wxInitDialogEvent& event)
{
  //m_obstruction_view.Create(this, &m_edit_tile);
  
  //todo:
  //wxRect rect = GetRect();
  //OnSize(0, rect.right, rect.bottom);

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void
wTileObstructionDialog::OnOK(wxCommandEvent &event)
{
  *m_tile = m_edit_tile;
  wxDialog::OnOK(event);
}

////////////////////////////////////////////////////////////////////////////////

void
wTileObstructionDialog::OnSize(wxSizeEvent &event)
{
  const int button_width  = 60;
  const int button_height = 30;
  int cx = event.GetSize().GetWidth();
  int cy = event.GetSize().GetHeight();
  // move the controls around
  if (m_obstruction_view != NULL) {
    m_obstruction_view->SetSize(0, 0, cx - button_width, cy);
  }

  if (m_OK != NULL) {
    m_OK->SetSize(cx - button_width, 0, button_width, button_height);
  }

  if (m_Cancel != NULL) {
    m_Cancel->SetSize(cx - button_width, button_height, button_width, button_height);
  }

  if (m_Presets != NULL) {
    m_Presets->SetSize(cx - button_width, button_height * 3, button_width, button_height);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wTileObstructionDialog::OnPresets(wxCommandEvent &event)
{
  int cx;
  int cy;
  ::wxGetMousePosition(&cx, &cy);
  PopupMenu(m_PopupMenu, cx, cy);

/*
  // get handle to button
  CWnd* button = GetDlgItem(IDC_PRESETS);
  
  // get coordinates of button
  RECT rect;
  button->GetWindowRect(&rect);

  HMENU obstruction_presets = LoadMenu(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_OBSTRUCTION_PRESETS));
  HMENU menu = GetSubMenu(obstruction_presets, 0);
  TrackPopupMenu(
    menu,
    TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON,
    rect.left,
    rect.bottom,
    0,
    m_hWnd,
    NULL
  );
*/
}

////////////////////////////////////////////////////////////////////////////////

void
wTileObstructionDialog::OnPresetUnblocked(wxEvent &event)
{
  sObstructionMap s;
  m_edit_tile.GetObstructionMap() = s;

  m_obstruction_view->Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wTileObstructionDialog::OnPresetBlocked(wxEvent &event)
{
  int w = m_tile->GetWidth()  - 1;
  int h = m_tile->GetHeight() - 1;

  sObstructionMap s;
  s.AddSegment(0, 0, w, 0);
  s.AddSegment(w, 0, w, h);
  s.AddSegment(0, h, w, h);
  s.AddSegment(0, 0, 0, h);
  m_edit_tile.GetObstructionMap() = s;

  m_obstruction_view->Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wTileObstructionDialog::OnPresetUpperRight(wxEvent &event)
{
  int w = m_tile->GetWidth()  - 1;
  int h = m_tile->GetHeight() - 1;

  sObstructionMap s;
  s.AddSegment(0, 0, w, 0);
  s.AddSegment(w, 0, w, h);
  s.AddSegment(0, 0, w, h);
  m_edit_tile.GetObstructionMap() = s;

  m_obstruction_view->Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wTileObstructionDialog::OnPresetLowerRight(wxEvent &event)
{
  int w = m_tile->GetWidth()  - 1;
  int h = m_tile->GetHeight() - 1;

  sObstructionMap s;
  s.AddSegment(w, 0, w, h);
  s.AddSegment(0, h, w, h);
  s.AddSegment(0, h, w, 0);
  m_edit_tile.GetObstructionMap() = s;

  m_obstruction_view->Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wTileObstructionDialog::OnPresetLowerLeft(wxEvent &event)
{
  int w = m_tile->GetWidth()  - 1;
  int h = m_tile->GetHeight() - 1;

  sObstructionMap s;
  s.AddSegment(0, 0, 0, h);
  s.AddSegment(0, h, w, h);
  s.AddSegment(0, 0, w, h);
  m_edit_tile.GetObstructionMap() = s;

  m_obstruction_view->Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wTileObstructionDialog::OnPresetUpperLeft(wxEvent &event)
{
  int w = m_tile->GetWidth()  - 1;
  int h = m_tile->GetHeight() - 1;

  sObstructionMap s;
  s.AddSegment(0, 0, w, 0);
  s.AddSegment(0, 0, 0, h);
  s.AddSegment(0, h, w, 0);
  m_edit_tile.GetObstructionMap() = s;

  m_obstruction_view->Refresh();
}

////////////////////////////////////////////////////////////////////////////////
