#pragma warning(disable : 4786)

#include <Scintilla.h>
#include <SciLexer.h>
#include "Editor.hpp"
#include "ScriptWindow.hpp"
#include "FileDialogs.hpp"
#include "NumberDialog.hpp"
//#include "Project.hpp"
#include "Scripting.hpp"
#include "FileSystem.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
#include "resource.h"

#include <afxdlgs.h>

const int ID_EDIT = 900;
const UINT s_FindReplaceMessage = ::RegisterWindowMessage(FINDMSGSTRING);


BEGIN_MESSAGE_MAP(CScriptWindow, CSaveableDocumentWindow)

  ON_WM_SIZE()
  ON_WM_SETFOCUS()

  ON_COMMAND(ID_SCRIPT_CHECKSYNTAX,      OnScriptCheckSyntax)
  ON_COMMAND(ID_SCRIPT_FIND,             OnScriptFind)
  ON_COMMAND(ID_SCRIPT_REPLACE,          OnScriptReplace)

  ON_COMMAND(ID_SCRIPT_OPTIONS_SET_FONT,      OnOptionsSetScriptFont)
  ON_COMMAND(ID_SCRIPT_OPTIONS_TOGGLE_COLORS, OnOptionsToggleColors)
  ON_COMMAND(ID_SCRIPT_OPTIONS_SET_TAB_SIZE,  OnOptionsSetTabSize)
  ON_COMMAND(ID_SCRIPT_OPTIONS_TOGGLE_LINE_NUMBERS,  OnOptionsToggleLineNumbers)
  ON_COMMAND(ID_SCRIPT_OPTIONS_SHOW_WHITESPACE, OnOptionsShowWhitespace)
  ON_COMMAND(ID_SCRIPT_OPTIONS_WORD_WRAP, OnOptionsWordWrap)

  ON_UPDATE_COMMAND_UI(ID_SCRIPT_CHECKSYNTAX,      OnUpdateScriptCheckSyntax)

  ON_UPDATE_COMMAND_UI(ID_SCRIPT_OPTIONS_TOGGLE_LINE_NUMBERS, OnUpdateOptionsToggleLineNumbers)
  ON_UPDATE_COMMAND_UI(ID_SCRIPT_OPTIONS_TOGGLE_COLORS, OnUpdateOptionsToggleColors)
  ON_UPDATE_COMMAND_UI(ID_SCRIPT_OPTIONS_SHOW_WHITESPACE, OnUpdateOptionsShowWhitespace)
  ON_UPDATE_COMMAND_UI(ID_SCRIPT_OPTIONS_WORD_WRAP, OnUpdateOptionsWordWrap)

  ON_NOTIFY(SCN_SAVEPOINTREACHED, ID_EDIT, OnSavePointReached)
  ON_NOTIFY(SCN_SAVEPOINTLEFT,    ID_EDIT, OnSavePointLeft)
  ON_NOTIFY(SCN_UPDATEUI,         ID_EDIT, OnPosChanged)
  ON_NOTIFY(SCN_CHARADDED,        ID_EDIT, OnCharAdded)

  ON_COMMAND(ID_FILE_ZOOM_IN, OnZoomIn)
  ON_COMMAND(ID_FILE_ZOOM_OUT, OnZoomOut)
  ON_COMMAND(ID_FILE_PASTE, OnPaste)

  ON_REGISTERED_MESSAGE(s_FindReplaceMessage, OnFindReplace)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CScriptWindow::CScriptWindow(const char* filename, bool create_from_clipboard)
