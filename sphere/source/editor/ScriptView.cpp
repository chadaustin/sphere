/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1998 by Juraj Rojko jrojko@twist.cz
// All rights reserved
//
// VBScriptEditorView.h : interface of the CVBScriptEditorView class
//
/////////////////////////////////////////////////////////////////////////////

/* 
   16th April 2000
   Code cleaned up and modified by Darklich for use with SDE 
   (Sphere Development Environment)
*/


#include "ScriptView.hpp"
#include "Editor.hpp"
#include "Scripting.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
#include "resource.h"
#include "Keys.hpp"
#include "Configuration.hpp"

static int s_ScriptViewID = 900;


#define DEFINE_COLOR(name, bold, color) \
  CScriptView::SymbolColor CScriptView::name = { bold, color };

DEFINE_COLOR(m_icComment,  false, RGB(0,   128, 0  ))
DEFINE_COLOR(m_icNumber,   false, RGB(255, 0,   255))
DEFINE_COLOR(m_icString,   false, RGB(128, 128, 128))
DEFINE_COLOR(m_icKeyword,  false, RGB(0,   0,   255))
DEFINE_COLOR(m_icFunction, false, RGB(0,   0,   128))

const UINT MsgFindReplace = ::RegisterWindowMessage(FINDMSGSTRING);

BEGIN_MESSAGE_MAP(CScriptView, CRichEditCtrl)

  ON_WM_SETFOCUS()
  ON_WM_KEYUP()
  ON_WM_KEYDOWN()
  ON_WM_CHAR()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONDBLCLK()
  ON_WM_RBUTTONDOWN()

  ON_COMMAND(ID_SCRIPTVIEW_UNDO,      OnUndo)
  ON_COMMAND(ID_SCRIPTVIEW_CUT,       OnCut)
  ON_COMMAND(ID_SCRIPTVIEW_COPY,      OnCopy)
  ON_COMMAND(ID_SCRIPTVIEW_PASTE,     OnPaste)
  ON_COMMAND(ID_SCRIPTVIEW_DELETE,    OnDelete)
  ON_COMMAND(ID_SCRIPTVIEW_SELECTALL, OnSelectAll)

  ON_REGISTERED_MESSAGE(MsgFindReplace, OnFindReplaceMsg)

END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////

CScriptView::CScriptView()
: m_SearchDialog(NULL)
{
    m_InForcedChange = false;
    m_ChangeType = ctUndo;
    m_OldSel.cpMin = m_OldSel.cpMax = 0;
    m_LineCount = 0;
}

///////////////////////////////////////////////////////////////////////////////

CScriptView::~CScriptView()
{
  Configuration::Set(KEY_SCRIPT_SHOWCOLOR, m_ShowColor);
  Configuration::Set(KEY_SCRIPT_TABSIZE, m_TabSize);
  Configuration::Set(KEY_SCRIPT_AUTOINDENT, m_AutoIndent);
  DestroyWindow();
}

///////////////////////////////////////////////////////////////////////////////

BOOL
CScriptView::Create(IScriptViewHandler* handler, CWnd* parent_window)
{
  m_Handler = handler;

  BOOL Val;
  Val = CRichEditCtrl::Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_NOHIDESEL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_HSCROLL | WS_VSCROLL,
        CRect(0,0,0,0),
        parent_window,
        s_ScriptViewID++);

  if (Val)
  {
    CFont font;
    font.CreatePointFont(
      Configuration::Get(KEY_SCRIPT_FONT_SIZE),
      Configuration::Get(KEY_SCRIPT_FONT_NAME).c_str());
    SetFont(&font);
  }
  m_ShowColor = Configuration::Get(KEY_SCRIPT_SHOWCOLOR);
  m_TabSize = Configuration::Get(KEY_SCRIPT_TABSIZE);
  m_AutoIndent = Configuration::Get(KEY_SCRIPT_AUTOINDENT);

  return Val;
}

///////////////////////////////////////////////////////////////////////////////

void 
CScriptView::FormatAll()
{
  FormatTextRange(0, GetTextLength());
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
  m_ShowColor = (m_ShowColor == true ? false : true);
  MessageBox("Showing/Hiding Colors will not be start\nuntil the window has restarted");
}

///////////////////////////////////////////////////////////////////////////////

