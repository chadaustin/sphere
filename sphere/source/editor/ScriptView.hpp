/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1998 by Juraj Rojko jrojko@twist.cz
// All rights reserved
//
// VBScriptEditorView.h : interface of the CVBScriptEditorView class
//
/////////////////////////////////////////////////////////////////////////////

/* 
   26th June 2000
   Find Dialog implemented manually due to CRichEditCtrl being a bitch.

   19th April 2000
   Code cleaned up and modified by Darklich for use with SDE 
   (Sphere Development Environment)
*/


#ifndef SCRIPT_VIEW_HPP
#define SCRIPT_VIEW_HPP


#include <afxwin.h>
#include <afxcmn.h>
#include <afxext.h>


class IScriptViewHandler
{
public:
  virtual void SV_ScriptChanged() = 0;
};


class CScriptView : public CRichEditCtrl
{
public:
  CScriptView();
  ~CScriptView();
  
  BOOL Create(IScriptViewHandler* pHandler, CWnd* pParentWindow);
  void FormatAll();

  void ScriptFindWord();
  void ScriptFindReplaceWord();
  void ScriptShowColors();
  void ScriptAutoIndent();

private:
  afx_msg void OnSetFocus(CWnd *pOldWnd);
  afx_msg void OnKeyUp(UINT nKey, UINT nRepCnt, UINT nFlags);
  afx_msg void OnKeyDown(UINT nKey, UINT nRepCnt, UINT nFlags);
  afx_msg void OnChar(UINT nKey, UINT nRepCnt, UINT nFlags);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
  afx_msg void OnRButtonDown(UINT flags, CPoint point);
  LRESULT OnFindReplaceMsg(WPARAM wParam, LPARAM lParam);

  afx_msg void OnUndo();
  afx_msg void OnCut();
  afx_msg void OnCopy();
  afx_msg void OnPaste();
  afx_msg void OnDelete();
  afx_msg void OnSelectAll();

  void AutoIndent();
  void UpdateLineInfo();
  void OnChange();
  void SetFormatRange(int nStart, int nEnd, BOOL bBold, COLORREF clr);
  void FormatTextLines(int nLineStart, int nLineEnd);
  void FormatTextRange(int nStart, int nEnd);

  void OnFindText();
  void OnReplaceText();
  void OnReplaceAll();

  bool JustFindText(bool ReplaceAll);
  void JustReplaceText();

private:
  struct SymbolColor
  {
    bool     bold;
    COLORREF color;
  };
  enum ChangeType { ctUndo, ctUnknown, ctReplSel, ctBack, ctDelete, ctCut, ctPaste, ctMove };

private:
  CFindReplaceDialog* m_SearchDialog;
  IScriptViewHandler* m_Handler;
  ChangeType          m_ChangeType;
	
  static SymbolColor m_icComment;
  static SymbolColor m_icNumber;
  static SymbolColor m_icString;
  static SymbolColor m_icKeyword;
  static SymbolColor m_icFunction;

  CHARRANGE m_OldSel;

  bool m_ShowColor;
  bool m_InForcedChange;
  bool m_ControlPressed;
  bool m_ShiftPressed;
  bool m_NumlockPressed;
  bool m_FindReplaceDialogCreated;

  long m_LineCount;
  int  m_TabSize;
  bool m_AutoIndent;

  DECLARE_MESSAGE_MAP()
};


#endif
