#include "TileObstructionDialog.hpp"
#include "resource.h"


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


////////////////////////////////////////////////////////////////////////////////

CTileObstructionDialog::CTileObstructionDialog(sTile* tile)
: CDialog(IDD_TILE_OBSTRUCTION_DIALOG)
, m_tile(tile)
{
  m_edit_tile = *tile;
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CTileObstructionDialog::OnInitDialog()
{
  m_obstruction_view.Create(this, &m_edit_tile);
  
  RECT rect;
  GetClientRect(&rect);
  OnSize(0, rect.right, rect.bottom);

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void
CTileObstructionDialog::OnOK()
{
  *m_tile = m_edit_tile;
  CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTileObstructionDialog::OnSize(UINT type, int cx, int cy)
{
  const int button_width  = 60;
  const int button_height = 30;

  // move the controls around
  if (m_obstruction_view.m_hWnd) {
    m_obstruction_view.MoveWindow(0, 0, cx - button_width, cy);
  }

  if (GetDlgItem(IDOK)) {
    GetDlgItem(IDOK)->MoveWindow(cx - button_width, 0, button_width, button_height);
  }

  if (GetDlgItem(IDCANCEL)) {
    GetDlgItem(IDCANCEL)->MoveWindow(cx - button_width, button_height, button_width, button_height);
  }

  if (GetDlgItem(IDC_PRESETS)) {
    GetDlgItem(IDC_PRESETS)->MoveWindow(cx - button_width, button_height * 3, button_width, button_height);
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTileObstructionDialog::OnPresets()
{
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
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTileObstructionDialog::OnPresetUnblocked()
{
  sObstructionMap s;
  m_edit_tile.GetObstructionMap() = s;

  m_obstruction_view.Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTileObstructionDialog::OnPresetBlocked()
{
  int w = m_tile->GetWidth()  - 1;
  int h = m_tile->GetHeight() - 1;

  sObstructionMap s;
  s.AddSegment(0, 0, w, 0);
  s.AddSegment(w, 0, w, h);
  s.AddSegment(0, h, w, h);
  s.AddSegment(0, 0, 0, h);
  m_edit_tile.GetObstructionMap() = s;

  m_obstruction_view.Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTileObstructionDialog::OnPresetUpperRight()
{
  int w = m_tile->GetWidth()  - 1;
  int h = m_tile->GetHeight() - 1;

  sObstructionMap s;
  s.AddSegment(0, 0, w, 0);
  s.AddSegment(w, 0, w, h);
  s.AddSegment(0, 0, w, h);
  m_edit_tile.GetObstructionMap() = s;

  m_obstruction_view.Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTileObstructionDialog::OnPresetLowerRight()
{
  int w = m_tile->GetWidth()  - 1;
  int h = m_tile->GetHeight() - 1;

  sObstructionMap s;
  s.AddSegment(w, 0, w, h);
  s.AddSegment(0, h, w, h);
  s.AddSegment(0, h, w, 0);
  m_edit_tile.GetObstructionMap() = s;

  m_obstruction_view.Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTileObstructionDialog::OnPresetLowerLeft()
{
  int w = m_tile->GetWidth()  - 1;
  int h = m_tile->GetHeight() - 1;

  sObstructionMap s;
  s.AddSegment(0, 0, 0, h);
  s.AddSegment(0, h, w, h);
  s.AddSegment(0, 0, w, h);
  m_edit_tile.GetObstructionMap() = s;

  m_obstruction_view.Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTileObstructionDialog::OnPresetUpperLeft()
{
  int w = m_tile->GetWidth()  - 1;
  int h = m_tile->GetHeight() - 1;

  sObstructionMap s;
  s.AddSegment(0, 0, w, 0);
  s.AddSegment(0, 0, 0, h);
  s.AddSegment(0, h, w, 0);
  m_edit_tile.GetObstructionMap() = s;

  m_obstruction_view.Invalidate();
}

////////////////////////////////////////////////////////////////////////////////