: CSaveableDocumentWindow(filename, IDR_SCRIPT)
, m_Created(false)
, m_SearchDialog(0)
, m_SyntaxHighlighted(true)
, m_ShowWhitespace(false)
, m_WordWrap(false)
{
  SetSaved(filename != NULL);
  SetModified(false);

  if (!Create()) {
    return;
  }

  if (!filename && create_from_clipboard) {
    OnPaste();
  }
  else
  if (filename) {
    LoadScript(filename);
  }

  //int HScrollWidth = 640; // scintilla's default scroll width of 2000 is too big
  /*
  int num_lines = SendEditor(SCI_GETLINECOUNT);
  for (int i = 0; i < num_lines; i++) {
    int linewidth =  SendEditor(SCI_LINELENGTH, i) * 8; // 8 = font size
    if (linewidth > HScrollWidth)
      HScrollWidth = linewidth;
  }
  */
  //SendEditor(SCI_SETSCROLLWIDTH, HScrollWidth);
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
  OnSize(0, Rect.right /*- Rect.left*/, Rect.bottom /*- Rect.top*/);

  // give the view focus
  ::SetFocus(m_Editor);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

CScriptWindow::ScriptType
CScriptWindow::GetScriptType()
{
  struct Local {
    static inline bool extension_compare(const char* path, const char* extension) {
      int path_length = strlen(path);
      int ext_length  = strlen(extension);
      return (
        path_length >= ext_length &&
        strcmp(path + path_length - ext_length, extension) == 0
      );
    }
  };

  if (Local::extension_compare(GetDocumentPath(), ".js")) {
    return SCRIPT_TYPE_JS;
  }

  if (Local::extension_compare(GetDocumentPath(), ".txt")) {
    return SCRIPT_TYPE_TXT;
  }

  return SCRIPT_TYPE_UNKNOWN;
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptWindow::SetScriptStyles()
{
  static const char key_words[] = 
    "break case catch continue default delete do else "
    "finally for function if in instanceof new return "
    "switch this throw try typeof var void while with "
    "true false undefined ";

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

  if (GetScriptType() == SCRIPT_TYPE_UNKNOWN
   || GetScriptType() == SCRIPT_TYPE_JS)
  {
    SendEditor(SCI_SETLEXER, SCLEX_CPP);  // JavaScript uses the C++ lexer
    SendEditor(SCI_SETSTYLEBITS, 5);
    SendEditor(SCI_SETKEYWORDS, 0, (LPARAM)key_words);
    SendEditor(SCI_SETKEYWORDS, 1, (LPARAM)reserved_words);
  }
  else {
    SendEditor(SCI_SETLEXER, SCLEX_CONTAINER);
  }

  if (!(m_FontSize >= 0 && m_FontSize <= 72))
    m_FontSize = 10;

  SetStyle(STYLE_DEFAULT, black, white, m_FontSize, m_Fontface.c_str());
  SendEditor(SCI_STYLECLEARALL);

  // set all margins to zero width
  SendEditor(SCI_SETMARGINWIDTHN, 0, 0);
  SendEditor(SCI_SETMARGINWIDTHN, 1, 0);
  SendEditor(SCI_SETMARGINWIDTHN, 2, 0);

  // set all margin types
  SendEditor(SCI_SETMARGINTYPEN,  0, SC_MARGIN_NUMBER);
  SendEditor(SCI_SETMARGINWIDTHN, 1, SC_MARGIN_SYMBOL);
  SendEditor(SCI_SETMARGINWIDTHN, 2, SC_MARGIN_SYMBOL);

  // for code folding
  //SendEditor(SCI_SETPROPERTY, (WPARAM)"fold", (LPARAM)"1");
  //SendEditor(SCI_SETMARGINTYPEN, 1, SC_MARGIN_SYMBOL);
  //SendEditor(SCI_SETMARGINMASKN, 1, SC_MASK_FOLDERS);
  //SendEditor(SCI_SETMARGINWIDTHN, 1, 20);

  if (m_ShowLineNumbers) {  // resize the line number margin so we can see it
    int margin_size = SendEditor(SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM) "_99999");
    SendEditor(SCI_SETMARGINWIDTHN, 0, margin_size);
  }

  SendEditor(SCI_SETWRAPMODE, ((m_WordWrap) ? (SC_WRAP_WORD) : (SC_WRAP_NONE)));

  SetStyle(SCE_C_DEFAULT, black, white, m_FontSize, m_Fontface.c_str());

  if (m_TabWidth > 0) {
    SendEditor(SCI_SETTABWIDTH, m_TabWidth);
  }

  if (GetScriptType() == SCRIPT_TYPE_UNKNOWN
   || GetScriptType() == SCRIPT_TYPE_JS) {
    if (m_SyntaxHighlighted) {
      SetStyle(SCE_C_COMMENT,     green);
      SetStyle(SCE_C_COMMENTLINE, green);
      SetStyle(SCE_C_COMMENTDOC,  green);
      SetStyle(SCE_C_COMMENTLINEDOC, green);
      
      SetStyle(SCE_C_NUMBER,      darkred);
      SetStyle(SCE_C_WORD,        blue);

      if (m_KeyWordStyleIsBold) {
        SendEditor(SCI_STYLESETBOLD, SCE_C_WORD, 1);
      }

      SetStyle(SCE_C_STRING,      green);
      SetStyle(SCE_C_CHARACTER,   green);
      SetStyle(SCE_C_OPERATOR,    purple);
      SetStyle(SCE_C_IDENTIFIER,  black);
      SetStyle(SCE_C_WORD2,       red);

      SetStyle(STYLE_BRACELIGHT,  green);
      SetStyle(STYLE_BRACEBAD,    red);
    }
  }

  SendEditor(SCI_SETVIEWWS, ((m_ShowWhitespace) ? (SCWS_VISIBLEALWAYS) : (SCWS_INVISIBLE)));
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptWindow::Initialize()
{
  m_Fontface = Configuration::Get(KEY_SCRIPT_FONT_NAME);
  m_FontSize = Configuration::Get(KEY_SCRIPT_FONT_SIZE);
  m_SyntaxHighlighted = Configuration::Get(KEY_SCRIPT_SYNTAX_HIGHLIGHTED);
  m_TabWidth = Configuration::Get(KEY_SCRIPT_TAB_SIZE);
  m_KeyWordStyleIsBold = Configuration::Get(KEY_SCRIPT_KEYWORDS_IN_BOLD);
  m_ShowLineNumbers    = Configuration::Get(KEY_SCRIPT_SHOW_LINE_NUMBERS);
  m_ShowWhitespace     = Configuration::Get(KEY_SCRIPT_SHOW_WHITESPACE);
  m_WordWrap           = Configuration::Get(KEY_SCRIPT_WORD_WRAP);
  SetScriptStyles();

  SetLineNumber(0);
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
  if (buffer == NULL) return false;
  fread(buffer, sizeof(char), file_size, file);
  
  // null-terminate the string
  buffer[file_size] = 0;

  // put the buffer into the edit control
  SendEditor(SCI_SETSEL,  0, 0);
  SendEditor(SCI_SETTEXT, 0, (LPARAM)"");
  SendEditor(SCI_ADDTEXT, file_size, (LPARAM)buffer);

  // delete the buffer and close the file
  delete[] buffer;
  fclose(file);

  SetModified(false);
  SendEditor(SCI_SETSAVEPOINT);

  SendEditor(SCI_EMPTYUNDOBUFFER);

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
  CSaveableDocumentWindow::OnSetFocus(old);
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
  SetScriptStyles();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnSavePointLeft(NMHDR* nmhdr, LRESULT* result)
{
  SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnPaste()
{
  SendEditor(SCI_PASTE);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnZoomIn()
{
  SendEditor(SCI_ZOOMIN);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnZoomOut()
{
  SendEditor(SCI_ZOOMOUT);
}

////////////////////////////////////////////////////////////////////////////////

static
bool is_brace(char ch) {
  return (ch == '{' || ch == '[' || ch == '('
       || ch == '}' || ch == ']' || ch == ')');
}

void
CScriptWindow::UpdateBraceHighlight()
{
  int pos = SendEditor(SCI_GETCURRENTPOS);
  if (is_brace(SendEditor(SCI_GETCHARAT, pos)))
  {
    int m = SendEditor(SCI_BRACEMATCH, pos, 0);
    if (m != -1) {
      SendEditor(SCI_BRACEHIGHLIGHT, pos, m);
    } 
    else {
      SendEditor(SCI_BRACEBADLIGHT, pos);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnPosChanged(NMHDR* nmhdr, LRESULT* result) {
  SCNotification* notify = (SCNotification*)nmhdr;
  int pos = SendEditor(SCI_GETCURRENTPOS);
  int line = SendEditor(SCI_LINEFROMPOSITION, pos);
  SetLineNumber(line);
  UpdateBraceHighlight();
}

////////////////////////////////////////////////////////////////////////////////

/*
static const char* sFunctionDefinitions[] = {
  "IsKeyPressed(key)",
  "GetTile(x, y, layer)",
  "MapEngine(map, fps)",
  "CreatePerson(name, spriteset, destroy_with_map)"
};
*/

afx_msg void
CScriptWindow::OnCharAdded(NMHDR* nmhdr, LRESULT* result) {

  SCNotification* notify = (SCNotification*)nmhdr;
/*
  static char string[100] = {0};
  if (strlen(string) < 100)
    string[strlen(string)] = notify->ch;

  int match = -1;

  for (int i = 0; i < 4; i++) {
    if (strstr(sFunctionDefinitions[i], string) != NULL)
      match = i;
  }

  if (match != -1)
    SendEditor(SCI_CALLTIPSHOW, 0, (LPARAM)sFunctionDefinitions[match]);
  else {
    SendEditor(SCI_CALLTIPCANCEL);
    memset(string, 0, 100);
  }
*/

  if (notify->ch == '\n') {
    int pos  = SendEditor(SCI_GETCURRENTPOS);
    int line = SendEditor(SCI_LINEFROMPOSITION, pos);

    if (line > 0) {
      char text[1024 * 16];
      int line_length = SendEditor(SCI_LINELENGTH, line - 1);
      if (line_length > 0 && line_length < sizeof(text)) {
        SendEditor(SCI_GETLINE, line - 1, (LRESULT)text);
        std::string str;
        int i = 0;
        while (text[i] == ' ' || text[i] == '\t') {
          str += text[i++];
        }

        /*
        if (1) {
          i = line_length;
          while (text[i] != '{' && i >= 0 && i > line_length - 5) { i--; }
          if (text[i] == '{') {
            str += "  ";
          }

          i = 0;
          while (i++ < 100) {
            int m = SendEditor(SCI_BRACEMATCH, i, 0);
            if (m != -1) {
              pos = pos;
            }
          }

          if (1) {

            while (text[i] != '}' && i >= 0 && i > line_length - 5) { i--; }
            if (text[i] == '}') {
              str.erase(0, 2);
            }
          }
        }
        */

        SendEditor(SCI_REPLACESEL, 0, (LRESULT)str.c_str());
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptWindow::SetLineNumber(int line) {
  char str[80];
  int pos = SendEditor(SCI_GETCURRENTPOS) - SendEditor(SCI_POSITIONFROMLINE, line);
  line += 1;
  sprintf(str, "Line: %d Position: %d", line, pos);
  GetStatusBar()->SetWindowText(str);
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
    BOOL search_down = m_SearchDialog->SearchDown();

    if (!search_down) {
      ttf.chrg.cpMin = ttf.chrg.cpMin - 2; // selection_start - 1
      ttf.chrg.cpMax = 0;
    }

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
      ttf.chrg.cpMin += strlen(replace_string);
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
      ttf.chrg.cpMin += strlen(replace_string);
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

afx_msg void
CScriptWindow::OnOptionsToggleColors()
{
  m_SyntaxHighlighted = !m_SyntaxHighlighted;
  SetScriptStyles();
  RememberConfiguration();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnOptionsSetScriptFont()
{
  // Show the font dialog with all the default settings.
  LOGFONT lf;
  memset((void*)&lf, 0, sizeof(lf));
  if (m_Fontface.size() < 31) {
    memcpy(lf.lfFaceName, m_Fontface.c_str(), m_Fontface.size());
  }

  CFontDialog dlg(&lf);
  if (dlg.DoModal() == IDOK) {

    m_Fontface = std::string(dlg.GetFaceName());
    m_FontSize = dlg.GetSize() / 10; // for a 12pt font dlg.GetSize() is 120
    SetScriptStyles();
    RememberConfiguration();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnOptionsSetTabSize()
{
  CNumberDialog dialog("Enter new tab width value", "Value", m_TabWidth, 2, 8);
  if (dialog.DoModal() == IDOK) {
    m_TabWidth = dialog.GetValue();
    SetScriptStyles();
    RememberConfiguration();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnOptionsToggleLineNumbers()
{
  m_ShowLineNumbers = !m_ShowLineNumbers;
  SetScriptStyles();
  RememberConfiguration();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnOptionsShowWhitespace()
{
  m_ShowWhitespace = !m_ShowWhitespace;
  SetScriptStyles();
  RememberConfiguration();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnOptionsWordWrap()
{
  m_WordWrap = !m_WordWrap;
  SetScriptStyles();
  RememberConfiguration();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnUpdateScriptCheckSyntax(CCmdUI* cmdui)
{
  if (!(GetScriptType() == SCRIPT_TYPE_UNKNOWN
    || GetScriptType() == SCRIPT_TYPE_JS)) {
    cmdui->Enable(FALSE);
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnUpdateOptionsToggleLineNumbers(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_ShowLineNumbers ? TRUE : FALSE);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnUpdateOptionsToggleColors(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_SyntaxHighlighted ? TRUE : FALSE);
  if (!(GetScriptType() == SCRIPT_TYPE_UNKNOWN
    || GetScriptType() == SCRIPT_TYPE_JS)) {
    cmdui->Enable(FALSE);
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnUpdateOptionsShowWhitespace(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_ShowWhitespace ? TRUE : FALSE);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnUpdateOptionsWordWrap(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_WordWrap ? TRUE : FALSE);
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptWindow::RememberConfiguration()
{
  Configuration::Set(KEY_SCRIPT_FONT_NAME, m_Fontface);
  Configuration::Set(KEY_SCRIPT_FONT_SIZE, m_FontSize);
  Configuration::Set(KEY_SCRIPT_TAB_SIZE, m_TabWidth);
  Configuration::Set(KEY_SCRIPT_SYNTAX_HIGHLIGHTED, m_SyntaxHighlighted);
  Configuration::Set(KEY_SCRIPT_SHOW_LINE_NUMBERS, m_ShowLineNumbers);
  Configuration::Set(KEY_SCRIPT_SHOW_WHITESPACE, m_ShowWhitespace);
  Configuration::Set(KEY_SCRIPT_WORD_WRAP, m_WordWrap);
}

////////////////////////////////////////////////////////////////////////////////
