//#define WXWIN_COMPATIBILITY_EVENT_TYPES 1

#include "HScrollWindow.hpp"
#include <wx/wx.h>

/*
BEGIN_MESSAGE_MAP(CHScrollWindow, CWnd)

  ON_WM_HSCROLL()

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wHScrollWindow, wxWindow)
  EVT_SCROLL(wHScrollWindow::OnHScroll)
  EVT_SIZE(wHScrollWindow::OnSize)
END_EVENT_TABLE()


////////////////////////////////////////////////////////////////////////////////

wHScrollWindow::wHScrollWindow(wxWindow *parent, wxWindowID id)
: m_Range(0)
, m_PageSize(0)
, m_Position(0)
, m_Scrollbar(NULL)
{
  Create(parent, id, wxDefaultPosition, wxDefaultSize, 0/*wxHSCROLL*/, wxString("wHScrollWindow"));
  m_Scrollbar = new wxScrollBar(this, -1, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL);

  OnSize(wxSizeEvent(GetSize()));
}

////////////////////////////////////////////////////////////////////////////////

void
wHScrollWindow::SetHScrollPosition(int position)
{
  m_Position = position;
  if (m_Position < 0) {
    m_Position = 0;
  } else if (m_Position > m_Range - m_PageSize) {
    m_Position = m_Range - m_PageSize;
  }

  SetHScrollInfo();
}

////////////////////////////////////////////////////////////////////////////////

void
wHScrollWindow::SetHScrollRange(int range, int page_size)
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
      OnHScrollChanged(m_Position);
    }
  }

  SetHScrollInfo();
}

////////////////////////////////////////////////////////////////////////////////

void
wHScrollWindow::OnHScroll(wxScrollEvent &event)
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

  SetHScrollRange(m_Range, m_PageSize);
  OnHScrollChanged(m_Position);
}

////////////////////////////////////////////////////////////////////////////////

void
wHScrollWindow::OnSize(const wxSizeEvent& event)
{
  wxSize size = GetClientSize();
  if(m_Scrollbar != NULL) {
#ifdef WIN32
    m_Scrollbar->SetSize(0,
                          size.y,
                          size.x,
                          wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y));
#else
    m_Scrollbar->SetSize(0, size.y, size.x, 400);
#endif
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wHScrollWindow::SetHScrollInfo()
{
  m_Scrollbar->SetScrollbar(m_Position, m_PageSize, m_Range, m_PageSize);
}

////////////////////////////////////////////////////////////////////////////////

wxSize
wHScrollWindow::GetClientSize()
{
  wxSize size = wxWindow::GetClientSize();
#ifdef WIN32
  return wxSize(size.x, 
                size.y - wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y));
#else
  return wxSize(size.x, size.y - 400);
#endif
}

////////////////////////////////////////////////////////////////////////////////
