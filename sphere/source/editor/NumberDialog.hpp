#ifndef NUMBER_DIALOG_HPP
#define NUMBER_DIALOG_HPP


#include <afxwin.h>
#include <string>


class CNumberDialog : public CDialog
{
public:
  CNumberDialog(const char* title, const char* text, int value, int min, int max);

  int GetValue() const;

private:
  afx_msg BOOL OnInitDialog();
  afx_msg void OnOK();

private:
  std::string m_Title;
  std::string m_Text;

  int m_Value;

  const int m_Min;
  const int m_Max;

  // dialog controls
  CStatic* m_Static;
  CEdit*   m_NumberEdit;
};


#endif
