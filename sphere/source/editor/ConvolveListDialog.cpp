#include "ConvolveListDialog.hpp"
#include "resource.h"
#include <algorithm>
//#include <fstream>

#include "../common/str_util.hpp"

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CConvolveListEditDialog, CDialog)
  ON_COMMAND(IDC_FILTER_LIST_EDIT_CLAMP, OnClampChanged)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

CConvolveListEditDialog::CConvolveListEditDialog(FilterInfo* filter_info)
: CDialog(IDD_CONVOLVE_LIST_EDIT)
{
  m_FilterInfo = filter_info;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CConvolveListEditDialog::OnClampChanged()
{
  int clamp = IsDlgButtonChecked(IDC_FILTER_LIST_EDIT_CLAMP) == BST_CHECKED ? 1 : 0;
  CEdit* NumberEdit = (CEdit*)GetDlgItem(IDC_FILTER_CLIP_LOW);
  NumberEdit->EnableWindow(clamp);

  NumberEdit = (CEdit*)GetDlgItem(IDC_FILTER_CLIP_HIGH);
  NumberEdit->EnableWindow(clamp);
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CConvolveListEditDialog::OnInitDialog()
{
  CEdit*   NumberEdit;

  int NumberEditList[25] =
   {IDC_FILTER_LIST_EDIT_1, IDC_FILTER_LIST_EDIT_2, IDC_FILTER_LIST_EDIT_3,
    IDC_FILTER_LIST_EDIT_4, IDC_FILTER_LIST_EDIT_5, IDC_FILTER_LIST_EDIT_6,
    IDC_FILTER_LIST_EDIT_7, IDC_FILTER_LIST_EDIT_8, IDC_FILTER_LIST_EDIT_9,
    IDC_FILTER_LIST_EDIT_10, IDC_FILTER_LIST_EDIT_11, IDC_FILTER_LIST_EDIT_12,
    IDC_FILTER_LIST_EDIT_13, IDC_FILTER_LIST_EDIT_14, IDC_FILTER_LIST_EDIT_15,
    IDC_FILTER_LIST_EDIT_16, IDC_FILTER_LIST_EDIT_17, IDC_FILTER_LIST_EDIT_18,
    IDC_FILTER_LIST_EDIT_19, IDC_FILTER_LIST_EDIT_20, IDC_FILTER_LIST_EDIT_21,
    IDC_FILTER_LIST_EDIT_22, IDC_FILTER_LIST_EDIT_23, IDC_FILTER_LIST_EDIT_24,
    IDC_FILTER_LIST_EDIT_25};

  for (int i = 0; i < 25; i++) {
    NumberEdit = (CEdit*)GetDlgItem(NumberEditList[i]);
    NumberEdit->SetWindowText("0");
  }

  int offsetx = 0;
  int offsety = 0;

  switch (m_FilterInfo->mask_width) {
    case 1: offsetx = 2; break;
    case 2: offsetx = 1; break;
    case 3: offsetx = 1; break;
    case 4: offsetx = 0; break;
    case 5: offsetx = 0; break;
  }

  switch (m_FilterInfo->mask_height) {
    case 1: offsety = 2; break;
    case 2: offsety = 1; break;
    case 3: offsety = 1; break;
    case 4: offsety = 0; break;
    case 5: offsety = 0; break;
  }

  for (int y = 0; y < m_FilterInfo->mask_height; y++) {
    for (int x = 0; x < m_FilterInfo->mask_width; x++) {
      char string[80];
      sprintf(string, "%0.2f", m_FilterInfo->mask[y * m_FilterInfo->mask_width + x]);
      NumberEdit = (CEdit*)GetDlgItem(NumberEditList[(y + offsety) * 5 + (x + offsetx)]);
      NumberEdit->SetWindowText(string);
    }
  }

  CheckDlgButton(IDC_FILTER_LIST_EDIT_CLAMP,   m_FilterInfo->clamp ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(IDC_FILTER_LIST_EDIT_WRAP,    m_FilterInfo->wrap  ? BST_CHECKED : BST_UNCHECKED);

  NumberEdit = (CEdit*)GetDlgItem(IDC_FILTER_CLIP_LOW);
  NumberEdit->SetWindowText("0");
  if (!m_FilterInfo->clamp) {
    NumberEdit->EnableWindow(false);
  }

  NumberEdit = (CEdit*)GetDlgItem(IDC_FILTER_CLIP_HIGH);
  NumberEdit->SetWindowText("255");
  if (!m_FilterInfo->clamp) {
    NumberEdit->EnableWindow(false);
  }

  NumberEdit = (CEdit*)GetDlgItem(IDC_FILTER_LIST_EDIT_NAME);
  NumberEdit->SetWindowText(m_FilterInfo->name.c_str());

  char string[80];
  sprintf(string, "%d", m_FilterInfo->offset);
  NumberEdit = (CEdit*)GetDlgItem(IDC_FILTER_LIST_EDIT_OFFSET);
  NumberEdit->SetWindowText(string);

  sprintf(string, "%.2f", m_FilterInfo->divisor);
  NumberEdit = (CEdit*)GetDlgItem(IDC_FILTER_LIST_EDIT_DIVISOR);
  NumberEdit->SetWindowText(string);

  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

void
CConvolveListEditDialog::OnOK()
{
  CEdit*   NumberEdit;

  int NumberEditList[25] =
   {IDC_FILTER_LIST_EDIT_1, IDC_FILTER_LIST_EDIT_2, IDC_FILTER_LIST_EDIT_3,
    IDC_FILTER_LIST_EDIT_4, IDC_FILTER_LIST_EDIT_5, IDC_FILTER_LIST_EDIT_6,
    IDC_FILTER_LIST_EDIT_7, IDC_FILTER_LIST_EDIT_8, IDC_FILTER_LIST_EDIT_9,
    IDC_FILTER_LIST_EDIT_10, IDC_FILTER_LIST_EDIT_11, IDC_FILTER_LIST_EDIT_12,
    IDC_FILTER_LIST_EDIT_13, IDC_FILTER_LIST_EDIT_14, IDC_FILTER_LIST_EDIT_15,
    IDC_FILTER_LIST_EDIT_16, IDC_FILTER_LIST_EDIT_17, IDC_FILTER_LIST_EDIT_18,
    IDC_FILTER_LIST_EDIT_19, IDC_FILTER_LIST_EDIT_20, IDC_FILTER_LIST_EDIT_21,
    IDC_FILTER_LIST_EDIT_22, IDC_FILTER_LIST_EDIT_23, IDC_FILTER_LIST_EDIT_24,
    IDC_FILTER_LIST_EDIT_25};

  double* mask = new double[5 * 5];
  if (!mask) {
    return;
  }

  int offsetx = 0;
  int offsety = 0;

  bool number_is_floating_point;
  bool number_is_percentage;

  // fill mask with values from dialog
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 5; x++) {
      CString string;

      NumberEdit = (CEdit*)GetDlgItem(NumberEditList[y * 5 + x]);
      NumberEdit->GetWindowText(string);

      mask[y * 5 + x] = atof(string);

      if (IsInvalidNumber(string, number_is_floating_point, number_is_percentage) || number_is_percentage) {
        MessageBox("Invalid number format.");
        delete[] mask;
        return;
      }
    }
  }

  // FIX: Work out offsetx/offsety by mask values
  switch (m_FilterInfo->mask_width) {
    case 1: offsetx = 2; break;
    case 2: offsetx = 1; break;
    case 3: offsetx = 1; break;
    case 4: offsetx = 0; break;
    case 5: offsetx = 0; break;
  }

  switch (m_FilterInfo->mask_height) {
    case 1: offsety = 2; break;
    case 2: offsety = 1; break;
    case 3: offsety = 1; break;
    case 4: offsety = 0; break;
    case 5: offsety = 0; break;
  }


  // fill in filter mask values
  for (int y = 0; y < m_FilterInfo->mask_height; y++) {
    for (int x = 0; x < m_FilterInfo->mask_width; x++) {
      m_FilterInfo->mask[y * m_FilterInfo->mask_width + x] = mask[(y + offsety) * 5 + (x + offsetx)];
    }
  }

  delete[] mask;

  m_FilterInfo->wrap  =  IsDlgButtonChecked(IDC_FILTER_LIST_EDIT_WRAP)  == BST_CHECKED ? 1 : 0;
  m_FilterInfo->clamp =  IsDlgButtonChecked(IDC_FILTER_LIST_EDIT_CLAMP) == BST_CHECKED ? 1 : 0;

  if (m_FilterInfo->clamp) {
    CString low_text; 
    NumberEdit = (CEdit*)GetDlgItem(IDC_FILTER_CLIP_LOW);
    NumberEdit->GetWindowText(low_text);
    if (IsInvalidNumber(low_text, number_is_floating_point, number_is_percentage) || number_is_floating_point || number_is_percentage) {
      MessageBox("Invalid number format");
      return;
    }

    CString high_text;
    NumberEdit = (CEdit*)GetDlgItem(IDC_FILTER_CLIP_HIGH);
    NumberEdit->GetWindowText(high_text);
    if (IsInvalidNumber(high_text, number_is_floating_point, number_is_percentage) || number_is_floating_point || number_is_percentage) {
      MessageBox("Invalid number format");
      return;
    }
  }

  // m_LowValue = atoi(low_text);
  // m_HighValue = atoi(high_text);

  CString offset_text;
  NumberEdit = (CEdit*)GetDlgItem(IDC_FILTER_LIST_EDIT_OFFSET);
  NumberEdit->GetWindowText(offset_text);
  if (IsInvalidNumber(offset_text, number_is_floating_point, number_is_percentage) || number_is_floating_point || number_is_percentage) {
    MessageBox("Invalid number format");
    return;
  }

  m_FilterInfo->offset = atoi(offset_text);

  CString divisor_text;
  NumberEdit = (CEdit*)GetDlgItem(IDC_FILTER_LIST_EDIT_DIVISOR);
  NumberEdit->GetWindowText(divisor_text);
  if (IsInvalidNumber(divisor_text, number_is_floating_point, number_is_percentage) || number_is_percentage) {
    MessageBox("Invalid number format");
    return;
  }

  m_FilterInfo->divisor = atoi(divisor_text);
  
  CString name_text;
  NumberEdit = (CEdit*)GetDlgItem(IDC_FILTER_LIST_EDIT_NAME);
  NumberEdit->GetWindowText(name_text);
  m_FilterInfo->name = name_text;

  CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CConvolveListDialog, CDialog)
  ON_CBN_SELCHANGE(IDC_FILTER_LIST, OnFilterChanged)
  ON_COMMAND(IDC_FILTER_LIST_EDIT, OnEditFilter)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

CConvolveListDialog::CConvolveListDialog()
: CDialog(IDD_CONVOLVE_LIST)
, m_CurrentFilter(-1)
{
}

////////////////////////////////////////////////////////////////////////////////

CConvolveListDialog::~CConvolveListDialog() {
  for (int i = 0; i < m_FilterList.size(); i++) {
    delete m_FilterList[i];
  }
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
      a->name = "Null_Filter";
      a->mask[4] = 1;
      m_FilterList.push_back(a);
    }

    FilterInfo* b = new FilterInfo(3, 3);
    if (b->mask) {
      b->name = "Invert_Filter";
      b->mask[4] = -1;
      m_FilterList.push_back(b);
    }

    FilterInfo* c = new FilterInfo(5, 5);
    if (c->mask) {
      c->name = "Blur_Filter";
      c->divisor = 25;
      for (int i = 0; i < 25; i++)
        c->mask[i] = 1;
      m_FilterList.push_back(c);
    }

    FilterInfo* d = new FilterInfo(3, 3);
    if (d->mask) {
      d->name = "Emboss_Filter";
      d->mask[0] = -2;
      d->mask[4] = 4;
      d->mask[8] = -2;
      d->offset = 128;
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
  return b->name > a->name;
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

  SendDlgItemMessage(IDC_FILTER_LIST, LB_SETCURSEL, m_CurrentFilter);
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CConvolveListDialog::OnInitDialog()
{
  LoadFilterList();
  SortFilters();

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

void
CConvolveListDialog::OnEditFilter()
{
  if (m_CurrentFilter >= 0 && m_CurrentFilter < m_FilterList.size()) {
    CConvolveListEditDialog dialog(m_FilterList[m_CurrentFilter]);
    if (dialog.DoModal() == IDOK) {
      SortFilters();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

