#pragma warning(disable : 4786)

#include "ImageWindow.hpp"
#include "ResizeDialog.hpp"
#include "FileDialogs.hpp"
#include "RotateDialog.hpp"
#include "resource.h"

#include "../common/Filters.hpp"

#define ID_ALPHASLIDER 900

#ifdef USE_SIZECBAR
IMPLEMENT_DYNAMIC(CImageWindow, CMDIChildWnd)
#endif

BEGIN_MESSAGE_MAP(CImageWindow, CSaveableDocumentWindow)

  ON_WM_SIZE()  
  ON_WM_KEYDOWN()

  ON_COMMAND(ID_IMAGE_RESIZE,             OnImageResize)
  ON_COMMAND(ID_IMAGE_RESCALE,            OnImageRescale)
  ON_COMMAND(ID_IMAGE_RESAMPLE,           OnImageResample)
  ON_COMMAND(ID_IMAGE_ROTATE,             OnImageRotate)
  ON_COMMAND(ID_IMAGE_COUNTCOLORS,        OnCountColorsUsed)
  ON_COMMAND(ID_IMAGE_VIEWATORIGINALSIZE, OnImageViewOriginalSize)
  ON_UPDATE_COMMAND_UI(ID_IMAGE_VIEWATORIGINALSIZE, OnUpdateImageViewOriginalSizeCommand)

  ON_COMMAND(ID_FILE_PASTE, OnPaste)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CImageWindow::CImageWindow(const char* image, bool create_from_clipboard)
: CSaveableDocumentWindow(image, IDR_IMAGE)
, m_Created(false)
{
  SetSaved(image != NULL);
  SetModified(false);

  if (image) {
    if (!m_Image.Load(image)) {
      char string[MAX_PATH + 1024];
      sprintf (string, "Error: Could not load image\n'%s'", image);
      AfxGetApp()->m_pMainWnd->MessageBox(string);
      delete this;
      return;
    }
  } else {

    if (!m_Image.Create(16, 16)) {
      AfxGetApp()->m_pMainWnd->MessageBox("Error: Could not create image");
      delete this;
      return;
    }

    if (create_from_clipboard && m_ImageView.m_Clipboard != NULL) {
      if (AfxGetApp()->m_pMainWnd->OpenClipboard() != FALSE) {

        int width = 0, height = 0;
        RGBA* pixels = NULL;

        if (pixels == NULL && m_ImageView.m_Clipboard->IsFlatImageOnClipbard()) {
          pixels = m_ImageView.m_Clipboard->GetFlatImageFromClipboard(width, height);
        }

        if (pixels == NULL && m_ImageView.m_Clipboard->IsBitmapImageOnClipboard()) {
          pixels = m_ImageView.m_Clipboard->GetBitmapImageFromClipboard(width, height);
        }

        CloseClipboard();

        if (width > 0 && height > 0 && pixels != NULL) {
          if (!m_Image.Create(width, height)) {
            delete[] pixels;
            pixels = NULL;
            AfxGetApp()->m_pMainWnd->MessageBox("Error: Could not create image");
            delete this;
            return;
          }

          for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
              m_Image.SetPixel(x, y, pixels[y * width + x]);
            }
          }
        }

        delete[] pixels;
        pixels = NULL;

      }
    }

  }

  m_DocumentType = WA_IMAGE;

  // create the window
  Create(AfxRegisterWndClass(0, NULL, (HBRUSH)(COLOR_WINDOW + 1), AfxGetApp()->LoadIcon(IDI_IMAGE)));

  m_ImageView.Create(this, this, this);
  m_PaletteView.Create(this, this);
  m_ColorView.Create(this, this);
  m_AlphaView.Create(this, this);

  m_Created = true;

#ifdef USE_SIZECBAR
	 LoadPaletteStates();
