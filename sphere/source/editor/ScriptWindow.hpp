#ifndef SCRIPT_WINDOW_HPP
#define SCRIPT_WINDOW_HPP


#include <afxext.h>
#include "SaveableDocumentWindow.hpp"
#include "ScriptView.hpp"


class CScriptWindow 
: public CSaveableDocumentWindow
, private IScriptViewHandler
{
public:
  CScriptWindow(const char* filename = NULL);
  ~CScriptWindow();

private:
  void Create();

  bool LoadScript(const char* filename);

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
  bool        m_Created;
  CScriptView m_Edit;

  DECLARE_MESSAGE_MAP()
};


#endif
