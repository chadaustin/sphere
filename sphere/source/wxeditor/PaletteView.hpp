#ifndef PALETTE_VIEW_HPP
#define PALETTE_VIEW_HPP



//#include <afxwin.h>
#include <wx/wx.h>
#include "../common/rgb.hpp"


class wDIBSection;  // #include "DIBSection.hpp"



class IPaletteViewHandler
{
public:
  virtual void PV_ColorChanged(RGB color) = 0;
};


class wPaletteView : public wxWindow
{
public:
  wPaletteView(wxWindow* pParentWindow, IPaletteViewHandler* pHandler);
  ~wPaletteView();

  //BOOL Create(IPaletteViewHandler* pHandler, CWnd* pParentWindow);

  RGB GetColor() const;

private:
  void UpdatePalette(int w, int h, int pitch, RGB* pixels);

  void OnPaint(wxPaintEvent &event);
  void OnLButtonDown(wxMouseEvent &event);
  void OnLButtonUp(wxMouseEvent &event);
  void OnMouseMove(wxMouseEvent &event);
  void OnSize(wxSizeEvent &event);

private:
  IPaletteViewHandler* m_pHandler;

  wDIBSection* m_pPaletteDIB;
  RGB          m_Color;
  bool         m_bMouseDown;

private:
//  DECLARE_CLASS(wPaletteView);
  DECLARE_EVENT_TABLE();
};



#endif
