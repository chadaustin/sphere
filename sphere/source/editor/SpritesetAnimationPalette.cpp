#include "SpritesetAnimationPalette.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
#include "resource.h"

const int ANIMATION_TIMER = 9001;

BEGIN_MESSAGE_MAP(CSpritesetAnimationPalette, CPaletteWindow)

  ON_WM_PAINT()   
  ON_WM_RBUTTONUP()
  ON_WM_TIMER()

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CSpritesetAnimationPalette::CSpritesetAnimationPalette(CDocumentWindow* owner, sSpriteset* spriteset)
: CPaletteWindow(owner, "Spriteset Animation",
  Configuration::Get(KEY_SPRITESET_ANIMATION_RECT),
  Configuration::Get(KEY_SPRITESET_ANIMATION_VISIBLE))
, m_Spriteset(spriteset)
, m_SelectedDirection(0) //m_SelectedDirection(-1)
, m_ZoomFactor(1)
, m_BlitImage(NULL)
{
  OnZoom(1);
	
	//make fixed size
	//TODO: Find out why a non resizing border causes funny results in painting
	//ModifyStyle(WS_THICKFRAME, 0, 0);	
	
	//init the animation timer
  m_Timer = SetTimer(ANIMATION_TIMER, Configuration::Get(KEY_ANIMATION_DELAY), NULL);
	ResetAnimation();
}

void
CSpritesetAnimationPalette::SetCurrentDirection(int direction)
{
	//only restart animation if it's really a new direction
	if (m_SelectedDirection == direction) return;

	m_SelectedDirection = direction;
	ResetAnimation();
	Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetAnimationPalette::Destroy()
{
  delete m_BlitImage;

  // save state
  RECT rect;
  GetWindowRect(&rect);
  Configuration::Set(KEY_SPRITESET_ANIMATION_RECT, rect);

  Configuration::Set(KEY_SPRITESET_ANIMATION_VISIBLE, IsWindowVisible() != FALSE);

  // destroy window
  DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetAnimationPalette::OnPaint()
{
  CPaintDC dc(this);

	RECT Rect;
	GetClientRect(&Rect);

  int blit_width  = m_BlitImage->GetWidth();
  int blit_height = m_BlitImage->GetHeight();

  // draw black rectangle
  dc.FillRect(&Rect, CBrush::FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH)));

	if (m_SelectedDirection != -1)
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

    // draw the tile into it
		int img =  m_Spriteset->GetFrameIndex(m_SelectedDirection, m_CurrentFrame);
    RGBA* tilepixels = m_Spriteset->GetImage(img).GetPixels();
    for (int iy = 0; iy < blit_height; iy++)
      for (int ix = 0; ix < blit_width; ix++)
      {
        int ty = iy / m_ZoomFactor;
        int tx = ix / m_ZoomFactor;
        int t = ty * m_Spriteset->GetFrameWidth() + tx;
        
        int d = iy * blit_width + ix;

        // this here would crash if the spriteset has been resized
        // and the spriteset animation palette hasn't been informed of the resize
        if (tx >= 0 && tx < m_Spriteset->GetFrameWidth()
          && ty >= 0 && ty < m_Spriteset->GetFrameHeight()) {
          int alpha = tilepixels[t].alpha;
          pixels[d].red   = (tilepixels[t].red   * alpha + pixels[d].red   * (255 - alpha)) / 256;
          pixels[d].green = (tilepixels[t].green * alpha + pixels[d].green * (255 - alpha)) / 256;
          pixels[d].blue  = (tilepixels[t].blue  * alpha + pixels[d].blue  * (255 - alpha)) / 256;
        }
			}
      
    // blit the frame
    CDC* tile = CDC::FromHandle(m_BlitImage->GetDC());
    dc.BitBlt(Rect.left, Rect.top, Rect.right - Rect.left, Rect.bottom - Rect.top, tile, 0, 0, SRCCOPY);
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetAnimationPalette::OnRButtonUp(UINT flags, CPoint point)
{
	//no menu needed for now
/*  // show pop-up menu
  ClientToScreen(&point);

  HMENU menu = ::LoadMenu(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_SPRITESET_ANIMATION_PALETTE));
  HMENU submenu = GetSubMenu(menu, 0);

  TrackPopupMenu(submenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, m_hWnd, NULL);
  DestroyMenu(menu);*/
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetAnimationPalette::OnZoom(double zoom) {
  m_ZoomFactor = zoom;

  if (m_BlitImage != NULL)
    delete m_BlitImage;

	int width  = m_Spriteset->GetFrameWidth() * m_ZoomFactor;
	int height = m_Spriteset->GetFrameHeight() * m_ZoomFactor; 

  m_BlitImage = new CDIBSection(width, height, 32);

	RECT rect;
	rect.left = 0;
	rect.top  = 0;
	rect.right = width;
	rect.bottom = height;
	AdjustWindowRect(&rect, GetStyle(), FALSE);
	SetWindowPos(NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
  Invalidate();
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetAnimationPalette::OnTimer(UINT event)
{
	if (m_SelectedDirection != -1)
	
	if (m_TicksLeft == 0)
	{
		m_CurrentFrame++;
		
		if (m_CurrentFrame >= m_Spriteset->GetNumFrames(m_SelectedDirection))
		{
			m_CurrentFrame = 0;
		}

		m_TicksLeft = m_Spriteset->GetFrameDelay(m_SelectedDirection, m_CurrentFrame);
		Invalidate();
	}
	else
	{
		m_TicksLeft--;
	}
}

//////////////////////////////////////////////////////////////////////////////

void
CSpritesetAnimationPalette::ResetAnimation()
{
	if (m_SelectedDirection != -1)
	{
		m_CurrentFrame = 0;
		m_TicksLeft = m_Spriteset->GetFrameDelay(m_SelectedDirection, m_CurrentFrame);
		Invalidate();
	}
}
