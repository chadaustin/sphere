#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

//#include <Scintilla.h>
//#include <SciLexer.h>

#ifdef WIN32
#include <wx/stc/stc.h>
#endif

#include "Editor.hpp"
#include "ScriptWindow.hpp"
#include "FileDialogs.hpp"
//#include "NumberDialog.hpp"
#include "Project.hpp"
#include "Scripting.hpp"
#include "FileSystem.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
//#include "resource.h"
#include "IDs.hpp"

//const int ID_EDIT = 900;
//const UINT s_FindReplaceMessage = ::RegisterWindowMessage(FINDMSGSTRING);

/*
BEGIN_MESSAGE_MAP(CScriptWindow, CSaveableDocumentWindow)

  ON_WM_SIZE()
  ON_WM_SETFOCUS()

  ON_COMMAND(ID_SCRIPT_CHECKSYNTAX,      OnScriptCheckSyntax)
  ON_COMMAND(ID_SCRIPT_FIND,             OnScriptFind)
  ON_COMMAND(ID_SCRIPT_REPLACE,          OnScriptReplace)

  ON_NOTIFY(SCN_SAVEPOINTREACHED, ID_EDIT, OnSavePointReached)
  ON_NOTIFY(SCN_SAVEPOINTLEFT,    ID_EDIT, OnSavePointLeft)
  ON_NOTIFY(SCN_UPDATEUI,         ID_EDIT, OnPosChanged)
  ON_NOTIFY(SCN_CHARADDED,        ID_EDIT, OnCharAdded)

  ON_REGISTERED_MESSAGE(s_FindReplaceMessage, OnFindReplace)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wScriptWindow, wSaveableDocumentWindow)
  EVT_SIZE(wScriptWindow::OnSize)
  EVT_SET_FOCUS(wScriptWindow::OnSetFocus)

  EVT_MENU(wID_SCRIPT_CHECKSYNTAX,  wScriptWindow::OnScriptCheckSyntax)

#ifdef WIN32
  EVT_MENU(wID_SCRIPT_FIND,         wScriptWindow::OnScriptFind)
  EVT_MENU(wID_SCRIPT_REPLACE,      wScriptWindow::OnScriptReplace)

  EVT_STC_SAVEPOINTREACHED(wID_SCRIPT_ID, wScriptWindow::OnSavePointReached)
  EVT_STC_SAVEPOINTLEFT(wID_SCRIPT_ID,    wScriptWindow::OnSavePointLeft)
  EVT_STC_UPDATEUI(wID_SCRIPT_ID,         wScriptWindow::OnPosChanged)
  EVT_STC_CHARADDED(wID_SCRIPT_ID,        wScriptWindow::OnCharAdded)

  EVT_FIND(-1, wScriptWindow::OnFindReplace)
#endif

END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wScriptWindow::wScriptWindow(const char* filename)
: wSaveableDocumentWindow(filename, wID_SCRIPT_base)
, m_Created(false)
#ifdef WIN32
, m_SearchDialog(0)
#endif
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
wScriptWindow::Create()
{
/*
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
*/
/*
  // create the child window
  wSaveableDocumentWindow::Create(
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
*/

#ifdef WIN32
  m_Editor = new wxStyledTextCtrl(this, wID_SCRIPT_ID);
#else
  m_Editor = new wxTextCtrl(this, wID_SCRIPT_ID, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
#endif

  Initialize();
  //::ShowWindow(m_Editor, SW_SHOW);
  //::UpdateWindow(m_Editor);

  m_Created = true;

  // update the size of the view
  //RECT Rect;
  //GetClientRect(&Rect);
  //OnSize(0, Rect.right - Rect.left, Rect.bottom - Rect.top);

  // give the view focus
  //::SetFocus(m_Editor);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
wScriptWindow::Initialize()
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

#ifdef WIN32
  static const wxColour black   = wxColour(0x00, 0x00, 0x00);
  static const wxColour white   = wxColour(0xff, 0xff, 0xff);
  static const wxColour red     = wxColour(0xff, 0x00, 0x00);
  static const wxColour green   = wxColour(0x00, 0x80, 0x00);
  static const wxColour blue    = wxColour(0x00, 0x00, 0xff);
  static const wxColour purple  = wxColour(0xFF, 0, 0xFF);
  static const wxColour yellow  = wxColour(0xFF, 0xFF, 0);
  static const wxColour brown   = wxColour(0xB5, 0x6F, 0x32);
  static const wxColour darkred = wxColour(0x80, 0, 0);

  m_Editor->SetLexer(wxSTC_LEX_CPP);  // JavaScript uses the C++ lexer
  m_Editor->SetStyleBits(5);
  m_Editor->SetKeyWords(0, key_words);
  m_Editor->SetKeyWords(1, reserved_words);

  m_Editor->SetProperty("fold", "1");

  SetStyle(wxSTC_STYLE_DEFAULT, black, white, 9, "Terminal");
  m_Editor->StyleClearAll();

  SetStyle(wxSTC_C_DEFAULT, black, white, 9, "Terminal");
  SetStyle(wxSTC_C_COMMENT,     green);
  SetStyle(wxSTC_C_COMMENTLINE, green);
  SetStyle(wxSTC_C_COMMENTDOC,  green);
  SetStyle(wxSTC_C_NUMBER,      darkred);
  SetStyle(wxSTC_C_WORD,        blue);
  m_Editor->StyleSetBold(wxSTC_C_WORD, 1);
  SetStyle(wxSTC_C_STRING,      green);
  SetStyle(wxSTC_C_CHARACTER,   green);
  SetStyle(wxSTC_C_OPERATOR,    purple);
  SetStyle(wxSTC_C_IDENTIFIER,  black);
  SetStyle(wxSTC_C_WORD2,       red);
#endif

  SetLineNumber(0);
}


