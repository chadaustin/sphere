#include <wx/wx.h>
#include "PaletteWindow.hpp"
#include "DocumentWindow.hpp"

/*
BEGIN_MESSAGE_MAP(CPaletteWindow, CMiniFrameWnd)

  ON_WM_DESTROY()
  ON_WM_CLOSE()
  ON_WM_SHOWWINDOW()

END_MESSAGE_MAP()
*/
BEGIN_EVENT_TABLE(wPaletteWindow, wxFrame)
  EVT_CLOSE(wPaletteWindow::OnClose)
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

//wDocumentWindow* owner, const char* name, RECT rect, bool visible)
wPaletteWindow::wPaletteWindow(wDocumentWindow* owner, const char* name, 
                               wxRect rect, bool visible)
: m_Owner(owner)
{
  owner->AttachPalette(this);
  wxSize size = rect.GetSize();
  wxPoint pos = rect.GetPosition();

  // make sure the palette is visible!
  if (size.GetWidth() <= 16 ||
      size.GetHeight() <= 16 ||
      pos.x < 0 ||
      pos.y < 0 ||
      pos.x >= wxSystemSettings::GetSystemMetric(wxSYS_SCREEN_X) - 1 ||
      pos.y >= wxSystemSettings::GetSystemMetric(wxSYS_SCREEN_Y) - 1) {
    pos.x = 64;
    pos.y = 64;
    size.Set(256,256);
  }

  // !!!!
  // for now, all palettes are shown by default
  // change this in the future
  Create(owner, -1, name, pos, size, wxTHICK_FRAME | wxCAPTION /*| wxTINY_CAPTION_HORIZ*/ | wxFRAME_TOOL_WINDOW | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR /*| wxSTAY_ON_TOP | wxMINIMIZE_BOX | wxSYSTEM_MENU*/);

  Show();/*todo:fix this*/
/*todo:
  ::SendMessage(::GetParent(m_hWnd), WM_UPDATE_PALETTE_MENU, 0, 0);
*/
}

////////////////////////////////////////////////////////////////////////////////

wPaletteWindow::~wPaletteWindow()
{
  m_Owner->DetachPalette(this);
}

/*
////////////////////////////////////////////////////////////////////////////////

afx_msg void
CPaletteWindow::OnDestroy()
{
  ::SendMessage(::GetParent(m_hWnd), WM_UPDATE_PALETTE_MENU, 0, 0);
}
*/
////////////////////////////////////////////////////////////////////////////////

void
wPaletteWindow::OnClose(wxCloseEvent& event)
{
/*todo:
//  ShowWindow(SW_HIDE);
//  ::SendMessage(::GetParent(m_hWnd), WM_UPDATE_PALETTE_MENU, 0, 0);
*/
    this->Destroy();
}
/* todo:
////////////////////////////////////////////////////////////////////////////////

afx_msg void
CPaletteWindow::OnShowWindow(BOOL show, UINT status)
{
//  ::SendMessage(::GetParent(m_hWnd), WM_UPDATE_PALETTE_MENU, 0, 0);
//  CMiniFrameWnd::OnShowWindow(show, status);
}

////////////////////////////////////////////////////////////////////////////////
*/
