#ifndef FONT_GRADIENT_DIALOG_HPP
#define FONT_GRADIENT_DIALOG_HPP


#include <afxwin.h>
#include "RGBAView.hpp"


class CFontGradientDialog : public CDialog
{
public:
  CFontGradientDialog();

  RGBA GetTopColor();
  RGBA GetBottomColor();

private:
  BOOL OnInitDialog();
  void InitializeView(CRGBAView& view, RGBA color, int control_id);
  void OnOK();

private:
  CRGBAView m_top_view;
  CRGBAView m_bottom_view;

  RGBA m_top_color;
  RGBA m_bottom_color;
};


#endif
