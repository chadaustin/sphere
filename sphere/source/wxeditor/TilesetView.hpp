#ifndef TILESET_VIEW_HPP
#define TILESET_VIEW_HPP


//#include <afxwin.h>
//#include <afxext.h>
#include <wx/wx.h>
#include "DIBSection.hpp"
#include "../common/Tileset.hpp"
#include "VScrollWindow.hpp"


class ITilesetViewHandler
{
public:
  virtual void TV_SelectedTileChanged(int tile) = 0;
  virtual void TV_TilesetChanged() = 0;
  virtual void TV_InsertedTiles(int at, int numtiles) = 0;
  virtual void TV_DeletedTiles(int at, int numtiles) = 0;
};


class wTilesetView : public wVScrollWindow
{
public:
  wTilesetView(wxWindow *parent, ITilesetViewHandler* handler, sTileset* tileset);
  ~wTilesetView();

  //BOOL Create(ITilesetViewHandler* handler, CWnd* parent, sTileset* tileset);

  void TileChanged(int tile);
  void TilesetChanged();

  void SetSelectedTile(int tile);
  int  GetSelectedTile() const;

private:
  int GetPageSize();
  int GetNumRows();
  void UpdateScrollBar();

  void OnPaint(wxPaintEvent &event);
  void OnSize(wxSizeEvent &event);
  //void OnVScroll(wxScrollEvent &event);
  void OnLButtonDown(wxMouseEvent &event);
  void OnMouseMove(wxMouseEvent &event);
  void OnRButtonUp(wxMouseEvent &event);

  void OnInsertTile(wxEvent &event);
  void OnAppendTile(wxEvent &event);
  void OnDeleteTile(wxEvent &event);
  void OnTileProperties(wxEvent &event);
  void OnInsertTiles(wxEvent &event);
  void OnAppendTiles(wxEvent &event);
  void OnDeleteTiles(wxEvent &event);
  void OnInsertTileset(wxEvent &event);
  void OnAppendTileset(wxEvent &event);

  void OnZoom1x(wxEvent &event);
  void OnZoom2x(wxEvent &event);
  void OnZoom4x(wxEvent &event);
  void OnZoom8x(wxEvent &event);

  void OnVScrollChanged(int y);

private:
  ITilesetViewHandler* m_Handler;

  sTileset* m_Tileset;

  int m_TopRow;
  int m_SelectedTile;
  int m_ZoomFactor;

  wDIBSection* m_BlitTile;

  bool m_MenuShown;
  wxMenu *m_PopupMenu;

private:
//  DECLARE_CLASS(wTilesetView);
  DECLARE_EVENT_TABLE();
};


#endif
