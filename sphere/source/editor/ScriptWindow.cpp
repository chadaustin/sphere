#pragma warning(disable : 4786)

#include <Scintilla.h>
#include <SciLexer.h>
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

  if (!Create()) {
    return;
  }

  if (filename) {
    LoadScript(filename);
  }
}

////////////////////////////////////////////////////////////////////////////////

bool
CScriptWindow::Create()
{
  static HINSTANCE scintilla;
  if (!scintilla) {
    scintilla = LoadLibrary("SciLexer.dll");
    if (!scintilla) {
      ::MessageBox(
        NULL,
        "Could not load Scintilla component (SciLexer.dll)",
        "Script Editor",
        MB_OK | MB_ICONERROR);
      delete this;
      return false;
    }
  }
  

  // create the child window
  CSaveableDocumentWindow::Create(
    AfxRegisterWndClass(0, LoadCursor(NULL, IDC_ARROW), NULL, AfxGetApp()->LoadIcon(IDI_SCRIPT)));

  // creates the script view
  m_Editor = ::CreateWindow(
    "Scintilla",
    "Source",
    WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN,
    0, 0,
    100, 100,
    m_hWnd,
    0,
    AfxGetApp()->m_hInstance,
    0);

  Initialize();
  ::ShowWindow(m_Editor, SW_SHOW);
  ::UpdateWindow(m_Editor);

  m_Created = true;

  // update the size of the view
  RECT Rect;
  GetClientRect(&Rect);
  OnSize(0, Rect.right - Rect.left, Rect.bottom - Rect.top);

  // give the view focus
  ::SetFocus(m_Editor);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptWindow::Initialize()
{
  static const char key_words[] = 
    "break case catch continue default delete do else "
    "finally for function if in instanceof new return "
    "switch this throw try typeof var void while with ";

  static const char reserved_words[] =
    "abstract enum int short "
    "boolean export interface static "
    "byte extends long super "
    "char final native synchronized "
    "class float package throws "
    "const goto private transient "
    "debugger implements protected volatile "
    "double import public ";

  static const COLORREF black  = 0x000000;
  static const COLORREF white  = 0xFFFFFF;
  static const COLORREF red    = RGB(0xFF, 0, 0);
  static const COLORREF green  = RGB(0, 0xFF, 0);
  static const COLORREF blue   = RGB(0, 0, 0xFF);
  static const COLORREF purple = RGB(0xFF, 0, 0xFF);
  static const COLORREF yellow = RGB(0xFF, 0xFF, 0);

  SendEditor(SCI_SETLEXERLANGUAGE, 0, (WPARAM)"javascript");
  SendEditor(SCI_SETSTYLEBITS, 5);
  SendEditor(SCI_SETKEYWORDS, 0, (LPARAM)key_words);

  SetStyle(STYLE_DEFAULT, black, white, 10, "Verdana");
  SendEditor(SCI_STYLECLEARALL);

  SetStyle(SCE_C_DEFAULT, red, white, 10, "Verdana");
  SetStyle(SCE_C_COMMENT, red);
  SetStyle(SCE_C_NUMBER, green);
  SetStyle(SCE_C_STRING, blue);
  SetStyle(SCE_C_IDENTIFIER, purple);
  SetStyle(SCE_C_WORD, yellow);
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptWindow::SetStyle(
  int style,
  COLORREF fore,
  COLORREF back,
  int size,
  const char* face)
{
  SendEditor(SCI_STYLESETFORE, style, fore);
  SendEditor(SCI_STYLESETBACK, style, back);
  if (size >= 1) {
    SendEditor(SCI_STYLESETSIZE, style, size);
  }
  if (face) {
    SendEditor(SCI_STYLESETFONT, style, (LPARAM)face);
  }
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
  SendEditor(SCI_SETTEXT, 0, (LPARAM)buffer);
  SendEditor(SCI_SETSEL,  0, 0);

  // delete the buffer and close the file
  delete[] buffer;
  fclose(file);

  SetModified(false);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptWindow::GetEditorText(CString& text)
{
  int length = SendEditor(SCI_GETLENGTH);
  char* str = new char[length + 1];
  str[length] = 0;

  SendEditor(SCI_GETTEXT, length + 1, (LPARAM)str);

  text = str;
  delete[] str;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnSize(UINT type, int cx, int cy)
{
  if (m_Created) {
    ::MoveWindow(m_Editor, 0, 0, cx, cy, TRUE);
  }

  CSaveableDocumentWindow::OnSize(type, cx, cy);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnSetFocus(CWnd* old)
{
  if (m_Created) {
    ::SetFocus(m_Editor);
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptCheckSyntax()
{
  CString text;
  GetEditorText(text);

/*
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
*/
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptFind()
{
//  m_Edit.ScriptFindWord();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptReplace()
{
//  m_Edit.ScriptFindReplaceWord();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptSetDefaultFont()
{
/*
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
*/
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptSetTabSize()
{
/*
  CNumberDialog dialog("Set Tab Size", "Tab Size", Configuration::Get(KEY_SCRIPT_TAB_SIZE), 1, 16);
  if (dialog.DoModal() == IDOK) {
    Configuration::Set(KEY_SCRIPT_TAB_SIZE, dialog.GetValue());
  }
*/
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptShowColors()
{
//  m_Edit.ScriptShowColors();
}

////////////////////////////////////////////////////////////////////////////////v

afx_msg void 
CScriptWindow::OnScriptEnableAutoIndent()
{
//  m_Edit.ScriptAutoIndent();
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
  GetEditorText(text);
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
