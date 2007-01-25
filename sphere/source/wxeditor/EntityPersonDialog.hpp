#ifndef ENTITY_PERSON_DIALOG_HPP
#define ENTITY_PERSON_DIALOG_HPP


//#include <afxwin.h>
#include <wx/wx.h>
#include "../common/Entities.hpp"


class wEntityPersonDialog : public wxDialog
{
public:
  wEntityPersonDialog(wxWindow *parent, sPersonEntity& person);

private:
  //bool OnInitDialog();
  void OnOK(wxCommandEvent &event);

  void OnBrowseSpriteset(wxCommandEvent &event);
  void OnCheckSyntax(wxCommandEvent &event);
  void OnScriptChanged(wxCommandEvent &event);

  void SetScript();
  void StoreScript();

private:
  sPersonEntity& m_Person;
  
  int     m_CurrentScript;
  wxString m_Scripts[5];

  wxTextCtrl *m_PersonNameCtrl;
  wxTextCtrl *m_SpritesetCtrl;
  wxTextCtrl *m_ScriptTextBox;
  wxRadioBox *m_ScriptRadioBox;

private:
//  DECLARE_CLASS(wEntityPersonDialog)
  DECLARE_EVENT_TABLE()
};


#endif
