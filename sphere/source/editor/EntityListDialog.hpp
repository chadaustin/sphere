#ifndef CONVOLVE_LIST_DIALOG_HPP
#define CONVOLVE_LIST_DIALOG_HPP

#include <afxwin.h>
#include <string>
#include <vector>

#include "../common/map.hpp"

///////////////////////////////////////////////////////////

class CEntityListDialog : public CDialog
{
public:
  CEntityListDialog(sMap* map);
  ~CEntityListDialog();

private:
  sMap* m_Map;
  std::vector<sEntity*> m_Entities;

private:
  afx_msg void OnEntityChanged();
  afx_msg void OnEditEntity();
  afx_msg void OnDeleteEntities();
  afx_msg void OnMoveEntities();
  afx_msg void OnMoveEntities(int dx, int dy, int layer);

  void UpdateButtons();
  void UpdateEntityDetails(char string[2048], int entity_index, sMap* map);
  std::vector<int> GetSelectedEntities();

private:
  BOOL OnInitDialog();
  void OnOK();

  DECLARE_MESSAGE_MAP()
};

///////////////////////////////////////////////////////////

#endif