#ifndef ZONE_EDIT_DIALOG_HPP
#define ZONE_EDIT_DIALOG_HPP


#include <afxwin.h>
#include "../common/Map.hpp"


class CZoneEditDialog : public CDialog
{
public:
  CZoneEditDialog(sMap::sZone& zone);

private:
  BOOL OnInitDialog();
  void OnOK();

  afx_msg void OnCheckSyntax();

private:
  sMap::sZone& m_Zone;

  DECLARE_MESSAGE_MAP()
};


#endif
