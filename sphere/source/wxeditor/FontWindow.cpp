#pragma warning(disable : 4786)

//#define WXWIN_COMPATIBILITY_EVENT_TYPES 1
#include <wx/event.h>
#include "FontWindow.hpp"
#include "FileDialogs.hpp"
#include "ResizeDialog.hpp"
#include "FontGradientDialog.hpp"
//#include "resource.h"
#include "IDs.hpp"


//#define IDC_FONTSCROLL 900

#define MIN_CHARACTER 32
#define MAX_CHARACTER 126

#define DEFAULT_CHARACTER_WIDTH  8
#define DEFAULT_CHARACTER_HEIGHT 12

#define COLOR_WIDTH      32
#define COLOR_HEIGHT     32
#define ALPHA_WIDTH      32
#define SCROLLBAR_HEIGHT 16


/*
BEGIN_MESSAGE_MAP(CFontWindow, CSaveableDocumentWindow)

  ON_WM_SIZE()
  ON_WM_HSCROLL()

  ON_COMMAND(ID_FONT_RESIZE,               OnFontResize)
  ON_COMMAND(ID_FONT_RESIZEALL,            OnFontResizeAll)
  ON_COMMAND(ID_FONT_SIMPLIFY,             OnFontSimplify)
  ON_COMMAND(ID_FONT_MAKECOLORTRANSPARENT, OnFontMakeColorTransparent)
  ON_COMMAND(ID_FONT_GENERATEGRADIENT,     OnFontGenerateGradient)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wFontWindow, wSaveableDocumentWindow)
  EVT_SIZE(wFontWindow::OnSize)
  EVT_COMMAND_SCROLL(wID_FONT_SCROLL,     wFontWindow::OnHScroll)

  EVT_MENU(wID_FONT_RESIZE,               wFontWindow::OnFontResize)
  EVT_MENU(wID_FONT_RESIZEALL,            wFontWindow::OnFontResizeAll)
  EVT_MENU(wID_FONT_SIMPLIFY,             wFontWindow::OnFontSimplify)
  EVT_MENU(wID_FONT_MAKECOLORTRANSPARENT, wFontWindow::OnFontMakeColorTransparent)
  EVT_MENU(wID_FONT_GENERATEGRADIENT,     wFontWindow::OnFontGenerateGradient)
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wFontWindow::wFontWindow(const char* font)
: wSaveableDocumentWindow(font, wID_FONT_base /*todo:IDR_FONT*/)
, m_CurrentCharacter(MIN_CHARACTER)
, m_CurrentColor(CreateRGBA(0, 0, 0, 255))
, m_Created(false)
{
  if (font) {
    if (m_Font.Load(font)) {
      SetSaved(true);
      SetModified(false);
    } else {
      //AfxGetApp()->m_pMainWnd->MessageBox("Could not load font, creating new");
      ::wxMessageBox("Could not load font, creating new");
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

wFontWindow::~wFontWindow()
{
}

////////////////////////////////////////////////////////////////////////////////

void
wFontWindow::Create()
{
  // create window
  //CSaveableDocumentWindow::Create(AfxRegisterWndClass(0, ::LoadCursor(NULL, IDC_ARROW), NULL, AfxGetApp()->LoadIcon(IDI_FONT)));
  
  // create children
  //m_ImageView.Create(this, this, this);
  //m_PaletteView.Create(this, this);
  //m_ColorView.Create(this, this);
  //m_AlphaView.Create(this, this);

  //m_ScrollBar.Create(WS_CHILD | WS_VISIBLE | SBS_HORZ, CRect(0, 0, 0, 0), this, IDC_FONTSCROLL);
  //m_ScrollBar.SetScrollRange(MIN_CHARACTER, MAX_CHARACTER);
  //m_ScrollBar.SetScrollPos(m_CurrentCharacter);

  m_ImageView = new wImageView(this, this, this);
  m_PaletteView = new wPaletteView(this, this);
  m_ColorView = new wColorView(this, this);
  m_AlphaView = new wAlphaView(this, this);

  m_ScrollBar = new wxScrollBar(this, wID_FONT_SCROLL, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL);
  m_ScrollBar->SetScrollbar(0, 1, MAX_CHARACTER - MIN_CHARACTER + 1, 1);

  m_Created = true;

/*
  // make sure everything is the right size
  RECT ClientRect;
  GetClientRect(&ClientRect);
  OnSize(0, ClientRect.right, ClientRect.bottom);
*/
  SetImage();

  RGB rgb = { m_CurrentColor.red, m_CurrentColor.green, m_CurrentColor.blue };
  m_ColorView->SetColor(rgb);
  m_AlphaView->SetAlpha(m_CurrentColor.alpha);

  UpdateWindowTitle();
}

////////////////////////////////////////////////////////////////////////////////

void
wFontWindow::UpdateWindowTitle()
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
wFontWindow::SetImage()
{
  sFontCharacter& c = m_Font.GetCharacter(m_CurrentCharacter);
  m_ImageView->SetImage(c.GetWidth(), c.GetHeight(), c.GetPixels());
  m_ScrollBar->SetThumbPosition(m_CurrentCharacter - MIN_CHARACTER);
}

////////////////////////////////////////////////////////////////////////////////

void
wFontWindow::OnSize(wxSizeEvent &event)
{
  int cx = GetClientSize().GetWidth();
  int cy = GetClientSize().GetHeight();
  if (m_Created)
  {
    m_ImageView->SetSize(0, 0, cx - COLOR_WIDTH - ALPHA_WIDTH, cy - SCROLLBAR_HEIGHT);
    m_PaletteView->SetSize(cx - COLOR_WIDTH - ALPHA_WIDTH, 0, COLOR_WIDTH, cy - SCROLLBAR_HEIGHT - COLOR_HEIGHT);
    m_ColorView->SetSize(cx - COLOR_WIDTH - ALPHA_WIDTH, cy - SCROLLBAR_HEIGHT - COLOR_HEIGHT, COLOR_WIDTH, COLOR_HEIGHT);
    m_AlphaView->SetSize(cx - ALPHA_WIDTH, 0, ALPHA_WIDTH, cy - SCROLLBAR_HEIGHT);
    m_ScrollBar->SetSize(0, cy - SCROLLBAR_HEIGHT, cx, SCROLLBAR_HEIGHT);
  }

  wSaveableDocumentWindow::OnSize(event);
}

////////////////////////////////////////////////////////////////////////////////

void
wFontWindow::OnHScroll(wxScrollEvent &event)
{
  // scroll bar
  int pos = m_ScrollBar->GetThumbPosition() + MIN_CHARACTER;
  if(event.GetEventType() == wxEVT_SCROLL_TOP)          m_CurrentCharacter = 0;
  if(event.GetEventType() == wxEVT_SCROLL_BOTTOM)       m_CurrentCharacter = m_Font.GetNumCharacters() - 1;
  if(event.GetEventType() == wxEVT_SCROLL_LINEUP)       m_CurrentCharacter--;
  if(event.GetEventType() == wxEVT_SCROLL_PAGEUP)       m_CurrentCharacter--;
  if(event.GetEventType() == wxEVT_SCROLL_LINEDOWN)     m_CurrentCharacter++;
  if(event.GetEventType() == wxEVT_SCROLL_PAGEDOWN)     m_CurrentCharacter++;
  if(event.GetEventType() == wxEVT_SCROLL_THUMBTRACK)   m_CurrentCharacter = pos;
  if(event.GetEventType() == wxEVT_SCROLL_THUMBRELEASE) m_CurrentCharacter = pos;

  if (m_CurrentCharacter < MIN_CHARACTER)
    m_CurrentCharacter = MIN_CHARACTER;
  if (m_CurrentCharacter > MAX_CHARACTER)
    m_CurrentCharacter = MAX_CHARACTER;

  SetImage();
  UpdateWindowTitle();
}

////////////////////////////////////////////////////////////////////////////////

void
wFontWindow::OnFontResize(wxCommandEvent &event)
{
  sFontCharacter& c = m_Font.GetCharacter(m_CurrentCharacter);
  wResizeDialog dialog(this, "Resize Font Character", c.GetWidth(), c.GetHeight());
  if (dialog.ShowModal() == wxID_OK)
  {
    c.Resize(dialog.GetWidth(), dialog.GetHeight());
    SetModified(true);
    SetImage();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wFontWindow::OnFontResizeAll(wxCommandEvent &event)
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

  wResizeDialog dialog(this, "Resize All Font Characters", max_x, max_y);
  if (dialog.ShowModal() == wxID_OK) {

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

void
wFontWindow::OnFontSimplify(wxCommandEvent &event)
{
  if (::wxMessageBox("This will convert the entire font to opaque and transparent.\nIs this okay?", "", wxYES_NO) == wxYES)
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

void
wFontWindow::OnFontMakeColorTransparent(wxCommandEvent &event) {
  RGB color = m_ColorView->GetColor();
  for (int i = 0; i < m_Font.GetNumCharacters(); ++i) {
    sFontCharacter& c = m_Font.GetCharacter(i);
    for (int j = 0; j < c.GetWidth() * c.GetHeight(); ++j) {
      if (c.GetPixels()[j].red   == color.red &&
          c.GetPixels()[j].green == color.green &&
          c.GetPixels()[j].blue  == color.blue)
      {
        c.GetPixels()[j].alpha = 0;
      }
    }
  }

  SetModified(true);
  SetImage();
}

////////////////////////////////////////////////////////////////////////////////

void
wFontWindow::OnFontGenerateGradient(wxCommandEvent &event)
{
  wFontGradientDialog dialog(this);
  if (dialog.ShowModal() == wxID_OK) {
    m_Font.GenerateGradient(dialog.GetTopColor(), dialog.GetBottomColor());

    SetModified(true);
    SetImage();
  }
}

////////////////////////////////////////////////////////////////////////////////

bool
wFontWindow::GetSavePath(char* path)
{
  wFontFileDialog Dialog(this, FDM_SAVE);
  if (Dialog.ShowModal() != wxID_OK)
    return false;

  strcpy(path, Dialog.GetPath());
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
wFontWindow::SaveDocument(const char* path)
{
  return m_Font.Save(path);
}

////////////////////////////////////////////////////////////////////////////////

void
wFontWindow::IV_ImageChanged()
{
  sFontCharacter& c = m_Font.GetCharacter(m_CurrentCharacter);
  memcpy(c.GetPixels(), m_ImageView->GetPixels(), c.GetWidth() * c.GetHeight() * sizeof(RGBA));

  SetModified(true);
  UpdateWindowTitle();
}

////////////////////////////////////////////////////////////////////////////////

void
wFontWindow::IV_ColorChanged(RGBA color)
{
  m_CurrentColor = color;
  RGB rgb = { color.red, color.green, color.blue };
  m_ColorView->SetColor(rgb);
  m_AlphaView->SetAlpha(color.alpha);
}

////////////////////////////////////////////////////////////////////////////////

void
wFontWindow::PV_ColorChanged(RGB color)
{
  m_CurrentColor.red   = color.red;
  m_CurrentColor.green = color.green;
  m_CurrentColor.blue  = color.blue;
  m_ImageView->SetColor(m_CurrentColor);
  m_ColorView->SetColor(color);
}

////////////////////////////////////////////////////////////////////////////////

void
wFontWindow::CV_ColorChanged(RGB color)
{
  m_CurrentColor.red   = color.red;
  m_CurrentColor.green = color.green;
  m_CurrentColor.blue  = color.blue;
  m_ImageView->SetColor(m_CurrentColor);
}

////////////////////////////////////////////////////////////////////////////////

void
wFontWindow::AV_AlphaChanged(byte alpha)
{
  m_CurrentColor.alpha = alpha;
  m_ImageView->SetColor(m_CurrentColor);
}

////////////////////////////////////////////////////////////////////////////////
