#ifndef TILESET_VIEW_HPP
#define TILESET_VIEW_HPP


#include <afxwin.h>
#include <afxext.h>
#include "DIBSection.hpp"
#include "../common/Tileset.hpp"

class ITilesetViewHandler
{
public:
  virtual void TV_SelectedTileChanged(int tile) = 0;
  virtual void TV_TilesetChanged() = 0;
  virtual void TV_InsertedTiles(int at, int numtiles) = 0;
  virtual void TV_DeletedTiles(int at, int numtiles) = 0;
  virtual void TV_SwapTiles(std::vector<int> list_a, std::vector<int> list_b) = 0;
};


class CTilesetView : public CWnd
{
public:
  CTilesetView();
  ~CTilesetView();

  BOOL Create(ITilesetViewHandler* handler, CWnd* parent, sTileset* tileset);

  void TileChanged(int tile);
  void TilesetChanged();

  void SetSelectedTile(int tile);
  int  GetSelectedTile() const;

private:
  int GetPageSize();
  int GetNumRows();
  void UpdateScrollBar();

  afx_msg void OnPaint();
  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnVScroll(UINT code, UINT pos, CScrollBar* scroll_bar);
  afx_msg void OnLButtonDown(UINT flags, CPoint point);
  afx_msg void OnMouseMove(UINT flags, CPoint point);
  afx_msg void OnRButtonUp(UINT flags, CPoint point);

  afx_msg void OnInsertTile();
  afx_msg void OnAppendTile();
  afx_msg void OnDeleteTile();
  afx_msg void OnTileProperties();
  afx_msg void OnInsertTiles();
  afx_msg void OnAppendTiles();
  afx_msg void OnDeleteTiles();
  afx_msg void OnInsertTileset();
  afx_msg void OnAppendTileset();

  void OnZoom(int zoom_factor);
  afx_msg void OnZoom1x();
  afx_msg void OnZoom2x();
  afx_msg void OnZoom4x();
  afx_msg void OnZoom8x();

  afx_msg void OnMoveBack();
  afx_msg void OnMoveForward();

  afx_msg void OnViewTileObstructions();

  afx_msg void OnEditRange();

private:
  ITilesetViewHandler* m_Handler;

  sTileset* m_Tileset;

  int m_TopRow;
  int m_SelectedTile;
  int m_ZoomFactor;

  CDIBSection* m_BlitTile;

  bool m_MenuShown;
  void OnSwap(int new_index);

  bool m_ShowTileObstructions;
  std::vector<sTile> m_TileObstructions;

  void UpdateObstructionTiles();
  void UpdateObstructionTile(int tile);

  DECLARE_MESSAGE_MAP()
};


#endif
