#ifndef SCRIPT_WINDOW_HPP
#define SCRIPT_WINDOW_HPP

//#include <afxext.h>
#include <wx/wx.h>

#ifdef WIN32
#include <wx/fdrepdlg.h>
#include <wx/stc/stc.h>
#else
#include <wx/textctrl.h>
#endif

#include "SaveableDocumentWindow.hpp"


class wScriptWindow 
: public wSaveableDocumentWindow
{
public:
  wScriptWindow(const char* filename = NULL);

private:
  bool Create();
  void Initialize();

/*todo:
  LRESULT SendEditor(UINT msg, WPARAM wparam = 0, LPARAM lparam = 0) {
    return ::SendMessage(m_Editor, msg, wparam, lparam);
  }
*/
  void SetStyle(int style, const wxColour &fore, const wxColour &back = wxColour(0xff, 0xff, 0xff), int size = -1, const char* face = NULL);

  bool LoadScript(const char* filename);
  void GetEditorText(wxString& text);
  wxString GetSelection();

  void OnSize(wxSizeEvent &event);
  void OnSetFocus(wxFocusEvent &event);

  void OnScriptCheckSyntax(wxCommandEvent &event);

#ifdef WIN32
  void OnScriptFind(wxCommandEvent &event);
  void OnScriptReplace(wxCommandEvent &event);

  void OnSavePointReached(wxStyledTextEvent &event);
  void OnSavePointLeft(wxStyledTextEvent &event);
  void OnPosChanged(wxStyledTextEvent &event);
  void OnCharAdded(wxStyledTextEvent &event);

  void OnFindReplace(wxFindDialogEvent &event);
#endif

  void SetLineNumber(int line);

  virtual bool GetSavePath(char* path);
  virtual bool SaveDocument(const char* path);

private:
  bool m_Created;

#ifdef WIN32
  wxStyledTextCtrl *m_Editor;

  wxFindReplaceDialog* m_SearchDialog;
  wxFindReplaceData *m_SearchData;
#else
  wxTextCtrl *m_Editor;
#endif // WIN32

private:
//  DECLARE_CLASS(wScriptWindow)
  DECLARE_EVENT_TABLE()
};


#endif
