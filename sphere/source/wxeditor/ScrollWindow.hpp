#ifndef SCROLL_WINDOW_HPP
#define SCROLL_WINDOW_HPP


//#include <afxwin.h>
#include <wx/wx.h>

class wScrollWindow : public wxWindow
{
public:
  wScrollWindow(wxWindow *parent, wxWindowID id = -1);

  void OnSize(wxSizeEvent &event);

  wxSize GetClientSize();

  void wScrollWindow::ScrollArea(int x, int y, wxRect rect = wxRect(wxPoint(0, 0), wxSize(-1, -1)));

protected:
  void SetHScrollPosition(int position);
  void SetHScrollRange(int range, int page_size);
  void SetVScrollPosition(int position);
  void SetVScrollRange(int range, int page_size);

private:
  void OnScroll(wxScrollEvent &event);

  void SetHScrollInfo();
  void SetVScrollInfo();

  virtual void OnHScrollChanged(int x) = 0;
  virtual void OnVScrollChanged(int y) = 0;


private:
  int m_XRange;
  int m_XPageSize;
  int m_XPosition;

  int m_YRange;
  int m_YPageSize;
  int m_YPosition;

  wxScrollBar *m_HScrollbar;
  wxScrollBar *m_VScrollbar;
  
private:
//  DECLARE_CLASS(wScrollWindow);
  DECLARE_EVENT_TABLE();
};


#endif
