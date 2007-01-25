#ifndef TILESET_EDIT_VIEW_HPP
#define TILESET_EDIT_VIEW_HPP


#include "HScrollWindow.hpp"
#include "ImageView.hpp"
#include "PaletteView.hpp"
#include "ColorView.hpp"
#include "AlphaView.hpp"
#include "SwatchPalette.hpp"
#include "../common/Tileset.hpp"


class ITilesetEditViewHandler
{
public:
  virtual void TEV_SelectedTileChanged(int tile) = 0;
  virtual void TEV_TileModified(int tile) = 0;
  virtual void TEV_TilesetModified() = 0;
};


class wTilesetEditView
  : public wHScrollWindow
  , private IImageViewHandler
  , private IPaletteViewHandler
  , private IColorViewHandler
  , private IAlphaViewHandler
{
public:
  wTilesetEditView(wxWindow* parent, wDocumentWindow* owner, ITilesetEditViewHandler* handler, sTileset* tileset);
  ~wTilesetEditView();

  //BOOL Create(CWnd* parent, CDocumentWindow* owner, ITilesetEditViewHandler* handler, sTileset* tileset);

  void TilesetChanged();
  void SelectTile(int tile);
  void SP_ColorSelected(RGBA color);

private:
  void UpdateImageView();

  void UpdateScrollBar();
  virtual void OnHScrollChanged(int x);

  void OnSize(wxSizeEvent &event);

  void OnTilesetInsertTiles(wxCommandEvent&event);
  void OnTilesetAppendTiles(wxCommandEvent&event);

  //void OnUpdateTilesetDeleteTile(CCmdUI* cmdui);
  //void OnUpdateTilesetDeleteTile(wxCommandEvent &event);
  void OnTilesetDeleteTile(wxCommandEvent&event);

  void OnTilesetReplaceWithImage(wxCommandEvent&event);
  void OnTilesetInsertImage(wxCommandEvent&event);
  void OnTilesetAppendImage(wxCommandEvent&event);

  virtual void IV_ImageChanged();
  virtual void IV_ColorChanged(RGBA color);
  virtual void PV_ColorChanged(RGB color);
  virtual void CV_ColorChanged(RGB color);
  virtual void AV_AlphaChanged(byte alpha);

private:
  ITilesetEditViewHandler* m_Handler;
  sTileset* m_Tileset;

  int   m_CurrentTile;
  sTile m_UndoTile;

  bool m_Created;

  wxMenu *m_Menu;

  // views
  wImageView   *m_ImageView;
  wPaletteView *m_PaletteView;
  wColorView   *m_ColorView;
  wAlphaView   *m_AlphaView;

private:
//  DECLARE_CLASS(wTilesetEditView);
  DECLARE_EVENT_TABLE();
};


#endif
