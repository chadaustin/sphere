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

  afx_msg void OnSize(UINT yype, int cx, int cy);
  afx_msg void OnSetFocus(CWnd* old);

  afx_msg void OnScriptCheckSyntax();
  afx_msg void OnScriptFind();
  afx_msg void OnScriptReplace();
  afx_msg void OnScriptSetDefaultFont();
  afx_msg void OnScriptSetTabSize();
  afx_msg void OnScriptShowColors();
  afx_msg void OnScriptEnableAutoIndent();

  virtual bool GetSavePath(char* path);
  virtual bool SaveDocument(const char* path);

  virtual void SV_ScriptChanged();

private:
  bool m_Created;
  HWND m_Editor;

  DECLARE_MESSAGE_MAP()
};


#endif
