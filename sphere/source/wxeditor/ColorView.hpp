#ifndef COLOR_VIEW_HPP
#define COLOR_VIEW_HPP


//#include <afxwin.h>
#include <wx/wx.h>
#include <wx/colour.h>
#include <wx/cmndata.h>
#include <wx/colordlg.h>
#include "../common/rgb.hpp"


class IColorViewHandler
{
public:
  virtual void CV_ColorChanged(RGB color) = 0;
};


class wColorView : public wxWindow
{
public:
  wColorView(wxWindow *parent, IColorViewHandler* handler);
  //~wColorView();

  //BOOL Create(IColorViewHandler* handler, CWnd* parent, RGB color = CreateRGB(0, 0, 0));

  void SetColor(RGB color);
  RGB  GetColor() const;

private:
  void OnPaint(wxPaintEvent& event);
  void OnLButtonDown(wxMouseEvent& event);

private:
  IColorViewHandler* m_Handler;

  RGB m_Color;

  wxColourData m_ColorData;

private:
//  DECLARE_CLASS(wColorView);
  DECLARE_EVENT_TABLE();
};


#endif
