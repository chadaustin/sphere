#ifndef MAP_WINDOW_HPP
#define MAP_WINDOW_HPP


//#include <afxcmn.h>
#include <wx/wx.h>
#include <wx/tabctrl.h>
#include "SaveableDocumentWindow.hpp"
#include "MapView.hpp"
#include "LayerView.hpp"
#include "TilesetEditView.hpp"
#include "TilePalette.hpp"
#include "SwatchPalette.hpp"
#include "../common/Map.hpp"
#include "../common/Tileset.hpp"


class wProject;  // #include "Project.hpp"


class wMapWindow
  : public wSaveableDocumentWindow
  , private IMapViewHandler
  , private ILayerViewHandler
  , private ITilesetEditViewHandler
  , private ITilesetViewHandler
  , private ISwatchPaletteHandler
{
public:
  wMapWindow(const char* map = NULL);
  ~wMapWindow();

private:
  void Create();
  //void Destroy();

  bool LoadMap(const char* map);

  void GetMapViewRect(wxRect* rect);
  void GetLayerViewRect(wxRect* rect);
  void TabChanged(int tab);

  void OnSize(wxSizeEvent &event);
//  void OnCommand(wxCommandEvent &event);
  void OnMapProperties(wxCommandEvent &event);
  void OnChangeTileSize(wxCommandEvent &event);
  void OnRescaleTileset(wxCommandEvent &event);
  void OnResizeAllLayers(wxCommandEvent &event);
  void OnExportTileset(wxCommandEvent &event);
  void OnImportTileset(wxCommandEvent &event);
  void OnPruneTileset(wxCommandEvent &event);
  void OnTabChanged(wxTabEvent &event);

  virtual bool GetSavePath(char* path);
  virtual bool SaveDocument(const char* path);

  // view handlers
  virtual void MV_MapChanged();
  virtual void MV_SelectedTileChanged(int tile);
  virtual void LV_MapChanged();
  virtual void LV_SelectedLayerChanged(int layer);
  virtual void TEV_SelectedTileChanged(int tile);
  virtual void TEV_TileModified(int tile);
  virtual void TEV_TilesetModified();
  virtual void TV_SelectedTileChanged(int tile);
  virtual void TV_TilesetChanged();
  virtual void TV_InsertedTiles(int at, int numtiles);
  virtual void TV_DeletedTiles(int at, int numtiles);
  virtual void SP_ColorSelected(RGBA color);

private:
  sMap m_Map;
  
  bool m_Created;  // whether or not the window has been created

  wxTabCtrl *m_TabControl;

  // views
  wMapView         *m_MapView;
  wLayerView       *m_LayerView;
  wTilesetEditView *m_TilesetEditView;

  // palettes
  wTilePalette* m_TilePalette;
//  CTilePreviewPalette* m_TilePreviewPalette;

private:
//  DECLARE_CLASS(wMapWindow)
  DECLARE_EVENT_TABLE()
};


#endif
