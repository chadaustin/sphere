#include "WindowStylePreviewPalette.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
#include "resource.h"

BEGIN_MESSAGE_MAP(CWindowStylePreviewPalette, CPaletteWindow)

  ON_WM_PAINT()   
  ON_WM_RBUTTONUP()

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CWindowStylePreviewPalette::CWindowStylePreviewPalette(CDocumentWindow* owner, sWindowStyle* windowstyle)
: CPaletteWindow(owner, "WindowStyle Preview",
  Configuration::Get(KEY_WINDOWSTYLE_PREVIEW_RECT),
  Configuration::Get(KEY_WINDOWSTYLE_PREVIEW_VISIBLE))
,  m_WindowStyle(windowstyle)
, m_ZoomFactor(1)
, m_BlitImage(NULL)
{
  OnZoom(1);
}

////////////////////////////////////////////////////////////////////////////////

void
CWindowStylePreviewPalette::OnWindowStyleChanged()
{
	Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void
CWindowStylePreviewPalette::Destroy()
{
  delete m_BlitImage;

  // save state
  RECT rect;
  GetWindowRect(&rect);
//  Configuration::Set(KEY_TILE_PREVIEW_RECT, rect);

//  Configuration::Set(KEY_TILE_PREVIEW_VISIBLE, IsWindowVisible() != FALSE);

  // destroy window
  DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CWindowStylePreviewPalette::OnPaint()
{
  CPaintDC dc(this);

	RECT ClientRect;
	GetClientRect(&ClientRect);

  int blit_width  = m_BlitImage->GetWidth();
  int blit_height = m_BlitImage->GetHeight();

  // draw black rectangle
  dc.FillRect(&ClientRect, CBrush::FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH)));

	for (int ty = 0; ty < 3; ty++)
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
		const int foo[9] = {0, 1, 2,
				                7, 8, 3, 
											  6, 5, 4};

		const CImage32& m_Image = m_WindowStyle->GetBitmap(foo[ty * 3 + tx]);
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

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CWindowStylePreviewPalette::OnRButtonUp(UINT flags, CPoint point)
{

}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CWindowStylePreviewPalette::OnZoom(double zoom) {
  m_ZoomFactor = zoom;

  if (m_BlitImage != NULL)
    delete m_BlitImage;

	int width  = 0;
	int height = 0;

	for (int i = 0; i < 9; i++) {
    if (m_WindowStyle->GetBitmap(0).GetWidth() > width)
      width = m_WindowStyle->GetBitmap(0).GetWidth();
    if (m_WindowStyle->GetBitmap(0).GetHeight() > height)
      height = m_WindowStyle->GetBitmap(0).GetHeight();
	}

  width *= m_ZoomFactor;
  height *= m_ZoomFactor;

  m_BlitImage = new CDIBSection(width, height, 32);

	RECT rect;
	rect.left = 0;
	rect.top  = 0;
	rect.right = width;
	rect.bottom = height;

  Invalidate();
}

//////////////////////////////////////////////////////////////////////////////