////////////////////////////////////////////////////////////////////////////////

void
wScriptWindow::SetStyle(
  int style,
  const wxColour &fore,
  const wxColour &back,
  int size,
  const char* face)
{
#ifdef WIN32
  m_Editor->StyleSetForeground(style, fore);
  m_Editor->StyleSetBackground(style, back);
  if (size >= 1) {
    m_Editor->StyleSetSize(style, size);
  }
  if (face) {
    m_Editor->StyleSetFaceName(style, face);
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////

bool
wScriptWindow::LoadScript(const char* filename)
{
#ifdef WIN32
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
  m_Editor->SetText(buffer);
  m_Editor->SetSelection(0, 0);

  // delete the buffer and close the file
  delete[] buffer;
  fclose(file);

  SetModified(false);

  m_Editor->SetSavePoint();
#else
  m_Editor->LoadFile(filename);
#endif

  SetModified(false);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
wScriptWindow::GetEditorText(wxString& text)
{
#ifdef WIN32
  text = m_Editor->GetText();
#else
  text = "";
#endif
}

////////////////////////////////////////////////////////////////////////////////

wxString
wScriptWindow::GetSelection()
{
#ifdef WIN32
  return m_Editor->GetSelectedText();
#else
  // return m_Editor->GetSelection();
  return "";
#endif
}

////////////////////////////////////////////////////////////////////////////////

void
wScriptWindow::OnSize(wxSizeEvent &event)
{
  if (m_Created) {
#ifdef WIN32
    m_Editor->SetSize(wxRect(wxPoint(0, 0), event.GetSize()));
#endif
  }

  wSaveableDocumentWindow::OnSize(event);
}

////////////////////////////////////////////////////////////////////////////////

void
wScriptWindow::OnSetFocus(wxFocusEvent &event)
{
  //wSaveableDocumentWindow::OnSetFocus(event);
  if (m_Created) {
#ifdef WIN32
    m_Editor->SetFocus();
#endif
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wScriptWindow::OnScriptCheckSyntax(wxCommandEvent &event)
{
#ifdef WIN32
  wxString text;
  GetEditorText(text);

  // verify the script
  sCompileError error;
  if (!VerifyScript(text.c_str(), error))
  {
    // show the error message
    ::wxMessageBox(error.m_Message.c_str(), "Check syntax");
    if (error.m_Token.length() > 0)
      m_Editor->SetSelection(
        error.m_TokenStart,
        error.m_TokenStart + error.m_Token.length());
  }
  else
    ::wxMessageBox("Script is valid");
#endif
}

////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void
wScriptWindow::OnScriptFind(wxCommandEvent &event)
{
  if (!m_SearchDialog) {
    if(!m_SearchData) {
      m_SearchData = new wxFindReplaceData(wxFR_DOWN);
      m_SearchData->SetFindString(m_Editor->GetSelectedText());
    }
    m_SearchDialog = new wxFindReplaceDialog(this, m_SearchData, "Find replace", wxFR_REPLACEDIALOG | wxFR_NOUPDOWN);
  }
}
#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void
wScriptWindow::OnScriptReplace(wxCommandEvent &event)
{
  if (!m_SearchDialog) {
    if(!m_SearchData) {
      m_SearchData = new wxFindReplaceData(wxFR_DOWN);
      m_SearchData->SetFindString(m_Editor->GetSelectedText());
    }
    m_SearchDialog = new wxFindReplaceDialog(this, m_SearchData, "Find replace", wxFR_REPLACEDIALOG | wxFR_NOUPDOWN);
  }
}
#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void
wScriptWindow::OnSavePointReached(wxStyledTextEvent &event)
{
  SetModified(false);
}
#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void
wScriptWindow::OnSavePointLeft(wxStyledTextEvent &event)
{
  SetModified(true);
}
#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void
wScriptWindow::OnPosChanged(wxStyledTextEvent &event) {
  int pos = m_Editor->GetCurrentPos();
  int line = m_Editor->LineFromPosition(pos);
  SetLineNumber(line);
}
#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void
wScriptWindow::OnCharAdded(wxStyledTextEvent &event) {
  if (event.GetKey() == '\n') {
    int pos = m_Editor->GetCurrentPos();
    int line = m_Editor->LineFromPosition(pos);

    if (line > 0) {
      wxString str = m_Editor->GetLine(line - 1);
      const char *text = str.c_str();
      int i;
      for (i = 0; text[i] == ' ' || text[i] == '\t'; i++) {
      }

      m_Editor->ReplaceSelection(str.Left(i));
    }
  }
}
#endif

////////////////////////////////////////////////////////////////////////////////

void
wScriptWindow::SetLineNumber(int line) {
  char str[80];
  sprintf(str, "Line: %d", line);
  SetStatus(wxString(str));
}

////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void
wScriptWindow::OnFindReplace(wxFindDialogEvent &event)
{
  wxString str;
  GetEditorText(str);
  int findmin;
  int findmax;
  int findflags;
  int findresult;
  wxString findtext;
  wxString replacetext;

  if(event.GetEventType() == wxEVT_COMMAND_FIND_CLOSE) {
    m_SearchDialog = NULL;
    return;
  }

  findmin = m_Editor->GetSelectionStart();
  findmax = str.length();
  findtext = m_SearchData->GetFindString();
  findflags = m_SearchData->GetFlags();
  findflags = 
      //(findflags & wxFR_DOWN) ? x : 0 |
      (findflags & wxFR_WHOLEWORD) ? wxSTC_FIND_WHOLEWORD : 0 |
      (findflags & wxFR_MATCHCASE) ? wxSTC_FIND_MATCHCASE : 0
    ;
  replacetext = m_SearchData->GetReplaceString();

  if(findtext.length() == 0) {
    return;
  }

  if(event.GetEventType() == wxEVT_COMMAND_FIND_NEXT) {
    findmin++;

    findresult = m_Editor->FindText(findmin, findmax, findtext, findflags);
    if (findresult == -1) {
      wxMessageBox("No more matches!", "Find", wxOK, m_SearchDialog);
    } else {
      m_Editor->SetSelection(findresult, findresult + findtext.length());
    }
  } else if(event.GetEventType() == wxEVT_COMMAND_FIND_REPLACE) {

    // if the selection is what we want to replace, then do so
    /*todo: make this match case sensitiviy of the dialog */
    if ((findflags & wxFR_MATCHCASE) ? 
          (findtext == m_Editor->GetSelectedText()) :
          (findtext.Lower() == m_Editor->GetSelectedText().Lower())
        ) 
    {
      // actually do the string replacing
      m_Editor->ReplaceSelection(replacetext);

      //adjust min/max
      findmin += findtext.length();
      findmax = m_Editor->GetLength();

    }
    // now try to find the next one
    findresult = m_Editor->FindText(findmin, findmax, findtext, findflags);
    if (findresult == -1) {
      wxMessageBox("No more matches!", "Replace", wxOK, m_SearchDialog);
    } else {
      m_Editor->SetSelection(findresult, findresult + findtext.length());
    }
  } else if(event.GetEventType() == wxEVT_COMMAND_FIND_REPLACE_ALL) {
    findresult = m_Editor->FindText(findmin, findmax, findtext, findflags);
    while(findresult != -1) {
      m_Editor->SetSelection(findresult, findresult + findtext.length());

      // actually do the string replacing
      m_Editor->ReplaceSelection(replacetext);

      //adjust min/max
      findmin += findtext.length();
      findmax = m_Editor->GetLength();

      findresult = m_Editor->FindText(findmin, findmax, findtext, findflags);
    }
  }

  return;
}
#endif

////////////////////////////////////////////////////////////////////////////////

bool
wScriptWindow::GetSavePath(char* path)
{
  wScriptFileDialog Dialog(this, FDM_SAVE);
  if (Dialog.ShowModal() != wxID_OK)
    return false;

  strcpy(path, Dialog.GetPath());
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
wScriptWindow::SaveDocument(const char* path)
{
#ifdef WIN32
  FILE* file = fopen(path, "wb");
  if (file == NULL)
    return false;

  wxString text;
  GetEditorText(text);
  fwrite((const char*)text, 1, text.length(), file);
  fclose(file);

  m_Editor->SetSavePoint();
#else
  m_Editor->SaveFile(path);
#endif
  return true;
}

////////////////////////////////////////////////////////////////////////////////


