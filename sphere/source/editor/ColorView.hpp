#ifndef COLOR_VIEW_HPP
#define COLOR_VIEW_HPP


#include <afxwin.h>
#include "../common/rgb.hpp"


class IColorViewHandler
{
public:
  virtual void CV_ColorChanged(RGB color) = 0;
};


class CColorView : public CWnd
{
public:
  CColorView();
  ~CColorView();

  BOOL Create(IColorViewHandler* handler, CWnd* parent, RGB color);
  BOOL Create(IColorViewHandler* handler, CWnd* parent);

  void SetColor(RGB color);
  RGB  GetColor() const;

private:
  afx_msg void OnPaint();
  afx_msg void OnLButtonDown(UINT flags, CPoint point);

private:
  IColorViewHandler* m_Handler;

  RGB m_Color;

  DECLARE_MESSAGE_MAP()
};


#endif
