#ifndef SCRIPT_WINDOW_HPP
#define SCRIPT_WINDOW_HPP


#include <afxext.h>
#include "SaveableDocumentWindow.hpp"


class CScriptWindow 
: public CSaveableDocumentWindow
{
public:
  CScriptWindow(const char* filename = NULL, bool create_from_clipboard = false);

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
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

  afx_msg void OnScriptCheckSyntax();
  afx_msg void OnScriptFind();
  afx_msg void OnScriptReplace();
  afx_msg void OnScriptGotoLine();

  /////////////////////////////////////

  afx_msg void OnUpdateScriptCheckSyntax(CCmdUI* cmdui);

  afx_msg void OnOptionsSetScriptFont();
  afx_msg void OnOptionsToggleColors();
  afx_msg void OnOptionsSetTabSize();
  afx_msg void OnOptionsToggleLineNumbers();
  afx_msg void OnOptionsShowWhitespace();
  afx_msg void OnOptionsWordWrap();
  afx_msg void OnOptionsShowWrapFlags();
  afx_msg void OnOptionsToggleAutoComplete();
  afx_msg void OnOptionsHighlightCurrentLine();
  afx_msg void OnOptionsViewList();

  afx_msg void OnOptionsSelectionStream();
  afx_msg void OnOptionsSelectionRectangle();
  afx_msg void OnOptionsSelectionLine();
  afx_msg void OnUpdateOptionsSelectionStream(CCmdUI* cmdui);
  afx_msg void OnUpdateOptionsSelectionRectangle(CCmdUI* cmdui);
  afx_msg void OnUpdateOptionsSelectionLine(CCmdUI* cmdui);
  afx_msg void OnUpdateOptionsHighlightCurrentLine(CCmdUI* cmdui);
  afx_msg void OnUpdateOptionsViewList(CCmdUI* cmdui);

#if 0
  afx_msg void OnOptionsCheckSpelling();
  afx_msg void OnOptionsSetSpellingLanguage();
  afx_msg void OnUpdateOptionsCheckSpelling(CCmdUI* cmdui);
#endif

  afx_msg void OnUpdateOptionsToggleLineNumbers(CCmdUI* cmdui);
  afx_msg void OnUpdateOptionsToggleColors(CCmdUI* cmdui);
  afx_msg void OnUpdateOptionsShowWhitespace(CCmdUI* cmdui);
  afx_msg void OnUpdateOptionsWordWrap(CCmdUI* cmdui);
  afx_msg void OnUpdateOptionsShowWrapFlags(CCmdUI* cmdui);
  afx_msg void OnUpdateOptionsToggleAutoComplete(CCmdUI* cmdui);

  /////////////////////////////////////

  afx_msg void OnScriptLineSorter();

  /////////////////////////////////////

  afx_msg void OnSavePointReached(NMHDR* nmhdr, LRESULT* result);
  afx_msg void OnSavePointLeft(NMHDR* nmhdr, LRESULT* result);
  afx_msg void OnPosChanged(NMHDR* nmhdr, LRESULT* result);
  afx_msg void OnCharAdded(NMHDR* nmhdr, LRESULT* result);
  afx_msg void OnMarginClick(NMHDR* nmhdr, LRESULT* result);
  void SetLineNumber(int line);

  void SpellCheck(const int start_line_number, const int num_lines_to_check);

#if 1
  bool MarginClick(int position, int modifiers);
  void Expand(int &line, const bool doExpand,
              bool force = false, int visLevels = 0, int level = -1);
  void FoldAll();
#endif

  afx_msg void OnCopy();
  afx_msg void OnPaste();
  afx_msg void OnZoomIn();
  afx_msg void OnZoomOut();

  void OnToolChanged(UINT id, int tool_index);
  BOOL IsToolAvailable(UINT id);

  afx_msg LRESULT OnFindReplace(WPARAM, LPARAM);

  virtual bool GetSavePath(char* path);
  virtual bool SaveDocument(const char* path);


  enum ScriptType {
    SCRIPT_TYPE_UNDETERMINABLE,
    SCRIPT_TYPE_UNKNOWN,
    SCRIPT_TYPE_JS,
    //SCRIPT_TYPE_PY,
    SCRIPT_TYPE_TXT,
  };

  ScriptType GetScriptType();
  bool IsSyntaxHighlightable();

private:
  afx_msg void OnScriptViewInsert();
  afx_msg void OnScriptViewDelete();
  afx_msg void OnScriptViewRefresh();
  afx_msg void OnUpdateScriptViewInsert(CCmdUI* cmdui);
  afx_msg void OnUpdateScriptViewDelete(CCmdUI* cmdui);

  afx_msg void OnScriptViewSphereFunctions();
  afx_msg void OnScriptViewControlStructures();
  afx_msg void OnScriptViewCurrentScriptFunctions();
  afx_msg void OnScriptViewClipboardHistory();
  afx_msg void OnScriptViewControlCharacters();
  afx_msg void OnUpdateScriptViewSphereFunctions(CCmdUI* cmdui);
  afx_msg void OnUpdateScriptViewControlStructures(CCmdUI* cmdui);
  afx_msg void OnUpdateScriptViewCurrentScriptFunctions(CCmdUI* cmdui);
  afx_msg void OnUpdateScriptViewClipboardHistory(CCmdUI* cmdui);
  afx_msg void OnUpdateScriptViewControlCharacters(CCmdUI* cmdui);

  void CreateList(int type);

#if 0
private:
  UINT m_Timer;
  afx_msg void OnTimer(UINT event);
#endif

private:
  bool m_Created;
  HWND m_Editor;
  HWND m_List;

  int m_ListType;

  std::string m_Fontface;
  bool m_SyntaxHighlighted;
  int m_TabWidth;
  bool m_KeyWordStyleIsBold;
  bool m_ShowLineNumbers;
  int m_FontSize;
  bool m_ShowWhitespace;
  bool m_WordWrap;
  bool m_AllowAutoComplete;
  int m_SelectionType;
  bool m_CheckSpelling;
  bool m_ShowWrapFlags;
  bool m_HighlightCurrentLine;

  void UpdateBraceHighlight();

  CFindReplaceDialog* m_SearchDialog;

  DECLARE_MESSAGE_MAP()
};


#endif
