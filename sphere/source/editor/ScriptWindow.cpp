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
const UINT s_FindReplaceMessage = ::RegisterWindowMessage(FINDMSGSTRING);


BEGIN_MESSAGE_MAP(CScriptWindow, CSaveableDocumentWindow)

  ON_WM_SIZE()
  ON_WM_SETFOCUS()

  ON_COMMAND(ID_SCRIPT_CHECKSYNTAX,      OnScriptCheckSyntax)
  ON_COMMAND(ID_SCRIPT_FIND,             OnScriptFind)
  ON_COMMAND(ID_SCRIPT_REPLACE,          OnScriptReplace)

  ON_NOTIFY(SCN_SAVEPOINTREACHED, ID_EDIT, OnSavePointReached)
  ON_NOTIFY(SCN_SAVEPOINTLEFT,    ID_EDIT, OnSavePointLeft)

  ON_REGISTERED_MESSAGE(s_FindReplaceMessage, OnFindReplace)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CScriptWindow::CScriptWindow(const char* filename)
: CSaveableDocumentWindow(filename, IDR_SCRIPT)
, m_Created(false)
, m_SearchDialog(0)
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
    (HMENU)ID_EDIT,
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

  static const COLORREF black   = 0x000000;
  static const COLORREF white   = 0xFFFFFF;
  static const COLORREF red     = RGB(0xFF, 0, 0);
  static const COLORREF green   = RGB(0, 0x80, 0);
  static const COLORREF blue    = RGB(0, 0, 0xFF);
  static const COLORREF purple  = RGB(0xFF, 0, 0xFF);
  static const COLORREF yellow  = RGB(0xFF, 0xFF, 0);
  static const COLORREF brown   = RGB(0xB5, 0x6F, 0x32);
  static const COLORREF darkred = RGB(0x80, 0, 0);

  SendEditor(SCI_SETLEXER, SCLEX_CPP);  // JavaScript uses the C++ lexer
  SendEditor(SCI_SETSTYLEBITS, 5);
  SendEditor(SCI_SETKEYWORDS, 0, (LPARAM)key_words);
  SendEditor(SCI_SETKEYWORDS, 1, (LPARAM)reserved_words);

  SendEditor(SCI_SETPROPERTY, (WPARAM)"fold", (LPARAM)"1");

  SetStyle(STYLE_DEFAULT, black, white, 10, "Verdana");
  SendEditor(SCI_STYLECLEARALL);

  SetStyle(SCE_C_DEFAULT, black, white, 10, "Verdana");
  SetStyle(SCE_C_COMMENT,     green);
  SetStyle(SCE_C_COMMENTLINE, green);
  SetStyle(SCE_C_COMMENTDOC,  green);
  SetStyle(SCE_C_NUMBER,      darkred);
  SetStyle(SCE_C_WORD,        blue);
  SendEditor(SCI_STYLESETBOLD, SCE_C_WORD, 1);
  SetStyle(SCE_C_STRING,      green);
  SetStyle(SCE_C_CHARACTER,   green);
  SetStyle(SCE_C_OPERATOR,    purple);
  SetStyle(SCE_C_IDENTIFIER,  black);
  SetStyle(SCE_C_WORD2,       red);
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
  SendEditor(SCI_SETSAVEPOINT);

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

