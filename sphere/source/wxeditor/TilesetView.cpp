#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

//#define WXWIN_COMPATIBILITY_EVENT_TYPES 1


#include "TilesetView.hpp"
//#include "NumberDialog.hpp"
#include "TilePropertiesDialog.hpp"
#include "Editor.hpp"
#include "FileDialogs.hpp"
//#include "resource.h"

#include "IDs.hpp"

const int UPDATE_TILEVIEW_TIMER = 200;


//static int s_iTilesetViewID = 1000;

/*
BEGIN_MESSAGE_MAP(CTilesetView, CWnd)

  ON_WM_PAINT()
  ON_WM_SIZE()
  ON_WM_VSCROLL()
  ON_WM_LBUTTONDOWN()
  ON_WM_MOUSEMOVE()
  ON_WM_RBUTTONUP()

  ON_COMMAND(ID_TILESETVIEW_INSERTTILE,    OnInsertTile)
  ON_COMMAND(ID_TILESETVIEW_APPENDTILE,    OnAppendTile)
  ON_COMMAND(ID_TILESETVIEW_DELETETILE,    OnDeleteTile)
  ON_COMMAND(ID_TILESETVIEW_PROPERTIES,    OnTileProperties)
  ON_COMMAND(ID_TILESETVIEW_INSERTTILES,   OnInsertTiles)
  ON_COMMAND(ID_TILESETVIEW_APPENDTILES,   OnAppendTiles)
  ON_COMMAND(ID_TILESETVIEW_DELETETILES,   OnDeleteTiles)
  ON_COMMAND(ID_TILESETVIEW_INSERTTILESET, OnInsertTileset)
  ON_COMMAND(ID_TILESETVIEW_APPENDTILESET, OnAppendTileset)

  ON_COMMAND(ID_TILESETVIEW_ZOOM_1X, OnZoom1x)
  ON_COMMAND(ID_TILESETVIEW_ZOOM_2X, OnZoom2x)
  ON_COMMAND(ID_TILESETVIEW_ZOOM_4X, OnZoom4x)
  ON_COMMAND(ID_TILESETVIEW_ZOOM_8X, OnZoom8x)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wTilesetView, wVScrollWindow)
  EVT_PAINT(wTilesetView::OnPaint)
  EVT_SIZE(wTilesetView::OnSize)
  //EVT_SCROLL(wTilesetView::OnVScroll)

  EVT_LEFT_DOWN(wTilesetView::OnLButtonDown)
  EVT_RIGHT_UP(wTilesetView::OnRButtonUp)
  EVT_MOTION(wTilesetView::OnMouseMove)

  EVT_MENU(wEVT_TILESETVIEW_INSERTTILE,     wTilesetView::OnInsertTile)
  EVT_MENU(wEVT_TILESETVIEW_APPENDTILE,     wTilesetView::OnAppendTile)
  EVT_MENU(wEVT_TILESETVIEW_DELETETILE,     wTilesetView::OnDeleteTile)
  EVT_MENU(wEVT_TILESETVIEW_PROPERTIES,     wTilesetView::OnTileProperties)
  EVT_MENU(wEVT_TILESETVIEW_INSERTTILES,    wTilesetView::OnInsertTiles)
  EVT_MENU(wEVT_TILESETVIEW_APPENDTILES,    wTilesetView::OnAppendTiles)
  EVT_MENU(wEVT_TILESETVIEW_DELETETILES,    wTilesetView::OnDeleteTiles)
  EVT_MENU(wEVT_TILESETVIEW_INSERTTILESET,  wTilesetView::OnInsertTileset)
  EVT_MENU(wEVT_TILESETVIEW_APPENDTILESET,  wTilesetView::OnAppendTileset)

  EVT_MENU(wEVT_TILESETVIEW_ZOOM_1X,        wTilesetView::OnZoom1x)
  EVT_MENU(wEVT_TILESETVIEW_ZOOM_2X,        wTilesetView::OnZoom2x)
  EVT_MENU(wEVT_TILESETVIEW_ZOOM_4X,        wTilesetView::OnZoom4x)
  EVT_MENU(wEVT_TILESETVIEW_ZOOM_8X,        wTilesetView::OnZoom8x)
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wTilesetView::wTilesetView(wxWindow *parent, ITilesetViewHandler* handler, sTileset* tileset)
: wVScrollWindow(parent, -1)
, m_Handler(handler)
, m_Tileset(tileset)
, m_TopRow(0)
, m_SelectedTile(0)
, m_ZoomFactor(1)
, m_BlitTile(NULL)
, m_MenuShown(false)
{
  m_BlitTile = new wDIBSection(
    m_Tileset->GetTileWidth()  * m_ZoomFactor,
    m_Tileset->GetTileHeight() * m_ZoomFactor
  );
  //Create(parent, -1, wxDefaultPosition, wxDefaultSize, wxVSCROLL, wxString("wTilesetView"));
  m_PopupMenu = new wxMenu("TilesetView");
  m_PopupMenu->Append(wEVT_TILESETVIEW_INSERTTILE,    "Insert Tile",      "Insert a single tile before selected tile");
  m_PopupMenu->Append(wEVT_TILESETVIEW_APPENDTILE,    "Append Tile",      "Append a single tile after all tiles");
  m_PopupMenu->Append(wEVT_TILESETVIEW_DELETETILE,    "Delete Tile",      "Delete select tile");
  m_PopupMenu->AppendSeparator();
  m_PopupMenu->Append(wEVT_TILESETVIEW_PROPERTIES,    "Tile Properties",  "Change properties of tile");
  m_PopupMenu->AppendSeparator();
  m_PopupMenu->Append(wEVT_TILESETVIEW_INSERTTILES,   "Insert Tiles",     "Insert several tiles before selected tile");
  m_PopupMenu->Append(wEVT_TILESETVIEW_APPENDTILES,   "Append Tiles",     "Append several tiles after all tiles");
  m_PopupMenu->Append(wEVT_TILESETVIEW_DELETETILES,   "Delete Tiles",     "Delete several tiles");
  m_PopupMenu->AppendSeparator();
  m_PopupMenu->Append(wEVT_TILESETVIEW_INSERTTILESET, "Insert Tileset",   "Load and insert a tileset");
  m_PopupMenu->Append(wEVT_TILESETVIEW_APPENDTILESET, "Append Tileset",   "Load and append a tileset");
  m_PopupMenu->AppendSeparator();
  m_PopupMenu->Append(wEVT_TILESETVIEW_ZOOM_1X,       "Zoom x1",          "Change zoom to x1", TRUE);
  m_PopupMenu->Append(wEVT_TILESETVIEW_ZOOM_2X,       "Zoom x2",          "Change zoom to x2", TRUE);
  m_PopupMenu->Append(wEVT_TILESETVIEW_ZOOM_4X,       "Zoom x4",          "Change zoom to x4", TRUE);
  m_PopupMenu->Append(wEVT_TILESETVIEW_ZOOM_8X,       "Zoom x8",          "Change zoom to x8", TRUE);

}

////////////////////////////////////////////////////////////////////////////////

wTilesetView::~wTilesetView()
{
  delete m_BlitTile;
  delete m_PopupMenu;

//  DestroyWindow();
}

/*
////////////////////////////////////////////////////////////////////////////////

BOOL
CTilesetView::Create(ITilesetViewHandler* handler, CWnd* parent, sTileset* tileset)
{
  m_Handler = handler;
  m_Tileset = tileset;
  
  m_BlitTile = new CDIBSection(
    m_Tileset->GetTileWidth()  * m_ZoomFactor,
    m_Tileset->GetTileHeight() * m_ZoomFactor,
    32
  );

  BOOL retval = CWnd::Create(
    AfxRegisterWndClass(0, LoadCursor(NULL, IDC_ARROW), NULL, NULL),
    "",
    WS_CHILD | WS_VISIBLE | WS_VSCROLL,
    CRect(0, 0, 0, 0),
    parent,
    s_iTilesetViewID++);

  UpdateScrollBar();

  return retval;
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::TileChanged(int tile)
{
  //RECT client_rect;
  //GetClientRect(&client_rect);
  //int num_tiles_x = client_rect.right / m_BlitTile->GetWidth();
  wxSize size = GetClientSize();
  int num_tiles_x = size.GetWidth() / m_BlitTile->GetWidth();
  if (num_tiles_x == 0)
    return;

  int col = tile % num_tiles_x;
  int row = tile / num_tiles_x;
  
  int x = col * m_BlitTile->GetWidth();
  int y = (row - m_TopRow) * m_BlitTile->GetHeight();

  //RECT rect;
  //SetRect(&rect, x, y, x + m_BlitTile->GetWidth(), y + m_BlitTile->GetHeight());
  //InvalidateRect(&rect);
  wxRect rect(x, y, m_BlitTile->GetWidth(), m_BlitTile->GetHeight());
  Refresh(TRUE, &rect);
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::TilesetChanged()
{
  // do a verification on the selected tile
  if (m_SelectedTile < 0) {
    m_SelectedTile = 0;
  }

  if (m_SelectedTile > m_Tileset->GetNumTiles() - 1) {
    m_SelectedTile = m_Tileset->GetNumTiles() - 1;
  }

  // resize blit tile if we must
  delete m_BlitTile;
  m_BlitTile = new wDIBSection(
    m_Tileset->GetTileWidth()  * m_ZoomFactor,
    m_Tileset->GetTileHeight() * m_ZoomFactor
  );

  Refresh();
  UpdateScrollBar();
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::SetSelectedTile(int tile)
{
  m_SelectedTile = tile;
  //Invalidate();
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

int
wTilesetView::GetSelectedTile() const
{
  return m_SelectedTile;
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::UpdateScrollBar()
{
// without this ifdef it crashes certain tilesets in linux, e.g. a tileset with only one tile crashes
#ifdef WIN32
  int num_rows  = GetNumRows();
  int page_size = GetPageSize();

  // validate the values
  if (m_TopRow > num_rows - page_size)
    m_TopRow = num_rows - page_size;
  if (m_TopRow < 0)
    m_TopRow = 0;

  /*
  SCROLLINFO si;
  si.cbSize = sizeof(si);
  si.fMask  = SIF_ALL;
  si.nMin   = 0;

  if (page_size - num_rows)
  {
    si.nMax   = num_rows - 1;
    si.nPage  = page_size;
    si.nPos   = m_TopRow;
  }
  else
  {
    si.nMax   = 0xFFFF;
    si.nPage  = 0xFFFE;
    si.nPos   = 0;
  }

  SetScrollInfo(SB_VERT, &si);
  */
  //SetScrollbar(wxVERTICAL, m_TopRow, page_size, num_rows);

  SetVScrollPosition(m_TopRow);
  SetVScrollRange(num_rows, page_size);
