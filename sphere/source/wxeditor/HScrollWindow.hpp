#ifndef H_SCROLL_WINDOW_HPP
#define H_SCROLL_WINDOW_HPP


//#include <afxwin.h>
#include <wx/wx.h>

class wHScrollWindow : public wxWindow
{
public:
  wHScrollWindow(wxWindow *parent, wxWindowID id = -1);

  void OnSize(const wxSizeEvent& event);

  wxSize GetClientSize();

protected:
  void SetHScrollPosition(int position);
  void SetHScrollRange(int range, int page_size);



private:
  void OnHScroll(wxScrollEvent &event);

  void SetHScrollInfo();

 
  virtual void OnHScrollChanged(int x) = 0;



private:
  int m_Range;
  int m_PageSize;
  int m_Position;



  

  wxScrollBar *m_Scrollbar;

private:
//  DECLARE_CLASS(wHScrollWindow);
  DECLARE_EVENT_TABLE();
};


#endif
