#ifndef AUDIO_SETUP_DIALOG_HPP
#define AUDIO_SETUP_DIALOG_HPP


#include <afxwin.h>
#include <afxcmn.h>


class CAudioSetupDialog : public CDialog
{
public:
  CAudioSetupDialog();
  ~CAudioSetupDialog();

private:
  BOOL OnInitDialog();
  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
  void OnOK();

private:
  CSliderCtrl* m_pMasterVolumeSlider;
  CStatic*     m_pMasterVolumeText;
  CComboBox*   m_pDriverCombo;
  CComboBox*   m_pMixerCombo;
  CButton*     m_p44kHzRadio;
  CButton*     m_p22kHzRadio;
  CButton*     m_p11kHzRadio;
  CButton*     m_p8kHzRadio;
  CButton*     m_pStereoRadio;
  CButton*     m_pMonoRadio;
  CButton*     m_p8BitRadio;
  CButton*     m_p16BitRadio;
  CStatic*     m_pWarningText;
  CButton*     m_pCancelButton;

  DECLARE_MESSAGE_MAP()
};


#endif