#endif
}

////////////////////////////////////////////////////////////////////////////////

int
wTilesetView::GetPageSize()
{
  //RECT ClientRect;
  //GetClientRect(&ClientRect);
  //return ClientRect.bottom / m_BlitTile->GetHeight();
  return GetClientSize().GetHeight() / m_BlitTile->GetHeight();
}

////////////////////////////////////////////////////////////////////////////////

int
wTilesetView::GetNumRows()
{
  //RECT client_rect;
  //GetClientRect(&client_rect);
  //int num_tiles_x = client_rect.right / m_BlitTile->GetWidth();
  int num_tiles_x = GetClientSize().GetWidth() / m_BlitTile->GetWidth();

  if (num_tiles_x == 0)
    return -1;
  else
    return (m_Tileset->GetNumTiles() + num_tiles_x - 1) / num_tiles_x;
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::OnPaint(wxPaintEvent &event)
{
  wxPaintDC dc(this);
  wxRegion region = GetUpdateRegion();
  //CPaintDC dc(this);
  
  //RECT client_rect;
  //GetClientRect(&client_rect);
  wxSize client_size = GetClientSize();

  dc.SetClippingRegion(wxRect(wxPoint(0, 0), client_size));

  int blit_width  = m_BlitTile->GetWidth();
  int blit_height = m_BlitTile->GetHeight();

  int num_tiles_x = client_size.GetWidth() / blit_width;

  for (int iy = 0; iy < client_size.GetHeight() / blit_height + 1; iy++)
    for (int ix = 0; ix < client_size.GetWidth() / blit_width + 1; ix++)
    {
      /*
      RECT Rect = {
        ix       * blit_width,
        iy       * blit_height,
        (ix + 1) * blit_width - 1,
        (iy + 1) * blit_height - 1,
      };
      //if (dc.RectVisible(&Rect) == FALSE)
      //  continue;
      */
      if(region.Contains(
          ix       * blit_width,
          iy       * blit_height,
          (ix + 1) * blit_width - 1,
          (iy + 1) * blit_height - 1
        ) == wxOutRegion) {
        continue;
      }

      int it = (iy + m_TopRow) * (client_size.GetWidth() / blit_width) + ix;
      if (ix < num_tiles_x
       && it >= 0 && it < m_Tileset->GetNumTiles())
      {
        // draw the tile
        // fill the DIB section
        //BGRA* pixels = (BGRA*)m_BlitTile->GetPixels();
        RGB *pixels = (RGB *)m_BlitTile->GetPixels();
        
        // make a checkerboard
        for (int jy = 0; jy < blit_height; jy++)
          for (int jx = 0; jx < blit_width; jx++)
          {
            pixels[jy * blit_width + jx] = 
              ((jx / 8 + jy / 8) % 2 ?
                CreateRGB(255, 255, 255) :
                CreateRGB(192, 192, 255)
                //CreateBGRA(255, 255, 255, 255) :
                //CreateBGRA(255, 192, 192, 255)
              );
          }        

        // draw the tile into it
        RGBA* tilepixels = m_Tileset->GetTile(it).GetPixels();
        {
          for (int iy = 0; iy < blit_height; iy++) {
            for (int ix = 0; ix < blit_width; ix++) {
              int ty = iy / m_ZoomFactor;
              int tx = ix / m_ZoomFactor;
              int t = ty * m_Tileset->GetTileWidth() + tx;
            
              int d = iy * blit_width + ix;

              int alpha = tilepixels[t].alpha;
              pixels[d].red   = (tilepixels[t].red   * alpha + pixels[d].red   * (255 - alpha)) / 256;
              pixels[d].green = (tilepixels[t].green * alpha + pixels[d].green * (255 - alpha)) / 256;
              pixels[d].blue  = (tilepixels[t].blue  * alpha + pixels[d].blue  * (255 - alpha)) / 256;
            }
          }
        }
        // blit the tile
        //CDC* tile = CDC::FromHandle(m_BlitTile->GetDC());
        //dc.BitBlt(Rect.left, Rect.top, Rect.right - Rect.left, Rect.bottom - Rect.top, tile, 0, 0, SRCCOPY);
        wxBitmap bitmap = m_BlitTile->GetImage()->ConvertToBitmap();
        dc.DrawBitmap(bitmap, ix * blit_width, iy * blit_height, FALSE);

        // if the tile is selected, draw a pink rectangle around it
        if (it == m_SelectedTile)
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
        dc.DrawRectangle(ix * blit_width, iy * blit_height, blit_width, blit_height);

        dc.SetBrush(wxNullBrush);
        dc.SetPen(wxNullPen);
      }
      
    }
  dc.DestroyClippingRegion();
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::OnSize(wxSizeEvent &event)
{
  int cx = GetClientSize().GetWidth();
//  int cy = GetClientSize().GetHeight();
  if (cx > 0)
  {
    // if the current top row is greater than the total number of rows minus the page size
    if (m_TopRow > GetNumRows() - GetPageSize())
    {
      // move the top row up
      m_TopRow = GetNumRows() - GetPageSize();
      if (m_TopRow < 0)
        m_TopRow = 0;
      Refresh();
    }
  }

  // reflect the changes
  UpdateScrollBar();
  wVScrollWindow::OnSize(event);
  Refresh();
}

/*
////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::OnVScroll(wxScrollEvent &event)
{
  if(event.GetEventType() == wxEVT_SCROLL_TOP)          m_TopRow = 0;
  if(event.GetEventType() == wxEVT_SCROLL_BOTTOM)       m_TopRow = GetNumRows() - GetPageSize();
  if(event.GetEventType() == wxEVT_SCROLL_LINEUP)       m_TopRow++;
  if(event.GetEventType() == wxEVT_SCROLL_LINEDOWN)     m_TopRow--;
  if(event.GetEventType() == wxEVT_SCROLL_PAGEUP)       m_TopRow += GetPageSize();
  if(event.GetEventType() == wxEVT_SCROLL_PAGEDOWN)     m_TopRow -= GetPageSize();
  if(event.GetEventType() == wxEVT_SCROLL_THUMBTRACK)   m_TopRow = event.GetPosition();
  if(event.GetEventType() == wxEVT_SCROLL_THUMBRELEASE) m_TopRow = event.GetPosition();

  UpdateScrollBar();
  Refresh();
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::OnVScrollChanged(int y)
{
  int old_y = m_TopRow;
  int new_y = y;
  m_TopRow = y;
  ScrollArea(0, (old_y - new_y) * m_BlitTile->GetHeight());
  UpdateScrollBar();
  //Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::OnLButtonDown(wxMouseEvent &event)
{
  if (m_MenuShown)
    return;

  //RECT client_rect;
  //GetClientRect(&client_rect);
  wxSize client_size = GetClientSize();

  int num_tiles_x = client_size.GetWidth() / m_BlitTile->GetWidth();

  int col = event.m_x / m_BlitTile->GetWidth();
  int row = event.m_y / m_BlitTile->GetHeight();

  // don't let user select tile off the right edge (and go to the next row)
  if (col >= num_tiles_x) {
    return;
  }

  int tile = (m_TopRow + row) * num_tiles_x + col;

  if (tile >= 0 && tile < m_Tileset->GetNumTiles())
    m_SelectedTile = tile;

  Refresh();

  // the selected tile changed, so tell the parent window
  m_Handler->TV_SelectedTileChanged(m_SelectedTile);
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::OnMouseMove(wxMouseEvent &event)
{
  //RECT client_rect;
  //GetClientRect(&client_rect);
  wxSize client_size = GetClientSize();
  int num_tiles_x = client_size.GetWidth() / m_BlitTile->GetWidth();

  int x = event.m_x / (m_Tileset->GetTileWidth()  * m_ZoomFactor);
  int y = event.m_y / (m_Tileset->GetTileHeight() * m_ZoomFactor);

  int tile = (m_TopRow + y) * num_tiles_x + x;

#ifdef WIN32
  if (tile >= 0 && tile <= m_Tileset->GetNumTiles() -1)
  {
    //CString tilenum;
    //tilenum.Format("Tile (%i/%i)", tile, m_Tileset->GetNumTiles());
    wxString tilenum = wxString::Format("Tile (%d/%d)", tile, m_Tileset->GetNumTiles());
    SetStatus(tilenum);
  } else {
    SetStatus(wxString(""));
  }
#endif

}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::OnRButtonUp(wxMouseEvent &event)
{
  if (m_MenuShown)
    return;

  // select the tile
  OnLButtonDown(event);

  // show pop-up menu
  
  m_PopupMenu->Check(wEVT_TILESETVIEW_ZOOM_1X, FALSE);
  m_PopupMenu->Check(wEVT_TILESETVIEW_ZOOM_2X, FALSE);
  m_PopupMenu->Check(wEVT_TILESETVIEW_ZOOM_4X, FALSE);
  m_PopupMenu->Check(wEVT_TILESETVIEW_ZOOM_8X, FALSE);
  switch (m_ZoomFactor) {
  case 1:
    m_PopupMenu->Check(wEVT_TILESETVIEW_ZOOM_1X, TRUE);
    break;
  case 2:
    m_PopupMenu->Check(wEVT_TILESETVIEW_ZOOM_2X, TRUE);
    break;
  case 4:
    m_PopupMenu->Check(wEVT_TILESETVIEW_ZOOM_4X, TRUE);
    break;
  case 8:
    m_PopupMenu->Check(wEVT_TILESETVIEW_ZOOM_8X, TRUE);
    break;
  }
  PopupMenu(m_PopupMenu, event.m_x, event.m_y);
/*
  int x = event.m_x;
  int y = event.m_y;
  ClientToScreen(&x, &y);

  HMENU menu_ = LoadMenu(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_TILESETVIEW));
  HMENU menu = GetSubMenu(menu_, 0);

  if (m_ZoomFactor == 1) {
    CheckMenuItem(menu, ID_TILESETVIEW_ZOOM_1X, MF_BYCOMMAND | MF_CHECKED);
  } else if (m_ZoomFactor == 2) {
    CheckMenuItem(menu, ID_TILESETVIEW_ZOOM_2X, MF_BYCOMMAND | MF_CHECKED);
  } else if (m_ZoomFactor == 4) {
    CheckMenuItem(menu, ID_TILESETVIEW_ZOOM_4X, MF_BYCOMMAND | MF_CHECKED);
  } else if (m_ZoomFactor == 8) {
    CheckMenuItem(menu, ID_TILESETVIEW_ZOOM_8X, MF_BYCOMMAND | MF_CHECKED);
  }

  m_MenuShown = true;
  TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, m_hWnd, NULL);
  m_MenuShown = false;

  DestroyMenu(menu_);
*/
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::OnInsertTile(wxEvent &event)
{
  // adjust map tile indices around
  m_Handler->TV_InsertedTiles(m_SelectedTile, 1);

  m_Tileset->InsertTiles(m_SelectedTile, 1);
  m_Handler->TV_TilesetChanged();
  UpdateScrollBar();
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::OnAppendTile(wxEvent &event)
{
  m_Tileset->AppendTiles(1);
  m_Handler->TV_TilesetChanged();
  UpdateScrollBar();
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::OnDeleteTile(wxEvent &event)
{
  // adjust map tile indices around
  m_Handler->TV_DeletedTiles(m_SelectedTile, 1);

  m_Tileset->DeleteTiles(m_SelectedTile, 1);

  // make sure selected tile is still valid
  if (m_SelectedTile >= m_Tileset->GetNumTiles()) {
    m_SelectedTile = m_Tileset->GetNumTiles() - 1;
    m_Handler->TV_SelectedTileChanged(m_SelectedTile);
  }

  m_Handler->TV_TilesetChanged();

  UpdateScrollBar();
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::OnTileProperties(wxEvent &event)
{
  wTilePropertiesDialog dialog(this, m_Tileset, m_SelectedTile);
  if (dialog.ShowModal() == wxOK)
  {
    m_Handler->TV_TilesetChanged();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::OnInsertTiles(wxEvent &event)
{
  int tiles = wxGetNumberFromUser(wxString("Insert Tiles"), wxString("Number of tiles to insert"), wxString(""), 1, 1, 255);
  if (tiles != -1)
  {
    // adjust map tile indices around
    m_Handler->TV_InsertedTiles(m_SelectedTile, tiles);

    m_Tileset->InsertTiles(m_SelectedTile, tiles);
    m_Handler->TV_TilesetChanged();
    UpdateScrollBar();
    Refresh();
  }
/*
  CNumberDialog dialog("Insert Tiles", "Number of Tiles", 1, 1, 255);
  if (dialog.DoModal() == IDOK)
  {
    // adjust map tile indices around
    m_Handler->TV_InsertedTiles(m_SelectedTile, dialog.GetValue());

    m_Tileset->InsertTiles(m_SelectedTile, dialog.GetValue());
    m_Handler->TV_TilesetChanged();
    UpdateScrollBar();
    Refresh();
  }
*/
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::OnAppendTiles(wxEvent &event)
{
  int tiles = wxGetNumberFromUser(wxString("Append Tiles"), wxString("Number of tiles to append"), wxString(""), 1, 1, 255);
  if (tiles != -1)
  {
    m_Tileset->AppendTiles(tiles);
    m_Handler->TV_TilesetChanged();
    UpdateScrollBar();
    Refresh();
  }
/*
  CNumberDialog dialog("Append Tiles", "Number of Tiles", 1, 1, 255);
  if (dialog.DoModal() == IDOK)
  {
    m_Tileset->AppendTiles(dialog.GetValue());
    m_Handler->TV_TilesetChanged();
    UpdateScrollBar();
    Refresh();
  }
*/
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::OnDeleteTiles(wxEvent &event)
{
  int tiles = wxGetNumberFromUser(wxString("Delete Tiles"), wxString("Number of tiles to delete"), wxString(""), 1, 1, 255);
  if (tiles != -1)
  {
    // adjust map tile indices around
    m_Handler->TV_DeletedTiles(m_SelectedTile, tiles);

    m_Tileset->DeleteTiles(m_SelectedTile, tiles);

    // make sure selected tile is still valid
    if (m_SelectedTile >= m_Tileset->GetNumTiles()) {
      m_SelectedTile = m_Tileset->GetNumTiles() - 1;
      m_Handler->TV_SelectedTileChanged(m_SelectedTile);
    }

    m_Handler->TV_SelectedTileChanged(m_SelectedTile);
    m_Handler->TV_TilesetChanged();
    UpdateScrollBar();
    Refresh();
  }
/*
  CNumberDialog dialog("Delete Tiles", "Number of Tiles", 1, 1, m_Tileset->GetNumTiles() - m_SelectedTile - 1);
  if (dialog.DoModal() == IDOK)
  {
    // adjust map tile indices around
    m_Handler->TV_DeletedTiles(m_SelectedTile, dialog.GetValue());

    m_Tileset->DeleteTiles(m_SelectedTile, dialog.GetValue());

    // make sure selected tile is still valid
    if (m_SelectedTile >= m_Tileset->GetNumTiles()) {
      m_SelectedTile = m_Tileset->GetNumTiles() - 1;
      m_Handler->TV_SelectedTileChanged(m_SelectedTile);
    }

    m_Handler->TV_SelectedTileChanged(m_SelectedTile);
    m_Handler->TV_TilesetChanged();
    UpdateScrollBar();
    Refresh();
  }
*/
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::OnInsertTileset(wxEvent &event)
{
  wTilesetFileDialog dialog(this, FDM_OPEN);
  if (dialog.DoModal() == wxID_OK) {

    // load the tileset
    sTileset tileset;
    if (!tileset.Load(dialog.GetPath())) {
      ::wxMessageBox(wxString("Could not load tileset"), wxString("Insert Tileset"), wxOK);
      return;
    }

    // make sure it's compatible with the new one
    if (m_Tileset->GetTileWidth() != tileset.GetTileWidth() ||
        m_Tileset->GetTileHeight() != tileset.GetTileHeight()) {
      ::wxMessageBox(wxString("Tilesets do not have the same tile size"), wxString("Insert Tileset"), wxOK);
      return;
    }

    // stick it in
    m_Tileset->InsertTiles(m_SelectedTile, tileset.GetNumTiles());
    for (int i = 0; i < tileset.GetNumTiles(); i++) {
      m_Tileset->GetTile(m_SelectedTile + i) = tileset.GetTile(i);
    }

    // adjust map tile indices around
    m_Handler->TV_InsertedTiles(m_SelectedTile, tileset.GetNumTiles());

    // notify window
    m_Handler->TV_TilesetChanged();
    UpdateScrollBar();
    Refresh();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::OnAppendTileset(wxEvent &event)
{
  wTilesetFileDialog dialog(this, FDM_OPEN);
  if (dialog.DoModal() == wxID_OK) {

    // load the tileset
    sTileset tileset;
    if (!tileset.Load(dialog.GetPath())) {
      ::wxMessageBox(wxString("Could not load tileset"), wxString("Append Tileset"), wxOK);
      return;
    }

    // make sure it's compatible with the new one
    if (m_Tileset->GetTileWidth() != tileset.GetTileWidth() ||
        m_Tileset->GetTileHeight() != tileset.GetTileHeight()) {
      ::wxMessageBox(wxString("Tilesets do not have the same tile size"), wxString("Append Tileset"), wxOK);
      return;
    }

    // stick it in
    int old_size = m_Tileset->GetNumTiles();
    m_Tileset->AppendTiles(tileset.GetNumTiles());
    for (int i = 0; i < tileset.GetNumTiles(); i++) {
      m_Tileset->GetTile(old_size + i) = tileset.GetTile(i);
    }

    // notify window
    m_Handler->TV_TilesetChanged();
    UpdateScrollBar();
    Refresh();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::OnZoom1x(wxEvent &event)
{
  m_ZoomFactor = 1;

  delete m_BlitTile;
  m_BlitTile = new wDIBSection(
    m_Tileset->GetTileWidth()  * m_ZoomFactor,
    m_Tileset->GetTileHeight() * m_ZoomFactor
  );

  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::OnZoom2x(wxEvent &event)
{
  m_ZoomFactor = 2;

  delete m_BlitTile;
  m_BlitTile = new wDIBSection(
    m_Tileset->GetTileWidth()  * m_ZoomFactor,
    m_Tileset->GetTileHeight() * m_ZoomFactor
  );

  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::OnZoom4x(wxEvent &event)
{
  m_ZoomFactor = 4;

  delete m_BlitTile;
  m_BlitTile = new wDIBSection(
    m_Tileset->GetTileWidth()  * m_ZoomFactor,
    m_Tileset->GetTileHeight() * m_ZoomFactor
  );

  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetView::OnZoom8x(wxEvent &event)
{
  m_ZoomFactor = 8;

  delete m_BlitTile;
  m_BlitTile = new wDIBSection(
    m_Tileset->GetTileWidth()  * m_ZoomFactor,
    m_Tileset->GetTileHeight() * m_ZoomFactor
  );

  Refresh();
}

////////////////////////////////////////////////////////////////////////////////
