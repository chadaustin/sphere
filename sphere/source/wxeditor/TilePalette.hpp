#ifndef TILE_PALETTE_HPP
#define TILE_PALETTE_HPP


#include <wx/wx.h>
#include "PaletteWindow.hpp"
#include "TilesetView.hpp"


class wMainWindow;


class wTilePalette : public wPaletteWindow
{
public:
  wTilePalette(wDocumentWindow* owner, ITilesetViewHandler* handler, sTileset* tileset);

  virtual bool Destroy();

  void TileChanged(int tile);
  void TilesetChanged();

  void SelectTile(int tile);
  int GetSelectedTile() const;

private:
  void OnSize(wxSizeEvent &event);

private:
  bool         m_Created;
  wTilesetView *m_TilesetView;

  friend class wMainWindow;

private:
//  DECLARE_CLASS(wTilePalette);
  DECLARE_EVENT_TABLE();
};


#endif
