#ifndef RESIZE_DIALOG_HPP
#define RESIZE_DIALOG_HPP


#include <string>
//#include <afxwin.h>
#include <wx/wx.h>
#include <wx/spinctrl.h>

class wResizeDialog : public wxDialog
{
public:
  wResizeDialog(wxWindow *parent, const char* caption, int default_width, int default_height);

  void SetRange(int min_width, int max_width, int min_height, int max_height);

  int GetWidth() const;
  int GetHeight() const;

private:
  //virtual bool OnInitDialog();
  virtual void OnOK(wxCommandEvent &event);

private:
  std::string m_Caption;

  wxSpinCtrl *m_WidthCtrl;
  wxSpinCtrl *m_HeightCtrl;

  int m_Width;
  int m_Height;

  int m_MinWidth;
  int m_MaxWidth;
  int m_MinHeight;
  int m_MaxHeight;
private:
  DECLARE_EVENT_TABLE()
};


#endif
