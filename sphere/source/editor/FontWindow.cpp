#pragma warning(disable : 4786)

#include "FontWindow.hpp"
#include "FileDialogs.hpp"
#include "ResizeDialog.hpp"
#include "FontGradientDialog.hpp"
#include "resource.h"


#define IDC_FONTSCROLL 900

#define MIN_CHARACTER 32
#define MAX_CHARACTER 126

#define DEFAULT_CHARACTER_WIDTH  8
#define DEFAULT_CHARACTER_HEIGHT 12

#define COLOR_WIDTH      32
#define COLOR_HEIGHT     32
#define ALPHA_WIDTH      32
#define SCROLLBAR_HEIGHT 16


BEGIN_MESSAGE_MAP(CFontWindow, CSaveableDocumentWindow)

  ON_WM_SIZE()
  ON_WM_HSCROLL()

  ON_COMMAND(ID_FONT_RESIZE,           OnFontResize)
  ON_COMMAND(ID_FONT_RESIZEALL,        OnFontResizeAll)
  ON_COMMAND(ID_FONT_SIMPLIFY,         OnFontSimplify)
  ON_COMMAND(ID_FONT_GENERATEGRADIENT, OnFontGenerateGradient)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CFontWindow::CFontWindow(const char* font)
: CSaveableDocumentWindow(font, IDR_FONT)
, m_CurrentCharacter(MIN_CHARACTER)
, m_CurrentColor(CreateRGBA(0, 0, 0, 255))
, m_Created(false)
{
  if (font) {
    if (m_Font.Load(font)) {
      SetSaved(true);
      SetModified(false);
    } else {
      AfxGetApp()->m_pMainWnd->MessageBox("Could not load font, creating new");
      m_Font.SetNumCharacters(256);
      for (int i = 0; i < 256; i++) {
        m_Font.GetCharacter(i).Resize(DEFAULT_CHARACTER_WIDTH, DEFAULT_CHARACTER_HEIGHT);
      }

      SetSaved(false);
      SetModified(false);
    }
  } else {

    m_Font.SetNumCharacters(256);
    for (int i = 0; i < 256; i++) {
      m_Font.GetCharacter(i).Resize(DEFAULT_CHARACTER_WIDTH, DEFAULT_CHARACTER_HEIGHT);
    }

    SetSaved(false);
    SetModified(false);
  }

  Create();
}

////////////////////////////////////////////////////////////////////////////////

CFontWindow::~CFontWindow()
{
}

////////////////////////////////////////////////////////////////////////////////

void
CFontWindow::Create()
{
  // create window
  CSaveableDocumentWindow::Create(AfxRegisterWndClass(0, ::LoadCursor(NULL, IDC_ARROW), NULL, AfxGetApp()->LoadIcon(IDI_FONT)));
  
  // create children
  m_ImageView.Create(this, this, this);
  m_PaletteView.Create(this, this);
  m_ColorView.Create(this, this);
  m_AlphaView.Create(this, this);

  m_ScrollBar.Create(WS_CHILD | WS_VISIBLE | SBS_HORZ, CRect(0, 0, 0, 0), this, IDC_FONTSCROLL);
  m_ScrollBar.SetScrollRange(MIN_CHARACTER, MAX_CHARACTER);
  m_ScrollBar.SetScrollPos(m_CurrentCharacter);

  m_Created = true;

  // make sure everything is the right size
  RECT ClientRect;
  GetClientRect(&ClientRect);
  OnSize(0, ClientRect.right, ClientRect.bottom);

  SetImage();
  RGB rgb = { m_CurrentColor.red, m_CurrentColor.green, m_CurrentColor.blue };
  m_ColorView.SetColor(rgb);
  m_AlphaView.SetAlpha(m_CurrentColor.alpha);

  UpdateWindowTitle();
}

////////////////////////////////////////////////////////////////////////////////

void
CFontWindow::UpdateWindowTitle()
{
  char title[520];
  strcpy(title, GetDocumentTitle());

  char append[520];
  sprintf(append, " - %c - %d/%d",
    (char)m_CurrentCharacter,
    m_CurrentCharacter,
    m_Font.GetNumCharacters());
  strcat(title, append);

  SetCaption(title);
}

////////////////////////////////////////////////////////////////////////////////

