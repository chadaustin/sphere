#include "SpritePropertiesDialog.hpp"
#include "../common/Spriteset.hpp"
#include "resource.h"


////////////////////////////////////////////////////////////////////////////////

CSpritePropertiesDialog::CSpritePropertiesDialog(sSpriteset* spriteset, int direction, int frame)
: CDialog(IDD_SPRITE_PROPERTIES)
, m_Spriteset(spriteset)
, m_Direction(direction)
, m_Frame(frame)
{
}

////////////////////////////////////////////////////////////////////////////////

CSpritePropertiesDialog::~CSpritePropertiesDialog()
{
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CSpritePropertiesDialog::OnInitDialog()
{
  SetDlgItemInt(IDC_DELAY, m_Spriteset->GetFrameDelay(m_Direction, m_Frame));

  CEdit* edit = (CEdit*)GetDlgItem(IDC_DELAY);
  edit->SetSel(0, -1);
  edit->SetFocus();

  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritePropertiesDialog::OnOK()
{
  int delay = GetDlgItemInt(IDC_DELAY);
  m_Spriteset->SetFrameDelay(m_Direction, m_Frame, delay);
  CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////////
