#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include "SaveableDocumentWindow.hpp"
#include "MainWindow.hpp"
//#include "resource.h"
#include "IDs.hpp"

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

/*
BEGIN_MESSAGE_MAP(CSaveableDocumentWindow, CDocumentWindow)
  
  ON_COMMAND(ID_FILE_SAVE,       OnFileSave)
  ON_COMMAND(ID_FILE_SAVEAS,     OnFileSaveAs)
  ON_COMMAND(ID_FILE_SAVECOPYAS, OnFileSaveCopyAs)

END_MESSAGE_MAP()
*/
BEGIN_EVENT_TABLE(wSaveableDocumentWindow, wDocumentWindow)
  EVT_MENU(wxID_SAVE, wSaveableDocumentWindow::OnFileSave)
  EVT_MENU(wxID_SAVEAS, wSaveableDocumentWindow::OnFileSaveAs)

  EVT_MENU(wID_FILE_SAVE, wSaveableDocumentWindow::OnFileSave)
  EVT_MENU(wID_FILE_SAVEAS, wSaveableDocumentWindow::OnFileSaveAs)
  EVT_MENU(wID_FILE_SAVECOPYAS, wSaveableDocumentWindow::OnFileSaveCopyAs)

  EVT_COMMAND(wxID_SAVE, -1, wSaveableDocumentWindow::OnFileSave)
  EVT_COMMAND(wxID_SAVEAS, -1, wSaveableDocumentWindow::OnFileSaveAs)

  EVT_COMMAND(wID_FILE_SAVE, -1, wSaveableDocumentWindow::OnFileSave)
  EVT_COMMAND(wID_FILE_SAVEAS, -1, wSaveableDocumentWindow::OnFileSaveAs)
  EVT_COMMAND(wID_FILE_SAVECOPYAS, -1, wSaveableDocumentWindow::OnFileSaveCopyAs)
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wSaveableDocumentWindow::wSaveableDocumentWindow(const char* document_path, int menu_resource, const wxSize& min_size)
: wDocumentWindow(document_path, menu_resource, min_size)
, m_Saved(false)
, m_Modified(false)
{
}

////////////////////////////////////////////////////////////////////////////////

wSaveableDocumentWindow::~wSaveableDocumentWindow()
{
}

////////////////////////////////////////////////////////////////////////////////

bool
wSaveableDocumentWindow::Save()
{
  wxCommandEvent evt;
  OnFileSave(evt);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
wSaveableDocumentWindow::Close()
{
  if (m_Modified)
  {
    int retval = wxMessageBox("File has been modified, save?", "Save before closing?", wxYES_NO | wxCANCEL);
    if (retval == wxID_CANCEL)
      return false;

    if (retval == wxYES) {
      wxCommandEvent evt;
      OnFileSave(evt);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
wSaveableDocumentWindow::SetSaved(bool saved)
{
  m_Saved = saved;
  UpdateWindowCaption();
}

////////////////////////////////////////////////////////////////////////////////

void
wSaveableDocumentWindow::SetModified(bool modified)
{
  m_Modified = modified;
  UpdateWindowCaption();
}

////////////////////////////////////////////////////////////////////////////////

bool
wSaveableDocumentWindow::IsSaved() const
{
  return m_Saved;
}

////////////////////////////////////////////////////////////////////////////////

bool
wSaveableDocumentWindow::IsModified() const
{
  return m_Modified;
}

////////////////////////////////////////////////////////////////////////////////

const char*
wSaveableDocumentWindow::GetDocumentTitle() const
{
  if (m_Saved)
    return wDocumentWindow::GetDocumentTitle();
  else
    return "Untitled";
}

////////////////////////////////////////////////////////////////////////////////

void
wSaveableDocumentWindow::UpdateWindowCaption()
{
/*
  if (!IsWindow(m_hWnd))
    return;
*/
  char* text = NULL;
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

  SetTitle(text);
  delete[] text;
}

////////////////////////////////////////////////////////////////////////////////

bool
wSaveableDocumentWindow::IsSaveable() const
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
wSaveableDocumentWindow::OnFileSave(wxCommandEvent &event)
{
  if (m_Saved)
  {
    if (!SaveDocument(GetDocumentPath()))
    {
      char message[1024];
      sprintf(message, "Error: Could not save document '%s'", GetDocumentPath());
      wxMessageBox(message);
    }

    // update project if it's there
    UpdateProject();

    m_Saved = true;
    m_Modified = false;
    UpdateWindowCaption();
  }
  else {
    wxCommandEvent evt;
    OnFileSaveAs(evt);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wSaveableDocumentWindow::OnFileSaveAs(wxCommandEvent &event)
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
      sprintf(message, "Error: Could not save document '%s'", GetDocumentPath());
      wxMessageBox(message);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wSaveableDocumentWindow::OnFileSaveCopyAs(wxCommandEvent &event)
{
  char document_path[MAX_PATH];
  if (GetSavePath(document_path))
  {
    if (!SaveDocument(document_path))
    {
      char message[1024];
      sprintf(message, "Error: Could not save document '%s'", GetDocumentPath());
      wxMessageBox(message);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wSaveableDocumentWindow::UpdateProject()
{
/*todo:
  AfxGetApp()->m_pMainWnd->SendMessage(WM_REFRESH_PROJECT);
*/
}

////////////////////////////////////////////////////////////////////////////////