void 
CScriptView::ScriptAutoIndent()
{
  m_AutoIndent = (m_AutoIndent ? false : true);
  if (m_AutoIndent)
    MessageBox("Auto Indent enabled.");
  else
    MessageBox("Auto Indent disabled.");
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptView::OnSetFocus(CWnd* pOldWnd)
{
  char ControlKey = GetAsyncKeyState(VK_CONTROL) & 2;
  char ShiftKey   = GetAsyncKeyState(VK_SHIFT) & 2;

  if (!ControlKey) m_ControlPressed = false;
  if (!ShiftKey)   m_ShiftPressed = false;

  CRichEditCtrl::OnSetFocus(pOldWnd);
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptView::OnKeyUp(UINT nKey, UINT nRepCnt, UINT nFlags)
{
  CRichEditCtrl::OnKeyUp(nKey, nRepCnt, nFlags);

  switch(nKey)
  {

  case VK_UP:
  case VK_DOWN:
  case VK_RIGHT:
  case VK_LEFT:
    if (!m_ShiftPressed)
    {
      m_ChangeType = ctMove;
      OnChange();
      UpdateLineInfo();
    }
    break;


  case VK_RETURN:
    //AutoIndent();
    UpdateLineInfo();
    break;

  case VK_CONTROL:
    m_ControlPressed = false;
    break;

  case VK_SHIFT:
    m_ShiftPressed = false;
    break;

  case VK_DELETE: 
    m_ChangeType = ctDelete; 
    OnChange();
    m_Handler->SV_ScriptChanged();
    UpdateLineInfo();
    break;

  case VK_BACK:   
    m_ChangeType = ctBack;
    OnChange();
    m_Handler->SV_ScriptChanged();
    UpdateLineInfo();
    break;

  case VK_PRIOR:  //VK_PAGE_UP:
  case VK_NEXT:   //VK_PAGE_DOWN:
  case VK_HOME:
  case VK_END:
    m_ChangeType = ctMove;
    OnChange();
    UpdateLineInfo();
    break;
  }
  }

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptView::OnKeyDown(UINT nKey, UINT nRepCnt, UINT nFlags)
{ 
  CRichEditCtrl::OnKeyDown(nKey, nRepCnt, nFlags);

  // numlock & using keypad...
  if ((nKey == VK_NUMPAD0 && nFlags == 82) ||
      (nKey == VK_NUMPAD1 && nFlags == 79) ||
      (nKey == VK_NUMPAD2 && nFlags == 80) ||
      (nKey == VK_NUMPAD3 && nFlags == 81) ||
      (nKey == VK_NUMPAD4 && nFlags == 75) ||
      (nKey == VK_NUMPAD5 && nFlags == 76) ||
      (nKey == VK_NUMPAD6 && nFlags == 77) ||
      (nKey == VK_NUMPAD7 && nFlags == 71) ||
      (nKey == VK_NUMPAD8 && nFlags == 72) ||
      (nKey == VK_NUMPAD9 && nFlags == 73) ||
      (nKey == VK_DECIMAL && nFlags == 83))
        m_Handler->SV_ScriptChanged();

  switch (nKey)
  {
  case VK_UP:
  case VK_DOWN:
  case VK_RIGHT:
  case VK_LEFT:
    if (!m_ShiftPressed)
    {
      m_ChangeType = ctMove;
      OnChange();
      
      char buf[80];
      sprintf(buf, "Line %i / %i", LineFromChar(-1) + 1, GetLineCount());
      GetStatusBar()->SetWindowText(buf);
    }
    break;


  case 'C':
  case 'V':
  case 'X':
    if (m_ControlPressed)
      m_Handler->SV_ScriptChanged();
    break;

  case 'A':
  case 'B':
  case 'D':
  case 'E':
  case 'F':
  case 'G':
  case 'H':
  case 'I':
  case 'J':
  case 'K':
  case 'L':
  case 'M':
  case 'N':
  case 'O':
  case 'P':
  case 'Q':
  case 'R':
  case 'S':
  case 'T':
  case 'U':
  case 'W':
  case 'Y':
  case 'Z':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
  case '0':
  case 0x00C0:  // `
  case 0x00BB:  // =
  case 0x00DB:  // {
  case 0x00DD:  // }
  case 186:     // ;
  case 0x00DE:  // '
  case 0x00BC:  // '
  case 0x00BE:  // .
  case 0x00BF:  // /
  case 0x00DC:  /* \ */
  case VK_BACK:
  case VK_RETURN:
  case VK_SPACE:
  case VK_ADD:
  case VK_SUBTRACT:
  case VK_DIVIDE:
  case VK_MULTIPLY:
  case VK_SEPARATOR:
  case VK_DELETE:
    m_Handler->SV_ScriptChanged();
    break;

  case VK_CONTROL:
    m_ControlPressed = true;
    break;

  case VK_SHIFT:
    m_ShiftPressed = true;
    break;

  default:
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptView::OnChar(UINT nKey, UINT nRepCnt, UINT nFlags)
{

  if (nKey == VK_TAB)
  {
    int tab_size = Configuration::Get(KEY_SCRIPT_TAB_SIZE);
    for (int i = 0; i < tab_size; i++) {
      ::SendMessage(this->m_hWnd, WM_CHAR, VK_SPACE, 0);
    }

    OnChange();
    m_Handler->SV_ScriptChanged();
    return;
  }

  m_ChangeType = ctReplSel;
  CRichEditCtrl::OnChar(nKey, nRepCnt, nFlags);

  if (nKey == VK_RETURN)
  {
    AutoIndent();
  }

  OnChange();
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptView::OnLButtonDown(UINT nFlags, CPoint point)
{
  CRichEditCtrl::OnLButtonUp(nFlags, point);
  UpdateLineInfo();
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void 
CScriptView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
  CRichEditCtrl::OnLButtonDblClk(nFlags, point);
  UpdateLineInfo();
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptView::OnRButtonDown(UINT flags, CPoint point)
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
CScriptView::OnFindReplaceMsg(WPARAM wParam, LPARAM lParam)
{
  if (m_SearchDialog->IsTerminating())
    m_SearchDialog = NULL;
  else if (m_SearchDialog->FindNext())
  {
    OnFindText();
  }
  else if (m_SearchDialog->ReplaceCurrent())
  {
    OnReplaceText();
  }
  else if (m_SearchDialog->ReplaceAll())
  {
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

///////////////////////////////////////////////////////////////////////////////

void
CScriptView::UpdateLineInfo()
{
  char buf[80];
  sprintf(buf, "Line %i / %i", LineFromChar(-1) + 1, GetLineCount());
  GetStatusBar()->SetWindowText(buf);
}

///////////////////////////////////////////////////////////////////////////////

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

      } else if (ch == '/' && pPtr[1] == '*') { // Process /* */ comment
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
