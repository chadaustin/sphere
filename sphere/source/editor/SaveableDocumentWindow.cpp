#pragma warning(disable : 4786)


#include "SaveableDocumentWindow.hpp"
#include "MainWindow.hpp"
#include "resource.h"


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
    strcpy(text, GetCaption());
  }
  else
  {
    text = new char[20];
    strcpy(text, "Untitled");
  }
  
  if (m_Modified)
    strcat(text, " *");

  SetWindowText(text);
  delete[] text;
}

////////////////////////////////////////////////////////////////////////////////

bool
CSaveableDocumentWindow::IsSaveable() const
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSaveableDocumentWindow::OnFileSave()
{
  if (m_Saved)
  {
    if (!SaveDocument(GetDocumentPath()))
    {
      char message[1024];
      sprintf(message, "Error: Could not save document '%s'", GetDocumentPath());
      MessageBox(message);
    }

    // update project if it's there
    UpdateProject();

    m_Saved = true;
    m_Modified = false;
    UpdateWindowCaption();
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
    if (SaveDocument(document_path))
    {
      // update project if it's there
      UpdateProject();

      m_Saved = true;
      m_Modified = false;
      SetDocumentPath(document_path);
      SetCaption(GetDocumentTitle());
      UpdateWindowCaption();
    }
    else
    {
      char message[1024];
      sprintf(message, "Error: Could not save document '%s'", document_path);
      MessageBox(message);
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
    if (!SaveDocument(document_path))
    {
      char message[1024];
      sprintf(message, "Error: Could not save document '%s'", GetDocumentPath());
      MessageBox(message);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CSaveableDocumentWindow::UpdateProject()
{
  AfxGetApp()->m_pMainWnd->SendMessage(WM_REFRESH_PROJECT);
}

////////////////////////////////////////////////////////////////////////////////