void
CFontWindow::SetImage()
{
  sFontCharacter& c = m_Font.GetCharacter(m_CurrentCharacter);
  m_ImageView.SetImage(c.GetWidth(), c.GetHeight(), c.GetPixels());
  m_ScrollBar.SetScrollPos(m_CurrentCharacter);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CFontWindow::OnSize(UINT type, int cx, int cy)
{
  if (m_Created)
  {
    m_ImageView.MoveWindow(0, 0, cx - COLOR_WIDTH - ALPHA_WIDTH, cy - SCROLLBAR_HEIGHT);
    m_PaletteView.MoveWindow(cx - COLOR_WIDTH - ALPHA_WIDTH, 0, COLOR_WIDTH, cy - SCROLLBAR_HEIGHT - COLOR_HEIGHT);
    m_ColorView.MoveWindow(cx - COLOR_WIDTH - ALPHA_WIDTH, cy - SCROLLBAR_HEIGHT - COLOR_HEIGHT, COLOR_WIDTH, COLOR_HEIGHT);
    m_AlphaView.MoveWindow(cx - ALPHA_WIDTH, 0, ALPHA_WIDTH, cy - SCROLLBAR_HEIGHT);
    m_ScrollBar.MoveWindow(0, cy - SCROLLBAR_HEIGHT, cx, SCROLLBAR_HEIGHT);
  }

  CSaveableDocumentWindow::OnSize(type, cx, cy);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CFontWindow::OnHScroll(UINT sbcode, UINT pos, CScrollBar* scroll_bar)
{
  // scroll bar
  if (scroll_bar->m_hWnd == m_ScrollBar.m_hWnd)
  {
    switch (sbcode)
    {
      case SB_LEFT:
        m_CurrentCharacter = 0;
        break;

      case SB_RIGHT:
        m_CurrentCharacter = m_Font.GetNumCharacters() - 1;
        break;

      case SB_LINELEFT:
      case SB_PAGELEFT:
        m_CurrentCharacter--;
        break;

      case SB_LINERIGHT:
      case SB_PAGERIGHT:
        m_CurrentCharacter++;
        break;

      case SB_THUMBPOSITION:
      case SB_THUMBTRACK:
        m_CurrentCharacter = pos;
        break;

      default:
        return;
     }

    if (m_CurrentCharacter < MIN_CHARACTER)
      m_CurrentCharacter = MIN_CHARACTER;
    if (m_CurrentCharacter > MAX_CHARACTER)
      m_CurrentCharacter = MAX_CHARACTER;

    SetImage();
    UpdateWindowTitle();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CFontWindow::OnFontResize()
{
  sFontCharacter& c = m_Font.GetCharacter(m_CurrentCharacter);
  CResizeDialog dialog("Resize Font Character", c.GetWidth(), c.GetHeight());
  if (dialog.DoModal() == IDOK)
  {
    c.Resize(dialog.GetWidth(), dialog.GetHeight());
    SetModified(true);
    SetImage();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CFontWindow::OnFontResizeAll()
{
  // first, calculate the max character size
  int max_x = 0;
  int max_y = 0;

  for (int i = 0; i < m_Font.GetNumCharacters(); i++) {
    sFontCharacter& c = m_Font.GetCharacter(i);
    if (c.GetWidth() > max_x) {
      max_x = c.GetWidth();
    }
    if (c.GetHeight() > max_y) {
      max_y = c.GetHeight();
    }
  }

  CResizeDialog dialog("Resize All Font Characters", max_x, max_y);
  if (dialog.DoModal() == IDOK) {

    for (int i = 0; i < m_Font.GetNumCharacters(); i++) {
      m_Font.GetCharacter(i).Resize(
        dialog.GetWidth(),
        dialog.GetHeight()
      );
    }

    SetModified(true);
    SetImage();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CFontWindow::OnFontSimplify()
{
  if (MessageBox("This will convert the entire font to opaque and transparent.\nIs this okay?", NULL, MB_YESNO) == IDYES)
  {
    for (int i = 0; i < m_Font.GetNumCharacters(); i++)
    {
      sFontCharacter& c = m_Font.GetCharacter(i);
      for (int j = 0; j < c.GetWidth() * c.GetHeight(); j++)
      {
        if (c.GetPixels()[j].alpha < 128)
          c.GetPixels()[j].alpha = 0;
        else
          c.GetPixels()[j].alpha = 255;
      }
    }

    SetModified(true);
    SetImage();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CFontWindow::OnFontGenerateGradient()
{
  CFontGradientDialog dialog;
  if (dialog.DoModal() == IDOK) {
    m_Font.GenerateGradient(dialog.GetTopColor(), dialog.GetBottomColor());

    SetModified(true);
    SetImage();
  }
}

////////////////////////////////////////////////////////////////////////////////

bool
CFontWindow::GetSavePath(char* path)
{
  CFontFileDialog Dialog(FDM_SAVE);
  if (Dialog.DoModal() != IDOK)
    return false;

  strcpy(path, Dialog.GetPathName());
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CFontWindow::SaveDocument(const char* path)
{
  return m_Font.Save(path);
}

////////////////////////////////////////////////////////////////////////////////

void
CFontWindow::IV_ImageChanged()
{
  sFontCharacter& c = m_Font.GetCharacter(m_CurrentCharacter);
  memcpy(c.GetPixels(), m_ImageView.GetPixels(), c.GetWidth() * c.GetHeight() * sizeof(RGBA));

  SetModified(true);
  UpdateWindowTitle();
}

////////////////////////////////////////////////////////////////////////////////

void
CFontWindow::IV_ColorChanged(RGBA color)
{
  m_CurrentColor = color;
  RGB rgb = { color.red, color.green, color.blue };
  m_ColorView.SetColor(rgb);
  m_AlphaView.SetAlpha(color.alpha);
}

////////////////////////////////////////////////////////////////////////////////

void
CFontWindow::PV_ColorChanged(RGB color)
{
  m_CurrentColor.red   = color.red;
  m_CurrentColor.green = color.green;
  m_CurrentColor.blue  = color.blue;
  m_ImageView.SetColor(m_CurrentColor);
  m_ColorView.SetColor(color);
}

////////////////////////////////////////////////////////////////////////////////

void
CFontWindow::CV_ColorChanged(RGB color)
{
  m_CurrentColor.red   = color.red;
  m_CurrentColor.green = color.green;
  m_CurrentColor.blue  = color.blue;
  m_ImageView.SetColor(m_CurrentColor);
}

////////////////////////////////////////////////////////////////////////////////

void
CFontWindow::AV_AlphaChanged(byte alpha)
{
  m_CurrentColor.alpha = alpha;
  m_ImageView.SetColor(m_CurrentColor);
}

////////////////////////////////////////////////////////////////////////////////