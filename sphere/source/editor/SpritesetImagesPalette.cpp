#include "SpritesetImagesPalette.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
#include "resource.h"


BEGIN_MESSAGE_MAP(CSpritesetImagesPalette, CPaletteWindow)

  ON_WM_SIZE()
  ON_WM_PAINT()
  ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONUP()

  ON_COMMAND(ID_SPRITESETIMAGESPALETTE_MOVEBACK,    OnMoveBack)
  ON_COMMAND(ID_SPRITESETIMAGESPALETTE_MOVEFORWARD, OnMoveForward)
  ON_COMMAND(ID_SPRITESETIMAGESPALETTE_INSERTIMAGE, OnInsertImage)
  ON_COMMAND(ID_SPRITESETIMAGESPALETTE_REMOVEIMAGE, OnRemoveImage)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CSpritesetImagesPalette::CSpritesetImagesPalette(CDocumentWindow* owner, ISpritesetImagesPaletteHandler* handler, sSpriteset* spriteset)
: CPaletteWindow(owner, "Spriteset Images",
  Configuration::Get(KEY_SPRITESET_IMAGES_RECT),
  Configuration::Get(KEY_SPRITESET_IMAGES_VISIBLE))
, m_Handler(handler)
, m_Spriteset(spriteset)
, m_TopRow(0)
, m_ZoomFactor(1)
, m_SelectedImage(0)
{
  m_BlitImage = new CDIBSection(
    spriteset->GetFrameWidth(),
    spriteset->GetFrameHeight(),
    32
  );
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetImagesPalette::Destroy()
{
  delete m_BlitImage;

  // save state
  RECT rect;
  GetWindowRect(&rect);
  Configuration::Set(KEY_SPRITESET_IMAGES_RECT, rect);

  Configuration::Set(KEY_SPRITESET_IMAGES_VISIBLE, IsWindowVisible() != FALSE);

  // destroy window
  DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetImagesPalette::OnSize(UINT type, int cx, int cy)
{
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetImagesPalette::OnPaint()
{
  CPaintDC dc(this);

  RECT client_rect;
  GetClientRect(&client_rect);

  int blit_width  = m_BlitImage->GetWidth();
  int blit_height = m_BlitImage->GetHeight();

  for (int iy = 0; iy < client_rect.bottom / blit_height + 1; iy++)
    for (int ix = 0; ix < client_rect.right / blit_width + 1; ix++)
    {
      RECT Rect = {
        ix       * blit_width,
        iy       * blit_height,
        (ix + 1) * blit_width,
        (iy + 1) * blit_height,
      };
      if (dc.RectVisible(&Rect) == FALSE)
        continue;
      
      int num_tiles_x = client_rect.right / blit_width;

      int it = (iy + m_TopRow) * (client_rect.right / blit_width) + ix;
      if (ix < num_tiles_x && it < m_Spriteset->GetNumImages())
      {
        // draw the tile
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
        RGBA* tilepixels = m_Spriteset->GetImage(it).GetPixels();
        for (int iy = 0; iy < blit_height; iy++)
          for (int ix = 0; ix < blit_width; ix++)
          {
            int ty = iy / m_ZoomFactor;
            int tx = ix / m_ZoomFactor;
            int t = ty * m_Spriteset->GetFrameWidth() + tx;
            
            int d = iy * blit_width + ix;

            int alpha = tilepixels[t].alpha;
            pixels[d].red   = (tilepixels[t].red   * alpha + pixels[d].red   * (255 - alpha)) / 256;
            pixels[d].green = (tilepixels[t].green * alpha + pixels[d].green * (255 - alpha)) / 256;
            pixels[d].blue  = (tilepixels[t].blue  * alpha + pixels[d].blue  * (255 - alpha)) / 256;
          }
        
        // blit the tile
        CDC* tile = CDC::FromHandle(m_BlitImage->GetDC());
        dc.BitBlt(Rect.left, Rect.top, Rect.right - Rect.left, Rect.bottom - Rect.top, tile, 0, 0, SRCCOPY);

        // if the tile is selected, draw a pink rectangle around it
        if (it == m_SelectedImage)
        {
          HBRUSH newbrush = (HBRUSH)GetStockObject(NULL_BRUSH);
          CBrush* oldbrush = dc.SelectObject(CBrush::FromHandle(newbrush));
          HPEN newpen = (HPEN)CreatePen(PS_SOLID, 1, RGB(0xFF, 0x00, 0xFF));
          CPen* oldpen = dc.SelectObject(CPen::FromHandle(newpen));

          dc.Rectangle(&Rect);

          dc.SelectObject(oldbrush);
          DeleteObject(newbrush);
          dc.SelectObject(oldpen);
          DeleteObject(newpen);
        }

      }
      else
      {
        // draw black rectangle
        dc.FillRect(&Rect, CBrush::FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH)));
      }
      
    }

}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetImagesPalette::OnLButtonDown(UINT flags, CPoint point)
{
  RECT client_rect;
  GetClientRect(&client_rect);
  int num_images_x = client_rect.right / m_BlitImage->GetWidth();

  int col = point.x / m_BlitImage->GetWidth();
  int row = point.y / m_BlitImage->GetHeight();

  // don't let user select tile off the right edge (and go to the next row)
  if (col >= num_images_x) {
    return;
  }

  int image = (m_TopRow + row) * num_images_x + col;
  if (image >= 0 && image < m_Spriteset->GetNumImages())
    m_SelectedImage = image;

  Invalidate();

  // the selected tile changed, so tell the parent window
  m_Handler->SIP_IndexChanged(m_SelectedImage);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetImagesPalette::OnRButtonUp(UINT flags, CPoint point)
{
  // select the image
  OnLButtonDown(flags, point);

  // show pop-up menu
  ClientToScreen(&point);

  HMENU menu_ = ::LoadMenu(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_SPRITESET_IMAGES_PALETTE));
  HMENU menu = GetSubMenu(menu_, 0);

  // disable move back if we're on the first image
  if (m_SelectedImage == 0) {
    EnableMenuItem(menu, ID_SPRITESETIMAGESPALETTE_MOVEBACK, MF_BYCOMMAND | MF_GRAYED);
  }

  // disable move forward if we're on the last image
  if (m_SelectedImage == m_Spriteset->GetNumImages() - 1) {
    EnableMenuItem(menu, ID_SPRITESETIMAGESPALETTE_MOVEFORWARD, MF_BYCOMMAND | MF_GRAYED);
  }

  // disable remove image if there is only one
  if (m_Spriteset->GetNumImages() == 1) {
    EnableMenuItem(menu, ID_SPRITESETIMAGESPALETTE_REMOVEIMAGE, MF_BYCOMMAND | MF_GRAYED);
  }

  TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, m_hWnd, NULL);

  DestroyMenu(menu_);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetImagesPalette::OnMoveBack()
{
  OnSwap(m_SelectedImage - 1);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetImagesPalette::OnMoveForward()
{
  OnSwap(m_SelectedImage + 1);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetImagesPalette::OnInsertImage()
{
  m_Spriteset->InsertImage(m_SelectedImage);

  // update indices in the spriteset
  for (int i = 0; i < m_Spriteset->GetNumDirections(); i++) {
    for (int j = 0; j < m_Spriteset->GetNumFrames(i); j++) {
      int k = m_Spriteset->GetFrameIndex(i, j);
      if (k >= m_SelectedImage) {
        m_Spriteset->SetFrameIndex(i, j, k + 1);
      }
    }
  }

  m_SelectedImage++;
  m_Handler->SIP_SpritesetModified();
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetImagesPalette::OnRemoveImage()
{
  m_Spriteset->DeleteImage(m_SelectedImage);

  // update indices in the spriteset
  for (int i = 0; i < m_Spriteset->GetNumDirections(); i++) {
    for (int j = 0; j < m_Spriteset->GetNumFrames(i); j++) {
      int k = m_Spriteset->GetFrameIndex(i, j);
      if (k == m_SelectedImage) {
        m_Spriteset->SetFrameIndex(i, j, 0);
      } else if (k >= m_SelectedImage) {
        m_Spriteset->SetFrameIndex(i, j, k - 1);
      }
    }
  }

  if (m_SelectedImage >= m_Spriteset->GetNumImages()) {
    m_SelectedImage--;
  }
  m_Handler->SIP_SpritesetModified();
  m_Handler->SIP_IndexChanged(m_SelectedImage);
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetImagesPalette::OnSwap(int new_index)
{
  int current = m_SelectedImage;
  
  // convenience
  int one = new_index;
  int two = current;

  // go through the spriteset and changes the indices
  for (int i = 0; i < m_Spriteset->GetNumDirections(); i++) {
    for (int j = 0; j < m_Spriteset->GetNumFrames(i); j++) {
      if (m_Spriteset->GetFrameIndex(i, j) == one) {
        m_Spriteset->SetFrameIndex(i, j, two);
      } else if (m_Spriteset->GetFrameIndex(i, j) == two) {
        m_Spriteset->SetFrameIndex(i, j, one);
      }
    }
  }

  // swap the images
  std::swap(
    m_Spriteset->GetImage(one),
    m_Spriteset->GetImage(two)
  );
  m_SelectedImage = new_index;

  m_Handler->SIP_IndexChanged(m_SelectedImage);
  m_Handler->SIP_SpritesetModified();
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////
