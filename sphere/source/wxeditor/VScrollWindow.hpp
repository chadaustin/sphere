#ifndef V_SCROLL_WINDOW_HPP
#define V_SCROLL_WINDOW_HPP


//#include <afxwin.h>
#include <wx/wx.h>

class wVScrollWindow : public wxWindow
{
public:
  wVScrollWindow(wxWindow *parent, wxWindowID id = -1);

  void OnSize(wxSizeEvent &event);

  wxSize GetClientSize();

  void wVScrollWindow::ScrollArea(int x, int y);

protected:
  void SetVScrollPosition(int position);
  void SetVScrollRange(int range, int page_size);



private:
  void OnVScroll(wxScrollEvent &event);

  void SetVScrollInfo();


  virtual void OnVScrollChanged(int y) = 0;



private:
  int m_Range;
  int m_PageSize;
  int m_Position;





  wxScrollBar *m_Scrollbar;

private:
//  DECLARE_CLASS(wVScrollWindow);
  DECLARE_EVENT_TABLE();
};


#endif
