#include "SpritePropertiesDialog.hpp"
#include "../common/Spriteset.hpp"
#include "resource.h"


BEGIN_MESSAGE_MAP(CSpritePropertiesDialog, CDialog)
  
  ON_BN_CLICKED(IDC_NEXT,     OnNextTile)
  ON_BN_CLICKED(IDC_PREVIOUS, OnPrevTile)

END_MESSAGE_MAP()

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

void
CSpritePropertiesDialog::UpdateDialog()
{
  SetDlgItemInt(IDC_DELAY, m_Spriteset->GetFrameDelay(m_Direction, m_Frame));

  CString title;
  title.Format("Frame Properties - %d/%d", m_Frame, m_Spriteset->GetNumFrames(m_Direction));
  SetWindowText(title);
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CSpritePropertiesDialog::OnInitDialog()
{
  UpdateDialog();

  CEdit* edit = (CEdit*)GetDlgItem(IDC_DELAY);
  edit->SetSel(0, -1);
  edit->SetFocus();

  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritePropertiesDialog::OnNextTile() {
  int delay = GetDlgItemInt(IDC_DELAY);
  m_Spriteset->SetFrameDelay(m_Direction, m_Frame, delay);

  m_Frame += 1;
  if (m_Frame >= m_Spriteset->GetNumFrames(m_Direction))
    m_Frame = 0;

  UpdateDialog();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritePropertiesDialog::OnPrevTile() {
  int delay = GetDlgItemInt(IDC_DELAY);
  m_Spriteset->SetFrameDelay(m_Direction, m_Frame, delay);

  m_Frame -= 1;
  if (m_Frame < 0)
    m_Frame = m_Spriteset->GetNumFrames(m_Direction) - 1;

  UpdateDialog();
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
