#ifndef RGBA_VIEW_HPP
#define RGBA_VIEW_HPP

#include <wx/wx.h>
#include "ColorView.hpp"
#include "AlphaView.hpp"


class wRGBAView : public wxWindow, public IColorViewHandler, public IAlphaViewHandler
{
public:
  wRGBAView(wxWindow *parent, int x, int y, int w, int h, RGBA color);
  ~wRGBAView();

  //BOOL Create(CWnd* parent, int x, int y, int w, int h, RGBA color);

  RGBA GetColor();
  void SetColor(RGBA color);

private:
  virtual void CV_ColorChanged(RGB color);
  virtual void AV_AlphaChanged(byte alpha);

private:
  wColorView *m_ColorView;
  wAlphaView *m_AlphaView;

  RGBA m_Color;
};


#endif
