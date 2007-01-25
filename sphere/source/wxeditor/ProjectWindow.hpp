#ifndef PROJECT_WINDOW_HPP
#define PROJECT_WINDOW_HPP


//#include <afxwin.h>
//#include <afxcmn.h>
#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/imaglist.h>
class wMainWindow; // MainWindow.hpp
class wProject;    // Project.hpp


class wProjectWindow : public wxMDIChildFrame
{
public:
  wProjectWindow(wMainWindow* main_window, wProject* project);
  ~wProjectWindow();
  
  //bool Create();
  //bool DestroyWindow();

  void Update();

private:
/*todo:
  void OnSysCommand(UINT nID, LPARAM lParam);
  void OnSize(UINT type, int cx, int cy);
  void OnSetFocus(CWnd* pOldWindow);
  void OnDropFiles(HDROP drop_info);
  void OnProjectGroupInsert();

  void OnProjectItemOpen();
  void OnProjectItemDelete();
  void OnKeyDown(NMHDR* notify, LRESULT* result);
  void OnDoubleClick(NMHDR* notify, LRESULT* result);
  void OnRightClick(NMHDR* notify, LRESULT* result);
  void SelectItemAtCursor();
*/
  void OnActivate(wxTreeEvent &event);
  void OnCollapsing(wxTreeEvent &event);

  void OnClose(wxCloseEvent &event);

private:
  wMainWindow* m_MainWindow;
  wProject*    m_Project;

  wxTreeCtrl *m_TreeControl;

#define PROJECT_TREESIZE 8

  wxImageList *m_TreeImages;
  wxTreeItemId m_TreeRoot;
  wxTreeItemId m_TreeGroups[PROJECT_TREESIZE];

  bool m_TreeGroupExpanded[PROJECT_TREESIZE];

  bool m_TreeCreated;
  enum {
    tMAP = 0,
    tSPRITESET,
    tSCRIPT,
    tSOUND,
    tFONT,
    tWINDOWSTYLE,
    tIMAGE,
    tANIMATION,
  };
  //std::vector <wxTreeItemID &> m_TreeList

  // keep handles to the group items
/*
  HTREEITEM m_htiMaps;         bool m_htiMapsExpanded;
  HTREEITEM m_htiSpritesets;   bool m_htiSpritesetsExpanded;
  HTREEITEM m_htiScripts;      bool m_htiScriptsExpanded;
  HTREEITEM m_htiSounds;       bool m_htiSoundsExpanded;
  HTREEITEM m_htiFonts;        bool m_htiFontsExpanded;
  HTREEITEM m_htiWindowStyles; bool m_htiWindowStylesExpanded;
  HTREEITEM m_htiImages;       bool m_htiImagesExpanded;
  HTREEITEM m_htiAnimations;   bool m_htiAnimationsExpanded;
*/
/*
  const wxTreeItemId& m_htiMaps;         bool m_htiMapsExpanded;
  const wxTreeItemId& m_htiSpritesets;   bool m_htiSpritesetsExpanded;
  const wxTreeItemId& m_htiScripts;      bool m_htiScriptsExpanded;
  const wxTreeItemId& m_htiSounds;       bool m_htiSoundsExpanded;
  const wxTreeItemId& m_htiFonts;        bool m_htiFontsExpanded;
  const wxTreeItemId& m_htiWindowStyles; bool m_htiWindowStylesExpanded;
  const wxTreeItemId& m_htiImages;       bool m_htiImagesExpanded;
  const wxTreeItemId& m_htiAnimations;   bool m_htiAnimationsExpanded;
*/
//  DECLARE_MESSAGE_MAP()
  private:
//    DECLARE_CLASS(wProjectWindow);
    DECLARE_EVENT_TABLE();

};


#endif
