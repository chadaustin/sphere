#ifndef SPRITE_PROPERTIES_DIALOG_HPP
#define SPRITE_PROPERTIES_DIALOG_HPP


//#include <afxwin.h>
#include <wx/wx.h>
#include <wx/spinctrl.h>

class sSpriteset;  // #include "../common/Spriteset.hpp"


class wSpritePropertiesDialog : public wxDialog
{
public:
  wSpritePropertiesDialog(wxWindow *parent, sSpriteset* spriteset, int direction, int frame);
  ~wSpritePropertiesDialog();

  //bool OnInitDialog();
  void OnOK(wxCommandEvent &event);

private:
  sSpriteset* m_Spriteset;
  int         m_Direction;
  int         m_Frame;

  wxSpinCtrl *m_Delay;
private:
  DECLARE_EVENT_TABLE()
};


#endif
