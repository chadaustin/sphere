#ifndef NEW_MAP_DIALOG_HPP
#define NEW_MAP_DIALOG_HPP


#include <afxwin.h>
#include <string>


class CNewMapDialog : public CDialog
{
public:
  CNewMapDialog();

  int GetMapWidth() const;
  int GetMapHeight() const;
  const char* GetTileset() const;

private:
  BOOL OnInitDialog();
  void OnOK();

  afx_msg void OnTilesetBrowse();

private:
  int         m_MapWidth;
  int         m_MapHeight;
  std::string m_Tileset;

  DECLARE_MESSAGE_MAP()
};


#endif
