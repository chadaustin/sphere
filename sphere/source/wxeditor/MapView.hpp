#ifndef MAP_VIEW_HPP
#define MAP_VIEW_HPP


#include "ScrollWindow.hpp"
//#include <afxext.h>
#include <wx/wx.h>
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


class wMapView : public wScrollWindow, public IToolPaletteHandler
{
public:
  wMapView(wxWindow* parent, wDocumentWindow* owner, IMapViewHandler* handler, sMap* map);
  ~wMapView();

  //BOOL Create(CDocumentWindow* owner, IMapViewHandler* handler, CWnd* parent, sMap* map);

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

  int RoundX(int x);
  int RoundY(int y);
  
  void Click(wxPoint point);
  bool SetTile(int tx, int ty);
  void SelectTileUnderPoint(wxPoint point);
  void PasteEntityUnderPoint(wxPoint point);
  void MapAreaCopy();
  void LayerAreaCopy();
  void PasteMapUnderPoint(wxPoint point);
  void FillArea();
  void EntityCopy(wxPoint point);
  void EntityPaste(wxPoint point);

  bool IsWithinSelectFillArea(int x, int y);
  void DrawTile(wxDC& dc, const wxRect& rect, int tx, int ty);
  void DrawObstructions(wxDC& dc);
  void DrawZones(wxDC& dc);
  void DrawPreviewLine(wxDC& dc, int x1, int y1, int x2, int y2);
  void DrawPreviewBox(wxDC& dc, int x1, int y1, int x2, int y2);

//  afx_msg void OnDestroy();
  void OnPaint(wxPaintEvent &event);
  void OnSize(wxSizeEvent &event);
  void OnLButtonDown(wxMouseEvent &event);
  void OnMouseMove(wxMouseEvent &event);
  void OnLButtonUp(wxMouseEvent &event);
  void OnRButtonUp(wxMouseEvent &event);
  void OnMenu(wxCommandEvent &event);

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
	  tool_ObsMoveSegmentPoint,
    tool_ZoneAdd,
    tool_ZoneEdit,
    tool_ZoneDelete,
  };

private:
  IMapViewHandler* m_Handler;

  wMapToolPalette* m_ToolPalette;

  sMap* m_Map;

  wDIBSection* m_BlitTile;  // for rendering tiles

  int m_ZoomFactor;
  RGB m_ObstructionColor;
  RGB m_HighlightColor;
  RGB m_ZoneColor;
  int m_CurrentTool;

  int m_CurrentX;
  int m_CurrentY;
  int m_CurrentCursorTileX;
  int m_CurrentCursorTileY;
  int m_StartCursorTileX;
  int m_StartCursorTileY;

  int m_StartX;
  int m_StartY;

  int m_MoveIndex; // index of the object we are moving currently

  int m_PreviewX;
  int m_PreviewY;
  int m_PreviewOldX;
  int m_PreviewOldY;
  int m_PreviewLineOn;
  int m_PreviewBoxOn;

  int m_RedrawWindow;
  int m_RedrawPreviewLine;

  int m_SelectedTile;
  int m_SelectedLayer;

  bool m_Clicked;

  wxMenu *m_Menu;
  wxPoint m_MenuPoint;

private:
//  DECLARE_CLASS(wMapView)
  DECLARE_EVENT_TABLE()
};


#endif
