#pragma warning(disable : 4786)

#include "FontWindow.hpp"
#include "FileDialogs.hpp"
#include "ResizeDialog.hpp"
#include "FontGradientDialog.hpp"
#include "resource.h"
#include "../common/minmax.hpp"

#include "EditRange.hpp"
#include "FileDialogs.hpp"

#define IDC_FONTSCROLL 900

#define MIN_CHARACTER 0
#define MAX_CHARACTER 255

#define DEFAULT_CHARACTER_WIDTH  8
#define DEFAULT_CHARACTER_HEIGHT 12

#define COLOR_WIDTH      32
#define COLOR_HEIGHT     32
#define ALPHA_WIDTH      32
#define SCROLLBAR_HEIGHT 16

#ifdef USE_SIZECBAR
IMPLEMENT_DYNAMIC(CFontWindow, CMDIChildWnd)
#endif

BEGIN_MESSAGE_MAP(CFontWindow, CSaveableDocumentWindow)

  ON_WM_CHAR()  
  ON_WM_KEYDOWN()
  ON_WM_SIZE()
  ON_WM_HSCROLL()

  ON_COMMAND(ID_FONT_RESIZE,               OnFontResize)
  ON_COMMAND(ID_FONT_RESIZEALL,            OnFontResizeAll)
  ON_COMMAND(ID_FONT_SIMPLIFY,             OnFontSimplify)
  ON_COMMAND(ID_FONT_MAKECOLORTRANSPARENT, OnFontMakeColorTransparent)
  ON_COMMAND(ID_FONT_GENERATEGRADIENT,     OnFontGenerateGradient)

  ON_COMMAND(ID_FONT_EXPORTTOIMAGE,      OnFontExportToImage)

//  ON_COMMAND(ID_FONT_ER_ROTATE_CW,             OnEditRangeRotateCW)
//  ON_COMMAND(ID_FONT_ER_ROTATE_CCW,            OnEditRangeRotateCCW)
  ON_COMMAND(ID_FONT_ER_SLIDE_UP,              OnEditRange)
  ON_COMMAND(ID_FONT_ER_SLIDE_RIGHT,           OnEditRange)
  ON_COMMAND(ID_FONT_ER_SLIDE_DOWN,            OnEditRange)
  ON_COMMAND(ID_FONT_ER_SLIDE_LEFT,            OnEditRange)
  ON_COMMAND(ID_FONT_ER_SLIDE_OTHER,           OnEditRange)
  ON_COMMAND(ID_FONT_ER_FLIP_HORIZONTALLY,     OnEditRange)
  ON_COMMAND(ID_FONT_ER_FLIP_VERTICALLY,       OnEditRange)
//  ON_COMMAND(ID_FONT_ER_FILL_RGB,              OnEditRangeFillRGB)
//  ON_COMMAND(ID_FONT_ER_FILL_ALPHA,            OnEditRangeFillAlpha)
//  ON_COMMAND(ID_FONT_ER_FILL_BOTH,             OnEditRangeFillBoth)
  ON_COMMAND(ID_FONT_ER_REPLACE_RGBA,            OnEditRange)
//  ON_COMMAND(ID_FONT_ER_REPLACE_RGB,           OnEditRangeReplaceRGB)
//  ON_COMMAND(ID_FONT_ER_REPLACE_ALPHA,         OnEditRangeReplaceAlpha)
//  ON_COMMAND(ID_FONT_ER_FLT_BLUR,              OnEditRangeFilterBlur)
//  ON_COMMAND(ID_FONT_ER_FLT_NOISE,              OnEditRangeFilterNoise)
//  ON_COMMAND(ID_FONT_ER_FLT_ADJUST_BRIGHTNESS,  OnEditRangeFilterAdjustBrightness)
//  ON_COMMAND(ID_FONT_ER_FLT_ADJUST_GAMMA,       OnEditRangeFilterAdjustGamma)
//  ON_COMMAND(ID_FONT_ER_FLT_NEGATIVE_IMAGE_RGB, OnEditRangeFilterNegativeImageRGB)
//  ON_COMMAND(ID_FONT_ER_FLT_NEGATIVE_IMAGE_ALPHA, OnEditRangeFilterNegativeImageAlpha)
//  ON_COMMAND(ID_FONT_ER_FLT_NEGATIVE_IMAGE_RGBA,  OnEditRangeFilterNegativeImageRGBA)
//  ON_COMMAND(ID_FONT_ER_FLT_SOLARIZE,             OnEditRangeFilterSolarize)
//  ON_COMMAND(ID_FONT_ER_SETCOLORALPHA,         OnEditRangeSetColorAlpha)
//  ON_COMMAND(ID_FONT_ER_SCALEALPHA,            OnEditRangeScaleAlpha)

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
      for (int i = 0; i < m_Font.GetNumCharacters(); i++) {
        m_Font.GetCharacter(i).Resize(DEFAULT_CHARACTER_WIDTH, DEFAULT_CHARACTER_HEIGHT);
      }

      SetSaved(false);
      SetModified(false);
    }
  } else {

    m_Font.SetNumCharacters(256);
    for (int i = 0; i < m_Font.GetNumCharacters(); i++) {
      m_Font.GetCharacter(i).Resize(DEFAULT_CHARACTER_WIDTH, DEFAULT_CHARACTER_HEIGHT);
    }

    SetSaved(false);
    SetModified(false);
  }

  m_DocumentType = WA_FONT;

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
	
