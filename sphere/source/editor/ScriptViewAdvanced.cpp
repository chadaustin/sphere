#include "ScriptView.hpp"
#include "Editor.hpp"
#include "Scripting.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
#include "resource.h"


const UINT s_FindReplaceMessage = ::RegisterWindowMessage(FINDMSGSTRING);


BEGIN_MESSAGE_MAP(CScriptView, CRichEditCtrl)

  ON_WM_PAINT()
  ON_WM_RBUTTONUP()

  ON_COMMAND(ID_SCRIPTVIEW_UNDO,      OnUndo)
  ON_COMMAND(ID_SCRIPTVIEW_CUT,       OnCut)
  ON_COMMAND(ID_SCRIPTVIEW_COPY,      OnCopy)
  ON_COMMAND(ID_SCRIPTVIEW_PASTE,     OnPaste)
  ON_COMMAND(ID_SCRIPTVIEW_DELETE,    OnDelete)
  ON_COMMAND(ID_SCRIPTVIEW_SELECTALL, OnSelectAll)

  ON_REGISTERED_MESSAGE(s_FindReplaceMessage, OnFindReplace)

END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////

CScriptView::CScriptView()
: m_Handler(NULL)
, m_SearchDialog(NULL)
{
/*
    m_InForcedChange = false;
    m_ChangeType = ctUndo;
    m_OldSel.cpMin = m_OldSel.cpMax = 0;
    m_LineCount = 0;
*/
}

///////////////////////////////////////////////////////////////////////////////

CScriptView::~CScriptView()
{
//  Configuration::Set(KEY_SCRIPT_SHOWCOLOR, m_ShowColor);
//  Configuration::Set(KEY_SCRIPT_TABSIZE, m_TabSize);
//  Configuration::Set(KEY_SCRIPT_AUTOINDENT, m_AutoIndent);
//  DestroyWindow();
}

///////////////////////////////////////////////////////////////////////////////

