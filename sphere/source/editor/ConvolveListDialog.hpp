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

public:
  FilterInfo() {
    mask = NULL;
    mask_width = 0;
    mask_height = 0;
    offset = 0;
    divisor = 0;
    clamp = 0;
    wrap = 0;
  }

  FilterInfo(int width, int height) {
    offset = 0;
    divisor = 0;
    clamp = 0;
    wrap = 0;
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

private:
  void SortFilters();
  void LoadFilterList();
  void SaveFilterList();

private:
  BOOL OnInitDialog();
  void OnOK();

  afx_msg void OnFilterChanged();

private:
  int     m_CurrentFilter;
  std::vector<FilterInfo*> m_FilterList;

  DECLARE_MESSAGE_MAP()
};

///////////////////////////////////////////////////////////


#endif
