#ifndef TILE_OBSTRUCTION_VIEW_HPP
#define TILE_OBSTRUCTION_VIEW_HPP


#include <afxwin.h>
#include "../common/Tile.hpp"


class CTileObstructionView : public CWnd
{
public:
  CTileObstructionView();
  ~CTileObstructionView();

  BOOL Create(CWnd* parent, sTile* tile);

private:
  afx_msg void OnPaint();
  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnLButtonDown(UINT flags, CPoint point);
  afx_msg void OnLButtonUp(UINT flags, CPoint point);
  afx_msg void OnMouseMove(UINT flags, CPoint point);
  afx_msg void OnRButtonDown(UINT flags, CPoint point);

  void RenderTile();
  CPoint WindowToTile(CPoint p);

private:
  sTile* m_tile;
  RGBA* m_pixels;

  bool m_mouse_down;
  CPoint m_start_point;
  CPoint m_current_point;

  DECLARE_MESSAGE_MAP()
};


#endif
