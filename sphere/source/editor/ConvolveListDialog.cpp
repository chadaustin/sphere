#include "ConvolveListDialog.hpp"
#include "resource.h"
#include <algorithm>

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
  // todo: load these from a file
  FilterInfo* a = new FilterInfo(3, 3);
  if (a->mask) {
    a->name = "Null filter";
    a->mask[4] = 1;
    m_FilterList.push_back(a);
  }

  FilterInfo* b = new FilterInfo(3, 3);
  if (b->mask) {
    b->name = "Invert filter";
    b->mask[4] = -1;
    m_FilterList.push_back(b);
  }

  FilterInfo* d = new FilterInfo(5, 5);
  if (d->mask) {
    d->name = "Blur Test";
    d->divisor = 25;
    for (int i = 0; i < 25; i++)
      d->mask[i] = 1;
    m_FilterList.push_back(d);
  }

}

////////////////////////////////////////////////////////////////////////////////

void
CConvolveListDialog::SaveFilterList() {

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
  return m_FilterList[m_CurrentFilter]->use_red;
}

////////////////////////////////////////////////////////////////////////////////

int
CConvolveListDialog::ShouldUseGreenChannel()
{
  return m_FilterList[m_CurrentFilter]->use_green;
}
////////////////////////////////////////////////////////////////////////////////

int
CConvolveListDialog::ShouldUseBlueChannel()
{
  return m_FilterList[m_CurrentFilter]->use_blue;
}
////////////////////////////////////////////////////////////////////////////////

int
CConvolveListDialog::ShouldUseAlphaChannel()
{
  return m_FilterList[m_CurrentFilter]->use_alpha;
}

////////////////////////////////////////////////////////////////////////////////

void
CConvolveListDialog::OnOK()
{
  if (m_CurrentFilter < 0 || m_CurrentFilter >= m_FilterList.size())
    CDialog::OnCancel();
  else {
    SaveFilterList();
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
