#ifndef RGBA_VIEW_HPP
#define RGBA_VIEW_HPP


#include "ColorView.hpp"
#include "AlphaView.hpp"


class CRGBAView : public IColorViewHandler, public IAlphaViewHandler
{
public:
  CRGBAView();

  BOOL Create(CWnd* parent, int x, int y, int w, int h, RGBA color);

  RGBA GetColor();

private:
  virtual void CV_ColorChanged(RGB color);
  virtual void AV_AlphaChanged(byte alpha);

private:
  CColorView m_ColorView;
  CAlphaView m_AlphaView;

  RGBA m_Color;
};


#endif
