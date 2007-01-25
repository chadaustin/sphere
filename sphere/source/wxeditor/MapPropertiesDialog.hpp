#ifndef MAP_PROPERTIES_DIALOG_HPP
#define MAP_PROPERTIES_DIALOG_HPP


//#include <afxwin.h>
#include <wx/wx.h>
#include "Project.hpp"


class sMap;     // #include "../sphere/common/Map.hpp"


class wMapPropertiesDialog : public wxDialog
{
public:
  wMapPropertiesDialog(wxWindow *parent, sMap* map);

private:
  enum {
    ENTRY,
    EXIT,
    NORTH,
    EAST,
    SOUTH,
    WEST,
  };

private:
  //BOOL OnInitDialog();
  void OnOK(wxCommandEvent &event);

  void OnCheckSyntax(wxCommandEvent &event);
  void OnScriptRadioBox(wxCommandEvent &event);
/*
  void OnClickEntry(wxCommandEvent &event);
  void OnClickExit(wxCommandEvent &event);
  void OnClickNorth(wxCommandEvent &event);
  void OnClickEast(wxCommandEvent &event);
  void OnClickSouth(wxCommandEvent &event);
  void OnClickWest(wxCommandEvent &event);
*/
  void StoreCurrentScript();
  void LoadNewScript();

private:
  sMap* m_Map;

  int m_CurrentScript;

  wxString m_EntryScript;
  wxString m_ExitScript;
  wxString m_NorthScript;
  wxString m_EastScript;
  wxString m_SouthScript;
  wxString m_WestScript;

  wxRadioBox *m_ScriptRadioBox;
  wxTextCtrl *m_ScriptTextBox;
  wxTextCtrl *m_MusicText;

private:
//  DECLARE_CLASS(wMapPropertiesDialog)
  DECLARE_EVENT_TABLE()
};


#endif
