#ifndef SPRITE_PROPERTIES_DIALOG_HPP
#define SPRITE_PROPERTIES_DIALOG_HPP


#include <afxwin.h>


class sSpriteset;  // #include "../common/Spriteset.hpp"


class CSpritePropertiesDialog : public CDialog
{
public:
  CSpritePropertiesDialog(sSpriteset* spriteset, int direction, int frame);
  ~CSpritePropertiesDialog();

  BOOL OnInitDialog();
  void OnOK();

private:
  sSpriteset* m_Spriteset;
  int         m_Direction;
  int         m_Frame;
};


#endif
