//#define WXWIN_COMPATIBILITY_EVENT_TYPES 1

#include "ScrollWindow.hpp"
#include <wx/wx.h>
#include "IDs.hpp"

/*
BEGIN_MESSAGE_MAP(CScrollWindow, CWnd)

  ON_WM_HSCROLL()
  ON_WM_VSCROLL()

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wScrollWindow, wxWindow)
  EVT_SCROLL(wScrollWindow::OnScroll)
  EVT_SIZE(wScrollWindow::OnSize)
END_EVENT_TABLE()


////////////////////////////////////////////////////////////////////////////////

wScrollWindow::wScrollWindow(wxWindow *parent, wxWindowID id)
: m_XRange(0)
, m_XPageSize(0)
, m_XPosition(0)

, m_YRange(0)
, m_YPageSize(0)
, m_YPosition(0)

, m_HScrollbar(NULL)
, m_VScrollbar(NULL)
{
  Create(parent, id, wxDefaultPosition, wxDefaultSize, 0/*wxHSCROLL | wxVSCROLL*/, wxString("wScrollWindow"));
  
  m_VScrollbar = new wxScrollBar(this, wID_SCROLL_VERTICAL, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);
  m_HScrollbar = new wxScrollBar(this, wID_SCROLL_HORIZONTAL, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL);

  wxSizeEvent evt(GetSize());
  OnSize(evt);
}

////////////////////////////////////////////////////////////////////////////////

void
wScrollWindow::SetHScrollPosition(int position)
{
  m_XPosition = position;
  if (m_XPosition < 0) {
    m_XPosition = 0;
  } else if (m_XPosition > m_XRange - m_XPageSize) {
    m_XPosition = m_XRange - m_XPageSize;
  }

  SetHScrollInfo();
}

////////////////////////////////////////////////////////////////////////////////

void
wScrollWindow::SetHScrollRange(int range, int page_size)
{
  m_XRange    = range;
  m_XPageSize = page_size;

  if(m_XPageSize == 0) {
    m_XPageSize = 1;
  }

  // make sure position is optimized
  if (m_XPosition > 0) {
    if (m_XPosition > m_XRange - m_XPageSize) {
      m_XPosition = m_XRange - m_XPageSize;
      if (m_XPosition < 0) {
        m_XPosition = 0;
      }
      OnHScrollChanged(m_XPosition);
    }
  }

  SetHScrollInfo();
}

////////////////////////////////////////////////////////////////////////////////

void
wScrollWindow::SetVScrollPosition(int position)
{
  m_YPosition = position;
  if (m_YPosition < 0) {
    m_YPosition = 0;
  } else if (m_YPosition > m_YRange - m_YPageSize) {
    m_YPosition = m_YRange - m_YPageSize;
  }

  SetVScrollInfo();
}

////////////////////////////////////////////////////////////////////////////////

void
wScrollWindow::SetVScrollRange(int range, int page_size)
{
  m_YRange    = range;
  m_YPageSize = page_size;
  if(m_YPageSize == 0) {
    m_YPageSize = 1;
  }

  // make sure position is optimized
  if (m_YPosition > 0) {
    if (m_YPosition > m_YRange - m_YPageSize) {
      m_YPosition = m_YRange - m_YPageSize;
      if (m_YPosition < 0) {
        m_YPosition = 0;
      }
      OnVScrollChanged(m_YPosition);
    }
  }

  SetVScrollInfo();
}

////////////////////////////////////////////////////////////////////////////////

