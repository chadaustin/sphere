/* main.cpp
   sde app intialization with project window */

#include <wx/wx.h>
#include <wx/filedlg.h>
#include "types.h"
#include "../common/configfile.hpp"
#include "main.h"

IMPLEMENT_APP(GlomeApp)


// INITIALIZATION ETC

// sde opens
bool GlomeApp::OnInit(){
  ProjectFrame *ProjectWin = new ProjectFrame("Project", 50, 50, 150, 200);
  ProjectWin->Show(true);
  SetTopWindow(ProjectWin);
  return true;
}

// project window initializes
ProjectFrame::ProjectFrame(const wxChar *title, int xpos, int ypos, int width, 
    int height) : wxFrame((wxFrame *)NULL, -1, title, wxPoint(xpos, ypos), 
    wxSize(width, height))
{
  // set up the project tree control
  m_ProjectTree = (wxTreeCtrl *) NULL;
  m_ProjectTree = new wxTreeCtrl(this, -1, wxDefaultPosition, wxDefaultSize, -1);
  wxTreeItemId Item_ProjectName  = m_ProjectTree->AddRoot("Project Name");
  // ? Should we have this be the actual name of the project, and
  // enable multiple projects being open simultaneously? -- riz
  wxTreeItemId MapsFolder         = m_ProjectTree->AppendItem(Item_ProjectName, "Maps"         );
  wxTreeItemId SpritesetsFolder   = m_ProjectTree->AppendItem(Item_ProjectName, "Spritesets"   );
  wxTreeItemId ScriptsFolder      = m_ProjectTree->AppendItem(Item_ProjectName, "Scripts"      );
  wxTreeItemId SoundsFolder       = m_ProjectTree->AppendItem(Item_ProjectName, "Sounds"       );
  wxTreeItemId ImagesFolder       = m_ProjectTree->AppendItem(Item_ProjectName, "Images"       );
  wxTreeItemId WindowStylesFolder = m_ProjectTree->AppendItem(Item_ProjectName, "Window Styles");
  wxTreeItemId FontsFolder        = m_ProjectTree->AppendItem(Item_ProjectName, "Fonts"        );
  wxTreeItemId AnimationsFolder   = m_ProjectTree->AppendItem(Item_ProjectName, "Animations"   );
  m_ProjectTree->SortChildren(Item_ProjectName);

  // set up the menubar
  m_FileMenu = new wxMenu;
  m_FileMenu->Append(PROJFILENEW,   "&New Project" );
  m_FileMenu->Append(PROJFILEOPEN,  "&Open Project");
  m_FileMenu->AppendSeparator();
  m_FileMenu->Append(PROJFILEABOUT, "&About"       );
  m_FileMenu->Append(PROJFILEEXIT,  "E&xit"        );
  m_NewMenu = new wxMenu;
  m_NewMenu->Append(PROJNEWFONT,        "&Font"        );
  m_NewMenu->Append(PROJNEWIMAGE,       "&Image"       );
  m_NewMenu->Append(PROJNEWMAP,         "&Map"         );
  m_NewMenu->Append(PROJNEWSCRIPT,      "&Script"      );
  m_NewMenu->Append(PROJNEWSPRITESET,   "S&priteset"   );
  m_NewMenu->Append(PROJNEWWINDOWSTYLE, "&Window Style");
  m_ImportMenu = new wxMenu;
  m_ImportMenu->Append(PROJIMPORTANIM,        "&Animation"   );
  m_ImportMenu->Append(PROJIMPORTFONT,        "&Font"        );
  m_ImportMenu->Append(PROJIMPORTIMAGE,       "&Image"       );
  m_ImportMenu->Append(PROJIMPORTMAP,         "&Map"         );
  m_ImportMenu->Append(PROJIMPORTSCRIPT,      "&Script"      );
  m_ImportMenu->Append(PROJIMPORTSOUND,       "S&ound"       );
  m_ImportMenu->Append(PROJIMPORTSPRITESET,   "S&priteset"   );
  m_ImportMenu->Append(PROJIMPORTWINDOWSTYLE, "&Window Style");
  m_TopMenu = new wxMenuBar;
  m_TopMenu->Append(m_FileMenu,   "&File"  );
  m_TopMenu->Append(m_NewMenu,    "&New"   );
  m_TopMenu->Append(m_ImportMenu, "&Import");
  SetMenuBar(m_TopMenu);
}

// project window destructs
ProjectFrame::~ProjectFrame(){
}


// EVENTS