#endif

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
    m_Image.GetPixels(), false);
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
CImageWindow::OnKeyDown(UINT vk, UINT repeat, UINT flags)
{
  m_ImageView.OnKeyDown(vk, repeat, flags);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageWindow::OnImageResize()
{
  CResizeDialog dialog("Resize Image", m_Image.GetWidth(), m_Image.GetHeight());
  if (dialog.DoModal() == IDOK) {
    if (dialog.GetWidth() > 0 && dialog.GetHeight() > 0
     && (dialog.GetWidth() != m_Image.GetWidth() || dialog.GetHeight() != m_Image.GetHeight())) {
      m_ImageView.BeforeImageChanged();
      m_Image.Resize(dialog.GetWidth(), dialog.GetHeight());
      SetModified(true);
      UpdateImageView();
      m_ImageView.AfterImageChanged();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageWindow::OnImageRescale()
{
  CResizeDialog dialog("Rescale Image", m_Image.GetWidth(), m_Image.GetHeight());
  if (dialog.DoModal() == IDOK) {
    if ((dialog.GetWidth() > 0 && dialog.GetHeight() > 0)
     && (dialog.GetWidth() != m_Image.GetWidth() || dialog.GetHeight() != m_Image.GetHeight())) {
      m_ImageView.BeforeImageChanged();
      m_Image.Rescale(dialog.GetWidth(), dialog.GetHeight());
      SetModified(true);
      UpdateImageView();
      m_ImageView.AfterImageChanged();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageWindow::OnImageResample()
{
  CResizeDialog dialog("Resample Image", m_Image.GetWidth(), m_Image.GetHeight());
  if (dialog.DoModal() == IDOK) {
    if ((dialog.GetWidth() > 0 && dialog.GetHeight() > 0)
     && (dialog.GetWidth() != m_Image.GetWidth() || dialog.GetHeight() != m_Image.GetHeight())) {
      m_ImageView.BeforeImageChanged();
      m_Image.Resample(dialog.GetWidth(), dialog.GetHeight());
      SetModified(true);
      UpdateImageView();
      m_ImageView.AfterImageChanged();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageWindow::OnImageRotate()
{
  CRotateDialog dialog("Rotate Image", "Rotate (0-360)", 90.0);
  if (dialog.DoModal() == IDOK) {
    m_ImageView.BeforeImageChanged();
    m_Image.Rotate(dialog.GetValue(), true);
    SetModified(true);
    UpdateImageView();
    m_ImageView.AfterImageChanged();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageWindow::OnCountColorsUsed()
{
  char string[255] = {0};
  unsigned long num_colors = CountColorsUsed(m_Image.GetPixels(), m_Image.GetWidth(), m_Image.GetHeight(),
                                             0, 0, m_Image.GetWidth(), m_Image.GetHeight());
  sprintf (string, "%u", num_colors);
  MessageBox(string, "Num Colors Used");
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageWindow::OnImageViewOriginalSize()
{
  if (IsZoomed())
    return;

  // FIXME: use correct GetSystemMetrics calls.  :)  Better yet, AdjustWindowRect
  CSaveableDocumentWindow::SetWindowPos(&wndTop, 0, 0, m_Image.GetWidth() + 72 + 12, m_Image.GetHeight() + 31, SWP_NOMOVE | SWP_NOOWNERZORDER);
  OnSize(0, m_Image.GetWidth() + 72, m_Image.GetHeight());
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageWindow::OnUpdateImageViewOriginalSizeCommand(CCmdUI* cmdui)
{
  cmdui->Enable(IsZoomed() ? FALSE : TRUE);
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
  m_Image.Resize(m_ImageView.GetWidth(), m_ImageView.GetHeight());
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
 
void
CImageWindow::OnToolCommand(UINT id) {
  if (m_Created) {
    m_ImageView.OnToolChanged(id);
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CImageWindow::IsToolCommandAvailable(UINT id) {
  BOOL available = FALSE;
  if (m_Created) {
    available = m_ImageView.IsToolAvailable(id);
  }
  return available;
}

////////////////////////////////////////////////////////////////////////////////

void
CImageWindow::OnPaste() {
  if (m_Created) {
    m_ImageView.SendMessage(WM_COMMAND, MAKEWPARAM(ID_IMAGEVIEW_PASTE, 0), 0);
  }
}
