#pragma warning(disable : 4786)

#include "ImageWindow.hpp"
#include "ResizeDialog.hpp"
#include "FileDialogs.hpp"
#include "resource.h"


#define ID_ALPHASLIDER 900


BEGIN_MESSAGE_MAP(CImageWindow, CSaveableDocumentWindow)

  ON_WM_SIZE()

  ON_COMMAND(ID_IMAGE_RESIZE,             OnImageResize)
  ON_COMMAND(ID_IMAGE_RESCALE,            OnImageRescale)
  ON_COMMAND(ID_IMAGE_VIEWATORIGINALSIZE, OnImageViewOriginalSize)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CImageWindow::CImageWindow(const char* image)
: CSaveableDocumentWindow(image, IDR_IMAGE)
, m_Created(false)
{
  SetSaved(image != NULL);
  SetModified(false);

  if (image) {
    if (!m_Image.Load(image)) {
      AfxGetApp()->m_pMainWnd->MessageBox("Error: Could not load image");
      delete this;
      return;
    }
  } else {
    m_Image.Create(16, 16);
  }

  // create the window
  Create(AfxRegisterWndClass(0, NULL, (HBRUSH)(COLOR_WINDOW + 1), AfxGetApp()->LoadIcon(IDI_IMAGE)));

  m_ImageView.Create(this, this, this);
  m_PaletteView.Create(this, this);
  m_ColorView.Create(this, this);
  m_AlphaView.Create(this, this);

  m_Created = true;

  // make sure everything is in the right position
  RECT ClientRect;
  GetClientRect(&ClientRect);
  OnSize(0, ClientRect.right, ClientRect.bottom);

  UpdateImageView();
}

////////////////////////////////////////////////////////////////////////////////

CImageWindow::~CImageWindow()
{
  m_ImageView.DestroyWindow();
  m_PaletteView.DestroyWindow();
  m_ColorView.DestroyWindow();
  m_AlphaView.DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////

void
CImageWindow::UpdateImageView()
{
  m_ImageView.SetImage(
    m_Image.GetWidth(),
    m_Image.GetHeight(),
    m_Image.GetPixels());
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageWindow::OnSize(UINT type, int cx, int cy)
{
  const int palette_width = 40;
  const int color_height  = 40;
  const int alpha_width   = 32;

  if (m_Created)
  {
    m_ImageView.MoveWindow(0, 0, cx - palette_width - alpha_width, cy, FALSE);
    m_PaletteView.MoveWindow(cx - palette_width - alpha_width, 0, palette_width, cy - color_height, FALSE);
    m_ColorView.MoveWindow(cx - palette_width - alpha_width, cy - color_height, palette_width, color_height, FALSE);
    m_AlphaView.MoveWindow(cx - alpha_width, 0, alpha_width, cy, FALSE);

    m_ImageView.Invalidate();
    m_PaletteView.Invalidate();
    m_ColorView.Invalidate();
    m_AlphaView.Invalidate();
  }

  CSaveableDocumentWindow::OnSize(type, cx, cy);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageWindow::OnImageResize()
{
  CResizeDialog dialog("Resize Image", m_Image.GetWidth(), m_Image.GetHeight());
  if (dialog.DoModal() == IDOK) {
    m_Image.Resize(dialog.GetWidth(), dialog.GetHeight());
    SetModified(true);
    UpdateImageView();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageWindow::OnImageRescale()
{
  CResizeDialog dialog("Rescale Image", m_Image.GetWidth(), m_Image.GetHeight());
  if (dialog.DoModal() == IDOK) {
    m_Image.Rescale(dialog.GetWidth(), dialog.GetHeight());
    SetModified(true);
    UpdateImageView();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageWindow::OnImageViewOriginalSize()
{
  // FIXME: use correct GetSystemMetrics calls.  :)  Better yet, AdjustWindowRect
  CSaveableDocumentWindow::SetWindowPos(&wndTop, 0, 0, m_Image.GetWidth() + 72 + 12, m_Image.GetHeight() + 31, SWP_NOMOVE | SWP_NOOWNERZORDER);
  OnSize(0, m_Image.GetWidth() + 72, m_Image.GetHeight());
}

////////////////////////////////////////////////////////////////////////////////

bool
CImageWindow::GetSavePath(char* path)
{
  CImageFileDialog Dialog(FDM_SAVE);
  if (Dialog.DoModal() != IDOK)
    return false;

  strcpy(path, Dialog.GetPathName());
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CImageWindow::SaveDocument(const char* path)
{
  return m_Image.Save(path);
}

////////////////////////////////////////////////////////////////////////////////

void
CImageWindow::IV_ImageChanged()
{
  memcpy(m_Image.GetPixels(), m_ImageView.GetPixels(), m_Image.GetWidth() * m_Image.GetHeight() * sizeof(RGBA));
  SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////

void
CImageWindow::IV_ColorChanged(RGBA color)
{
  RGB rgb = { color.red, color.green, color.blue };
  m_ColorView.SetColor(rgb);
  m_AlphaView.SetAlpha(color.alpha);
}

////////////////////////////////////////////////////////////////////////////////

void
CImageWindow::PV_ColorChanged(RGB color)
{
  m_ColorView.SetColor(color);

  RGBA rgba = { color.red, color.green, color.blue, m_ImageView.GetColor().alpha };
  m_ImageView.SetColor(rgba);
}

////////////////////////////////////////////////////////////////////////////////

void
CImageWindow::CV_ColorChanged(RGB color)
{
  RGBA rgba = { color.red, color.green, color.blue, m_ImageView.GetColor().alpha };
  m_ImageView.SetColor(rgba);
}

////////////////////////////////////////////////////////////////////////////////

void
CImageWindow::AV_AlphaChanged(byte alpha)
{
  RGBA rgba = m_ImageView.GetColor();
  rgba.alpha = alpha;
  m_ImageView.SetColor(rgba);
}

////////////////////////////////////////////////////////////////////////////////
