#ifndef ENTITY_TRIGGER_DIALOG_HPP
#define ENTITY_TRIGGER_DIALOG_HPP


//#include <afxwin.h>
#include <wx/wx.h>
#include "../common/Entities.hpp"


class wEntityTriggerDialog : public wxDialog
{
public:
  wEntityTriggerDialog(wxWindow *parent, sTriggerEntity& trigger);

private:
  //bool OnInitDialog();
  void OnOK(wxCommandEvent &event);

  void OnCheckSyntax(wxCommandEvent &event);

private:
  sTriggerEntity& m_Trigger;

  wxTextCtrl *m_ScriptBox;

private:
//  DECLARE_CLASS(wEntityTriggerDialog)
  DECLARE_EVENT_TABLE()
};


#endif
