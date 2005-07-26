#ifndef MAP_PROPERTIES_DIALOG_HPP
#define MAP_PROPERTIES_DIALOG_HPP


#include <afxwin.h>
#include "Project.hpp"


class sMap;     // #include "../sphere/common/Map.hpp"


class CMapPropertiesDialog : public CDialog
{
public:
  CMapPropertiesDialog(sMap* map, const char* document_path);

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
  BOOL OnInitDialog();
  void OnOK();

  afx_msg void OnBrowseBackgroundMusic();
  afx_msg void OnBrowseTileset();

  afx_msg void OnCheckSyntax();
  afx_msg void OnClickEntry();
  afx_msg void OnClickExit();
  afx_msg void OnClickNorth();
  afx_msg void OnClickEast();
  afx_msg void OnClickSouth();
  afx_msg void OnClickWest();

  void StoreCurrentScript();
  void LoadNewScript();

private:
  sMap* m_Map;
  const char* m_DocumentPath;

  int m_CurrentScript;

  CString m_EntryScript;
  CString m_ExitScript;
  CString m_NorthScript;
  CString m_EastScript;
  CString m_SouthScript;
  CString m_WestScript;

  DECLARE_MESSAGE_MAP()
};


#endif
