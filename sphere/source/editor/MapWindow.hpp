#ifndef MAP_WINDOW_HPP
#define MAP_WINDOW_HPP


#include <afxcmn.h>
#include "SaveableDocumentWindow.hpp"
#include "MapView.hpp"
#include "LayerView.hpp"
#include "TilesetEditView.hpp"
#include "TilePalette.hpp"
#include "SwatchPalette.hpp"
#include "../common/Map.hpp"
#include "../common/Tileset.hpp"


class CProject;  // #include "Project.hpp"


class CMapWindow
  : public CSaveableDocumentWindow
  , private IMapViewHandler
  , private ILayerViewHandler
  , private ITilesetEditViewHandler
  , private ITilesetViewHandler
  , private ISwatchPaletteHandler
{
public:
  CMapWindow(const char* map = NULL);
  ~CMapWindow();

private:
  void Create();
  void Destroy();

  bool LoadMap(const char* map);

  void GetMapViewRect(RECT* rect);
  void GetLayerViewRect(RECT* rect);
  void TabChanged(int tab);

  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg BOOL OnCommand(WPARAM wparam, LPARAM lparam);
  afx_msg void OnMapProperties();
  afx_msg void OnChangeTileSize();
  afx_msg void OnRescaleTileset();
  afx_msg void OnResizeAllLayers();
  afx_msg void OnExportTileset();
  afx_msg void OnImportTileset();
  afx_msg void OnPruneTileset();
  afx_msg void OnTabChanged(NMHDR* ns, LRESULT* result);

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

  CTabCtrl m_TabControl;

  // views
  CMapView         m_MapView;
  CLayerView       m_LayerView;
  CTilesetEditView m_TilesetEditView;

  // palettes
  CTilePalette* m_TilePalette;
//  CTilePreviewPalette* m_TilePreviewPalette;

  DECLARE_MESSAGE_MAP()
};


#endif
