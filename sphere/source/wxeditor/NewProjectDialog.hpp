#ifndef NEW_PROJECT_DIALOG_HPP
#define NEW_PROJECT_DIALOG_HPP


//#include <afxwin.h>
#include <wx/wx.h>


class wNewProjectDialog : public wxDialog
{
public:
  wNewProjectDialog(wxWindow* parent);
  ~wNewProjectDialog();

  const char* GetProjectName();
  const char* GetGameTitle();

private:
  void OnOK(wxCommandEvent &event);
  //afx_msg BOOL OnInitDialog();

private:
  wxString m_ProjectName;
  wxString m_GameTitle;

  wxTextCtrl *m_ProjectNameCtrl;
  wxTextCtrl *m_GameTitleCtrl;
private:
  DECLARE_EVENT_TABLE()
};


#endif
