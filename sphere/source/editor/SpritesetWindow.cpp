// identifier too long
#pragma warning(disable : 4786)


#include "SpritesetWindow.hpp"
#include "FileDialogs.hpp"
#include "ResizeDialog.hpp"
#include "SpritePropertiesDialog.hpp"
#include "SpritesetImagesPalette.hpp"
#include "NumberDialog.hpp"
#include "resource.h"


#define IDC_TAB 800
#define TAB_HEIGHT 24


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


////////////////////////////////////////////////////////////////////////////////

CSpritesetWindow::CSpritesetWindow(const char* filename)
: CSaveableDocumentWindow(filename, IDR_SPRITESET, CSize(160, 120))
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
      MessageBox("Could not load spriteset.\nCreating empty spriteset.");
    }
    m_Spriteset.Create(16, 32, 1, 8, 1);
    SetSaved(false);
  }

  Create();
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetWindow::Create()
{
  CSaveableDocumentWindow::Create(AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, LoadCursor(NULL, IDC_ARROW), NULL, AfxGetApp()->LoadIcon(IDI_SPRITESET)));

  m_TabControl.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_TAB);
  m_TabControl.SetFont(CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT)));
  m_TabControl.InsertItem(0, "Frames");
  m_TabControl.InsertItem(1, "Edit");
  m_TabControl.InsertItem(2, "Base");

  // create the views
  m_SpritesetView.Create(this, this, &m_Spriteset);
  m_ImageView.Create(this, this, this);
  m_PaletteView.Create(this, this);
  m_ColorView.Create(this, this);
  m_AlphaView.Create(this, this);
  m_SpriteBaseView.Create(this, this, &m_Spriteset);
  int frame = m_Spriteset.GetFrameIndex(m_CurrentDirection, m_CurrentFrame);
  m_SpriteBaseView.SetSprite(&m_Spriteset.GetImage(frame));

  // create the palette
  m_ImagesPalette = new CSpritesetImagesPalette(this, this, &m_Spriteset);

  // the window and its children are ready!
  m_Created = true;

  // make sure everything is moved to the correct place
  RECT client_rect;
  GetClientRect(&client_rect);
  OnSize(SIZE_RESTORED, client_rect.right, client_rect.bottom);

  UpdateImageView();
  TabChanged(0);

  OnZoom2x();
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetWindow::TabChanged(int tab)
{
  ShowFramesTab(tab == 0 ? SW_SHOW : SW_HIDE);
  ShowEditTab  (tab == 1 ? SW_SHOW : SW_HIDE);
  ShowBaseTab  (tab == 2 ? SW_SHOW : SW_HIDE);
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetWindow::ShowFramesTab(int show)
{
  m_SpritesetView.ShowWindow(show);
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetWindow::ShowEditTab(int show)
{
  m_ImageView.ShowWindow(show);
  m_PaletteView.ShowWindow(show);
  m_ColorView.ShowWindow(show);
  m_AlphaView.ShowWindow(show);
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetWindow::ShowBaseTab(int show)
{
  m_SpriteBaseView.ShowWindow(show);
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetWindow::UpdateImageView()
{  
  int index = m_Spriteset.GetFrameIndex(m_CurrentDirection, m_CurrentFrame);
  CImage32& sprite = m_Spriteset.GetImage(index);
  m_ImageView.SetImage(sprite.GetWidth(), sprite.GetHeight(), sprite.GetPixels());
  m_SpriteBaseView.SetSprite(&sprite);
}

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

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetWindow::OnSize(UINT type, int cx, int cy)
{
  const int SEPARATOR     = 96;
  const int PALETTE_WIDTH = 64;
  const int COLOR_HEIGHT  = 64;
  const int ALPHA_WIDTH   = SEPARATOR - PALETTE_WIDTH;

  if (m_Created)
  {
    m_TabControl.MoveWindow(0, 0, cx, TAB_HEIGHT);

    // frames tab
    m_SpritesetView.MoveWindow(0, TAB_HEIGHT, cx, cy - TAB_HEIGHT);

    // edit tab

    // left side
    m_PaletteView.MoveWindow(0, TAB_HEIGHT, PALETTE_WIDTH, cy - COLOR_HEIGHT - TAB_HEIGHT);
    m_ColorView.MoveWindow(0, cy - COLOR_HEIGHT, PALETTE_WIDTH, COLOR_HEIGHT);
    m_AlphaView.MoveWindow(PALETTE_WIDTH, TAB_HEIGHT, ALPHA_WIDTH, cy - TAB_HEIGHT);

    // right side
    m_ImageView.MoveWindow(SEPARATOR, TAB_HEIGHT, cx - SEPARATOR, cy - TAB_HEIGHT);

    // base tab
    m_SpriteBaseView.MoveWindow(0, TAB_HEIGHT, cx, cy - TAB_HEIGHT);
  }

  CSaveableDocumentWindow::OnSize(type, cx, cy);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetWindow::OnZoom1x()
{
  m_SpritesetView.SetZoomFactor(1);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetWindow::OnZoom2x()
{
  m_SpritesetView.SetZoomFactor(2);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetWindow::OnZoom4x()
{
  m_SpritesetView.SetZoomFactor(4);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetWindow::OnZoom8x()
{
  m_SpritesetView.SetZoomFactor(8);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetWindow::OnResize()
{
  CResizeDialog dialog("Resize Spriteset", m_Spriteset.GetFrameWidth(), m_Spriteset.GetFrameHeight());
  if (dialog.DoModal() == IDOK) {
    m_Spriteset.ResizeFrames(dialog.GetWidth(), dialog.GetHeight());
    m_SpritesetView.SpritesetResized();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetWindow::OnFillDelay()
{
  CNumberDialog dialog("Spriteset Delay", "Delay", 8, 1, 4096);
  if (dialog.DoModal() == IDOK) {
    
    for (int i = 0; i < m_Spriteset.GetNumDirections(); i++) {
      for (int j = 0; j < m_Spriteset.GetNumFrames(i); j++) {
        m_Spriteset.SetFrameDelay(i, j, dialog.GetValue());
      }
    }

  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetWindow::OnFrameProperties()
{
  CSpritePropertiesDialog Dialog(&m_Spriteset, m_CurrentDirection, m_CurrentFrame);
  if (Dialog.DoModal() == IDOK)
    SetModified(true);
}

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

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetWindow::OnTabChanged(NMHDR* ns, LRESULT* result)
{
  if (ns->idFrom == IDC_TAB)
    TabChanged(m_TabControl.GetCurSel());
}

////////////////////////////////////////////////////////////////////////////////

bool
CSpritesetWindow::GetSavePath(char* path)
{
  CSpritesetFileDialog Dialog(FDM_SAVE);
  if (Dialog.DoModal() != IDOK)
    return false;

  strcpy(path, Dialog.GetPathName());
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CSpritesetWindow::SaveDocument(const char* path)
{
  return m_Spriteset.Save(path);
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetWindow::SV_CurrentFrameChanged(int direction, int frame)
{
  m_CurrentDirection = direction;
  m_CurrentFrame = frame;
  UpdateImageView();
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetWindow::SV_EditFrame()
{
  // switch to the edit tab
  m_TabControl.SetCurSel(1);
  TabChanged(1);
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetWindow::SV_SpritesetModified()
{
  UpdateImageView();
  SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetWindow::SV_CopyCurrentFrame()
{
  m_ImageView.Copy();
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetWindow::SV_PasteCurrentFrame()
{
  m_ImageView.Paste();
  m_SpritesetView.Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetWindow::IV_ImageChanged()
{
  // store the current sprite
  int index = m_Spriteset.GetFrameIndex(m_CurrentDirection, m_CurrentFrame);
  CImage32& frame = m_Spriteset.GetImage(index);
  memcpy(frame.GetPixels(), m_ImageView.GetPixels(), frame.GetWidth() * frame.GetHeight() * sizeof(RGBA));

  SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetWindow::IV_ColorChanged(RGBA color)
{
  RGB rgb = { color.red, color.green, color.blue };
  m_ColorView.SetColor(rgb);
  m_AlphaView.SetAlpha(color.alpha);
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetWindow::PV_ColorChanged(RGB color)
{
  RGBA rgba = { color.red, color.green, color.blue, m_AlphaView.GetAlpha() };
  m_ImageView.SetColor(rgba);
  m_ColorView.SetColor(color);
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetWindow::CV_ColorChanged(RGB color)
{
  RGBA rgba = { color.red, color.green, color.blue, m_AlphaView.GetAlpha() };
  m_ImageView.SetColor(rgba);
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetWindow::AV_AlphaChanged(byte alpha)
{
  RGBA rgba = m_ImageView.GetColor();
  rgba.alpha = alpha;
  m_ImageView.SetColor(rgba);
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetWindow::SBV_SpritesetModified()
{
  SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////

void 
CSpritesetWindow::SP_ColorSelected(RGBA color)
{
  byte alpha = color.alpha;
  RGB  rgb   = { color.red, color.green, color.blue };

  m_ImageView.SetColor(color);
  m_ColorView.SetColor(rgb);
  m_AlphaView.SetAlpha(alpha);
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetWindow::SIP_IndexChanged(int index)
{
  int old_index = m_Spriteset.GetFrameIndex(m_CurrentDirection, m_CurrentFrame);
  if (old_index != index) {
    m_Spriteset.SetFrameIndex(m_CurrentDirection, m_CurrentFrame, index);
    SetModified(true);
    Invalidate();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetWindow::SIP_SpritesetModified()
{
  SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////