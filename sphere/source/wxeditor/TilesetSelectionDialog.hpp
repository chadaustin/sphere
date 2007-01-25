#ifndef TILESET_SELECTION_DIALOG_HPP
#define TILESET_SELECTION_DIALOG_HPP


//#include <afxwin.h>
#include <wx/wx.h>


class wTilesetSelectionDialog : public wxDialog
{
public:
  wTilesetSelectionDialog(wxWindow *parent);
  ~wTilesetSelectionDialog();

  const char* GetTilesetPath() const;

private:
  //BOOL OnInitDialog();
  void OnOK(wxCommandEvent &event);

  void OnBrowseForTileset(wxCommandEvent &event);

private:
  wxString m_SelectedTileset;

  wxTextCtrl *m_TilesetCtrl;

private:
  DECLARE_EVENT_TABLE()
};


#endif
