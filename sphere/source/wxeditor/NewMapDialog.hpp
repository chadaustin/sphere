#ifndef NEW_MAP_DIALOG_HPP
#define NEW_MAP_DIALOG_HPP


//#include <afxwin.h>
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <string>


class wNewMapDialog : public wxDialog
{
public:
  wNewMapDialog(wxWindow *parent);

  int GetMapWidth() const;
  int GetMapHeight() const;
  const char* GetTileset() const;

private:
  //BOOL OnInitDialog();
  void OnOK(wxCommandEvent &event);

  void OnTilesetBrowse(wxCommandEvent &event);

private:
  int         m_MapWidth;
  int         m_MapHeight;
  std::string m_Tileset;

  wxSpinCtrl *m_WidthCtrl;
  wxSpinCtrl *m_HeightCtrl;
  wxTextCtrl *m_TilesetCtrl;


private:
//  DECLARE_CLASS(wNewMapDialog)
  DECLARE_EVENT_TABLE()
};


#endif
