#include "ConvolveListDialog.hpp"
#include "resource.h"
#include <algorithm>
//#include <fstream>

BEGIN_MESSAGE_MAP(CConvolveListDialog, CDialog)
  ON_CBN_SELCHANGE(IDC_FILTER_LIST, OnFilterChanged)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

CConvolveListDialog::CConvolveListDialog()
: CDialog(IDD_CONVOLVE_LIST)
, m_CurrentFilter(-1)
{
}

////////////////////////////////////////////////////////////////////////////////

CConvolveListDialog::~CConvolveListDialog() {
  m_FilterList.clear();
}

////////////////////////////////////////////////////////////////////////////////

void
CConvolveListDialog::LoadFilterList() {
/*
  std::ifstream file("FilterList.dat");
  if (file) {
    while (file.eof()) {
      int clamp;
      double divisor;
      int offset;
      int wrap;
      int mask_width;
      int mask_height;
      file >> clamp;
      file >> divisor;
      file >> offset;
      file >> wrap;
      file >> mask_width;
      file >> mask_height;
      
     if (mask_width >= 1 && mask_width < 9
      && mask_height >= 1 && mask_height < 9) {
      FilterInfo* flt = new FilterInfo(mask_width, mask_height);
      if (flt) {
        flt->clamp = clamp;
        flt->divisor = divisor;
        flt->offset = offset;
        flt->wrap = wrap;
        for (int i = 0; i < flt->mask_width * flt->mask_height; i++)
          file >> flt->mask[i];
        file >> flt->name;

        m_FilterList.push_back(flt);

        file >> flt->name; // chomp the endline character
      }
     }
    }
  }
  file.close();
*/

  if (m_FilterList.size() == 0) {
    FilterInfo* a = new FilterInfo(3, 3);
    if (a->mask) {
      a->name = "Null_filter";
      a->mask[4] = 1;
      m_FilterList.push_back(a);
    }

    FilterInfo* b = new FilterInfo(3, 3);
    if (b->mask) {
      b->name = "Invert_filter";
      b->mask[4] = -1;
      m_FilterList.push_back(b);
    }

    FilterInfo* d = new FilterInfo(5, 5);
    if (d->mask) {
      d->name = "Blur_Test";
      d->divisor = 25;
      for (int i = 0; i < 25; i++)
        d->mask[i] = 1;
      m_FilterList.push_back(d);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CConvolveListDialog::SaveFilterList() {
/*
  std::ofstream file("FilterList.dat");
  if (file == NULL)
    return;

  for (int i = 0; i < m_FilterList.size(); i++) {
    FilterInfo* flt = m_FilterList[i];
    if (flt) {
      file << flt->clamp << " ";
      file << flt->divisor << " ";
      file << flt->offset << " ";
      file << flt->wrap << " ";
      file << flt->mask_width << " ";
      file << flt->mask_height << " ";
      for (int i = 0; i < flt->mask_width * flt->mask_height; i++)
        file << flt->mask[i] << " ";
      file << flt->name << "\n";
    }
  }
  
  file.close();
*/
}

////////////////////////////////////////////////////////////////////////////////

int FilterCompare(const void* x, const void* y) {
  FilterInfo* a = (FilterInfo*) x;
  FilterInfo* b = (FilterInfo*) y;
  return strcmp(b->name.c_str(), a->name.c_str());
}

////////////////////////////////////////////////////////////////////////////////

void
CConvolveListDialog::SortFilters()
{
  // delete all the list items
  for (int i = m_FilterList.size() - 1; i >= 0; i--) {
    SendDlgItemMessage(IDC_FILTER_LIST, LB_DELETESTRING, i, 0);
  }

  std::sort(m_FilterList.begin(), m_FilterList.end(), FilterCompare);

  // add them again
  for (unsigned int i = 0; i < m_FilterList.size(); i++) {
    SendDlgItemMessage(IDC_FILTER_LIST, LB_ADDSTRING, 0, (LPARAM)m_FilterList[i]->name.c_str());
  }

}

////////////////////////////////////////////////////////////////////////////////

BOOL
CConvolveListDialog::OnInitDialog()
{
  LoadFilterList();

  SortFilters();
  SendDlgItemMessage(IDC_FILTER_LIST, LB_SETCURSEL, m_CurrentFilter);

  CheckDlgButton(IDC_FILTER_USE_RED,   BST_CHECKED);
  CheckDlgButton(IDC_FILTER_USE_GREEN, BST_CHECKED);
  CheckDlgButton(IDC_FILTER_USE_BLUE,  BST_CHECKED);
  CheckDlgButton(IDC_FILTER_USE_ALPHA, BST_UNCHECKED);

  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

const double*
CConvolveListDialog::GetMask()
{
  return m_FilterList[m_CurrentFilter]->mask;
}

////////////////////////////////////////////////////////////////////////////////

int
CConvolveListDialog::GetMaskWidth()
{
  return m_FilterList[m_CurrentFilter]->mask_width;
}

int
CConvolveListDialog::GetMaskHeight()
{
  return m_FilterList[m_CurrentFilter]->mask_height;
}

////////////////////////////////////////////////////////////////////////////////

int
CConvolveListDialog::GetOffset()
{
  return m_FilterList[m_CurrentFilter]->offset;
}

////////////////////////////////////////////////////////////////////////////////

double
CConvolveListDialog::GetDivisor()
{
  return m_FilterList[m_CurrentFilter]->divisor;
}

////////////////////////////////////////////////////////////////////////////////

int
CConvolveListDialog::ShouldClamp()
{
  return m_FilterList[m_CurrentFilter]->clamp;
}

////////////////////////////////////////////////////////////////////////////////

int
CConvolveListDialog::ShouldWrap()
{
  return m_FilterList[m_CurrentFilter]->wrap;
}

////////////////////////////////////////////////////////////////////////////////

int
CConvolveListDialog::ShouldUseRedChannel()
{
  return m_UseRed;
}

////////////////////////////////////////////////////////////////////////////////

int
CConvolveListDialog::ShouldUseGreenChannel()
{
  return m_UseGreen;
}
////////////////////////////////////////////////////////////////////////////////

int
CConvolveListDialog::ShouldUseBlueChannel()
{
  return m_UseBlue;
}
////////////////////////////////////////////////////////////////////////////////

int
CConvolveListDialog::ShouldUseAlphaChannel()
{
  return m_UseAlpha;
}

////////////////////////////////////////////////////////////////////////////////

void
CConvolveListDialog::OnOK()
{
  if (m_CurrentFilter < 0 || m_CurrentFilter >= m_FilterList.size())
    CDialog::OnCancel();
  else {
    SaveFilterList();
    m_UseRed   =  IsDlgButtonChecked(IDC_FILTER_USE_RED)   == BST_CHECKED ? 1 : 0;
    m_UseGreen =  IsDlgButtonChecked(IDC_FILTER_USE_BLUE)  == BST_CHECKED ? 1 : 0;
    m_UseBlue  =  IsDlgButtonChecked(IDC_FILTER_USE_GREEN) == BST_CHECKED ? 1 : 0;
    m_UseAlpha =  IsDlgButtonChecked(IDC_FILTER_USE_ALPHA) == BST_CHECKED ? 1 : 0;
    CDialog::OnOK();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CConvolveListDialog::OnFilterChanged()
{
  m_CurrentFilter = SendDlgItemMessage(IDC_FILTER_LIST, LB_GETCURSEL);
}

////////////////////////////////////////////////////////////////////////////////
