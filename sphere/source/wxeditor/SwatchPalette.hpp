#ifndef SWATCH_PALETTE_HPP
#define SWATCH_PALETTE_HPP

#include <wx/wx.h>

#include "PaletteWindow.hpp"
#include "TilesetView.hpp"
#include "SwatchServer.hpp"

class wMainWindow;


class ISwatchPaletteHandler
{
public:
  virtual void SP_ColorSelected(RGBA color) = 0;
};


class wSwatchPalette : public wPaletteWindow
{
public:
  wSwatchPalette(wDocumentWindow* owner, ISwatchPaletteHandler* handler);

  bool Destroy();
  
  void SetColor(RGBA color);

private:
  void OnSize(wxSizeEvent &event);
  void OnPaint(wxPaintEvent &event);
  void OnVScroll(wxScrollEvent &event);
  void OnLButtonDown(wxMouseEvent &event);
  void OnRButtonDown(wxMouseEvent &event);

  void OnFileLoad(wxEvent &event);
  void OnFileSave(wxEvent &event);
  void OnDefaultDOS(wxEvent &event);
  void OnDefaultVERGE(wxEvent &event);
  void OnInsertColorBefore(wxEvent &event);
  void OnInsertColorAfter(wxEvent &event);
  void OnReplaceColor(wxEvent &event);
  void OnDeleteColor(wxEvent &event);

  void UpdateScrollBar();
  int  GetPageSize();
  int  GetNumRows();

private:
  ISwatchPaletteHandler* m_Handler;

  RGBA m_Color;

  int m_TopRow;
  int m_SelectedColor;
  int m_RightClickColor;

  wxMenu *m_Menu;

  friend class wMainWindow;

private:
//  DECLARE_CLASS(wSwatchPalette);
  DECLARE_EVENT_TABLE();
};


#endif
