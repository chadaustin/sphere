#ifndef TILE_OBSTRUCTION_DIALOG_HPP
#define TILE_OBSTRUCTION_DIALOG_HPP


#include <afxwin.h>
#include "TileObstructionView.hpp"
#include "../common/Tile.hpp"


class CTileObstructionDialog : public CDialog
{
public:
  CTileObstructionDialog(sTile* tile);

private:
  virtual BOOL OnInitDialog();
  virtual void OnOK();

  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnPresets();

  afx_msg void OnPresetUnblocked();
  afx_msg void OnPresetBlocked();
  afx_msg void OnPresetUpperRight();
  afx_msg void OnPresetLowerRight();
  afx_msg void OnPresetLowerLeft();
  afx_msg void OnPresetUpperLeft();

private:
  sTile* m_tile;
  sTile m_edit_tile;  // the tile you edit

  CTileObstructionView m_obstruction_view;

  DECLARE_MESSAGE_MAP()
};


#endif