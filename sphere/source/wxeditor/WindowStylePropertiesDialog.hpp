#ifndef WINDOW_STYLE_PROPERTIES_DIALOG_HPP
#define WINDOW_STYLE_PROPERTIES_DIALOG_HPP


#include <wx/wx.h>
#include "RGBAView.hpp"
#include "../common/WindowStyle.hpp"


class wWindowStylePropertiesDialog : public wxDialog
{
public:
  wWindowStylePropertiesDialog(wxWindow *parent, sWindowStyle* ws);
  ~wWindowStylePropertiesDialog();

private:
  //BOOL OnInitDialog();
  //void InitializeView(wRGBAView& view, int corner, int control_id);
  void OnOK(wxCommandEvent &event);

private:

  wxRadioBox *m_BackgroundMode;

  wRGBAView *m_UpperLeftView;
  wRGBAView *m_UpperRightView;
  wRGBAView *m_LowerLeftView;
  wRGBAView *m_LowerRightView;

  sWindowStyle* m_WindowStyle;
private:
  DECLARE_EVENT_TABLE()
};


#endif
