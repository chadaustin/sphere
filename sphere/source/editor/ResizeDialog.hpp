#ifndef RESIZE_DIALOG_HPP
#define RESIZE_DIALOG_HPP


#include <string>
#include <afxwin.h>


class CResizeDialog : public CDialog
{
public:
  CResizeDialog(const char* caption, int default_width, int default_height);

  void SetRange(int min_width, int max_width, int min_height, int max_height);

  int GetWidth() const;
  int GetHeight() const;

private:
  virtual BOOL OnInitDialog();
  virtual void OnOK();

private:
  std::string m_Caption;
  int m_Width;
  int m_Height;

  int m_MinWidth;
  int m_MaxWidth;
  int m_MinHeight;
  int m_MaxHeight;
};


#endif