#ifndef TILE_PROPERTIES_DIALOG_HPP
#define TILE_PROPERTIES_DIALOG_HPP


//#include <afxwin.h>
#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/spinctrl.h>
#include "../common/Tileset.hpp"


class wTilePropertiesDialog : public wxDialog
{
public:
  wTilePropertiesDialog(wxWindow *parent, sTileset* tileset, int tile);
  ~wTilePropertiesDialog();

private:
  virtual bool OnInitDialog(wxInitDialogEvent& event);
  virtual void OnOK(wxCommandEvent &event);

  void OnAnimatedClicked(wxCommandEvent &event);

  void OnNextTile(wxCommandEvent &event);
  void OnPrevTile(wxCommandEvent &event);

  void OnEditObstructions(wxCommandEvent &event);

private:
  void UpdateDialog();
  bool StoreCurrentTile();

private:
  sTileset* m_Tileset;
  sTile*    m_Tiles;  // modified by dialog
  int       m_Tile;

  wxCheckBox *m_wAnimated;
  wxSpinCtrl *m_wNextFrame;
  wxSpinCtrl *m_wNextDelay;

private:
//  DECLARE_CLASS(wTilePropertiesDialog);
  DECLARE_EVENT_TABLE();
};


#endif
