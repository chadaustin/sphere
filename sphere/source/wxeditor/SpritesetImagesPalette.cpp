#include "SpritesetImagesPalette.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
//#include "resource.h"
#include "IDs.hpp"

/*
BEGIN_MESSAGE_MAP(CSpritesetImagesPalette, CPaletteWindow)

  ON_WM_SIZE()
  ON_WM_PAINT()
  ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONUP()

  ON_COMMAND(ID_SPRITESETIMAGESPALETTE_MOVE_BACK,    OnMoveBack)
  ON_COMMAND(ID_SPRITESETIMAGESPALETTE_MOVE_FORWARD, OnMoveForward)
  ON_COMMAND(ID_SPRITESETIMAGESPALETTE_INSERT_IMAGE, OnInsertImage)
  ON_COMMAND(ID_SPRITESETIMAGESPALETTE_REMOVE_IMAGE, OnRemoveImage)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wSpritesetImagesPalette, wPaletteWindow)

  EVT_SIZE(wSpritesetImagesPalette::OnSize)
  EVT_PAINT(wSpritesetImagesPalette::OnPaint)
  EVT_LEFT_DOWN(wSpritesetImagesPalette::OnLButtonDown)
  EVT_RIGHT_UP(wSpritesetImagesPalette::OnRButtonUp)

  EVT_MENU(wID_SPRITESETIMAGESPALETTE_MOVE_BACK,     wSpritesetImagesPalette::OnMoveBack)
  EVT_MENU(wID_SPRITESETIMAGESPALETTE_MOVE_FORWARD,  wSpritesetImagesPalette::OnMoveForward)
  EVT_MENU(wID_SPRITESETIMAGESPALETTE_INSERT_IMAGE,  wSpritesetImagesPalette::OnInsertImage)
  EVT_MENU(wID_SPRITESETIMAGESPALETTE_REMOVE_IMAGE,  wSpritesetImagesPalette::OnRemoveImage)


END_EVENT_TABLE()


////////////////////////////////////////////////////////////////////////////////

wSpritesetImagesPalette::wSpritesetImagesPalette(wDocumentWindow* owner, ISpritesetImagesPaletteHandler* handler, sSpriteset* spriteset)
: wPaletteWindow(owner, "Spriteset Images",
  Configuration::Get(KEY_SPRITESET_IMAGES_RECT),
  Configuration::Get(KEY_SPRITESET_IMAGES_VISIBLE))
, m_Handler(handler)
, m_Spriteset(spriteset)
, m_TopRow(0)
, m_ZoomFactor(1)
, m_SelectedImage(0)
{
  m_BlitImage = new wDIBSection(
    spriteset->GetFrameWidth(),
    spriteset->GetFrameHeight()  
  );
  m_Menu = new wxMenu();
  
  m_Menu->Append(wID_SPRITESETIMAGESPALETTE_MOVE_BACK,     "Move Back");
  m_Menu->Append(wID_SPRITESETIMAGESPALETTE_MOVE_FORWARD,  "Move Forward");
  m_Menu->Append(wID_SPRITESETIMAGESPALETTE_INSERT_IMAGE,  "Insert Image");
  m_Menu->Append(wID_SPRITESETIMAGESPALETTE_REMOVE_IMAGE,  "Remove Image");
}

////////////////////////////////////////////////////////////////////////////////

bool
wSpritesetImagesPalette::Destroy()
{
  delete m_BlitImage;
  delete m_Menu;
  // save state
  wxRect rect = GetRect();
  Configuration::Set(KEY_SPRITESET_IMAGES_RECT, rect);

  Configuration::Set(KEY_SPRITESET_IMAGES_VISIBLE, true); /*todo:IsWindowVisible() != FALSE);*/

  // destroy window
  return wPaletteWindow::Destroy();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetImagesPalette::OnSize(wxSizeEvent &event)
{
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetImagesPalette::OnPaint(wxPaintEvent &event)
{
  wxPaintDC dc(this);

  wxSize client_size(GetClientSize());
  wxRegion region = GetUpdateRegion();

  int blit_width  = m_BlitImage->GetWidth();
  int blit_height = m_BlitImage->GetHeight();

  for (int iy = 0; iy < client_size.GetHeight() / blit_height + 1; iy++)
    for (int ix = 0; ix < client_size.GetWidth() / blit_width + 1; ix++)
    {
      wxRect Rect(
        ix       * blit_width,
        iy       * blit_height,
        (ix + 1) * blit_width,
        (iy + 1) * blit_height
      );
      if(region.Contains(Rect) == wxOutRegion) {
        continue;
      }
      int num_tiles_x = client_size.GetWidth() / blit_width;

      int it = (iy + m_TopRow) * (client_size.GetWidth() / blit_width) + ix;
      if (ix < num_tiles_x && it < m_Spriteset->GetNumImages())
      {
        // draw the tile
        // fill the DIB section
        RGB* pixels = (RGB*)m_BlitImage->GetPixels();
        
        // make a checkerboard
        for (int iy = 0; iy < blit_height; iy++)
          for (int ix = 0; ix < blit_width; ix++)
          {
            pixels[iy * blit_width + ix] = 
              ((ix / 8 + iy / 8) % 2 ?
                CreateRGB(255, 255, 255) :
                CreateRGB(192, 192, 255));
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
        wxBitmap bitmap = m_BlitImage->GetImage()->ConvertToBitmap();
        dc.DrawBitmap(bitmap, ix * blit_width, iy * blit_height, FALSE);

        // if the tile is selected, draw a pink rectangle around it
        if (it == m_SelectedImage)
        {
          /*
          HBRUSH newbrush = (HBRUSH)GetStockObject(NULL_BRUSH);
          CBrush* oldbrush = dc.SelectObject(CBrush::FromHandle(newbrush));
          HPEN newpen = (HPEN)CreatePen(PS_SOLID, 1, RGB(0xFF, 0x00, 0xFF));
          CPen* oldpen = dc.SelectObject(CPen::FromHandle(newpen));

          dc.Rectangle(&Rect);

          dc.SelectObject(oldbrush);
          DeleteObject(newbrush);
          dc.SelectObject(oldpen);
          DeleteObject(newpen);
          */
          dc.SetBrush(wxBrush(wxColour(0xff, 0x00, 0xff), wxTRANSPARENT));
          dc.SetPen(wxPen(wxColour(0xff, 0x00, 0xff), 1, wxSOLID));
          dc.DrawRectangle(ix * blit_width, iy * blit_height, blit_width, blit_height);

          dc.SetBrush(wxNullBrush);
          dc.SetPen(wxNullPen);
        }

      }
      else
      {
        // draw black rectangle
        //dc.FillRect(&Rect, CBrush::FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH)));
        dc.SetBrush(wxBrush(wxColour(0x00, 0x00, 0x00), wxSOLID));
        dc.SetPen(wxPen(wxColour(0x00, 0x00, 0x00), 1, wxSOLID));
        dc.DrawRectangle(Rect);

        dc.SetBrush(wxNullBrush);
        dc.SetPen(wxNullPen);
      }
      
    }

}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetImagesPalette::OnLButtonDown(wxMouseEvent &event)
{
  wxSize client_size(GetClientSize());
  int num_images_x = client_size.GetWidth() / m_BlitImage->GetWidth();

  int col = event.GetX() / m_BlitImage->GetWidth();
  int row = event.GetY() / m_BlitImage->GetHeight();

  if(col == 0) {
    col = 1;
  }
  // don't let user select tile off the right edge (and go to the next row)
  if (col >= num_images_x) {
    return;
  }

  int image = (m_TopRow + row) * num_images_x + col;
  if (image >= 0 && image < m_Spriteset->GetNumImages())
    m_SelectedImage = image;

  Refresh();

  // the selected tile changed, so tell the parent window
  m_Handler->SIP_IndexChanged(m_SelectedImage);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetImagesPalette::OnRButtonUp(wxMouseEvent &event)
{
  // select the image
  OnLButtonDown(event);

  // show pop-up menu
  //ClientToScreen(&point);

  //HMENU menu = ::LoadMenu(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_SPRITESET_IMAGES_PALETTE));

  // disable move back if we're on the first image
  if (m_SelectedImage == 0) {
    //::EnableMenuItem(menu, ID_SPRITESETIMAGESPALETTE_MOVE_BACK, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    m_Menu->Enable(wID_SPRITESETIMAGESPALETTE_MOVE_BACK, FALSE);
  } else {
    m_Menu->Enable(wID_SPRITESETIMAGESPALETTE_MOVE_BACK, TRUE);
  }

  // disable move forward if we're on the last image
  if (m_SelectedImage == m_Spriteset->GetNumImages() - 1) {
    //::EnableMenuItem(menu, ID_SPRITESETIMAGESPALETTE_MOVE_FORWARD, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    m_Menu->Enable(wID_SPRITESETIMAGESPALETTE_MOVE_FORWARD, FALSE);
  } else {
    m_Menu->Enable(wID_SPRITESETIMAGESPALETTE_MOVE_FORWARD, TRUE);
  }

  // disable remove image if there is only one
  if (m_Spriteset->GetNumImages() == 1) {
    //::EnableMenuItem(menu, ID_SPRITESETIMAGESPALETTE_REMOVE_IMAGE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    m_Menu->Enable(wID_SPRITESETIMAGESPALETTE_REMOVE_IMAGE, FALSE);
  } else {
    m_Menu->Enable(wID_SPRITESETIMAGESPALETTE_REMOVE_IMAGE, TRUE);
  }

  //TrackPopupMenu(GetSubMenu(menu, 0), TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, m_hWnd, NULL);
  //DestroyMenu(menu);
  PopupMenu(m_Menu, event.GetPosition());
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetImagesPalette::OnMoveBack(wxEvent &event)
{
  if (m_SelectedImage > 0) {
    OnSwap(m_SelectedImage - 1);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetImagesPalette::OnMoveForward(wxEvent &event)
{
  if (m_SelectedImage < m_Spriteset->GetNumImages() - 1) {
    OnSwap(m_SelectedImage + 1);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetImagesPalette::OnInsertImage(wxEvent &event)
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
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetImagesPalette::OnRemoveImage(wxEvent &event)
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
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetImagesPalette::OnSwap(int new_index)
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
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////
