#include "DocumentWindow.hpp"
#include "WindowCommands.hpp"
#include "PaletteWindow.hpp"
#include "resource.h"


BEGIN_MESSAGE_MAP(CDocumentWindow, CMDIChildWnd)

  ON_WM_CREATE()
  ON_WM_CLOSE()
  ON_WM_SIZING()
  ON_WM_MDIACTIVATE()

  ON_UPDATE_COMMAND_UI(ID_FILE_SAVE,       OnUpdateSaveableCommand)
  ON_UPDATE_COMMAND_UI(ID_FILE_SAVEAS,     OnUpdateSaveableCommand)
  ON_UPDATE_COMMAND_UI(ID_FILE_SAVECOPYAS, OnUpdateSaveableCommand)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CDocumentWindow::CDocumentWindow(const char* document_path, int menu_resource, const CSize& min_size)
: m_MenuResource(menu_resource)
, m_MinSize(min_size)
{
  if (document_path) {
    strcpy(m_DocumentPath, document_path);
  } else {
    strcpy(m_DocumentPath, "");
  }
  strcpy(m_Caption, GetDocumentTitle());
}

////////////////////////////////////////////////////////////////////////////////

CDocumentWindow::~CDocumentWindow()
{
  // this destructor can be called from a constructor, and the main window would
  // try to remove the window pointer before it was added
  AfxGetApp()->m_pMainWnd->PostMessage(WM_DW_CLOSING, 0, (LPARAM)this);
}

////////////////////////////////////////////////////////////////////////////////

bool
CDocumentWindow::Close()
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////

const char*
CDocumentWindow::GetFilename() const
{
  return m_DocumentPath;
}

////////////////////////////////////////////////////////////////////////////////

void
CDocumentWindow::AttachPalette(CPaletteWindow* palette)
{
  m_AttachedPalettes.push_back(palette);
}

////////////////////////////////////////////////////////////////////////////////

void
CDocumentWindow::DetachPalette(CPaletteWindow* palette)
{
  for (int i = 0; i < m_AttachedPalettes.size(); i++) {
    if (m_AttachedPalettes[i] == palette) {
      m_AttachedPalettes.erase(m_AttachedPalettes.begin() + i);
      return;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

int
CDocumentWindow::GetNumPalettes() const
{
  return m_AttachedPalettes.size();
}

////////////////////////////////////////////////////////////////////////////////

CPaletteWindow*
CDocumentWindow::GetPalette(int i) const
{
  return m_AttachedPalettes[i];
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CDocumentWindow::Create(LPCTSTR class_name)
{
  BOOL result = CMDIChildWnd::Create(class_name, "");
  UpdateWindowCaption();
  return result;
}

////////////////////////////////////////////////////////////////////////////////

const char*
CDocumentWindow::GetDefaultWindowClass()
{
  return AfxRegisterWndClass(0, ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)), NULL, NULL);
}

////////////////////////////////////////////////////////////////////////////////

void
CDocumentWindow::SetDocumentPath(const char* path)
{
  strcpy(m_DocumentPath, path);
}

////////////////////////////////////////////////////////////////////////////////

const char*
CDocumentWindow::GetDocumentPath() const
{
  return m_DocumentPath;
}

////////////////////////////////////////////////////////////////////////////////

const char*
CDocumentWindow::GetDocumentTitle() const
{
  if (strrchr(m_DocumentPath, '\\') == NULL)
    return m_DocumentPath;
  else
    return strrchr(m_DocumentPath, '\\') + 1;
}

////////////////////////////////////////////////////////////////////////////////

void
CDocumentWindow::SetCaption(const char* caption)
{
  strcpy(m_Caption, caption);
  UpdateWindowCaption();
}

////////////////////////////////////////////////////////////////////////////////

const char*
CDocumentWindow::GetCaption()
{
  return m_Caption;
}

////////////////////////////////////////////////////////////////////////////////

void
CDocumentWindow::UpdateWindowCaption()
{
  SetWindowText(m_Caption);
}

////////////////////////////////////////////////////////////////////////////////

bool
CDocumentWindow::IsSaveable() const
{
  return false;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CDocumentWindow::OnCreate(LPCREATESTRUCT cs)
{
  SetWindowLong(m_hWnd, GWL_USERDATA, WA_DOCUMENT_WINDOW | (IsSaveable() ? WA_SAVEABLE : 0));
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CDocumentWindow::OnClose()
{
  if (Close())
    CMDIChildWnd::OnClose();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CDocumentWindow::OnSizing(UINT side, LPRECT rect)
{
  CMDIChildWnd::OnSizing(side, rect);

  if (m_MinSize == CSize(0, 0))
    return;

  int width = rect->right - rect->left;
  int height = rect->bottom - rect->top;

  // check sides; if they are too small, resize them

  if (side == WMSZ_TOPRIGHT ||
      side == WMSZ_RIGHT ||
      side == WMSZ_BOTTOMRIGHT)
  {
    if (width < m_MinSize.cx)
      rect->right = rect->left + m_MinSize.cx;
  }

  if (side == WMSZ_TOPLEFT ||
      side == WMSZ_LEFT ||
      side == WMSZ_BOTTOMLEFT)
  {
    if (width < m_MinSize.cx)
      rect->left = rect->right - m_MinSize.cx;
  }

  if (side == WMSZ_BOTTOMLEFT ||
      side == WMSZ_BOTTOM ||
      side == WMSZ_BOTTOMRIGHT)
  {
    if (height < m_MinSize.cy)
      rect->bottom = rect->top + m_MinSize.cy;
  }

  if (side == WMSZ_TOPLEFT ||
      side == WMSZ_TOP ||
      side == WMSZ_TOPRIGHT)
  {
    if (height < m_MinSize.cy)
      rect->top = rect->bottom - m_MinSize.cy;
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CDocumentWindow::OnMDIActivate(BOOL activate, CWnd* active_window, CWnd* inactive_window)
{
  if (activate)
  {
    // set the child menu resource
    AfxGetApp()->m_pMainWnd->SendMessage(WM_SET_CHILD_MENU, m_MenuResource);

    // display the palettes
    for (int i = 0; i < m_AttachedPalettes.size(); i++) {
      m_AttachedPalettes[i]->ShowWindow(SW_SHOW);
    }
  }
  else
  {
    // clear the child menu
    AfxGetApp()->m_pMainWnd->SendMessage(WM_CLEAR_CHILD_MENU);

    // hide the palettes
    for (int i = 0; i < m_AttachedPalettes.size(); i++) {
      m_AttachedPalettes[i]->ShowWindow(SW_HIDE);
    }
  }
  
  CMDIChildWnd::OnMDIActivate(activate, active_window, inactive_window);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CDocumentWindow::OnUpdateSaveableCommand(CCmdUI* cmdui)
{
  cmdui->Enable(IsSaveable() == true);
}

////////////////////////////////////////////////////////////////////////////////