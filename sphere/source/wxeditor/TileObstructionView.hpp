#ifndef TILE_OBSTRUCTION_VIEW_HPP
#define TILE_OBSTRUCTION_VIEW_HPP


//#include <afxwin.h>
#include <wx/wx.h>
#include "../common/Tile.hpp"


class wTileObstructionView : public wxWindow
{
public:
  wTileObstructionView(wxWindow* parent, sTile* tile);
  ~wTileObstructionView();

  //BOOL Create(CWnd* parent, sTile* tile);

private:
  void OnPaint(wxPaintEvent& event);
  void OnSize(wxSizeEvent& event);
  void OnLButtonDown(wxMouseEvent& event);
  void OnLButtonUp(wxMouseEvent& event);
  void OnMouseMove(wxMouseEvent& event);
  void OnRButtonDown(wxMouseEvent& event);

  void RenderTile();
  wxPoint WindowToTile(wxPoint p);

private:
  sTile* m_tile;
  RGBA* m_pixels;

  bool m_mouse_down;
  wxPoint m_start_point;
  wxPoint m_current_point;

private:
//  DECLARE_CLASS(wTileObstructionView);
  DECLARE_EVENT_TABLE();
};


#endif
