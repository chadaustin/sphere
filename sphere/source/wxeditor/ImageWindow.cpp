#pragma warning(disable : 4786)

#include "ImageWindow.hpp"
#include "ResizeDialog.hpp"
#include "FileDialogs.hpp"
//#include "resource.h"
#include "IDs.hpp"


//#define ID_ALPHASLIDER 900

/*
BEGIN_MESSAGE_MAP(CImageWindow, CSaveableDocumentWindow)

  ON_WM_SIZE()

  ON_COMMAND(ID_IMAGE_RESIZE,             OnImageResize)
  ON_COMMAND(ID_IMAGE_RESCALE,            OnImageRescale)
  ON_COMMAND(ID_IMAGE_VIEWATORIGINALSIZE, OnImageViewOriginalSize)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wImageWindow, wSaveableDocumentWindow)
  EVT_SIZE(OnSize)

  EVT_MENU(wID_IMAGE_RESIZE,            OnImageResize)
  EVT_MENU(wID_IMAGE_RESCALE,           OnImageRescale)
  EVT_MENU(wID_IMAGE_VIEWORIGINALSIZE,  OnImageViewOriginalSize)
END_EVENT_TABLE()


////////////////////////////////////////////////////////////////////////////////

wImageWindow::wImageWindow(const char* image)
: wSaveableDocumentWindow(image, wID_IMAGE_base /*todo:IDR_IMAGE*/)
, m_Created(false)
{
  SetSaved(image != NULL);
  SetModified(false);

  if (image) {
    if (!m_Image.Load(image)) {
      ::wxMessageBox("Error: Could not load image");
      Destroy();
      return;
    }
  } else {
    m_Image.Create(16, 16);
  }

  // create the window
  //Create(AfxRegisterWndClass(0, NULL, (HBRUSH)(COLOR_WINDOW + 1), AfxGetApp()->LoadIcon(IDI_IMAGE)));

  m_ImageView = new wImageView(this, this, this);
  m_PaletteView = new wPaletteView(this, this);
  m_ColorView = new wColorView(this, this);
  m_AlphaView = new wAlphaView(this, this);

  m_Created = true;
/*todo:
  // make sure everything is in the right position
  wxSize ClientSize(GetClientSize());
  OnSize(0, ClientRect.right, ClientRect.bottom);
*/
  UpdateImageView();
}

////////////////////////////////////////////////////////////////////////////////

wImageWindow::~wImageWindow()
{
/*
  delete m_ImageView;//.DestroyWindow();
  delete m_PaletteView;//.DestroyWindow();
  delete m_ColorView;//.DestroyWindow();
  delete m_AlphaView;//.DestroyWindow();
*/
}

////////////////////////////////////////////////////////////////////////////////

void
wImageWindow::UpdateImageView()
{
  m_ImageView->SetImage(
    m_Image.GetWidth(),
    m_Image.GetHeight(),
    m_Image.GetPixels());
}

////////////////////////////////////////////////////////////////////////////////

void
wImageWindow::OnSize(wxSizeEvent &event)
{
  int cx = GetClientSize().GetWidth();
  int cy = GetClientSize().GetHeight();
  const int palette_width = 40;
  const int color_height  = 40;
  const int alpha_width   = 32;

  if (m_Created)
  {
    m_ImageView->SetSize(0, 0, cx - palette_width - alpha_width, cy, FALSE);
    m_PaletteView->SetSize(cx - palette_width - alpha_width, 0, palette_width, cy - color_height, FALSE);
    m_ColorView->SetSize(cx - palette_width - alpha_width, cy - color_height, palette_width, color_height, FALSE);
    m_AlphaView->SetSize(cx - alpha_width, 0, alpha_width, cy, FALSE);

    m_ImageView->Refresh();
    m_PaletteView->Refresh();
    m_ColorView->Refresh();
    m_AlphaView->Refresh();
  }

  wSaveableDocumentWindow::OnSize(event);
}

////////////////////////////////////////////////////////////////////////////////

void
wImageWindow::OnImageResize(wxCommandEvent &event)
{
  wResizeDialog dialog(this, "Resize Image", m_Image.GetWidth(), m_Image.GetHeight());
  if (dialog.ShowModal() == wxID_OK) {
    m_Image.Resize(dialog.GetWidth(), dialog.GetHeight());
    SetModified(true);
    UpdateImageView();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wImageWindow::OnImageRescale(wxCommandEvent &event)
{
  wResizeDialog dialog(this, "Rescale Image", m_Image.GetWidth(), m_Image.GetHeight());
  if (dialog.ShowModal() == wxID_OK) {
    m_Image.Rescale(dialog.GetWidth(), dialog.GetHeight());
    SetModified(true);
    UpdateImageView();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wImageWindow::OnImageViewOriginalSize(wxCommandEvent &event)
{
  // FIXME: use correct GetSystemMetrics calls.  :)  Better yet, AdjustWindowRect
  wSaveableDocumentWindow::SetSize(/*WindowPos(&wndTop,*/ 0, 0, m_Image.GetWidth() + 72 + 12, m_Image.GetHeight() + 31);//, SWP_NOMOVE | SWP_NOOWNERZORDER);
  //OnSize(0, m_Image.GetWidth() + 72, m_Image.GetHeight());
}

////////////////////////////////////////////////////////////////////////////////

bool
wImageWindow::GetSavePath(char* path)
{
  wImageFileDialog Dialog(this, FDM_SAVE);
  if (Dialog.ShowModal() != wxID_OK)
    return false;

  strcpy(path, Dialog.GetPath());
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
wImageWindow::SaveDocument(const char* path)
{
  return m_Image.Save(path);
}

////////////////////////////////////////////////////////////////////////////////

void
wImageWindow::IV_ImageChanged()
{
  memcpy(m_Image.GetPixels(), m_ImageView->GetPixels(), m_Image.GetWidth() * m_Image.GetHeight() * sizeof(RGBA));
  SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////

void
wImageWindow::IV_ColorChanged(RGBA color)
{
  RGB rgb = { color.red, color.green, color.blue };
  m_ColorView->SetColor(rgb);
  m_AlphaView->SetAlpha(color.alpha);
}

////////////////////////////////////////////////////////////////////////////////

void
wImageWindow::PV_ColorChanged(RGB color)
{
  m_ColorView->SetColor(color);

  RGBA rgba = { color.red, color.green, color.blue, m_ImageView->GetColor().alpha };
  m_ImageView->SetColor(rgba);
}

////////////////////////////////////////////////////////////////////////////////

void
wImageWindow::CV_ColorChanged(RGB color)
{
  RGBA rgba = { color.red, color.green, color.blue, m_ImageView->GetColor().alpha };
  m_ImageView->SetColor(rgba);
}

////////////////////////////////////////////////////////////////////////////////

void
wImageWindow::AV_AlphaChanged(byte alpha)
{
  RGBA rgba = m_ImageView->GetColor();
  rgba.alpha = alpha;
  m_ImageView->SetColor(rgba);
}

////////////////////////////////////////////////////////////////////////////////
