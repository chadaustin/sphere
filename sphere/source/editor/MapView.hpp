#ifndef MAP_VIEW_HPP
#define MAP_VIEW_HPP


#include "ScrollWindow.hpp"
#include <afxext.h>
#include "../common/Map.hpp"
#include "../common/Tileset.hpp"
#include "DIBSection.hpp"
#include "MapToolPalette.hpp"


class IMapViewHandler
{
public:
  virtual void MV_MapChanged() = 0;
  virtual void MV_SelectedTileChanged(int tile) = 0;
};


class CMapView : public CScrollWindow, public IToolPaletteHandler
{
public:
  CMapView();
  ~CMapView();

  BOOL Create(CDocumentWindow* owner, IMapViewHandler* handler, CWnd* parent, sMap* map);

  void SetZoomFactor(int factor);
  int  GetZoomFactor();

  void TilesetChanged();

  void SelectTile(int tile);
  void SelectLayer(int layer);

  void UpdateScrollBars();

private:
  int GetPageSizeX();
  int GetPageSizeY();
  int GetTotalTilesX();
  int GetTotalTilesY();
  
  void Click(CPoint point);
  bool SetTile(int tx, int ty);
  void SelectTileUnderPoint(CPoint point);
  void PasteEntityUnderPoint(CPoint point);
  void MapAreaCopy();
  void LayerAreaCopy();
  void PasteMapUnderPoint(CPoint point);
  void FillArea();
  void EntityCopy(CPoint point);
  void EntityPaste(CPoint point);

  bool IsWithinSelectFillArea(int x, int y);
  void DrawTile(CDC& dc, const RECT& rect, int tx, int ty);
  void DrawObstructions(CDC& dc);

  afx_msg void OnDestroy();
  afx_msg void OnPaint();
  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnLButtonDown(UINT flags, CPoint point);
  afx_msg void OnMouseMove(UINT flags, CPoint point);
  afx_msg void OnLButtonUp(UINT flags, CPoint point);
  afx_msg void OnRButtonUp(UINT flags, CPoint point);

  virtual void OnHScrollChanged(int x);
  virtual void OnVScrollChanged(int y);
  virtual void TP_ToolSelected(int tool);

public:
  enum editMode {
    tool_1x1Tile,
    tool_3x3Tile,
    tool_5x5Tile,
    tool_SelectTile,
    tool_FillArea,
    tool_CopyArea,
    tool_Paste,
    tool_CopyEntity,
    tool_PasteEntity,
    tool_ObsSegment,
    tool_ObsDeleteSegment,
  };

private:
  IMapViewHandler* m_Handler;

  CMapToolPalette* m_ToolPalette;

  sMap* m_Map;

  CDIBSection* m_BlitTile;  // for rendering tiles

  int m_ZoomFactor;
  RGB m_ObstructionColor;
  RGB m_HighlightColor;
  int m_CurrentTool;

  int m_CurrentX;
  int m_CurrentY;
  int m_CurrentCursorTileX;
  int m_CurrentCursorTileY;
  int m_StartCursorTileX;
  int m_StartCursorTileY;

  int m_StartX;
  int m_StartY;

  int m_SelectedTile;
  int m_SelectedLayer;

  bool m_Clicked;

  DECLARE_MESSAGE_MAP()
};


#endif