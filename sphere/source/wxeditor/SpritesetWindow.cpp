#ifdef _MSC_VER
// identifier too long
#pragma warning(disable : 4786)
#endif


#include "SpritesetWindow.hpp"
#include "FileDialogs.hpp"
#include "ResizeDialog.hpp"
#include "SpritePropertiesDialog.hpp"
#include "SpritesetImagesPalette.hpp"
//#include "NumberDialog.hpp"
//#include "resource.h"
#include "IDs.hpp"


//#define IDC_TAB 800
#define TAB_HEIGHT 24


/*
BEGIN_MESSAGE_MAP(CSpritesetWindow, CSaveableDocumentWindow)

  ON_WM_DESTROY()
  ON_WM_SIZE()

  ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, OnTabChanged)

  ON_COMMAND(ID_SPRITESET_ZOOM_1X,         OnZoom1x)
  ON_COMMAND(ID_SPRITESET_ZOOM_2X,         OnZoom2x)
  ON_COMMAND(ID_SPRITESET_ZOOM_4X,         OnZoom4x)
  ON_COMMAND(ID_SPRITESET_ZOOM_8X,         OnZoom8x)
  ON_COMMAND(ID_SPRITESET_RESIZE,          OnResize)
  ON_COMMAND(ID_SPRITESET_FILLDELAY,       OnFillDelay)
  ON_COMMAND(ID_SPRITESET_FRAMEPROPERTIES, OnFrameProperties)

  ON_UPDATE_COMMAND_UI(ID_SPRITESET_ZOOM_1X, OnUpdateZoom1x)
  ON_UPDATE_COMMAND_UI(ID_SPRITESET_ZOOM_2X, OnUpdateZoom2x)
  ON_UPDATE_COMMAND_UI(ID_SPRITESET_ZOOM_4X, OnUpdateZoom4x)
  ON_UPDATE_COMMAND_UI(ID_SPRITESET_ZOOM_8X, OnUpdateZoom8x)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wSpritesetWindow, wSaveableDocumentWindow)
  EVT_SIZE(wSpritesetWindow::OnSize)

  EVT_TAB_SEL_CHANGED(wID_SPRITESET_TAB, wSpritesetWindow::OnTabChanged)

  EVT_MENU(wID_SPRITESET_ZOOM_1X,         wSpritesetWindow::OnZoom1x)
  EVT_MENU(wID_SPRITESET_ZOOM_2X,         wSpritesetWindow::OnZoom2x)
  EVT_MENU(wID_SPRITESET_ZOOM_4X,         wSpritesetWindow::OnZoom4x)
  EVT_MENU(wID_SPRITESET_ZOOM_8X,         wSpritesetWindow::OnZoom8x)
  EVT_MENU(wID_SPRITESET_RESIZE,          wSpritesetWindow::OnResize)
  EVT_MENU(wID_SPRITESET_FILLDELAY,       wSpritesetWindow::OnFillDelay)
  EVT_MENU(wID_SPRITESET_FRAMEPROPERTIES, wSpritesetWindow::OnFrameProperties)
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wSpritesetWindow::wSpritesetWindow(const char* filename)
: wSaveableDocumentWindow(filename, wID_SPRITESET_base /*IDR_SPRITESET*/, wxSize(160, 120))
, m_CurrentDirection(0)
, m_CurrentFrame(0)
, m_Created(false)
, m_ImagesPalette()
{
  SetSaved(filename != NULL);
  SetModified(false);

  // load the spriteset
  if (filename == NULL || m_Spriteset.Load(filename) == false) {
    if (filename) {
      ::wxMessageBox("Could not load spriteset.\nCreating empty spriteset.");
    }
    m_Spriteset.Create(16, 32, 1, 8, 1);

    m_Spriteset.SetDirectionName(0, "north");
    m_Spriteset.SetDirectionName(1, "northeast");
    m_Spriteset.SetDirectionName(2, "east");
    m_Spriteset.SetDirectionName(3, "southeast");
    m_Spriteset.SetDirectionName(4, "south");
    m_Spriteset.SetDirectionName(5, "southwest");
    m_Spriteset.SetDirectionName(6, "west");
    m_Spriteset.SetDirectionName(7, "northwest");

    SetSaved(false);
  }

  Create();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::Create()
{
  //wSaveableDocumentWindow::Create(AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, LoadCursor(NULL, IDC_ARROW), NULL, AfxGetApp()->LoadIcon(IDI_SPRITESET)));

  m_TabControl = new wxTabCtrl(this, wID_SPRITESET_TAB);
  m_TabControl->InsertItem(0, "Frames");
  m_TabControl->InsertItem(1, "Edit");
  m_TabControl->InsertItem(2, "Base");

  // create the views
  m_SpritesetView = new wSpritesetView(this, this, &m_Spriteset);
  m_ImageView = new wImageView(this, this, this);
  m_PaletteView = new wPaletteView(this, this);
  m_ColorView = new wColorView(this, this);
  m_AlphaView = new wAlphaView(this, this);
  m_SpriteBaseView = new wSpriteBaseView(this, this, &m_Spriteset);
  int frame = m_Spriteset.GetFrameIndex(m_CurrentDirection, m_CurrentFrame);
  m_SpriteBaseView->SetSprite(&m_Spriteset.GetImage(frame));

  // create the palette
  m_ImagesPalette = new wSpritesetImagesPalette(this, this, &m_Spriteset);

  // the window and its children are ready!
  m_Created = true;

/*todo:
  // make sure everything is moved to the correct place
  RECT client_rect;
  GetClientRect(&client_rect);
  OnSize(SIZE_RESTORED, client_rect.right, client_rect.bottom);
*/

  UpdateImageView();
  TabChanged(0);

  wxCommandEvent evt;
  OnZoom2x(evt);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::TabChanged(int tab)
{
  ShowFramesTab(tab == 0);
  ShowEditTab  (tab == 1);
  ShowBaseTab  (tab == 2);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::ShowFramesTab(bool show)
{
  m_SpritesetView->Show(show);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::ShowEditTab(bool show)
{
  m_ImageView->Show(show);
  m_PaletteView->Show(show);
  m_ColorView->Show(show);
  m_AlphaView->Show(show);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::ShowBaseTab(bool show)
{
  m_SpriteBaseView->Show(show);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::UpdateImageView()
{  
  int index = m_Spriteset.GetFrameIndex(m_CurrentDirection, m_CurrentFrame);
  CImage32& sprite = m_Spriteset.GetImage(index);
  m_ImageView->SetImage(sprite.GetWidth(), sprite.GetHeight(), sprite.GetPixels());
  m_SpriteBaseView->SetSprite(&sprite);
}

/*
////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetWindow::OnDestroy()
{
  m_ImagesPalette->Destroy();

  m_SpritesetView.DestroyWindow();
  m_ImageView.DestroyWindow();
  m_PaletteView.DestroyWindow();
  m_ColorView.DestroyWindow();
  m_AlphaView.DestroyWindow();
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::OnSize(wxSizeEvent &event)
{
  const int SEPARATOR     = 96;
  const int PALETTE_WIDTH = 64;
  const int COLOR_HEIGHT  = 64;
  const int ALPHA_WIDTH   = SEPARATOR - PALETTE_WIDTH;

  int cx = GetClientSize().GetWidth();
  int cy = GetClientSize().GetHeight();

  if (m_Created)
  {
    m_TabControl->SetSize(0, 0, cx, TAB_HEIGHT);

    // frames tab
    m_SpritesetView->SetSize(0, TAB_HEIGHT, cx, cy - TAB_HEIGHT);
    // edit tab

    // left side
    m_PaletteView->SetSize(0, TAB_HEIGHT, PALETTE_WIDTH, cy - COLOR_HEIGHT - TAB_HEIGHT);
    m_ColorView->SetSize(0, cy - COLOR_HEIGHT, PALETTE_WIDTH, COLOR_HEIGHT);
    m_AlphaView->SetSize(PALETTE_WIDTH, TAB_HEIGHT, ALPHA_WIDTH, cy - TAB_HEIGHT);

    // right side
    m_ImageView->SetSize(SEPARATOR, TAB_HEIGHT, cx - SEPARATOR, cy - TAB_HEIGHT);

    // base tab
    m_SpriteBaseView->SetSize(0, TAB_HEIGHT, cx, cy - TAB_HEIGHT);
  }

  wSaveableDocumentWindow::OnSize(event);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::OnZoom1x(wxCommandEvent &event)
{
  m_SpritesetView->SetZoomFactor(1);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::OnZoom2x(wxCommandEvent &event)
{
  m_SpritesetView->SetZoomFactor(2);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::OnZoom4x(wxCommandEvent &event)
{
  m_SpritesetView->SetZoomFactor(4);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::OnZoom8x(wxCommandEvent &event)
{
  m_SpritesetView->SetZoomFactor(8);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::OnResize(wxCommandEvent &event)
{
  wResizeDialog dialog(this, "Resize Spriteset", m_Spriteset.GetFrameWidth(), m_Spriteset.GetFrameHeight());
  if (dialog.ShowModal() == wxID_OK) {
    m_Spriteset.ResizeFrames(dialog.GetWidth(), dialog.GetHeight());
    m_SpritesetView->SpritesetResized();
    UpdateImageView();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::OnFillDelay(wxCommandEvent &event)
{
  //wNumberDialog dialog(this, "Spriteset Delay", "Delay", 8, 1, 4096);
  //if (dialog.ShowModal() == wxID_OK) {
  int delay = ::wxGetNumberFromUser("", "Spriteset Delay", "Spriset Fill Delay", 8, 1, 4096, this);
  if(delay != -1) {
    for (int i = 0; i < m_Spriteset.GetNumDirections(); i++) {
      for (int j = 0; j < m_Spriteset.GetNumFrames(i); j++) {
        m_Spriteset.SetFrameDelay(i, j, delay);
      }
    }

  }
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::OnFrameProperties(wxCommandEvent &event)
{
  wSpritePropertiesDialog Dialog(this, &m_Spriteset, m_CurrentDirection, m_CurrentFrame);
  if (Dialog.ShowModal() == wxID_OK)
    SetModified(true);
}

/*todo:
////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetWindow::OnUpdateZoom1x(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_SpritesetView.GetZoomFactor() == 1);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetWindow::OnUpdateZoom2x(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_SpritesetView.GetZoomFactor() == 2);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetWindow::OnUpdateZoom4x(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_SpritesetView.GetZoomFactor() == 4);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetWindow::OnUpdateZoom8x(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_SpritesetView.GetZoomFactor() == 8);
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::OnTabChanged(wxTabEvent &event)
{
  TabChanged(m_TabControl->GetCurFocus());
}

////////////////////////////////////////////////////////////////////////////////

bool
wSpritesetWindow::GetSavePath(char* path)
{
  wSpritesetFileDialog Dialog(this, FDM_SAVE);
  if (Dialog.ShowModal() != wxID_OK)
    return false;

  strcpy(path, Dialog.GetPath());
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
wSpritesetWindow::SaveDocument(const char* path)
{
  return m_Spriteset.Save(path);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::SV_CurrentFrameChanged(int direction, int frame)
{
  m_CurrentDirection = direction;
  m_CurrentFrame = frame;
  UpdateImageView();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::SV_EditFrame()
{
  // switch to the edit tab
  m_TabControl->SetSelection(1);
  TabChanged(1);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::SV_SpritesetModified()
{
  UpdateImageView();
  SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::SV_CopyCurrentFrame()
{
  m_ImageView->Copy();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::SV_PasteCurrentFrame()
{
  m_ImageView->Paste();
  m_SpritesetView->Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::IV_ImageChanged()
{
  // store the current sprite
  int index = m_Spriteset.GetFrameIndex(m_CurrentDirection, m_CurrentFrame);
  CImage32& frame = m_Spriteset.GetImage(index);
  memcpy(frame.GetPixels(), m_ImageView->GetPixels(), frame.GetWidth() * frame.GetHeight() * sizeof(RGBA));

  SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::IV_ColorChanged(RGBA color)
{
  RGB rgb = { color.red, color.green, color.blue };
  m_ColorView->SetColor(rgb);
  m_AlphaView->SetAlpha(color.alpha);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::PV_ColorChanged(RGB color)
{
  RGBA rgba = { color.red, color.green, color.blue, m_AlphaView->GetAlpha() };
  m_ImageView->SetColor(rgba);
  m_ColorView->SetColor(color);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::CV_ColorChanged(RGB color)
{
  RGBA rgba = { color.red, color.green, color.blue, m_AlphaView->GetAlpha() };
  m_ImageView->SetColor(rgba);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::AV_AlphaChanged(byte alpha)
{
  RGBA rgba = m_ImageView->GetColor();
  rgba.alpha = alpha;
  m_ImageView->SetColor(rgba);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::SBV_SpritesetModified()
{
  SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////

void 
wSpritesetWindow::SP_ColorSelected(RGBA color)
{
  byte alpha = color.alpha;
  RGB  rgb   = { color.red, color.green, color.blue };

  m_ImageView->SetColor(color);
  m_ColorView->SetColor(rgb);
  m_AlphaView->SetAlpha(alpha);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::SIP_IndexChanged(int index)
{
  int old_index = m_Spriteset.GetFrameIndex(m_CurrentDirection, m_CurrentFrame);
  if (old_index != index) {
    m_Spriteset.SetFrameIndex(m_CurrentDirection, m_CurrentFrame, index);
    UpdateImageView();
    SetModified(true);
    Refresh();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetWindow::SIP_SpritesetModified()
{
  SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////