// project window event table
  BEGIN_EVENT_TABLE(ProjectFrame, wxFrame)
    // File
    EVT_MENU(PROJFILENEW,           ProjectFrame::OnFileNew          )
    EVT_MENU(PROJFILEOPEN,          ProjectFrame::OnFileOpen         )
    EVT_MENU(PROJFILEABOUT,         ProjectFrame::OnFileAbout        )
    EVT_MENU(PROJFILEEXIT,          ProjectFrame::OnFileExit         )
    // New
    EVT_MENU(PROJNEWFONT,           ProjectFrame::OnNewFont          )
    EVT_MENU(PROJNEWIMAGE,          ProjectFrame::OnNewImage         )
    EVT_MENU(PROJNEWMAP,            ProjectFrame::OnNewMap           )
    EVT_MENU(PROJNEWSCRIPT,         ProjectFrame::OnNewScript        )
    EVT_MENU(PROJNEWSPRITESET,      ProjectFrame::OnNewSpriteset     )
    EVT_MENU(PROJNEWWINDOWSTYLE,    ProjectFrame::OnNewWindowStyle   )
    // Import
    EVT_MENU(PROJIMPORTANIM,        ProjectFrame::OnImportAnim       )
    EVT_MENU(PROJIMPORTFONT,        ProjectFrame::OnImportFont       )
    EVT_MENU(PROJIMPORTIMAGE,       ProjectFrame::OnImportImage      )
    EVT_MENU(PROJIMPORTMAP,         ProjectFrame::OnImportMap        )
    EVT_MENU(PROJIMPORTSCRIPT,      ProjectFrame::OnImportScript     )
    EVT_MENU(PROJIMPORTSOUND,       ProjectFrame::OnImportSound      )
    EVT_MENU(PROJIMPORTSPRITESET,   ProjectFrame::OnImportSpriteset  )
    EVT_MENU(PROJIMPORTWINDOWSTYLE, ProjectFrame::OnImportWindowStyle)
  END_EVENT_TABLE()


// File Menu
  
  void ProjectFrame::OnFileNew(wxCommandEvent & event){ //create a new project
    // ... create the project
  }
  
  
  void ProjectFrame::OnFileOpen(wxCommandEvent & event){ //open a project
    // ... load the project
    const wxChar *proj_file_types = _T("Project Files|*.sgm;*.inf|"
                                  "All files|*.*");
    wxFileDialog *OpenFileDialog = new wxFileDialog(this, "Open Project", "", "", proj_file_types, wxOPEN, wxDefaultPosition);
    if (OpenFileDialog->ShowModal() == wxID_OK){
      m_GameFile = OpenFileDialog->GetFilename();
      CConfigFile config(m_GameFile);
      // ... do something with that filename
      
    }
  }
  
  void ProjectFrame::OnFileAbout(wxCommandEvent & event){ //about dialog
    // set up caption
    wxString pt = _T(gs_PROJTITLE);
    pt.append(_T(gs_PROJVERS));
    pt.append(_T(gs_PROJAUTH));
    // display dialog
    wxMessageDialog dialog_about(this, pt, "About SDE", wxOK);
    dialog_about.ShowModal();
  }
  
  void ProjectFrame::OnFileExit(wxCommandEvent & event){ //exit sde via menu
    Close(true);
  }

  
// New Menu
  void ProjectFrame::OnNewFont(wxCommandEvent & event){ //new font
    // ... new font dialog
  }
  
  void ProjectFrame::OnNewImage(wxCommandEvent & event){ //new image
    // ... new image dialog
  }
  
  void ProjectFrame::OnNewMap(wxCommandEvent & event){ //new map
    // ... new map dialog
  }
  
  void ProjectFrame::OnNewScript(wxCommandEvent & event){ //new script
    // ... new script dialog
  }
  
  void ProjectFrame::OnNewSpriteset(wxCommandEvent & event){ //new spriteset
    // ... new spriteset dialog
  }
  
  void ProjectFrame::OnNewWindowStyle(wxCommandEvent & event){ //new windowstyle
    // ... new windowstyle dialog
  }

  
// Import Menu
  void ProjectFrame::OnImportAnim(wxCommandEvent & event){ // import animation
    // ... import animation dialog
  }
  
  void ProjectFrame::OnImportFont(wxCommandEvent & event){ // import font
    // ... import font dialog
  }
  
  void ProjectFrame::OnImportImage(wxCommandEvent & event){ // import image
    // ... import image dialog
  }
  
  void ProjectFrame::OnImportMap(wxCommandEvent & event){ // import map
    // ... import map dialog
  }
  
  void ProjectFrame::OnImportScript(wxCommandEvent & event){ // import script
    // ... import script dialog
  }
  
  void ProjectFrame::OnImportSound(wxCommandEvent & event){ // import sound
    // ... import sound dialog
  }
  
  void ProjectFrame::OnImportSpriteset(wxCommandEvent & event){ // import spriteset
    // ... import spriteset dialog
  }
  
  void ProjectFrame::OnImportWindowStyle(wxCommandEvent & event){ // import windowstyle
    // ... import windowstyle dialog
  }
