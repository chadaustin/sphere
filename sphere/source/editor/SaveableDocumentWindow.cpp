#pragma warning(disable : 4786)


#include "SaveableDocumentWindow.hpp"
#include "MainWindow.hpp"
#include "resource.h"


#include "Editor.hpp"

#include <errno.h>


BEGIN_MESSAGE_MAP(CSaveableDocumentWindow, CDocumentWindow)

  ON_COMMAND(ID_FILE_SAVE,       OnFileSave)
  ON_COMMAND(ID_FILE_SAVEAS,     OnFileSaveAs)
  ON_COMMAND(ID_FILE_SAVECOPYAS, OnFileSaveCopyAs)

END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

CSaveableDocumentWindow::CSaveableDocumentWindow(const char* document_path, int menu_resource, const CSize& min_size)
: CDocumentWindow(document_path, menu_resource, min_size)
, m_Saved(false)
, m_Modified(false)
{
}

////////////////////////////////////////////////////////////////////////////////

CSaveableDocumentWindow::~CSaveableDocumentWindow()
{
}

////////////////////////////////////////////////////////////////////////////////

bool
CSaveableDocumentWindow::Save()
{
  OnFileSave();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CSaveableDocumentWindow::Close()
{
  if (m_Modified)
  {
    int retval = MessageBox("File has been modified, save?", NULL, MB_YESNOCANCEL);
    if (retval == IDCANCEL)
      return false;

    if (retval == IDYES)
      OnFileSave();
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
CSaveableDocumentWindow::SetSaved(bool saved)
{
  m_Saved = saved;
  UpdateWindowCaption();
}

////////////////////////////////////////////////////////////////////////////////

void
CSaveableDocumentWindow::SetModified(bool modified)
{
  m_Modified = modified;
  UpdateWindowCaption();
}

////////////////////////////////////////////////////////////////////////////////

bool
CSaveableDocumentWindow::IsSaved() const
{
  return m_Saved;
}

////////////////////////////////////////////////////////////////////////////////

bool
CSaveableDocumentWindow::IsModified() const
{
  return m_Modified;
}

////////////////////////////////////////////////////////////////////////////////

const char*
CSaveableDocumentWindow::GetDocumentTitle() const
{
  if (m_Saved)
    return CDocumentWindow::GetDocumentTitle();
  else
    return "Untitled";
}

////////////////////////////////////////////////////////////////////////////////

void
CSaveableDocumentWindow::UpdateWindowCaption()
{
  if (!IsWindow(m_hWnd))
    return;

  char* text;
  if (m_Saved)
  {
    text = new char[strlen(GetCaption()) + 3];
    if (text) strcpy(text, GetCaption());
  }
  else
  {
    text = new char[20];
    if (text) strcpy(text, "Untitled");
  }

  if (m_Modified)
    if (text) strcat(text, " *");

  SetWindowText(text ? text : "(null)");
  delete[] text;

#ifdef TABBED_WINDOW_LIST
  CFrameWnd* pFrame = (CFrameWnd*)AfxGetApp()->m_pMainWnd;
  ((CMainWindow*)pFrame)->OnUpdateFrameTitle(TRUE);
#endif
}

////////////////////////////////////////////////////////////////////////////////

bool
CSaveableDocumentWindow::IsSaveable() const
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CSaveableDocumentWindow::__SaveDocument__(const char* document_path)
{
  errno = 0;
  if (!SaveDocument(document_path)) {
    const int error_code = errno;

    char message[MAX_PATH + 1024];
    sprintf(message, "Error: Could not save document '%s'\n", GetDocumentPath());

    if (error_code != 0) {
      sprintf(message + strlen(message), "Reason: %s", strerror(error_code));
    }

    MessageBox(message);

    errno = 0;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSaveableDocumentWindow::OnFileSave()
{
  if (m_Saved)
  {
    if (__SaveDocument__(GetDocumentPath()))
    {
      // update project if it's there
      UpdateProject();

      m_Saved = true;
      m_Modified = false;
      UpdateWindowCaption();
    }
  }
  else
    OnFileSaveAs();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSaveableDocumentWindow::OnFileSaveAs()
{
  char document_path[MAX_PATH];
  if (GetSavePath(document_path))
  {
    if (__SaveDocument__(document_path))
    {
      // update project if it's there
      UpdateProject();

      m_Saved = true;
      m_Modified = false;
      SetDocumentPath(document_path);
      SetCaption(GetDocumentTitle());
      UpdateWindowCaption();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSaveableDocumentWindow::OnFileSaveCopyAs()
{
  char document_path[MAX_PATH];
  if (GetSavePath(document_path))
  {
    __SaveDocument__(document_path);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CSaveableDocumentWindow::UpdateProject()
{
  AfxGetApp()->m_pMainWnd->SendMessage(WM_REFRESH_PROJECT);
}

////////////////////////////////////////////////////////////////////////////////

void CSaveableDocumentWindow::OnToolChanged(UINT id, int tool_index) { }
BOOL CSaveableDocumentWindow::IsToolAvailable(UINT id) { return FALSE; }

////////////////////////////////////////////////////////////////////////////////

