#ifndef GAME_SETTINGS_DIALOG_HPP
#define GAME_SETTINGS_DIALOG_HPP


//#include <afxwin.h>
#include <wx/wx.h>
#include <wx/spinctrl.h>


class wProject; // #include "Project.hpp"


class wGameSettingsDialog : public wxDialog
{
public:
  wGameSettingsDialog(wxWindow *parent, wProject* project);

private:
  //BOOL OnInitDialog();
  void OnOK(wxCommandEvent &event);

private:
  wProject* m_Project;

  wxTextCtrl *m_ProjectTitle;
  wxComboBox *m_StartScript;
  wxSpinCtrl *m_ScreenWidth;
  wxSpinCtrl *m_ScreenHeight;
private:
  DECLARE_EVENT_TABLE()
};


#endif