void
wScrollWindow::OnScroll(wxScrollEvent &event)
{
  switch(event.GetId()) {
  //case wxHORIZONTAL:
  case wID_SCROLL_HORIZONTAL:
    if(event.GetEventType() == wxEVT_SCROLL_TOP)          m_XPosition = 0;
    if(event.GetEventType() == wxEVT_SCROLL_BOTTOM)       m_XPosition = m_XRange - m_XPageSize;
    if(event.GetEventType() == wxEVT_SCROLL_LINEUP)       m_XPosition--;
    if(event.GetEventType() == wxEVT_SCROLL_LINEDOWN)     m_XPosition++;
    if(event.GetEventType() == wxEVT_SCROLL_PAGEUP)       m_XPosition -= m_XPageSize;
    if(event.GetEventType() == wxEVT_SCROLL_PAGEDOWN)     m_XPosition += m_XPageSize;
    if(event.GetEventType() == wxEVT_SCROLL_THUMBTRACK)   m_XPosition = m_HScrollbar->GetThumbPosition();
    if(event.GetEventType() == wxEVT_SCROLL_THUMBRELEASE) m_XPosition = m_HScrollbar->GetThumbPosition();

    if (m_XPosition > m_XRange - m_XPageSize) {
      m_XPosition = m_XRange - m_XPageSize;
    }

    if (m_XPosition < 0) {
      m_XPosition = 0;
    }

    SetHScrollRange(m_XRange, m_XPageSize);
    OnHScrollChanged(m_XPosition);
    break;
  case wID_SCROLL_VERTICAL:
    if(event.GetEventType() == wxEVT_SCROLL_TOP)          m_YPosition = 0;
    if(event.GetEventType() == wxEVT_SCROLL_BOTTOM)       m_YPosition = m_YRange - m_YPageSize;
    if(event.GetEventType() == wxEVT_SCROLL_LINEUP)       m_YPosition--;
    if(event.GetEventType() == wxEVT_SCROLL_LINEDOWN)     m_YPosition++;
    if(event.GetEventType() == wxEVT_SCROLL_PAGEUP)       m_YPosition -= m_YPageSize;
    if(event.GetEventType() == wxEVT_SCROLL_PAGEDOWN)     m_YPosition += m_YPageSize;
    if(event.GetEventType() == wxEVT_SCROLL_THUMBTRACK)   m_YPosition = m_VScrollbar->GetThumbPosition();
    if(event.GetEventType() == wxEVT_SCROLL_THUMBRELEASE) m_YPosition = m_VScrollbar->GetThumbPosition();

    if (m_YPosition > m_YRange - m_YPageSize) {
      m_YPosition = m_YRange - m_YPageSize;
    }

    if (m_YPosition < 0) {
      m_YPosition = 0;
    }

    SetVScrollRange(m_YRange, m_YPageSize);
    OnVScrollChanged(m_YPosition);
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wScrollWindow::OnSize(wxSizeEvent &event)
{
  wxSize size = GetClientSize();
  if(m_VScrollbar != NULL) {
#ifdef WIN32
    m_VScrollbar->SetSize(size.x, 
                          0,
                          wxSystemSettings::GetMetric(wxSYS_VSCROLL_X),
                          size.y);
#else
    m_VScrollbar->SetSize(size.x, 0, 400, size.y);
#endif
  }
  if(m_HScrollbar != NULL) {
#ifdef WIN32
    m_HScrollbar->SetSize(0,
                          size.y,
                          size.x,
                          wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y));
#else
    m_HScrollbar->SetSize(0, size.y, size.x, 400);
#endif
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wScrollWindow::SetHScrollInfo()
{
  m_HScrollbar->SetScrollbar(m_XPosition, m_XPageSize, m_XRange, m_XPageSize);
}

////////////////////////////////////////////////////////////////////////////////

void
wScrollWindow::SetVScrollInfo()
{
  m_VScrollbar->SetScrollbar(m_YPosition, m_YPageSize, m_YRange, m_YPageSize);
}

////////////////////////////////////////////////////////////////////////////////

wxSize
wScrollWindow::GetClientSize()
{
  wxSize size = wxWindow::GetClientSize();
#ifdef WIN32
  return wxSize(size.x - wxSystemSettings::GetMetric(wxSYS_VSCROLL_X), 
                size.y - wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y));
#else
  return wxSize(size.x - 400, 
                size.y - 400);
#endif
}

////////////////////////////////////////////////////////////////////////////////

void
wScrollWindow::ScrollArea(int x, int y, wxRect rect)
{
  wxSize size = rect.GetSize(); //GetClientSize();
  if(size.GetWidth() < 0) {
    size.SetWidth(GetClientSize().GetWidth());
  }
  if(size.GetHeight() < 0) {
    size.SetHeight(GetClientSize().GetHeight());
  }
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

  dc.Blit(dstx + rect.x, dsty + rect.y, scrollsize.x, scrollsize.y, &dc, srcx + rect.x, srcy + rect.y);
  
  if(x != 0) {
    if(dstx == 0) {
      wxRect rect(scrollsize.x + rect.x, 0 + rect.y, size.x - scrollsize.x, size.y);
      Refresh(TRUE, &rect);
    } else {
      wxRect rect(0 + rect.x, 0 + rect.y, size.x - scrollsize.x, size.y);
      Refresh(TRUE, &rect);
    }
  }
  if(y != 0) {
    if(dsty == 0) {
      wxRect rect(0 + rect.x, scrollsize.y + rect.y, size.x, size.y - scrollsize.y);
      Refresh(TRUE, &rect);
    } else {
      wxRect rect(0 + rect.x, 0 + rect.y, size.x, size.y - scrollsize.y);
      Refresh(TRUE, &rect);
    }
  }

}

////////////////////////////////////////////////////////////////////////////////
