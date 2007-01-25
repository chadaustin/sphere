#ifndef OPTIONS_DIALOG_HPP
#define OPTIONS_DIALOG_HPP


//#include <afxwin.h>


class COptionsDialog : public CDialog
{
public:
  COptionsDialog();

private:
  void RegisterFileType(const char* extension, const char* filetype);

  afx_msg BOOL OnInitDialog();
  afx_msg void OnOK();

  afx_msg void OnRegisterMap();
  afx_msg void OnRegisterSpriteset();
  afx_msg void OnRegisterWindowStyle();
  afx_msg void OnRegisterFont();

  DECLARE_MESSAGE_MAP()
};


#endif
