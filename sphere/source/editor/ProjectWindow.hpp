#ifndef PROJECT_WINDOW_HPP
#define PROJECT_WINDOW_HPP


#include <afxwin.h>
#include <afxcmn.h>
#include "PaletteWindow.hpp"

class CMainWindow; // MainWindow.hpp
class CProject;    // Project.hpp


class CProjectWindow : public CPaletteWindow
{
public:
  CProjectWindow(CMainWindow* main_window, CProject* project);
  
  BOOL Create();
  BOOL DestroyWindow();

  void Update();

private:
  afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnSetFocus(CWnd* pOldWindow);
  afx_msg void OnDropFiles(HDROP drop_info);

  afx_msg void OnProjectGroupInsert();

  afx_msg void OnProjectItemOpen();
  afx_msg void OnProjectItemDelete();

  afx_msg void OnKeyDown(NMHDR* notify, LRESULT* result);
  afx_msg void OnDoubleClick(NMHDR* notify, LRESULT* result);
  afx_msg void OnRightClick(NMHDR* notify, LRESULT* result);

  void SelectItemAtCursor();

private:
  CMainWindow* m_MainWindow;
  CProject*    m_Project;

  CTreeCtrl m_TreeControl;

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

  DECLARE_MESSAGE_MAP()
};


#endif
