#ifndef TILE_OBSTRUCTION_DIALOG_HPP
#define TILE_OBSTRUCTION_DIALOG_HPP


//#include <afxwin.h>
#include <wx/wx.h>
#include "TileObstructionView.hpp"
#include "../common/Tile.hpp"


class wTileObstructionDialog : public wxDialog
{
public:
  wTileObstructionDialog(wxWindow *parent, sTile* tile);
  ~wTileObstructionDialog();

private:
  virtual bool OnInitDialog(wxInitDialogEvent& event);
  virtual void OnOK(wxCommandEvent &event);

  void OnSize(wxSizeEvent &event);
  void OnPresets(wxCommandEvent &event);

  void OnPresetUnblocked(wxEvent &event);
  void OnPresetBlocked(wxEvent &event);
  void OnPresetUpperRight(wxEvent &event);
  void OnPresetLowerRight(wxEvent &event);
  void OnPresetLowerLeft(wxEvent &event);
  void OnPresetUpperLeft(wxEvent &event);

private:
  sTile* m_tile;
  sTile m_edit_tile;  // the tile you edit

  wTileObstructionView *m_obstruction_view;
  wxButton *m_OK;
  wxButton *m_Cancel;
  wxButton *m_Presets;
  wxMenu *m_PopupMenu;


private:
//  DECLARE_CLASS(wTileObstructionDialog);
  DECLARE_EVENT_TABLE();
};


#endif
