#ifndef MAIN_H
#define MAIN_H

#include "DefaultFileSystem.hpp"
#include "DefaultFile.hpp"

static const wxChar *gs_PROJTITLE = "Glome";
static const wxChar *gs_PROJVERS = "\nv.01a (2002.04.27)\n\n";
static const wxChar *gs_PROJAUTH = "Brandon Mechtley, Ted Reed";

// our application
class GlomeApp : public wxApp {
  public:
    virtual bool OnInit();
};

//project window's frame
class ProjectFrame : public wxFrame {
  public:
    //constructor/destructor
    ProjectFrame(const wxChar *title, int xpos, int ypos, int width, int height);
    ~ProjectFrame();
  
    //controls
    wxTreeCtrl *m_ProjectTree;
    wxMenuBar  *m_TopMenu;
    wxMenu     *m_FileMenu;
    wxMenu     *m_ImportMenu;
    wxMenu     *m_NewMenu;
    int        m_ScreenWidth;
    int        m_ScreenHeight;
    wxString   m_GameTitle;
    wxString   m_GameScript;
    wxString   m_GameFile;
  
    //menu functions
    void OnFileNew           (wxCommandEvent & event);
    void OnFileOpen          (wxCommandEvent & event);
    void OnFileExit          (wxCommandEvent & event);
    void OnFileAbout         (wxCommandEvent & event);
    void OnNewFont           (wxCommandEvent & event);
    void OnNewImage          (wxCommandEvent & event);
    void OnNewMap            (wxCommandEvent & event);
    void OnNewScript         (wxCommandEvent & event);
    void OnNewSpriteset      (wxCommandEvent & event);
    void OnNewWindowStyle    (wxCommandEvent & event);
    void OnImportAnim        (wxCommandEvent & event);
    void OnImportFont        (wxCommandEvent & event);
    void OnImportImage       (wxCommandEvent & event);
    void OnImportMap         (wxCommandEvent & event);
    void OnImportScript      (wxCommandEvent & event);
    void OnImportSound       (wxCommandEvent & event);
    void OnImportSpriteset   (wxCommandEvent & event);
    void OnImportWindowStyle (wxCommandEvent & event);
    
  //menubar event table
  DECLARE_EVENT_TABLE();
};

//menu events
enum {
  //File
  PROJFILEEXIT,
  PROJFILEOPEN,
  PROJFILEABOUT,
  PROJFILENEW,
  //New
  PROJNEWFONT,
  PROJNEWIMAGE,
  PROJNEWMAP,
  PROJNEWSCRIPT,
  PROJNEWSPRITESET,
  PROJNEWWINDOWSTYLE,
  //Import
  PROJIMPORTANIM,
  PROJIMPORTFONT,
  PROJIMPORTIMAGE,
  PROJIMPORTMAP,
  PROJIMPORTSCRIPT,
  PROJIMPORTSOUND,
  PROJIMPORTSPRITESET,
  PROJIMPORTWINDOWSTYLE
};

#endif
