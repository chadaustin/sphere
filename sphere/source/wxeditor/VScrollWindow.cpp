//#define WXWIN_COMPATIBILITY_EVENT_TYPES 1

#include "VScrollWindow.hpp"
#include <wx/wx.h>
#include "IDs.hpp"

/*
BEGIN_MESSAGE_MAP(CVScrollWindow, CWnd)

  ON_WM_VSCROLL()

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wVScrollWindow, wxWindow)
  EVT_SCROLL(wVScrollWindow::OnVScroll)
  EVT_SIZE(wVScrollWindow::OnSize)
END_EVENT_TABLE()


////////////////////////////////////////////////////////////////////////////////

wVScrollWindow::wVScrollWindow(wxWindow *parent, wxWindowID id)
: m_Range(0)
, m_PageSize(0)
, m_Position(0)
, m_Scrollbar(NULL)
{
  Create(parent, id, wxDefaultPosition, wxDefaultSize, 0/*wxVSCROLL*/, wxString("wVScrollWindow"));
  m_Scrollbar = new wxScrollBar(this, wID_SCROLL_VERTICAL, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);

  wxSizeEvent evt(GetSize());
  OnSize(evt);
}

////////////////////////////////////////////////////////////////////////////////

void
wVScrollWindow::SetVScrollPosition(int position)
{
  m_Position = position;
  if (m_Position < 0) {
    m_Position = 0;
  } else if (m_Position > m_Range - m_PageSize) {
    m_Position = m_Range - m_PageSize;
  }

  SetVScrollInfo();
}

////////////////////////////////////////////////////////////////////////////////

void
wVScrollWindow::SetVScrollRange(int range, int page_size)
{
  m_Range    = range;
  m_PageSize = page_size;

  // make sure position is optimized
  if (m_Position > 0) {
    if (m_Position > m_Range - m_PageSize) {
      m_Position = m_Range - m_PageSize;
      if (m_Position < 0) {
        m_Position = 0;
      }
      OnVScrollChanged(m_Position);
    }
  }

  SetVScrollInfo();
}

////////////////////////////////////////////////////////////////////////////////

void
wVScrollWindow::OnVScroll(wxScrollEvent &event)
{
  if(event.GetEventType() == wxEVT_SCROLL_TOP)          m_Position = 0;
  if(event.GetEventType() == wxEVT_SCROLL_BOTTOM)       m_Position = m_Range - m_PageSize;
  if(event.GetEventType() == wxEVT_SCROLL_LINEUP)       m_Position--;
  if(event.GetEventType() == wxEVT_SCROLL_LINEDOWN)     m_Position++;
  if(event.GetEventType() == wxEVT_SCROLL_PAGEUP)       m_Position -= m_PageSize;
  if(event.GetEventType() == wxEVT_SCROLL_PAGEDOWN)     m_Position += m_PageSize;
  if(event.GetEventType() == wxEVT_SCROLL_THUMBTRACK)   m_Position = m_Scrollbar->GetThumbPosition();
  if(event.GetEventType() == wxEVT_SCROLL_THUMBRELEASE) m_Position = m_Scrollbar->GetThumbPosition();

  if (m_Position > m_Range - m_PageSize) {
    m_Position = m_Range - m_PageSize;
  }

  if (m_Position < 0) {
    m_Position = 0;
  }

  SetVScrollRange(m_Range, m_PageSize);
  OnVScrollChanged(m_Position);
}

////////////////////////////////////////////////////////////////////////////////

void
wVScrollWindow::OnSize(wxSizeEvent &event)
{
  wxSize size = GetClientSize();
  if(m_Scrollbar != NULL) {
    m_Scrollbar->SetSize(size.x,
                          0,
                          wxSystemSettings::GetMetric(wxSYS_VSCROLL_X),
                          size.y);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wVScrollWindow::SetVScrollInfo()
{
  m_Scrollbar->SetScrollbar(m_Position, m_PageSize, m_Range, m_PageSize);
}

////////////////////////////////////////////////////////////////////////////////

wxSize
wVScrollWindow::GetClientSize()
{
  wxSize size = wxWindow::GetClientSize();
  return wxSize(size.x - wxSystemSettings::GetMetric(wxSYS_VSCROLL_X), 
                size.y);
}

////////////////////////////////////////////////////////////////////////////////

void
wVScrollWindow::ScrollArea(int x, int y)
{
  wxSize size = GetClientSize();
  if(x == 0 && y == 0) {
    return;
  }
  if(abs(x) >= size.x || abs(y) >= size.y) {
    Refresh(FALSE);
    return;
  }
  int srcx = 0;
  int srcy = 0;
  int dstx = 0;
  int dsty = 0;

  wxSize scrollsize(size.x - abs(x), size.y - abs(y));
  if(x < 0) {
    srcx = -x;
  } else {
    dstx = x;
  }
  if(y < 0) {
    srcy = -y;
  } else {
    dsty = y;
  }
  wxClientDC dc(this);

  dc.Blit(dstx, dsty, scrollsize.x, scrollsize.y, &dc, srcx, srcy);
  
  if(x != 0) {
    if(dstx == 0) {
      wxRect rect(scrollsize.x, 0, size.x - scrollsize.x, size.y);
      Refresh(TRUE, &rect);
    } else {
      wxRect rect(0, 0, size.x - scrollsize.x, size.y);
      Refresh(TRUE, &rect);
    }
  }
  if(y != 0) {
    if(dsty == 0) {
      wxRect rect(0, scrollsize.y, size.x, size.y - scrollsize.y);
      Refresh(TRUE, &rect);
    } else {
      wxRect rect(0, 0, size.x, size.y - scrollsize.y);
      Refresh(TRUE, &rect);
    }
  }

}

////////////////////////////////////////////////////////////////////////////////
