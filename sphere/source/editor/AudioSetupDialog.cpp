#include "AudioSetupDialog.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
#include <fmod.h>
#include "resource.h"


BEGIN_MESSAGE_MAP(CAudioSetupDialog, CDialog)
  ON_WM_VSCROLL()
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CAudioSetupDialog::CAudioSetupDialog()
: CDialog(IDD_AUDIOSETUP)
{
}

////////////////////////////////////////////////////////////////////////////////

CAudioSetupDialog::~CAudioSetupDialog()
{
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CAudioSetupDialog::OnInitDialog()
{
  m_pDriverCombo = (CComboBox *)GetDlgItem(IDC_AUDIO_DRIVERCOMBO);
  m_pMixerCombo  = (CComboBox *)GetDlgItem(IDC_AUDIO_MIXERCOMBO);
  m_pMasterVolumeText = (CStatic *)GetDlgItem(IDC_AUDIO_MASTERVOLUMETEXT);
  m_pMasterVolumeSlider = (CSliderCtrl *)GetDlgItem(IDC_AUDIO_MASTERVOLUMESLIDER);
  m_p44kHzRadio  = (CButton *)GetDlgItem(IDC_AUDIO_44KHZ);
  m_p22kHzRadio  = (CButton *)GetDlgItem(IDC_AUDIO_22KHZ);
  m_p11kHzRadio  = (CButton *)GetDlgItem(IDC_AUDIO_11KHZ);
  m_p8kHzRadio   = (CButton *)GetDlgItem(IDC_AUDIO_8KHZ);
  m_p16BitRadio  = (CButton *)GetDlgItem(IDC_AUDIO_16BIT);
  m_p8BitRadio   = (CButton *)GetDlgItem(IDC_AUDIO_8BIT);
  m_pStereoRadio = (CButton *)GetDlgItem(IDC_AUDIO_STEREO);
  m_pMonoRadio   = (CButton *)GetDlgItem(IDC_AUDIO_MONO);
  m_pWarningText = (CStatic *)GetDlgItem(IDC_AUDIO_WARNINGTEXT);
  m_pCancelButton = (CButton *)GetDlgItem(IDCANCEL);

  // adding and setting up the Driver Box;
  m_pDriverCombo->AddString("DirectSound");
  m_pDriverCombo->AddString("Windows Multimedia");
  m_pDriverCombo->SetCurSel(Configuration::Get(KEY_AUDIO_DRIVER));

  // adding and setting up the mixer
  m_pMixerCombo->AddString("Normal AutoDetect");
  m_pMixerCombo->AddString("AutoDetect for Quality");
  m_pMixerCombo->AddString("No Interpolate - Non MMX blend");
  m_pMixerCombo->AddString("No Interpolate - MMX & Pentium blend");
  m_pMixerCombo->AddString("No Interpolate - MMX&PPro/P2/P3 blend");
  m_pMixerCombo->AddString("Interpolate - Non MMX blend");
  m_pMixerCombo->AddString("Interpolate - MMX & Pentium blend");
  m_pMixerCombo->AddString("Interpolate - MMX & PPro/P2/P3 blend");
  m_pMixerCombo->SetCurSel(Configuration::Get(KEY_AUDIO_MIXER));

  m_pMasterVolumeSlider->SetRange(0, 255, false);
  m_pMasterVolumeSlider->SetPos(255 - Configuration::Get(KEY_AUDIO_MASTER_VOLUME));
  m_pMasterVolumeSlider->SetPageSize(16);
  OnVScroll(0, 0, NULL);
  
  // setting up the Bit Rate
  if (Configuration::Get(KEY_AUDIO_BITDEPTH) == 16)
    m_p16BitRadio->SetCheck(BST_CHECKED);
  else
    m_p8BitRadio->SetCheck(BST_CHECKED);

  // setting up the stereo output
  if (Configuration::Get(KEY_AUDIO_STEREO))
    m_pStereoRadio->SetCheck(BST_CHECKED);
  else
    m_pMonoRadio->SetCheck(BST_CHECKED);

  // setting up the audio quality
  switch (Configuration::Get(KEY_AUDIO_SAMPLE_RATE))
  {
    case 44100: m_p44kHzRadio->SetCheck(BST_CHECKED); break;
    case 22050: m_p22kHzRadio->SetCheck(BST_CHECKED); break;
    case 11025: m_p11kHzRadio->SetCheck(BST_CHECKED); break;
    case 8000:  m_p8kHzRadio->SetCheck(BST_CHECKED);  break;
  }

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CAudioSetupDialog::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
  char abc[80];
  int iTempVal;

  iTempVal = 255 - m_pMasterVolumeSlider->GetPos();
  iTempVal = (int)(iTempVal / 2.55);

  sprintf(abc, "SFX Volume    at      %i%%", iTempVal);
  m_pMasterVolumeText->SetWindowText(abc);
}

////////////////////////////////////////////////////////////////////////////////

void
CAudioSetupDialog::OnOK()
{
  // store the new settings into the config
  Configuration::Set(KEY_AUDIO_DRIVER, m_pDriverCombo->GetCurSel());
  Configuration::Set(KEY_AUDIO_MIXER,  m_pMixerCombo->GetCurSel());

  if (m_p44kHzRadio->GetCheck() == BST_CHECKED)
    Configuration::Set(KEY_AUDIO_SAMPLE_RATE, 44100);
  else if (m_p22kHzRadio->GetCheck() == BST_CHECKED)
    Configuration::Set(KEY_AUDIO_SAMPLE_RATE, 22050);
  else if (m_p11kHzRadio->GetCheck() == BST_CHECKED)
    Configuration::Set(KEY_AUDIO_SAMPLE_RATE, 11025);
  else // m_p8kHzRadio->GetCheck() == BST_CHECKED
    Configuration::Set(KEY_AUDIO_SAMPLE_RATE, 8000);

  if (m_pStereoRadio->GetCheck() == BST_CHECKED)
    Configuration::Set(KEY_AUDIO_STEREO, true);
  else // m_pMonoRadio->GetCheck() == 1
    Configuration::Set(KEY_AUDIO_STEREO, false);

  if (m_p16BitRadio->GetCheck() == BST_CHECKED)
    Configuration::Set(KEY_AUDIO_BITDEPTH, 16);
  else // m_p8BitRadio->GetCheck() == BST_CHECKED
    Configuration::Set(KEY_AUDIO_BITDEPTH, 8);

  Configuration::Set(KEY_AUDIO_MASTER_VOLUME, 255 - m_pMasterVolumeSlider->GetPos());
  CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////////
