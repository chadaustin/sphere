#include "PaletteWindow.hpp"
#include "DocumentWindow.hpp"


BEGIN_MESSAGE_MAP(CPaletteWindow, CMiniFrameWnd)

  ON_WM_DESTROY()
  ON_WM_CLOSE()
  ON_WM_SHOWWINDOW()

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CPaletteWindow::CPaletteWindow(CDocumentWindow* owner, const char* name, RECT rect, bool visible)
: m_Owner(owner)
{
  owner->AttachPalette(this);

  // make sure the palette is visible!
  if (rect.right <= rect.left ||
      rect.bottom <= rect.top ||
      rect.left < 0 ||
      rect.top < 0 ||
      rect.left >= GetSystemMetrics(SM_CXSCREEN) - 1 ||
      rect.top >= GetSystemMetrics(SM_CYSCREEN) - 1) {
    rect.left = 64;
    rect.top = 64;
    rect.right = 256;
    rect.bottom = 256;
  }

  // !!!!
  // for now, all palettes are shown by default
  // change this in the future
  
  Create(
    AfxRegisterWndClass(
      0,
      ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW))
    ),
    name,
    WS_VISIBLE | WS_THICKFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN,
    rect,
    AfxGetApp()->m_pMainWnd,
    0
  );
  ShowWindow(SW_SHOW);

  ::SendMessage(::GetParent(m_hWnd), WM_UPDATE_PALETTE_MENU, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////

CPaletteWindow::~CPaletteWindow()
{
  m_Owner->DetachPalette(this);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CPaletteWindow::OnDestroy()
{
  ::SendMessage(::GetParent(m_hWnd), WM_UPDATE_PALETTE_MENU, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CPaletteWindow::OnClose()
{
  ShowWindow(SW_HIDE);
  ::SendMessage(::GetParent(m_hWnd), WM_UPDATE_PALETTE_MENU, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CPaletteWindow::OnShowWindow(BOOL show, UINT status)
{
  ::SendMessage(::GetParent(m_hWnd), WM_UPDATE_PALETTE_MENU, 0, 0);
  CMiniFrameWnd::OnShowWindow(show, status);
}

////////////////////////////////////////////////////////////////////////////////