#ifdef USE_SIZECBAR
		LoadPaletteStates();
#endif
}

////////////////////////////////////////////////////////////////////////////////

void
CFontWindow::UpdateWindowTitle()
{
  char title[520];
  strcpy(title, GetDocumentTitle());

  char append[520];
  sprintf(append, " - %d/%d - %c",
    m_CurrentCharacter,
    m_Font.GetNumCharacters() - 1,
    (unsigned char)m_CurrentCharacter
    );
  strcat(title, append);

  SetCaption(title);
}

////////////////////////////////////////////////////////////////////////////////

void
CFontWindow::SetImage()
{
  sFontCharacter& c = m_Font.GetCharacter(m_CurrentCharacter);
  m_ImageView.SetImage(c.GetWidth(), c.GetHeight(), c.GetPixels(), true);
  m_ScrollBar.SetScrollPos(m_CurrentCharacter);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CFontWindow::OnChar(UINT c, UINT repeat, UINT flags)
{
  if (c >= MIN_CHARACTER && c <= MAX_CHARACTER) {
    m_CurrentCharacter = c;
    SetImage();
    UpdateWindowTitle();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CFontWindow::OnKeyDown(UINT vk, UINT repeat, UINT flags)
{
  int old_char = m_CurrentCharacter;
  if (vk == VK_RIGHT) {
    m_CurrentCharacter = std::min(m_CurrentCharacter + 1, MAX_CHARACTER);
  } else if (vk == VK_LEFT) {
    m_CurrentCharacter = std::max(m_CurrentCharacter - 1, MIN_CHARACTER);
  }

  if (m_CurrentCharacter != old_char) {
    SetImage();
    UpdateWindowTitle();
  }
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
	dialog.SetRange(1, 4096, 1, 4096);

  if (dialog.DoModal() == IDOK)
  {
    if (dialog.GetWidth() > 0 && dialog.GetHeight() > 0
		 && dialog.GetWidth() <= 4096 && dialog.GetHeight() <= 4096)
    {
			if ( !(dialog.GetWidth() == c.GetWidth() && dialog.GetHeight() == c.GetHeight()) ) {
        c.Resize(dialog.GetWidth(), dialog.GetHeight());
        SetModified(true);
        SetImage();
			}
    }
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
	dialog.SetRange(1, 4096, 1, 4096);

  if (dialog.DoModal() == IDOK) {

    if (dialog.GetWidth() > 0 && dialog.GetHeight() > 0
		 && dialog.GetWidth() <= 4096 && dialog.GetHeight() <= 4096)
    {

			bool modified = false;

      for (int i = 0; i < m_Font.GetNumCharacters(); i++) {

				sFontCharacter& c = m_Font.GetCharacter(i);
				modified |= ( !(dialog.GetWidth() == c.GetWidth() && dialog.GetHeight() == c.GetHeight()) );
        c.Resize(dialog.GetWidth(), dialog.GetHeight());
      }

			if (modified) {
        SetModified(true);
        SetImage();
			}
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CFontWindow::OnFontSimplify()
{
  if (MessageBox("This will convert the entire font to opaque and transparent.\nIs this okay?", NULL, MB_YESNO) == IDYES)
  {
	  bool modified = false;
    for (int i = 0; i < m_Font.GetNumCharacters(); i++)
    {
      sFontCharacter& c = m_Font.GetCharacter(i);
      int j;

      if (!modified) {
        for (j = 0; j < c.GetWidth() * c.GetHeight(); j++) {
          if (c.GetPixels()[j].alpha != 0
           && c.GetPixels()[j].alpha != 255) {
            modified = true;
            break;
          }
        }
      }

      if (modified) {
        for (j = 0; j < c.GetWidth() * c.GetHeight(); j++)
        {
          if (c.GetPixels()[j].alpha < 128)
            c.GetPixels()[j].alpha = 0;
          else
            c.GetPixels()[j].alpha = 255;
        }
      }
    }

    if (modified) {
      SetModified(true);
      SetImage();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CFontWindow::OnFontMakeColorTransparent() {
  RGB color = m_ColorView.GetColor();
  bool modified = false;
  for (int i = 0; i < m_Font.GetNumCharacters(); ++i) {
    sFontCharacter& c = m_Font.GetCharacter(i);
    for (int j = 0; j < c.GetWidth() * c.GetHeight(); ++j) {
      if (c.GetPixels()[j].red   == color.red &&
          c.GetPixels()[j].green == color.green &&
          c.GetPixels()[j].blue  == color.blue)
      {
        if (c.GetPixels()[j].alpha != 0) {
          c.GetPixels()[j].alpha = 0;
          modified = true;
        }
      }
    }
  }

  if (modified) {
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

afx_msg void
CFontWindow::OnFontExportToImage()
{
  CImageFileDialog FileDialog(FDM_SAVE);
  if (FileDialog.DoModal() != IDOK)
    return;

  int font_width = 0;
  int font_height = 0;

  for (int i = 0; i < m_Font.GetNumCharacters(); i++) {
    if (font_width < m_Font.GetCharacter(i).GetWidth())
      font_width = m_Font.GetCharacter(i).GetWidth();
    if (font_height < m_Font.GetCharacter(i).GetHeight())
      font_height = m_Font.GetCharacter(i).GetHeight();
  }

  if (!(font_width > 0 && font_height > 0))
    return;

  int border_size = 0;
  if (MessageBox("Use a 1 pixel border?", "Font", MB_ICONQUESTION | MB_YESNO) == IDYES) {
    border_size = 1;
  }

  int num_characters_per_row = 16;
  int num_characters_per_col = 16;

  CImage32 image;
  if (!image.Create(font_width * num_characters_per_row + ((num_characters_per_row + 1) * border_size),
                    font_height * num_characters_per_col + ((num_characters_per_col + 1) * border_size)))
    return;

  image.Rectangle(0, 0, image.GetWidth(), image.GetHeight(), CreateRGBA(255, 0, 0, 255));
  image.SetBlendMode(CImage32::REPLACE);

  int i = 0;

  for (int fy = 0; fy < num_characters_per_col; fy++)
  {
    for (int fx = 0; fx < num_characters_per_row; fx++)
    {
      CImage32 c = m_Font.GetCharacter(i);
      int x = ((fx + 1) * border_size) + (font_width  * fx);
      int y = ((fy + 1) * border_size) + (font_height * fy);
      //image.Rectangle(x, y, x + font_width - 1, y + font_height - 1, CreateRGBA(255, 255, 0, 255));
      image.BlitImage(c, x, y);
      i += 1;
    }
  }

  if (!image.Save(FileDialog.GetPathName())) {
    MessageBox("Could not save image");
  }
  else {
    MessageBox("Exported font!");
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

afx_msg void
CFontWindow::OnEditRange()
{
  const unsigned int id = GetCurrentMessage()->wParam;
  if (EditRange::OnEditRange("font", id, false, (void*) &m_Font, m_CurrentCharacter)) {
    SetModified(true);
    SetImage();
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CFontWindow::IsToolCommandAvailable(UINT id)
{
  return m_ImageView.IsToolAvailable(id);
}

////////////////////////////////////////////////////////////////////////////////

void
CFontWindow::OnToolCommand(UINT id)
{
  m_ImageView.OnToolChanged(id);
}

////////////////////////////////////////////////////////////////////////////////
