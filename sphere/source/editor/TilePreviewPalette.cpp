#include "TilePreviewPalette.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
#include "resource.h"

BEGIN_MESSAGE_MAP(CTilePreviewPalette, CPaletteWindow)

  ON_WM_PAINT()   
  ON_WM_RBUTTONUP()
  ON_WM_ERASEBKGND()

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CTilePreviewPalette::CTilePreviewPalette(CDocumentWindow* owner, CImage32 image)
: CPaletteWindow(owner, "Tile Preview",
  Configuration::Get(KEY_TILE_PREVIEW_RECT),
  Configuration::Get(KEY_TILE_PREVIEW_VISIBLE))
,  m_Image(image)
, m_ZoomFactor(1)
, m_BlitImage(NULL)
{
  OnZoom(1);
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CTilePreviewPalette::OnEraseBkgnd(CDC* pDC)
{
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void
CTilePreviewPalette::OnImageChanged(CImage32 image)
{
	m_Image = image;
	Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void
CTilePreviewPalette::Destroy()
{
  delete m_BlitImage;

  // save state
  RECT rect;
  GetWindowRect(&rect);
  Configuration::Set(KEY_TILE_PREVIEW_RECT, rect);
  // FIXME: IsWindowVisible() always returns FALSE here
  //Configuration::Set(KEY_TILE_PREVIEW_VISIBLE, IsWindowVisible() != FALSE);

  // destroy window
  DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilePreviewPalette::OnPaint()
{
  CPaintDC dc(this);

	RECT ClientRect;
	GetClientRect(&ClientRect);

  if (!m_BlitImage || m_BlitImage->GetPixels() == NULL
    || m_Image.GetWidth() == 0 || m_Image.GetHeight() == 0 || m_Image.GetPixels() == NULL) {
    // draw black rectangle
    dc.FillRect(&ClientRect, CBrush::FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH)));
    return;
  }

  int blit_width  = m_BlitImage->GetWidth();
  int blit_height = m_BlitImage->GetHeight();

  // draw black rectangle around tile
  if (1) {
    RECT rect = ClientRect;
    rect.left += blit_width * 3;
    dc.FillRect(&rect, CBrush::FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH)));
    rect.left -= blit_width * 3;
    rect.top += blit_height * 3;
    dc.FillRect(&rect, CBrush::FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH)));
    rect.top -= blit_height * 3;
  }

	for (int ty = 0; ty < 3; ty++)
  {
   	for (int tx = 0; tx < 3; tx++)
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
              CreateBGRA(255, 255, 255, 255) :
              CreateBGRA(255, 192, 192, 255));
        }

      // draw the frame into it
		  const RGBA* source = m_Image.GetPixels();
      for (int iy = 0; iy < blit_height; iy++) {
        for (int ix = 0; ix < blit_width; ix++)
        {
          int ty = iy / m_ZoomFactor;
          int tx = ix / m_ZoomFactor;

          // this here would crash if the tileset has been resized
          // and the spriteset animation palette hasn't been informed of the resize
          if (tx >= 0 && tx < m_Image.GetWidth()
            && ty >= 0 && ty < m_Image.GetHeight()) {
   
            int t = (ty * m_Image.GetWidth()) + tx;    
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
      dc.BitBlt(ClientRect.left + (tx * m_Image.GetWidth()) * m_ZoomFactor,
			          ClientRect.top + (ty * m_Image.GetHeight()) * m_ZoomFactor,
                ClientRect.right - ClientRect.left,
						  	ClientRect.bottom - ClientRect.top,
                tile, 0, 0, SRCCOPY);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilePreviewPalette::OnRButtonUp(UINT flags, CPoint point)
{
  /* //no menu needed for now
  // show pop-up menu
  ClientToScreen(&point);

  HMENU menu = ::LoadMenu(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_TILE_PREVIEW_PALETTE));
  HMENU submenu = GetSubMenu(menu, 0);

  TrackPopupMenu(submenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, m_hWnd, NULL);
  DestroyMenu(menu);
  */
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilePreviewPalette::OnZoom(double zoom) {
  m_ZoomFactor = zoom;

  if (m_BlitImage != NULL)
    delete m_BlitImage;

	int width  = m_Image.GetWidth() * m_ZoomFactor;
	int height = m_Image.GetHeight() * m_ZoomFactor; 

  m_BlitImage = new CDIBSection(width, height, 32);

	RECT rect;
	rect.left = 0;
	rect.top  = 0;
	rect.right = width;
	rect.bottom = height;
	/*AdjustWindowRect(&rect, GetStyle(), FALSE);
	SetWindowPos(NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);*/
  Invalidate();
}

//////////////////////////////////////////////////////////////////////////////
