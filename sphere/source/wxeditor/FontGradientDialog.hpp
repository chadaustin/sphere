#ifndef FONT_GRADIENT_DIALOG_HPP
#define FONT_GRADIENT_DIALOG_HPP


//#include <afxwin.h>
#include <wx/wx.h>
#include "RGBAView.hpp"


class wFontGradientDialog : public wxDialog
{
public:
  wFontGradientDialog(wxWindow *parent);

  RGBA GetTopColor();
  RGBA GetBottomColor();

private:
  //BOOL OnInitDialog();
  //void InitializeView(CRGBAView& view, RGBA color, int control_id);
  void OnOK(wxCommandEvent &event);

private:
  wRGBAView *m_top_view;
  wRGBAView *m_bottom_view;

  RGBA m_top_color;
  RGBA m_bottom_color;

private:
  DECLARE_EVENT_TABLE();
};


#endif
