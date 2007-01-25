#ifndef ALPHA_VIEW_HPP
#define ALPHA_VIEW_HPP


//#include <afxwin.h>
//#include <afxcmn.h>
#include <wx/wx.h>
#include <wx/slider.h>
#include "../common/types.h"



class IAlphaViewHandler
{
public:
  virtual void AV_AlphaChanged(byte alpha) = 0;
};


class wAlphaView : public wxWindow
{
public:
  wAlphaView(wxWindow *parent, IAlphaViewHandler* handler);
  ~wAlphaView();

//  BOOL Create(IAlphaViewHandler* handler, CWnd* parent);

  void SetAlpha(byte alpha);
  byte GetAlpha() const;

private:
  void UpdateSlider();

  void OnSize(wxSizeEvent& event);
  void OnVScroll(wxScrollWinEvent &event);

private:
  IAlphaViewHandler* m_Handler;

  wxSlider* m_AlphaSlider;
  wxStaticText* m_AlphaStatic;

  byte m_Alpha;

private:
  //DECLARE_CLASS(wAlphaView);
  DECLARE_EVENT_TABLE();
};


#endif
