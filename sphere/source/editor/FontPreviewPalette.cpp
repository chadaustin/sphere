#include "FontPreviewPalette.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
#include "resource.h"
#include "StringDialog.hpp"

BEGIN_MESSAGE_MAP(CFontPreviewPalette, CPaletteWindow)

  ON_WM_PAINT()   
  ON_WM_RBUTTONUP()

  ON_COMMAND(ID_FILE_ZOOM_IN,  OnZoomIn)
  ON_COMMAND(ID_FILE_ZOOM_OUT, OnZoomOut)

  ON_COMMAND(ID_FONT_PREVIEW_SET_TEXT, OnSetText)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CFontPreviewPalette::CFontPreviewPalette(CDocumentWindow* owner, sFont* font)
: CPaletteWindow(owner, "Font Preview",
  Configuration::Get(KEY_FONT_PREVIEW_RECT),
  Configuration::Get(KEY_FONT_PREVIEW_VISIBLE))
,  m_Font(font)
, m_ZoomFactor(1)
, m_RedrawCharacter(-1)
, m_BlitImage(NULL)
{
  m_Text = "The quick brown fox jumped over the lazy dog";
  OnZoom(2);
}

////////////////////////////////////////////////////////////////////////////////

void
CFontPreviewPalette::OnCharacterChanged(int character)
{
	m_RedrawCharacter = character;
	Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CFontPreviewPalette::OnSetText()
{
  CStringDialog dialog("Set Preview Text", m_Text.c_str());
  if (dialog.DoModal()) {
    m_Text = dialog.GetValue();
    Invalidate();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CFontPreviewPalette::Destroy()
{
  delete m_BlitImage;
  m_BlitImage = NULL;

  // save state
  RECT rect;
  GetWindowRect(&rect);
  Configuration::Set(KEY_FONT_PREVIEW_RECT, rect);
  // FIXME: IsWindowVisible() always returns FALSE here
  //Configuration::Set(KEY_TILE_PREVIEW_VISIBLE, IsWindowVisible() != FALSE);

  // destroy window
  DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CFontPreviewPalette::OnPaint()
{
  CPaintDC dc(this);

	RECT ClientRect;
	GetClientRect(&ClientRect);

  if (!m_BlitImage || m_BlitImage->GetPixels() == NULL) {
    // draw black rectangle
    dc.FillRect(&ClientRect, CBrush::FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH)));
    return;
  }

  int blit_width  = m_BlitImage->GetWidth();
  int blit_height = m_BlitImage->GetHeight();

  int offset_x = 0;
  const char* text = m_Text.c_str();

	for (int i = 0; i < int(strlen(text)); i++)
  {
    if ((int)text[i] < 0 || (int)text[i] >= m_Font->GetNumCharacters())
      continue;

    const CImage32& image = m_Font->GetCharacter(text[i]);

   	if (text[i] == m_RedrawCharacter || m_RedrawCharacter == -1)
    {
		  // draw the frame
		  // fill the DIB section
      BGRA* pixels = (BGRA*)m_BlitImage->GetPixels();
      
      // make a checkerboard
      for (int iy = 0; iy < blit_height; iy++)
        for (int ix = 0; ix < blit_width; ix++)
        {
          pixels[iy * blit_width + ix] = 
            ((ix / 8 + iy / 8) % 2 ?
              CreateBGRA(255, 0, 0, 255) :
              CreateBGRA(255, 0, 0, 255));
        }

      // draw the frame into it
		  const RGBA* source = image.GetPixels();
      for (int iy = 0; iy < blit_height; iy++) {
        for (int ix = 0; ix < blit_width; ix++)
        {
          int ty = iy / m_ZoomFactor;
          int tx = ix / m_ZoomFactor;

          // this here would crash if the tileset has been resized
          // and the spriteset animation palette hasn't been informed of the resize
          if (tx >= 0 && tx < image.GetWidth()
            && ty >= 0 && ty < image.GetHeight()) {
   
            int t = (ty * image.GetWidth()) + tx;    
            int d = (iy * blit_width) + ix;
            int alpha = source[t].alpha;

            pixels[d].red   = (source[t].red   * alpha + pixels[d].red   * (255 - alpha)) / 256;
            pixels[d].green = (source[t].green * alpha + pixels[d].green * (255 - alpha)) / 256;
            pixels[d].blue  = (source[t].blue  * alpha + pixels[d].blue  * (255 - alpha)) / 256;
          }
        }
      }
      
      // blit the frame
      CDC* tile = CDC::FromHandle(m_BlitImage->GetDC());
      dc.BitBlt(ClientRect.left + offset_x,
			          ClientRect.top,
                ClientRect.right - ClientRect.left,
						  	ClientRect.bottom - ClientRect.top,
                tile, 0, 0, SRCCOPY);
    }

    offset_x += image.GetWidth() * m_ZoomFactor;
  }

  // draw black rectangle around tile
  if (1) {
    RECT rect = ClientRect;
    rect.left += offset_x;
    dc.FillRect(&rect, CBrush::FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH)));
    rect.left -= offset_x;
    rect.top += blit_height;
    dc.FillRect(&rect, CBrush::FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH)));
    rect.top -= blit_height;
  }

  m_RedrawCharacter = -1;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CFontPreviewPalette::OnRButtonUp(UINT flags, CPoint point)
{
  // show pop-up menu
  ClientToScreen(&point);

  HMENU menu = ::LoadMenu(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_FONT_PREVIEW));
  HMENU submenu = GetSubMenu(menu, 0);

  TrackPopupMenu(submenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, m_hWnd, NULL);
  DestroyMenu(menu);
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CFontPreviewPalette::OnZoom(double zoom) {
  m_ZoomFactor = zoom;

  int width  = 0;
	int height = 0; 

  if (m_BlitImage != NULL)
    delete m_BlitImage;

  for (int i = 0; i < m_Font->GetNumCharacters(); i++) {   
    if (width < m_Font->GetCharacter(i).GetWidth())
      width = m_Font->GetCharacter(i).GetWidth();
    if (height < m_Font->GetCharacter(i).GetHeight())
      height = m_Font->GetCharacter(i).GetHeight();
  }

  if (width > 0 && height > 0) {
    width  *= m_ZoomFactor;
    height *= m_ZoomFactor;

    if (width > 0 && height > 0) {
      m_BlitImage = new CDIBSection(width, height, 32);
    }
  }

  m_RedrawCharacter = -1;

  Invalidate();
}

//////////////////////////////////////////////////////////////////////////////

afx_msg void
CFontPreviewPalette::OnZoomIn()
{
  switch ((int)m_ZoomFactor) {
    case 1: OnZoom(2); break;
    case 2: OnZoom(4); break;
    case 4: OnZoom(8); break;
  }
}

//////////////////////////////////////////////////////////////////////////////

afx_msg void
CFontPreviewPalette::OnZoomOut()
{
  switch ((int)m_ZoomFactor) {
    case 2: OnZoom(1); break;
    case 4: OnZoom(2); break;
    case 8: OnZoom(4); break;
  }
}

//////////////////////////////////////////////////////////////////////////////
