#pragma warning(disable : 4786)

#include "ScriptWindow.hpp"
#include "FileDialogs.hpp"
#include "NumberDialog.hpp"
#include "Project.hpp"
#include "Scripting.hpp"
#include "FileSystem.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
#include "resource.h"


const int ID_EDIT = 900;


BEGIN_MESSAGE_MAP(CScriptWindow, CSaveableDocumentWindow)

  ON_WM_SIZE()
  ON_WM_SETFOCUS()

  ON_COMMAND(ID_SCRIPT_CHECKSYNTAX,      OnScriptCheckSyntax)
  ON_COMMAND(ID_SCRIPT_FIND,             OnScriptFind)
  ON_COMMAND(ID_SCRIPT_REPLACE,          OnScriptReplace)
  ON_COMMAND(ID_SCRIPT_SETDEFAULTFONT,   OnScriptSetDefaultFont)
  ON_COMMAND(ID_SCRIPT_SETTABSIZE,       OnScriptSetTabSize)
  ON_COMMAND(ID_SCRIPT_SHOWCOLORS,       OnScriptShowColors)
  ON_COMMAND(ID_SCRIPT_ENABLEAUTOINDENT, OnScriptEnableAutoIndent)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CScriptWindow::CScriptWindow(const char* filename)
: CSaveableDocumentWindow(filename, IDR_SCRIPT)
, m_Created(false)
{
  SetSaved(filename != NULL);
  SetModified(false);

  Create();
  if (filename) {
    LoadScript(filename);
  }
}

////////////////////////////////////////////////////////////////////////////////

CScriptWindow::~CScriptWindow()
{
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptWindow::Create()
{
  // create the child window
  CSaveableDocumentWindow::Create(
    AfxRegisterWndClass(0, LoadCursor(NULL, IDC_ARROW), NULL, AfxGetApp()->LoadIcon(IDI_SCRIPT)));

  // creates the script view
  m_Edit.Create(this, this);

  m_Created = true;

  // update the size of the view
  RECT Rect;
  GetClientRect(&Rect);
  OnSize(0, Rect.right - Rect.left, Rect.bottom - Rect.top);

  // give the view focus
  m_Edit.SetFocus();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg bool
CScriptWindow::LoadScript(const char* filename)
{
  // get the file size
  int file_size = FileSize(GetDocumentPath());
  if (file_size == -1)
    return false;

  // open the file
  FILE* file = fopen(filename, "rb");
  if (file == NULL)
    return false;

  // allocate a temporary storage buffer and read it
  char* buffer = new char[file_size + 1];
  fread(buffer, sizeof(char), file_size, file);
  
  // null-terminate the string
  buffer[file_size] = 0;

  // put the buffer into the edit control
  m_Edit.SetWindowText(buffer);
  m_Edit.FormatAll();
  m_Edit.SetSel(0, 0);

  // delete the buffer and close the file
  delete[] buffer;
  fclose(file);

  SetModified(false);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnSize(UINT type, int cx, int cy)
{
  if (m_Created)
    m_Edit.MoveWindow(0, 0, cx, cy);

  CSaveableDocumentWindow::OnSize(type, cx, cy);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnSetFocus(CWnd* old)
{
  if (m_Created)
    m_Edit.SetFocus();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptCheckSyntax()
{
  CString text;
  m_Edit.GetWindowText(text);

  // verify the script
  sCompileError error;
  if (!VerifyScript(text, error))
  {
    // show the error message
    MessageBox(error.m_Message.c_str(), "Check syntax", MB_OK);
    if (error.m_Token.length() > 0)
      m_Edit.SetSel(error.m_TokenStart, error.m_TokenStart + error.m_Token.length());
  }
  else
    MessageBox("Script is valid");
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptFind()
{
  m_Edit.ScriptFindWord();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptReplace()
{
  m_Edit.ScriptFindReplaceWord();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptSetDefaultFont()
{
  LOGFONT lf;
  memset(&lf, 0, sizeof(lf));
  HDC screen = ::GetDC(NULL);
  lf.lfHeight = -MulDiv(Configuration::Get(KEY_SCRIPT_FONT_SIZE) / 10, GetDeviceCaps(screen, LOGPIXELSY), 72);
  ::ReleaseDC(NULL, screen);
  strcpy(lf.lfFaceName, Configuration::Get(KEY_SCRIPT_FONT_NAME).c_str());

  CFontDialog dialog(&lf, CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT);
  if (dialog.DoModal() == IDOK) {
    
    // put the dialog options into the configuration
    int size = dialog.GetSize();
    Configuration::Set(KEY_SCRIPT_FONT_SIZE, size);
    
    CString face = dialog.GetFaceName();
    const char* face_str = face.GetBuffer(0);
    Configuration::Set(KEY_SCRIPT_FONT_NAME, face_str);
    face.ReleaseBuffer();

    // apply the font to the current editor
    CFont font;
    font.CreatePointFont(size, face);
    m_Edit.SetFont(&font);
    m_Edit.FormatAll();
    m_Edit.Invalidate();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptSetTabSize()
{
  CNumberDialog dialog("Set Tab Size", "Tab Size", Configuration::Get(KEY_SCRIPT_TAB_SIZE), 1, 16);
  if (dialog.DoModal() == IDOK) {
    Configuration::Set(KEY_SCRIPT_TAB_SIZE, dialog.GetValue());
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptShowColors()
{
  m_Edit.ScriptShowColors();
}

////////////////////////////////////////////////////////////////////////////////v

afx_msg void 
CScriptWindow::OnScriptEnableAutoIndent()
{
  m_Edit.ScriptAutoIndent();
}

////////////////////////////////////////////////////////////////////////////////v

bool
CScriptWindow::GetSavePath(char* path)
{
  CScriptFileDialog Dialog(FDM_SAVE);
  if (Dialog.DoModal() != IDOK)
    return false;

  strcpy(path, Dialog.GetPathName());
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CScriptWindow::SaveDocument(const char* path)
{
  FILE* file = fopen(path, "wb");
  if (file == NULL)
    return false;

  CString text;
  m_Edit.GetWindowText(text);

  fwrite((const char*)text, 1, text.GetLength(), file);

  fclose(file);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptWindow::SV_ScriptChanged()
{
  if (!IsModified())
    SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////
