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

  void SetStyle(int style, COLORREF fore, COLORREF back = 0xFFFFFF, int size = -1, const char* face = 0);

  bool LoadScript(const char* filename);
  void GetEditorText(CString& text);
  CString GetSelection();

  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnSetFocus(CWnd* old);

  afx_msg void OnScriptCheckSyntax();
  afx_msg void OnScriptFind();
  afx_msg void OnScriptReplace();

  afx_msg void OnSavePointReached(NMHDR* nmhdr, LRESULT* result);
  afx_msg void OnSavePointLeft(NMHDR* nmhdr, LRESULT* result);
  afx_msg void OnPosChanged(NMHDR* nmhdr, LRESULT* result);
  void SetLineNumber(int line);

  afx_msg LRESULT OnFindReplace(WPARAM, LPARAM);

  virtual bool GetSavePath(char* path);
  virtual bool SaveDocument(const char* path);

private:
  bool m_Created;
  HWND m_Editor;

  CFindReplaceDialog* m_SearchDialog;

  DECLARE_MESSAGE_MAP()
};


#endif
