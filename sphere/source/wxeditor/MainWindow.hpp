#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP


#ifdef _MSC_VER
#pragma warning(disable : 4786)  // identifier too long
#endif


#include <vector>
//#include <afxwin.h>
//#include <afxext.h>
#include <wx/wx.h>
#include <wx/docmdi.h>
#include "Project.hpp"
#include "SwatchServer.hpp"

//#ifndef MAX_PATH
//#define MAX_PATH 1024
//#endif

class wDocumentWindow;  // DocumentWindow.hpp
class wProjectWindow;   // ProjectWindow.hpp


// send this to the main window to insert a file
#define WM_INSERT_PROJECT_FILE (WM_APP + 923) // wparam = 0, path = (char*)lparam
#define WM_REFRESH_PROJECT     (WM_APP + 924) // wparam = 0, lparam = 0


class wMainWindow : public wxMDIParentFrame
{
public:
  wMainWindow(
              const wxString& title, const wxPoint& pos,
              const wxSize& size, long type);
  
  bool Create();

  void CreateProject(const char* projectname, const char* gametitle);
  void OpenProject(const char* filename);
  void CloseProject();

  void CloseNotice(wDocumentWindow *doc);

  void OpenGameSettings();
  void OpenGameFile(const char* filename);
  void OpenDocumentWindow(int grouptype, const char* filename);

  void SetChildMenu(int childmenu, wxEvtHandler *receiver);
  void ClearChildMenu();

  bool ProcessEvent(wxEvent &event);

private:
  bool AddToDirectory(const char* pathname, const char* sub_directory);
  bool CheckDirectory(const char* filename, const char* sub_directory);
  void InsertProjectFile(wxFileDialog* file_dialog, int grouptype, const char* predefined_path = NULL);
  void GetGamesDirectory(char games_directory[]); //[MAX_PATH]);
  void UpdateMenu();
  void UpdatePaletteMenu();

/*todo:
  void OnDropFiles(HDROP hDropInfo);
*/
  void OnClose(wxCloseEvent &event);

  void OnFileOpen(wxCommandEvent &event);
  void wMainWindow::OnFileClose(wxCommandEvent &event);

  void OnFileNewProject(wxCommandEvent &event);
  void OnFileOpenProject(wxCommandEvent &event);
  void OnFileCloseProject(wxCommandEvent &event);
  void OnFileOpenLastProject(wxCommandEvent &event);

  void OnFileNewMap(wxCommandEvent &event);
  void OnFileNewSpriteset(wxCommandEvent &event);
  void OnFileNewScript(wxCommandEvent &event);
  void OnFileNewFont(wxCommandEvent &event);
  void OnFileNewWindowStyle(wxCommandEvent &event);
  void OnFileNewImage(wxCommandEvent &event);
  void OnFileNewDoodad(wxCommandEvent &event);

  void OnFileOpenMap(wxCommandEvent &event);
  void OnFileOpenSpriteset(wxCommandEvent &event);
  void OnFileOpenScript(wxCommandEvent &event);
  void OnFileOpenSound(wxCommandEvent &event);
  void OnFileOpenFont(wxCommandEvent &event);
  void OnFileOpenWindowStyle(wxCommandEvent &event);
  void OnFileOpenImage(wxCommandEvent &event);
  void OnFileOpenAnimation(wxCommandEvent &event);
  void OnFileOpenDoodad(wxCommandEvent &event);

  void OnFileImportImageToMap(wxCommandEvent &event);
  void OnFileImportBitmapToRWS(wxCommandEvent &event);
  void OnFileImportBitmapToRSS(wxCommandEvent &event);
  void OnFileImportBitmapToRTS(wxCommandEvent &event);
  void OnFileImportVergeFontTemplate(wxCommandEvent &event);
  void OnFileImportVergeMap(wxCommandEvent &event);
  void OnFileImportVergeSpriteset(wxCommandEvent &event);
  void OnFileImportMergeRGBA(wxCommandEvent &event);
  void OnFileImportWindowsFont(wxCommandEvent &event);

  void OnFileSaveAll(wxCommandEvent &event);

  void OnFileOptions(wxCommandEvent &event);

  void OnProjectInsertMap(wxCommandEvent &event);
  void OnProjectInsertSpriteset(wxCommandEvent &event);
  void OnProjectInsertScript(wxCommandEvent &event);
  void OnProjectInsertSound(wxCommandEvent &event);
  void OnProjectInsertFont(wxCommandEvent &event);
  void OnProjectInsertWindowStyle(wxCommandEvent &event);
  void OnProjectInsertImage(wxCommandEvent &event);
  void OnProjectInsertAnimation(wxCommandEvent &event);
  void OnProjectInsertDoodad(wxCommandEvent &event);

  void OnProjectRefresh(wxCommandEvent &event);
  void OnProjectRunSphere(wxCommandEvent &event);
  void OnProjectConfigureSphere(wxCommandEvent &event);
  void OnProjectPackageGame(wxCommandEvent &event);

  void OnWindowCloseAll(wxCommandEvent &event);

  void OnHelpContents(wxCommandEvent &event);
  void OnHelpAbout(wxCommandEvent &event);

/*todo:
  BOOL OnNeedText(UINT id, NMHDR* nmhdr, LRESULT* result);
*/

/*todo:
  void OnUpdateOpenLastProject(CCmdUI* cmdui);
  void OnUpdateProjectCommand(CCmdUI* cmdui);
  void OnUpdateSaveCommand(CCmdUI* cmdui);
  void OnUpdateSaveAllCommand(CCmdUI* cmdui);
  void OnUpdateWindowCloseAll(CCmdUI* cmdui);
*/
/*todo:
  void OnInsertProjectFile(WPARAM wparam, LPARAM lparam);
  void OnRefreshProject(WPARAM wparam, LPARAM lparam);

  LRESULT OnDocumentWindowClosing(WPARAM wparam, LPARAM lparam);
  LRESULT OnSetChildMenu(WPARAM wparam, LPARAM lparam);
  LRESULT OnClearChildMenu(WPARAM wparam, LPARAM lparam);
  LRESULT OnUpdatePaletteMenu(WPARAM wparam, LPARAM lparam);
  void OnViewPalette(UINT id);
*/
private:
  wxToolBar   m_ToolBar;
  wxStatusBar m_StatusBar;

  bool     m_ProjectOpen;
  wProject m_Project;

  wProjectWindow* m_ProjectWindow;

  wxMenuBar *m_MenuBar;

  // manage pointers to document windows so we can enumerate and send messages to them
  std::vector<wDocumentWindow*> m_DocumentWindows;

  int m_ChildMenuResource;
  int m_PreviousChildMenuResource;
  wxString m_ChildMenuTitle;
  wxEvtHandler *m_MenuReceiver;
  int m_InsideEventProcess;

//  DECLARE_MESSAGE_MAP()
  private:
//    DECLARE_CLASS(wMainWindow);
    DECLARE_EVENT_TABLE();

};


#endif
