#ifndef CONVOLVE_LIST_DIALOG_HPP
#define CONVOLVE_LIST_DIALOG_HPP


#include <afxwin.h>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////

class FilterInfo {
public:
  std::string name;
  double* mask;
  int mask_width;
  int mask_height;
  int offset;
  double divisor;
  int clamp;
  int wrap;
  int clamp_low;
  int clamp_high;

public:
  FilterInfo() {
    mask = NULL;
    mask_width = 0;
    mask_height = 0;
    offset = 0;
    divisor = 0;
    clamp = 0;
    wrap = 0;
    clamp_low = 0;
    clamp_high = 255;
  }

  FilterInfo(int width, int height) {
    offset = 0;
    divisor = 0;
    clamp = 0;
    wrap = 0;
    clamp_low = 0;
    clamp_high = 255;
    mask_width = width;
    mask_height = height;
    mask = new double[mask_width * mask_height];
    if (mask)
      memset(mask, 0, mask_width * mask_height * sizeof(double));
  }

  ~FilterInfo() {
    if (mask)
      delete[] mask;
    mask = NULL;
  }
};

///////////////////////////////////////////////////////////

class CConvolveListDialog : public CDialog
{
public:
  CConvolveListDialog();
  ~CConvolveListDialog();

  const double* GetMask();
  int GetMaskWidth();
  int GetMaskHeight();
  int GetOffset();
  double GetDivisor();
  int ShouldClamp();
  int ShouldWrap();
  int ShouldUseRedChannel();
  int ShouldUseGreenChannel();
  int ShouldUseBlueChannel();
  int ShouldUseAlphaChannel();
  int GetClampLow();
  int GetClampHigh();
  const char* GetConvolveType();

private:
  void SortFilters();
  void LoadFilterList();
  void SaveFilterList();

  int m_UseRed, m_UseGreen, m_UseBlue, m_UseAlpha;

private:
  BOOL OnInitDialog();
  void OnOK();

  afx_msg void OnFilterChanged();
  afx_msg void OnEditFilter();

private:
  int     m_CurrentFilter;
  std::vector<FilterInfo*> m_FilterList;

  DECLARE_MESSAGE_MAP()
};

///////////////////////////////////////////////////////////

class CConvolveListEditDialog : public CDialog
{
public:
  CConvolveListEditDialog(FilterInfo* filter_info);

private:
  FilterInfo* m_FilterInfo;
  
private:
  BOOL OnInitDialog();
  void OnOK();

  afx_msg void OnClampChanged();

  DECLARE_MESSAGE_MAP()
};

///////////////////////////////////////////////////////////
#endif
