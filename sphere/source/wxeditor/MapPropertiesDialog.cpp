#include "MapPropertiesDialog.hpp"
#include "Scripting.hpp"
#include "../common/Map.hpp"
//#include "resource.h"
#include "IDs.hpp"

/*
BEGIN_MESSAGE_MAP(CMapPropertiesDialog, CDialog)

  ON_BN_CLICKED(IDC_CHECK_SYNTAX, OnCheckSyntax)
  ON_BN_CLICKED(IDC_ENTRY,        OnClickEntry)
  ON_BN_CLICKED(IDC_EXIT,         OnClickExit)
  ON_BN_CLICKED(IDC_NORTH,        OnClickNorth)
  ON_BN_CLICKED(IDC_EAST,         OnClickEast)
  ON_BN_CLICKED(IDC_SOUTH,        OnClickSouth)
  ON_BN_CLICKED(IDC_WEST,         OnClickWest)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wMapPropertiesDialog, wxDialog)
  EVT_BUTTON(wxID_OK, OnOK)
  EVT_BUTTON(wID_MAPPROPERTIESDIALOG_CHECK_SYNTAX, wMapPropertiesDialog::OnCheckSyntax)
  EVT_RADIOBOX(wID_MAPPROPERTIESDIALOG_RADIOBOX,   wMapPropertiesDialog::OnScriptRadioBox)
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wMapPropertiesDialog::wMapPropertiesDialog(wxWindow *parent, sMap* map)
: wxDialog(parent, -1, "Map Properties", wxDefaultPosition, wxSize(208, 72), wxDIALOG_MODAL | wxCAPTION | wxSYSTEM_MENU)
, m_Map(map)
, m_CurrentScript(ENTRY)
{
  m_EntryScript = m_Map->GetEntryScript();
  m_ExitScript  = m_Map->GetExitScript();
  m_NorthScript = m_Map->GetEdgeScript(sMap::NORTH);
  m_EastScript  = m_Map->GetEdgeScript(sMap::EAST);
  m_SouthScript = m_Map->GetEdgeScript(sMap::SOUTH);
  m_WestScript  = m_Map->GetEdgeScript(sMap::WEST);

  wxBoxSizer *mainsizer;
  wxBoxSizer *subsizer;
  mainsizer = new wxBoxSizer(wxVERTICAL);

    subsizer = new wxBoxSizer(wxHORIZONTAL);
      subsizer->Add(new wxStaticText(this, -1, "Background Music"), 1, wxALL, 10);
      subsizer->Add(m_MusicText = new wxTextCtrl(this, -1, m_Map->GetMusicFile()), 1, wxALL | wxGROW, 10);

      subsizer->Add(new wxButton(this, wID_MAPPROPERTIESDIALOG_CHECK_SYNTAX, "Check Syntax"), 0, wxALL | wxALIGN_RIGHT , 10);
    mainsizer->Add(subsizer);

    wxString choices[] = {"Entry", "Exit", "North", "East", "South", "West"};
    subsizer = new wxBoxSizer(wxHORIZONTAL);
      subsizer->Add(m_ScriptRadioBox = new wxRadioBox(
            this, wID_MAPPROPERTIESDIALOG_RADIOBOX, "Scripts", wxDefaultPosition, wxDefaultSize, 
            6, choices, 1, wxRA_SPECIFY_COLS), 0, wxALL, 10);
      subsizer->Add(m_ScriptTextBox = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxSize(374, 128), wxTE_MULTILINE), 1, wxALL | wxGROW, 10);
    mainsizer->Add(subsizer);

    subsizer = new wxBoxSizer(wxHORIZONTAL);
      subsizer->Add(new wxButton(this, wxID_OK, "OK"), 0, wxALL, 10);
      subsizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 10);
    mainsizer->Add(subsizer, 0, wxALIGN_CENTRE_HORIZONTAL, 0);

  SetSizer(mainsizer);
  mainsizer->SetSizeHints(this);

  m_ScriptRadioBox->SetSelection(0);
  LoadNewScript();
}

/*
////////////////////////////////////////////////////////////////////////////////

BOOL
CMapPropertiesDialog::OnInitDialog()
{
  // set music file
  SetDlgItemText(IDC_MUSIC, m_Map->GetMusicFile());

  // set script file
  CheckDlgButton(IDC_ENTRY, BST_CHECKED);

  // put script in edit control
  LoadNewScript();

  return TRUE;
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wMapPropertiesDialog::OnOK(wxCommandEvent &event)
{
  // get music file
  m_Map->SetMusicFile(m_MusicText->GetValue().c_str());

  StoreCurrentScript();

  m_Map->SetEntryScript(            m_EntryScript);
  m_Map->SetExitScript(             m_ExitScript);
  m_Map->SetEdgeScript(sMap::NORTH, m_NorthScript);
  m_Map->SetEdgeScript(sMap::EAST,  m_EastScript);
  m_Map->SetEdgeScript(sMap::SOUTH, m_SouthScript);
  m_Map->SetEdgeScript(sMap::WEST,  m_WestScript);

  wxDialog::OnOK(event);
}

////////////////////////////////////////////////////////////////////////////////

void
wMapPropertiesDialog::OnCheckSyntax(wxCommandEvent &event)
{
  wxString script = m_ScriptTextBox->GetValue();

  // check for errors
  sCompileError error;
  if (VerifyScript(script.c_str(), error)) {
    ::wxMessageBox("No syntax errors", "Check Syntax");
  } else {
    // show error
    ::wxMessageBox(("Script error:\n" + error.m_Message).c_str(), "Check Syntax");

    // select text
    m_ScriptTextBox->SetSelection(
      error.m_TokenStart,
      error.m_TokenStart + error.m_Token.length()
    );
    m_ScriptTextBox->SetFocus();
  }
}

void 
wMapPropertiesDialog::OnScriptRadioBox(wxCommandEvent &event) {
  StoreCurrentScript();
  switch (m_ScriptRadioBox->GetSelection()) {
  case 0: m_CurrentScript = ENTRY; break;
  case 1: m_CurrentScript = EXIT; break;
  case 2: m_CurrentScript = NORTH; break;
  case 3: m_CurrentScript = EAST; break;
  case 4: m_CurrentScript = SOUTH; break;
  case 5: m_CurrentScript = WEST; break;
  default: return;
  }
  LoadNewScript();
}

/*
////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapPropertiesDialog::OnClickEntry()
{
  StoreCurrentScript();
  m_CurrentScript = ENTRY;
  LoadNewScript();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapPropertiesDialog::OnClickExit()
{
  StoreCurrentScript();
  m_CurrentScript = EXIT;
  LoadNewScript();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapPropertiesDialog::OnClickNorth()
{
  StoreCurrentScript();
  m_CurrentScript = NORTH;
  LoadNewScript();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapPropertiesDialog::OnClickEast()
{
  StoreCurrentScript();
  m_CurrentScript = EAST;
  LoadNewScript();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapPropertiesDialog::OnClickSouth()
{
  StoreCurrentScript();
  m_CurrentScript = SOUTH;
  LoadNewScript();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapPropertiesDialog::OnClickWest()
{
  StoreCurrentScript();
  m_CurrentScript = WEST;
  LoadNewScript();
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wMapPropertiesDialog::StoreCurrentScript()
{
  wxString script = m_ScriptTextBox->GetValue();
  switch (m_CurrentScript) {
    case ENTRY: m_EntryScript = script; break;
    case EXIT:  m_ExitScript = script;  break;
    case NORTH: m_NorthScript = script; break;
    case EAST:  m_EastScript = script;  break;
    case SOUTH: m_SouthScript = script; break;
    case WEST:  m_WestScript = script;  break;
    default:    return;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMapPropertiesDialog::LoadNewScript()
{
  wxString script;
  switch (m_CurrentScript) {
    case ENTRY: script = m_EntryScript; break;
    case EXIT:  script = m_ExitScript;  break;
    case NORTH: script = m_NorthScript; break;
    case EAST:  script = m_EastScript;  break;
    case SOUTH: script = m_SouthScript; break;
    case WEST:  script = m_WestScript;  break;
    default:    return;
  }
  m_ScriptTextBox->SetValue(script);
}

////////////////////////////////////////////////////////////////////////////////
