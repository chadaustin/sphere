#ifndef SCRIPT_WINDOW_HPP
#define SCRIPT_WINDOW_HPP


#include <afxext.h>
#include "SaveableDocumentWindow.hpp"


class CScriptWindow 
: public CSaveableDocumentWindow
{
public:
  CScriptWindow(const char* filename = NULL);

private:
  bool Create();
  void Initialize();

  LRESULT SendEditor(UINT msg, WPARAM wparam = 0, LPARAM lparam = 0) {
    return ::SendMessage(m_Editor, msg, wparam, lparam);
  }

  void SetScriptStyles();
  void SetStyle(int style, COLORREF fore, COLORREF back = 0xFFFFFF, int size = -1, const char* face = 0);
  void RememberConfiguration();

  bool LoadScript(const char* filename);
  void GetEditorText(CString& text);
  CString GetSelection();

  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnSetFocus(CWnd* old);

  afx_msg void OnScriptCheckSyntax();
  afx_msg void OnScriptFind();
  afx_msg void OnScriptReplace();

  afx_msg void OnOptionsSetScriptFont();
  afx_msg void OnOptionsToggleColors();
  afx_msg void OnOptionsSetTabSize();
  afx_msg void OnOptionsToggleLineNumbers();
  afx_msg void OnOptionsShowWhitespace();
  afx_msg void OnOptionsWordWrap();

  afx_msg void OnUpdateOptionsToggleLineNumbers(CCmdUI* cmdui);
  afx_msg void OnUpdateOptionsToggleColors(CCmdUI* cmdui);
  afx_msg void OnUpdateOptionsShowWhitespace(CCmdUI* cmdui);
  afx_msg void OnUpdateOptionsWordWrap(CCmdUI* cmdui);

  afx_msg void OnSavePointReached(NMHDR* nmhdr, LRESULT* result);
  afx_msg void OnSavePointLeft(NMHDR* nmhdr, LRESULT* result);
  afx_msg void OnPosChanged(NMHDR* nmhdr, LRESULT* result);
  afx_msg void OnCharAdded(NMHDR* nmhdr, LRESULT* result);
  void SetLineNumber(int line);

  afx_msg LRESULT OnFindReplace(WPARAM, LPARAM);

  virtual bool GetSavePath(char* path);
  virtual bool SaveDocument(const char* path);

private:
  bool m_Created;
  HWND m_Editor;

  std::string m_Fontface;
  bool m_SyntaxHighlighted;
  int m_TabWidth;
  bool m_KeyWordStyleIsBold;
  bool m_ShowLineNumbers;
  int m_FontSize;
  bool m_ShowWhitespace;
  bool m_WordWrap;

  CFindReplaceDialog* m_SearchDialog;

  DECLARE_MESSAGE_MAP()
};


#endif
