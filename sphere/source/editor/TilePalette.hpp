#ifndef TILE_PALETTE_HPP
#define TILE_PALETTE_HPP


#include "PaletteWindow.hpp"
#include "TilesetView.hpp"


class CMainWindow;


class CTilePalette : public CPaletteWindow
{
public:
  CTilePalette(CDocumentWindow* owner, ITilesetViewHandler* handler, sTileset* tileset);

  virtual void Destroy();

  void TileChanged(int tile);
  void TilesetChanged();

  void SelectTile(int tile);
  int GetSelectedTile() const;

private:
  afx_msg void OnSize(UINT type, int cx, int cy);

private:
  bool         m_Created;
  CTilesetView m_TilesetView;

  friend CMainWindow;

  DECLARE_MESSAGE_MAP()
};


#endif