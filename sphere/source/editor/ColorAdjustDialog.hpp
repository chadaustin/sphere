#ifndef COLOR_ADJUST_DIALOG_HPP
#define COLOR_ADJUST_DIALOG_HPP


#include <afxwin.h>
#include <string>
#include <vector>

#include "../common/rgb.hpp"

///////////////////////////////////////////////////////////

class CColorAdjustDialog : public CDialog
{
public:
  CColorAdjustDialog(const int width, const int height, const RGBA* pixels);
  ~CColorAdjustDialog();

  int ShouldUseRedChannel();
  int ShouldUseGreenChannel();
  int ShouldUseBlueChannel();
  int ShouldUseAlphaChannel();

private:
  const int m_Width;
  const int m_Height;
  const RGBA* m_Pixels;

private:
  void UpdateButtons();
  int m_UseRed, m_UseGreen, m_UseBlue, m_UseAlpha;

private:
  BOOL OnInitDialog();
  void OnOK();

  bool DrawPreview(CPaintDC& dc, RECT* rect);
  afx_msg void OnPaint();

  afx_msg void OnValueChanged();
  afx_msg void OnChannelChanged();

private:
  DECLARE_MESSAGE_MAP()
};

///////////////////////////////////////////////////////////

#endif