CString
CScriptWindow::GetSelection()
{
  int start = SendEditor(SCI_GETSELECTIONSTART);
  int end   = SendEditor(SCI_GETSELECTIONEND);
  if (end < start) {
    end = start;
  }
  int length = end - start;
  char* str = new char[length + 1];
  str[length] = 0;
  SendEditor(SCI_GETSELTEXT, 0, (LPARAM)str);
  CString result(str);
  delete[] str;
  return result;
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

  // verify the script
  sCompileError error;
  if (!VerifyScript(text, error))
  {
    // show the error message
    MessageBox(error.m_Message.c_str(), "Check syntax", MB_OK);
    if (error.m_Token.length() > 0)
      SendEditor(
        SCI_SETSEL,
        error.m_TokenStart,
        error.m_TokenStart + error.m_Token.length());
  }
  else
    MessageBox("Script is valid");
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptFind()
{
  if (!m_SearchDialog) {
    m_SearchDialog = new CFindReplaceDialog;
    m_SearchDialog->Create(true, GetSelection(), NULL, FR_DOWN, this);
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptReplace()
{
  if (!m_SearchDialog) {
    m_SearchDialog = new CFindReplaceDialog;
    m_SearchDialog->Create(false, GetSelection(), NULL, FR_DOWN, this);
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnSavePointReached(NMHDR* nmhdr, LRESULT* result)
{
  SetModified(false);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnSavePointLeft(NMHDR* nmhdr, LRESULT* result)
{
  SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg LRESULT
CScriptWindow::OnFindReplace(WPARAM, LPARAM)
{
  CString str;
  GetEditorText(str);

  if (m_SearchDialog->IsTerminating()) {
    m_SearchDialog = NULL;
  } else if (m_SearchDialog->FindNext()) {

    TextToFind ttf;
    ttf.chrg.cpMin = SendEditor(SCI_GETSELECTIONSTART) + 1;
    ttf.chrg.cpMax = str.GetLength();
    CString find_string(m_SearchDialog->GetFindString());
    ttf.lpstrText = find_string.GetBuffer(0);
    
    int options = 0;
    options |= m_SearchDialog->MatchCase() ? SCFIND_MATCHCASE : 0;
    options |= m_SearchDialog->MatchWholeWord() ? SCFIND_WHOLEWORD : 0;
    if (SendEditor(SCI_FINDTEXT, options, (LPARAM)&ttf) == -1) {
      m_SearchDialog->MessageBox("No more matches!");
    } else {
      SendEditor(SCI_SETSEL, ttf.chrgText.cpMin, ttf.chrgText.cpMax);
    }
    
  } else if (m_SearchDialog->ReplaceCurrent()) {

    // get currently selected text
    CString selection(GetSelection());
    CString find_string(m_SearchDialog->GetFindString());

    // build the data structure we need to find the next text
    // before we replace the string
    TextToFind ttf;
    ttf.chrg.cpMin = SendEditor(SCI_GETSELECTIONSTART);
    ttf.chrg.cpMax = str.GetLength();
    ttf.lpstrText = find_string.GetBuffer(0);
    int options = 0;  // ?

    // if the selection is what we want to replace, then do so
    CString replace_string(m_SearchDialog->GetReplaceString());
    if (selection == find_string) {

      // actually do the string replacing
      SendEditor(SCI_REPLACESEL, 0, (LPARAM)(const char*)replace_string);

      // now try to find the next one
      ++ttf.chrg.cpMin;
      if (SendEditor(SCI_FINDTEXT, options, (LPARAM)&ttf) == -1) {
        m_SearchDialog->MessageBox("No more matches!");
      } else {
        SendEditor(SCI_SETSEL, ttf.chrgText.cpMin, ttf.chrgText.cpMax);
      }
    } else {

      if (SendEditor(SCI_FINDTEXT, options, (LPARAM)&ttf) == -1) {
        m_SearchDialog->MessageBox("No matches!");
      } else {
        SendEditor(SCI_SETSEL, ttf.chrgText.cpMin, ttf.chrgText.cpMax);
      }
    }

  } else if (m_SearchDialog->ReplaceAll()) {

    // get currently selected text
    CString selection(GetSelection());
    CString find_string(m_SearchDialog->GetFindString());
    CString replace_string(m_SearchDialog->GetReplaceString());

    // build the data structure we need to find the next text
    // before we replace the string
    TextToFind ttf;
    ttf.chrg.cpMin = SendEditor(SCI_GETSELECTIONSTART);
    ttf.chrg.cpMax = str.GetLength();
    ttf.lpstrText = find_string.GetBuffer(0);
    int options = 0;  // ?

    if (selection != find_string) {
      if (SendEditor(SCI_FINDTEXT, options, (LPARAM)&ttf) != -1) {
        SendEditor(SCI_SETSEL, ttf.chrgText.cpMin, ttf.chrgText.cpMax);
      }
    }

    // if the selection is what we want to replace, then do so
    while (GetSelection() == find_string) {

      // actually do the string replacing
      SendEditor(SCI_REPLACESEL, 0, (LPARAM)(const char*)replace_string);

      // now try to find the next one
      ++ttf.chrg.cpMin;
      if (SendEditor(SCI_FINDTEXT, options, (LPARAM)&ttf) != -1) {
        SendEditor(SCI_SETSEL, ttf.chrgText.cpMin, ttf.chrgText.cpMax);
      }
    }
    
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

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

  SendEditor(SCI_SETSAVEPOINT);
  return true;
}

////////////////////////////////////////////////////////////////////////////////