BOOL
CScriptView::Create(IScriptViewHandler* handler, CWnd* parent)
{
  m_Handler = handler;

  BOOL result = CRichEditCtrl::Create(
    WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_NOHIDESEL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_HSCROLL | WS_VSCROLL,
    CRect(0, 0, 0, 0),
    parent,
    0
  );

  if (!result) {
    return FALSE;
  }

/*
  CFont font;
  font.CreatePointFont(
    Configuration::Get(KEY_SCRIPT_FONT_SIZE),
    Configuration::Get(KEY_SCRIPT_FONT_NAME).c_str());
  SetFont(&font);
*/

//  m_ShowColor = Configuration::Get(KEY_SCRIPT_SHOWCOLOR);
//  m_TabSize = Configuration::Get(KEY_SCRIPT_TABSIZE);
//  m_AutoIndent = Configuration::Get(KEY_SCRIPT_AUTOINDENT);

  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

void
CScriptView::ScriptFindWord()
{
  if (m_SearchDialog == NULL)
  {
    m_SearchDialog = new CFindReplaceDialog;
    m_SearchDialog->Create(true, GetSelText(), NULL, FR_DOWN, this);
  }
}

///////////////////////////////////////////////////////////////////////////////

void
CScriptView::ScriptFindReplaceWord()
{
  if (m_SearchDialog == NULL)
  {
    m_SearchDialog = new CFindReplaceDialog;
    m_SearchDialog->Create(false, NULL, NULL, FR_DOWN, this);
  }
}

///////////////////////////////////////////////////////////////////////////////

void
CScriptView::ScriptShowColors()
{
//  m_ShowColor = (m_ShowColor == true ? false : true);
//  MessageBox("Showing/Hiding Colors will not be start\nuntil the window has restarted");
}

///////////////////////////////////////////////////////////////////////////////

void 
CScriptView::ScriptAutoIndent()
{
//  m_AutoIndent = (m_AutoIndent ? false : true);
//  if (m_AutoIndent)
//    MessageBox("Auto Indent enabled.");
//  else
//    MessageBox("Auto Indent disabled.");
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptView::OnPaint()
{
  CPaintDC dc(this);

  // client rectangle
  RECT client_rect;
  GetClientRect(&client_rect);

  // create background brush
  CBrush white_brush;
  white_brush.CreateSolidBrush(0xFFFFFF); // white

  // set DC properties
  dc.SaveDC();
  dc.SetBkMode(OPAQUE);
  dc.SetBkColor(0xFFFFFF);    // white (change)
  dc.SetTextColor(0x000000);  // black (change)


  // get window text
  CString text_;
  GetWindowText(text_);
  const char* text = text_;

  
  while (*text) {

    // get the first line
    int start_index = text - text_;
    int start_y = GetCharPos(start_index).y;
    while (start_y == GetCharPos(text - text_).y) {
      text++;
    }



    // new line
    long index = text - text_;
    if (GetCharPos(index).y != current_y) {
      
      // ignore \n and \r on a line end
      const char* end = line_begin;
      while (*end && *end != '\n' && *end != '\r') {
        end++;
      }

      // pre-calculate the line's rectangle
      RECT rect;
      rect.left   = client_rect.left + base_x;
      rect.top    = current_y;
      rect.right  = client_rect.right;
      rect.bottom = GetCharPos(index).y;

      // do actual rectangle calculations
      dc.DrawText(line_begin, end - line_begin, &rect, DT_LEFT | DT_TOP | DT_NOPREFIX | DT_SINGLELINE | DT_CALCRECT);

      // do clipping to the client area window
      if (rect.right > client_rect.right) {
        rect.right = client_rect.right;
      }
      if (rect.bottom > client_rect.bottom) {
        rect.bottom = client_rect.bottom;
      }

      if (dc.RectVisible(&rect)) {

        // draw the line
        dc.DrawText(line_begin, end - line_begin, &rect, DT_LEFT | DT_TOP | DT_NOPREFIX | DT_SINGLELINE);

      }

      // fill the rest of the line
      rect.left = rect.right;
      rect.right = client_rect.right;
      if (dc.RectVisible(&rect)) {
        dc.FillRect(&rect, &white_brush);
      }

      line_begin = text;
      current_y = GetCharPos(index).y;
    }

    text++;
  }

  dc.RestoreDC(-1);
  white_brush.DeleteObject();
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptView::OnRButtonUp(UINT flags, CPoint point)
{
  HMENU _menu = LoadMenu(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_SCRIPTVIEW));
  HMENU menu = GetSubMenu(_menu, 0);

  bool has_selection = (GetSelectionType() != SEL_EMPTY);
  EnableMenuItem(menu, ID_SCRIPTVIEW_UNDO,   MF_BYCOMMAND | (CanUndo()     ? MF_ENABLED : MF_GRAYED));
  EnableMenuItem(menu, ID_SCRIPTVIEW_CUT,    MF_BYCOMMAND | (has_selection ? MF_ENABLED : MF_GRAYED));
  EnableMenuItem(menu, ID_SCRIPTVIEW_COPY,   MF_BYCOMMAND | (has_selection ? MF_ENABLED : MF_GRAYED));
  EnableMenuItem(menu, ID_SCRIPTVIEW_PASTE,  MF_BYCOMMAND | (CanPaste()    ? MF_ENABLED : MF_GRAYED));
  EnableMenuItem(menu, ID_SCRIPTVIEW_DELETE, MF_BYCOMMAND | (has_selection ? MF_ENABLED : MF_GRAYED));

  ClientToScreen(&point);
  TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, m_hWnd, NULL);

  DestroyMenu(_menu);
}

///////////////////////////////////////////////////////////////////////////////

LRESULT
CScriptView::OnFindReplace(WPARAM, LPARAM)
{
  if (m_SearchDialog->IsTerminating()) {
    m_SearchDialog = NULL;
  } else if (m_SearchDialog->FindNext()) {
    OnFindText();
  } else if (m_SearchDialog->ReplaceCurrent()) {
    OnReplaceText();
  } else if (m_SearchDialog->ReplaceAll()) {
    OnReplaceAll();
  }

  return 0;
}


///////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptView::OnUndo()
{
  Undo();
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptView::OnCut()
{
  Cut();
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptView::OnCopy()
{
  Copy();
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptView::OnPaste()
{
  Paste();
  m_Handler->SV_ScriptChanged();
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptView::OnDelete()
{
  Clear();
  m_Handler->SV_ScriptChanged();
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptView::OnSelectAll()
{
  SetSel(0, GetWindowTextLength() - 1);
}

///////////////////////////////////////////////////////////////////////////////

/*
void
CScriptView::AutoIndent()
{
  if (!m_AutoIndent)
    return;

  CString LineBuffer;
  LPTSTR  TempBuffer;
  int Length;
  int PrevLine;
  int SpaceSize;

  PrevLine = LineFromChar(-1) - 1;
  if (PrevLine < 0) return;
  
  Length = LineLength(LineIndex(PrevLine));
  TempBuffer = new char[Length + 4];
  GetLine(PrevLine, TempBuffer, Length + 4);
  TempBuffer[Length] = 0;

  LineBuffer = TempBuffer;
  LineBuffer.TrimLeft();
  SpaceSize = Length - LineBuffer.GetLength();

  for (int i=0; i<SpaceSize; i++)
    ::SendMessage(this->m_hWnd, WM_CHAR, VK_SPACE, 0);

  delete[] TempBuffer;
}
*/

///////////////////////////////////////////////////////////////////////////////

void
CScriptView::UpdateLineInfo()
{
  char buf[80];
  sprintf(buf, "Line %i / %i", LineFromChar(-1) + 1, GetLineCount());
  GetStatusBar()->SetWindowText(buf);
}

///////////////////////////////////////////////////////////////////////////////

/*
void 
CScriptView::OnChange() 
{
  CHARRANGE CurSel; 
  GetSel(CurSel);

  if (m_ChangeType == ctMove && CurSel.cpMin == CurSel.cpMax) {
    // cut was canceled, so this is paste operation
    m_ChangeType = ctPaste;
  }

  switch(m_ChangeType) {
  case ctReplSel:// old=(x,y) -> cur=(x+len,x+len)
  case ctPaste:  // old=(x,y) -> cur=(x+len,x+len)
    //FormatTextLines(CurSel.cpMin, CurSel.cpMax);
    FormatTextLines(m_OldSel.cpMin, CurSel.cpMax);
    break;
  case ctDelete: // old=(x,y) -> cur=(x,x)
  case ctBack:   // old=(x,y) -> cur=(x,x), newline del => old=(x,x+1) -> cur=(x-1,x-1)
  //case ctCut:    // old=(x,y) -> cur=(x,x)
    FormatTextLines(CurSel.cpMin, CurSel.cpMax);
    break;
  case ctUndo:   // old=(?,?) -> cur=(x,y)
    FormatTextLines(CurSel.cpMin, CurSel.cpMax);
    break;
  case ctMove:   // old=(x,x+len) -> cur=(y-len,y) | cur=(y,y+len)
    FormatTextLines(CurSel.cpMin, CurSel.cpMax);
    if (CurSel.cpMin > m_OldSel.cpMin) // move after
      FormatTextLines(m_OldSel.cpMin, m_OldSel.cpMin);
    else // move before
      FormatTextLines(m_OldSel.cpMax, m_OldSel.cpMax);
    break;
  default:
    FormatAll();
    break;
  }

  //undo action does not call OnProtected, so make it default
  m_ChangeType = ctUndo;
}

///////////////////////////////////////////////////////////////////////////////

void 
CScriptView::SetFormatRange(int nStart, int nEnd, BOOL bBold, COLORREF clr)
{
  if (nStart >= nEnd)
    return;

  SetSel(nStart, nEnd);

  DWORD dwEffects = bBold?CFE_BOLD:0;

  CHARFORMAT cfm;
  cfm.cbSize = sizeof(cfm);
    GetSelectionCharFormat(cfm);
  
  if ((cfm.dwMask & CFM_COLOR)  && cfm.crTextColor == clr && 
    (cfm.dwMask & CFM_BOLD) && (cfm.dwEffects & CFE_BOLD) == dwEffects)
    return;
  
  cfm.dwEffects = dwEffects;
  cfm.crTextColor = clr;
  cfm.dwMask = CFM_BOLD | CFM_COLOR;

  SetSelectionCharFormat(cfm);
}

///////////////////////////////////////////////////////////////////////////////

void 
CScriptView::FormatTextLines(int nLineStart, int nLineEnd)
{
  long nStart = LineIndex(LineFromChar(nLineStart));
  long nEnd = LineIndex(LineFromChar(nLineEnd));
  nEnd += LineLength(nLineEnd);

  FormatTextRange(nStart, nEnd);
}

///////////////////////////////////////////////////////////////////////////////

void 
CScriptView::FormatTextRange(int nStart, int nEnd)
{
    if (!m_ShowColor)
      return;

    // FIXME: stop refresh flickering (with Lock&UnlockWindowUpdate) OR
    //        stop cursor flickering (without Lock&UnlockWindowUpdate)
    if (nStart >= nEnd)
      return;

    //LockWindowUpdate();

    CHARRANGE crOldSel;

    GetSel(crOldSel);
    HideSelection(TRUE, FALSE);

    SetSel(nStart, nEnd);
    TCHAR *pBuffer = new TCHAR[nEnd - nStart + 1];
    long nLen = GetSelText(pBuffer);

    pBuffer[nLen] = 0;

    TCHAR *pStart, *pPtr;
    SymbolColor ic;
    pStart = pPtr = pBuffer;

    TCHAR* pSymbolStart = NULL;

    while (*pPtr != 0) {
      TCHAR ch = *pPtr;

  
      if (ch == '/' && pPtr[1] == '/') { // Process // comment
        pSymbolStart = pPtr;
        do { ch = *(++pPtr); } while (ch != 0 && ch != '\r' && ch != '\n');
        ic = m_icComment;

      } else if (ch == '/' && pPtr[1] == '*') { // Process comment
        pSymbolStart = pPtr++;
        do { ch = *(++pPtr);
             if (ch == '*' && pPtr[1] != 0)
               if (pPtr[1] == '/') 
               { pPtr+=2; break; }
           } while(ch != 0);
        ic = m_icComment;

      } else if (ch == '"') { // Process strings
        pSymbolStart = pPtr;
        do { ch = *(++pPtr); } while (ch != 0 && ch != '"');
        if (ch == '"') pPtr++;
        ic = m_icString;

      } else if (_istdigit(ch)) { // Process numbers
        pSymbolStart = pPtr;
        _tcstod(pSymbolStart, &pPtr);
        ic = m_icNumber;

      } else if (_istalpha(ch) || ch == '_') { // Process keywords
        pSymbolStart = pPtr;
        do { ch = *(++pPtr); } while (_istalnum(ch) || ch == '_');
        *pPtr = 0;
        if (IsKeyword(pSymbolStart)) 
          ic = m_icKeyword;
        else 
          pSymbolStart = NULL;
        *pPtr = ch;

      } else {
        pPtr++;
      }

      if (pSymbolStart != NULL) {
        SetFormatRange(nStart + pStart - pBuffer, nStart + pSymbolStart - pBuffer, FALSE, RGB(0,0,0));
        SetFormatRange(nStart + pSymbolStart - pBuffer, nStart + pPtr - pBuffer, ic.bold, ic.color);
        pStart = pPtr;
        pSymbolStart = 0;
      } else if (*pPtr == 0)
        SetFormatRange(nStart + pStart - pBuffer, nStart + pPtr - pBuffer, FALSE, RGB(0,0,0));
    }

  delete [] pBuffer;

  SetSel(crOldSel);
  HideSelection(FALSE, FALSE);
  //UnlockWindowUpdate();
}
*/

///////////////////////////////////////////////////////////////////////////////

void
CScriptView::OnFindText()
{
  JustFindText(false);
}

///////////////////////////////////////////////////////////////////////////////

void
CScriptView::OnReplaceText()
{
  // replace it if it is the text
  JustReplaceText();

  // go find me another one!
  JustFindText(false);
}

///////////////////////////////////////////////////////////////////////////////

void
CScriptView::OnReplaceAll()
{
  bool hasText = true;
  SetSel(0, 0);

  while (hasText)
  {
    JustReplaceText();
    hasText = JustFindText(true);
  }
}

///////////////////////////////////////////////////////////////////////////////

bool
CScriptView::JustFindText(bool ReplaceAll)
{
  CString Word;
  CString String;
  CHARRANGE crOldSel;
  CHARRANGE crSel;
  long Location;
  long Offset;
  
  GetSel(crOldSel);

  // do some logic for offsets
  if (crOldSel.cpMax - crOldSel.cpMin < 1) Offset = 0;
  else Offset = 1;

  // set up the text that will be used.
  Word = m_SearchDialog->GetFindString();
  GetWindowText(String);

  // retrieve the text and find the text
  if (m_SearchDialog->SearchDown()) // down
  {
    // old method. uses the selection to acquire text
    //crSel.cpMin = crOldSel.cpMax;
    //crSel.cpMax = -1;
    //SetSel(crSel);
    //String = GetSelText();

    // trim the fat...
    String.Delete(0, crOldSel.cpMin + Offset);
  }
  else // up
  {
    // trim the fat...
    String.Delete(crOldSel.cpMax - Offset, String.GetLength() - crOldSel.cpMax + Offset);

    // reverse the strings to let it start searching
    String.MakeReverse();
    Word.MakeReverse();
  }

  // make the string and word uppercase if the word is case insensitive
  if (!m_SearchDialog->MatchCase())
  { String.MakeUpper(); Word.MakeUpper(); }

  // now search!
  Location = String.Find(Word);

  // if the word is whole word, then...
  if (m_SearchDialog->MatchWholeWord() && Location != -1)
  {
    bool done = false;

    while (!done)
    {
      if (Location == 0)
        done = true; 

      if (!done)
        if (__iscsym(String.GetAt(Location - 1)) == 0)
          if (!done)
            done = true;

      if (done)
        if (Location + Word.GetLength() < String.GetLength())
          if (__iscsym(String.GetAt(Location + Word.GetLength())) != 0)
            if (done)
              done = false;

      if (!done)
      {
        long tempLocation;
        CString tempString;

        tempString = String.Mid(Location + Word.GetLength());
        tempLocation = tempString.Find(Word);

        if (tempLocation != -1)
          Location += tempLocation + Word.GetLength();
        else
          done = true;
      }
    }

    // final pass...
    if (Location == -1) done = true;
    if (Location == 0) done = true; 
      if (!done)
        if (__iscsym(String.GetAt(Location - 1)) == 0)
          if (!done)
            done = true;
    if (done)
      if (Location + Word.GetLength() < String.GetLength())
        if (__iscsym(String.GetAt(Location + Word.GetLength())) != 0)
          if (done)
            done = false;
    if (!done) Location = -1;
  }

  
  if (Location != -1)
  {
    if (m_SearchDialog->SearchDown())
    {
      crSel.cpMin = Location + crOldSel.cpMin + Offset;
      crSel.cpMax = crOldSel.cpMin + Offset + Location + strlen(Word);
    }
    else
    {
      Location = String.GetLength() - Location;
      crSel.cpMin = Location - strlen(Word);
      crSel.cpMax = Location;
    }

    SetSel(crSel);

    return true;
  }
  else
  {
    SetSel(crOldSel);

    if (!ReplaceAll)
      MessageBox("String not found!", "", MB_OK | MB_ICONINFORMATION);
    else
      MessageBox("Editor has finish searching the document", "", MB_OK | MB_ICONINFORMATION);

    return false;
  }
/*
  CString Word;
  CHARRANGE crSearchRegion;
  CHARRANGE crOldSel;
  long Loc;
  unsigned long abc;
  
  GetSel(crSearchRegion);
  GetSel(crOldSel);
  crSearchRegion.cpMax = -1;

  SetSel(crSearchRegion);
  
  //FINDTEXTEX* t = new FINDTEXTEX;
  //Location = FindText(t, 0);
  Word = GetSelText();

  Location = Word.Find("int");
  if (Location > 0)
  {
    crOldSel.cpMin += Location;
    crOldSel.cpMax = crOldSel.cpMin + 3;
  }

  SetSel(crOldSel);
*/

}

///////////////////////////////////////////////////////////////////////////////

void
CScriptView::JustReplaceText()
{
    CString Word;
  bool okay = false;
  
  // check if the stuff highlighted isn't some kind of dud
  Word = GetSelText();
  if (m_SearchDialog->MatchCase())
  {
    if (Word.Compare(m_SearchDialog->GetFindString()) == 0)
      okay = true;
  }
  else
  {
    if (Word.CompareNoCase(m_SearchDialog->GetFindString()) == 0)
      okay = true;
  }

  if (okay)
    if (m_SearchDialog->MatchWholeWord())
    {
      CHARRANGE crSel;

      GetSel(crSel);
      GetWindowText(Word);

      // reset this to search for stuff
      okay = false; 

      if (crSel.cpMin == 0)
        okay = true;

      if (!okay)
        if (__iscsym(Word.GetAt(crSel.cpMin - 1)) == 0)
          okay = true;

      if (okay)
        if (crSel.cpMax + 1 < Word.GetLength())
          if (__iscsym(Word.GetAt(crSel.cpMax)) != 0)
            okay = false;
    }

  // now we passed the test, time to replace it!
  if (okay)
    ReplaceSel(m_SearchDialog->GetReplaceString(), true);
}
