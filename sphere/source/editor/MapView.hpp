#ifndef MAP_VIEW_HPP
#define MAP_VIEW_HPP


#include "ScrollWindow.hpp"
#include <afxext.h>
#include "../common/Map.hpp"
#include "../common/Tileset.hpp"
#include "DIBSection.hpp"
#include "MapToolPalette.hpp"
#include "Clipboard.hpp"

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

  int GetSelectedTile();
  void SelectTile(int tile);
  void SelectLayer(int layer);

  void UpdateScrollBars();

#if 1
  void SetTileSelection(int width, int height, unsigned int* tiles);

private:
  int m_MultiTileWidth;
  int m_MultiTileHeight;
  unsigned int* m_MultiTileData;
#endif

private:
  int GetPageSizeX();
  int GetPageSizeY();
  int GetTotalTilesX();
  int GetTotalTilesY();

  int RoundX(int x);
  int RoundY(int y);
  
  void Click(CPoint point);
  bool SetTile(int tx, int ty);
  void SelectTileUnderPoint(CPoint point);
  void PasteEntityUnderPoint(CPoint point);
  void MapAreaCopy();
  void LayerAreaCopy();
  void PasteMapUnderPoint(CPoint point);
  void FillMe(int x, int y, int layer, int tileToReplace);
  void FillRectArea();
  void FillArea();
  void EntityCopy(CPoint point);
  void EntityPaste(CPoint point);

  bool IsWithinSelectFillArea(int x, int y);
  void DrawTile(CDC& dc, const RECT& rect, int tx, int ty);
  void DrawObstructions(CDC& dc);
  void DrawZones(CDC& dc);
  void DrawPreviewLine(CDC& dc, int x1, int y1, int x2, int y2);
  void DrawPreviewBox(CDC& dc, int x1, int y1, int x2, int y2);

  void UpdateObstructionTile(int tile);
  void UpdateObstructionTiles();

	//void InitAnimations();

  int FindSpritesetImageIconsIndex(int person_index, std::string filename);

private:
  afx_msg void OnDestroy();
  afx_msg void OnPaint();
  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnLButtonDown(UINT flags, CPoint point);
  afx_msg void OnMouseMove(UINT flags, CPoint point);
  afx_msg void OnLButtonUp(UINT flags, CPoint point);
  afx_msg void OnRButtonUp(UINT flags, CPoint point);
  //afx_msg void OnTimer(UINT event);

  virtual void OnHScrollChanged(int x);
  virtual void OnVScrollChanged(int y);
  virtual void TP_ToolSelected(int tool);

public:
  void OnToolChanged(UINT id);
  BOOL IsToolAvailable(UINT id);

public:
  enum editMode {
    tool_1x1Tile,
    tool_3x3Tile,
    tool_5x5Tile,
    tool_SelectTile,
    tool_FillRectArea,
    tool_FillArea,
    tool_CopyArea,
    tool_Paste,
    tool_CopyEntity,
    tool_PasteEntity,
    tool_MoveEntity,
    tool_ObsSegment,
    tool_ObsDeleteSegment,
	  tool_ObsMoveSegmentPoint,
    tool_ZoneAdd,
    tool_ZoneEdit,
    tool_ZoneMove,
    tool_ZoneDelete,
  };

private:
  IMapViewHandler* m_Handler;

  //CMapToolPalette* m_ToolPalette;

  sMap* m_Map;

  CDIBSection* m_BlitTile;  // for rendering tiles
  CClipboard* m_Clipboard;

  int m_ZoomFactor;
  RGB m_ObstructionColor;
  RGB m_HighlightColor;
  RGB m_ZoneColor;
  int m_CurrentTool;
  int m_PreviousTool;

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

  bool m_ShowGrid;
  bool m_ShowTileObstructions;
  std::vector<sTile> m_TileObstructions; // when View->Tile Obstructions is on, we render these
	//bool m_ShowAnimations;

  //UINT m_Timer;
	//int  m_FrameTick;

  void GetRedrawRect(int& offset_x, int& offset_y, int& width, int& height);

  enum SpritesetDrawTypes {
    SDT_ICON,
    SDT_MINI_IMAGE,
    SDT_IMAGE,
  };
  int m_SpritesetDrawType;

  class SpritesetImageIcon {
    public:
      SpritesetImageIcon() {
        created = false;
      }
      std::string filename;
      CImage32 image;
      CImage32 icon;
      int x1;
      int y1;
      int x2;
      int y2;
      bool created;
  };

  std::vector<SpritesetImageIcon> m_SpritesetImageIcons;


  DECLARE_MESSAGE_MAP()
};


#endif
