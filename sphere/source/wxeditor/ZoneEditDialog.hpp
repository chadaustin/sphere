#ifndef ZONE_EDIT_DIALOG_HPP
#define ZONE_EDIT_DIALOG_HPP


//#include <afxwin.h>
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include "../common/Map.hpp"


class wZoneEditDialog : public wxDialog
{
public:
  wZoneEditDialog(wxWindow *parent, sMap::sZone& zone);

private:
  //bool OnInitDialog();
  void OnOK(wxCommandEvent &event);

  void OnCheckSyntax(wxCommandEvent &event);

private:
  sMap::sZone& m_Zone;

  wxSpinCtrl *m_StepsCtrl;
  wxTextCtrl *m_ScriptBox;


private:
//  DECLARE_CLASS(wZoneEditDialog)
  DECLARE_EVENT_TABLE()
};


#endif
