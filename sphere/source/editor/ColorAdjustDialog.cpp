#include "ColorAdjustDialog.hpp"
#include "resource.h"

#include "../common/str_util.hpp"

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CColorAdjustDialog, CDialog)
  ON_BN_CLICKED(IDC_USE_RED,   OnChannelChanged)
  ON_BN_CLICKED(IDC_USE_GREEN, OnChannelChanged)
  ON_BN_CLICKED(IDC_USE_BLUE,  OnChannelChanged)
  ON_BN_CLICKED(IDC_USE_ALPHA, OnChannelChanged)
  ON_WM_PAINT()
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

CColorAdjustDialog::CColorAdjustDialog(const int width, const int height, const RGBA* pixels)
: CDialog(IDD_COLOR_ADJUST)
, m_Width(width)
, m_Height(height)
, m_Pixels(pixels)
{
}

////////////////////////////////////////////////////////////////////////////////

CColorAdjustDialog::~CColorAdjustDialog()
{
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CColorAdjustDialog::OnInitDialog()
{
  UpdateButtons();

  CheckDlgButton(IDC_USE_RED,   BST_CHECKED);
  CheckDlgButton(IDC_USE_GREEN, BST_CHECKED);
  CheckDlgButton(IDC_USE_BLUE,  BST_CHECKED);
  CheckDlgButton(IDC_USE_ALPHA, BST_UNCHECKED);
  
  OnChannelChanged();
  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

#include "DIBSection.hpp"

bool
CColorAdjustDialog::DrawPreview(CPaintDC& dc, RECT* rect)
{
  CDIBSection* blit_tile = new CDIBSection(32, 32, 32);
  if (!blit_tile || blit_tile->GetPixels() == NULL)
  {
    return false;
  }

  /////////////////////////////////////////////////////////

  int width  = m_Width;
  int height = m_Height;
  if (width > 32)
    width = 32;
  if (height > 32)
    height = 32;

  RGBA* pixels = (RGBA*) blit_tile->GetPixels();

  for (int iy = 0; iy < height; iy++) {
    for (int ix = 0; ix < width; ix++) {
      pixels[iy * 32 + ix].red   = m_Pixels[iy * m_Width + ix].red;
      pixels[iy * 32 + ix].green = m_Pixels[iy * m_Width + ix].green;
      pixels[iy * 32 + ix].blue  = m_Pixels[iy * m_Width + ix].blue;
      pixels[iy * 32 + ix].alpha = m_Pixels[iy * m_Width + ix].alpha;
    }
  }

  /////////////////////////////////////////////////////////

  int red_value = 0;
  int green_value = 0;
  int blue_value = 0;
  int alpha_value = 0;

  int use_red   = ShouldUseRedChannel();
  int use_green = ShouldUseGreenChannel();
  int use_blue  = ShouldUseBlueChannel();
  int use_alpha = ShouldUseAlphaChannel();

  for (int iy = 0; iy < height; iy++) {
    for (int ix = 0; ix < width; ix++) {
      if (use_red)   pixels[iy * 32 + ix].red   += red_value;
      if (use_green) pixels[iy * 32 + ix].green += green_value;
      if (use_blue)  pixels[iy * 32 + ix].blue  += blue_value;
      if (use_alpha) pixels[iy * 32 + ix].alpha += alpha_value;
    }
  } 

  /////////////////////////////////////////////////////////

  dc.BitBlt(rect->left, rect->top, width, height, CDC::FromHandle(blit_tile->GetDC()), 0, 0, SRCCOPY);

  delete blit_tile;
  blit_tile = NULL;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
CColorAdjustDialog::OnPaint()
{
	CPaintDC dc(this);

  RECT rect;
  GetDlgItem(IDC_PREVIEW_FRAME)->GetWindowRect(&rect);
  ScreenToClient(&rect);

  if (DrawPreview(dc, &rect) == false) {    
    FillRect(dc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
  }
}

////////////////////////////////////////////////////////////////////////////////

int
CColorAdjustDialog::ShouldUseRedChannel()
{
  return m_UseRed;
}

////////////////////////////////////////////////////////////////////////////////

int
CColorAdjustDialog::ShouldUseGreenChannel()
{
  return m_UseGreen;
}
////////////////////////////////////////////////////////////////////////////////

int
CColorAdjustDialog::ShouldUseBlueChannel()
{
  return m_UseBlue;
}
////////////////////////////////////////////////////////////////////////////////

int
CColorAdjustDialog::ShouldUseAlphaChannel()
{
  return m_UseAlpha;
}

////////////////////////////////////////////////////////////////////////////////

void
CColorAdjustDialog::OnOK()
{
  CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////////

void
CColorAdjustDialog::UpdateButtons()
{

}

////////////////////////////////////////////////////////////////////////////////

void
CColorAdjustDialog::OnValueChanged()
{
  UpdateButtons();
  Invalidate(FALSE);
}

////////////////////////////////////////////////////////////////////////////////

void
CColorAdjustDialog::OnChannelChanged()
{
  m_UseRed   =  IsDlgButtonChecked(IDC_USE_RED)   == BST_CHECKED ? 1 : 0;
  m_UseGreen =  IsDlgButtonChecked(IDC_USE_GREEN) == BST_CHECKED ? 1 : 0;
  m_UseBlue  =  IsDlgButtonChecked(IDC_USE_BLUE)  == BST_CHECKED ? 1 : 0;
  m_UseAlpha =  IsDlgButtonChecked(IDC_USE_ALPHA) == BST_CHECKED ? 1 : 0;
  Invalidate(FALSE);
}

////////////////////////////////////////////////////////////////////////////////

