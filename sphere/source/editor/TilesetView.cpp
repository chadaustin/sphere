#pragma warning(disable : 4786)


#include "TilesetView.hpp"
#include "NumberDialog.hpp"
#include "TilePropertiesDialog.hpp"
#include "Editor.hpp"
#include "FileDialogs.hpp"
#include "resource.h"


const int UPDATE_TILEVIEW_TIMER = 200;


static int s_iTilesetViewID = 1000;


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


////////////////////////////////////////////////////////////////////////////////

CTilesetView::CTilesetView()
: m_Handler(NULL)
, m_Tileset(NULL)
, m_TopRow(0)
, m_SelectedTile(0)
, m_ZoomFactor(1)
, m_BlitTile(NULL)
, m_MenuShown(false)
{
}

////////////////////////////////////////////////////////////////////////////////

CTilesetView::~CTilesetView()
{
  delete m_BlitTile;
  DestroyWindow();
}

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

////////////////////////////////////////////////////////////////////////////////

void
CTilesetView::TileChanged(int tile)
{
  RECT client_rect;
  GetClientRect(&client_rect);
  int num_tiles_x = client_rect.right / m_BlitTile->GetWidth();
  if (num_tiles_x == 0)
    return;

  int col = tile % num_tiles_x;
  int row = tile / num_tiles_x;
  
  int x = col * m_BlitTile->GetWidth();
  int y = (row - m_TopRow) * m_BlitTile->GetHeight();

  RECT rect;
  SetRect(&rect, x, y, x + m_BlitTile->GetWidth(), y + m_BlitTile->GetHeight());
  InvalidateRect(&rect);
}

////////////////////////////////////////////////////////////////////////////////

void
CTilesetView::TilesetChanged()
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
  m_BlitTile = new CDIBSection(
    m_Tileset->GetTileWidth()  * m_ZoomFactor,
    m_Tileset->GetTileHeight() * m_ZoomFactor,
    32
  );

  Invalidate();
  UpdateScrollBar();
}

////////////////////////////////////////////////////////////////////////////////

void
CTilesetView::SetSelectedTile(int tile)
{
  m_SelectedTile = tile;
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

int
CTilesetView::GetSelectedTile() const
{
  return m_SelectedTile;
}

////////////////////////////////////////////////////////////////////////////////

void
CTilesetView::UpdateScrollBar()
{
  int num_rows  = GetNumRows();
  int page_size = GetPageSize();

  // validate the values
  if (m_TopRow > num_rows - page_size)
    m_TopRow = num_rows - page_size;
  if (m_TopRow < 0)
    m_TopRow = 0;

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
}

////////////////////////////////////////////////////////////////////////////////

int
CTilesetView::GetPageSize()
{
  RECT ClientRect;
  GetClientRect(&ClientRect);
  return ClientRect.bottom / m_BlitTile->GetHeight();
}

////////////////////////////////////////////////////////////////////////////////

int
CTilesetView::GetNumRows()
{
  RECT client_rect;
  GetClientRect(&client_rect);
  int num_tiles_x = client_rect.right / m_BlitTile->GetWidth();

  if (num_tiles_x == 0)
    return -1;
  else
    return (m_Tileset->GetNumTiles() + num_tiles_x - 1) / num_tiles_x;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetView::OnPaint()
{
  CPaintDC dc(this);
  
  RECT client_rect;
  GetClientRect(&client_rect);

  int blit_width  = m_BlitTile->GetWidth();
  int blit_height = m_BlitTile->GetHeight();

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
      if (ix < num_tiles_x && it < m_Tileset->GetNumTiles())
      {
        // draw the tile
        // fill the DIB section
        BGRA* pixels = (BGRA*)m_BlitTile->GetPixels();
        
        // make a checkerboard
        for (int iy = 0; iy < blit_height; iy++)
          for (int ix = 0; ix < blit_width; ix++)
          {
            pixels[iy * blit_width + ix] = 
              ((ix / 8 + iy / 8) % 2 ?
                CreateBGRA(255, 255, 255, 255) :
                CreateBGRA(255, 192, 192, 255)
              );
          }        

        // draw the tile into it
        RGBA* tilepixels = m_Tileset->GetTile(it).GetPixels();
        for (int iy = 0; iy < blit_height; iy++)
          for (int ix = 0; ix < blit_width; ix++)
          {
            int ty = iy / m_ZoomFactor;
            int tx = ix / m_ZoomFactor;
            int t = ty * m_Tileset->GetTileWidth() + tx;
            
            int d = iy * blit_width + ix;

            int alpha = tilepixels[t].alpha;
            pixels[d].red   = (tilepixels[t].red   * alpha + pixels[d].red   * (255 - alpha)) / 256;
            pixels[d].green = (tilepixels[t].green * alpha + pixels[d].green * (255 - alpha)) / 256;
            pixels[d].blue  = (tilepixels[t].blue  * alpha + pixels[d].blue  * (255 - alpha)) / 256;
          }
        
        // blit the tile
        CDC* tile = CDC::FromHandle(m_BlitTile->GetDC());
        dc.BitBlt(Rect.left, Rect.top, Rect.right - Rect.left, Rect.bottom - Rect.top, tile, 0, 0, SRCCOPY);

        // if the tile is selected, draw a pink rectangle around it
        if (it == m_SelectedTile)
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
CTilesetView::OnSize(UINT type, int cx, int cy)
{
  if (cx > 0)
  {
    // if the current top row is greater than the total number of rows minus the page size
    if (m_TopRow > GetNumRows() - GetPageSize())
    {
      // move the top row up
      m_TopRow = GetNumRows() - GetPageSize();
      if (m_TopRow < 0)
        m_TopRow = 0;
      Invalidate();
    }
  }

  // reflect the changes
  UpdateScrollBar();
  Invalidate();

  CWnd::OnSize(type, cx, cy);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetView::OnVScroll(UINT code, UINT pos, CScrollBar* scroll_bar)
{
  switch (code)
  {
    case SB_LINEDOWN:   m_TopRow++;                break;
    case SB_LINEUP:     m_TopRow--;                break;
    case SB_PAGEDOWN:   m_TopRow += GetPageSize(); break;
    case SB_PAGEUP:     m_TopRow -= GetPageSize(); break;
    case SB_THUMBTRACK: m_TopRow = (int)pos;       break;
  }

  UpdateScrollBar();
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetView::OnLButtonDown(UINT flags, CPoint point)
{
  if (m_MenuShown)
    return;

  RECT client_rect;
  GetClientRect(&client_rect);
  int num_tiles_x = client_rect.right / m_BlitTile->GetWidth();

  int col = point.x / m_BlitTile->GetWidth();
  int row = point.y / m_BlitTile->GetHeight();

  // don't let user select tile off the right edge (and go to the next row)
  if (col >= num_tiles_x) {
    return;
  }

  int tile = (m_TopRow + row) * num_tiles_x + col;

  if (tile >= 0 && tile < m_Tileset->GetNumTiles())
    m_SelectedTile = tile;

  Invalidate();

  // the selected tile changed, so tell the parent window
  m_Handler->TV_SelectedTileChanged(m_SelectedTile);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetView::OnMouseMove(UINT flags, CPoint point)
{
  RECT client_rect;
  GetClientRect(&client_rect);
  int num_tiles_x = client_rect.right / m_BlitTile->GetWidth();

  int x = point.x / (m_Tileset->GetTileWidth()  * m_ZoomFactor);
  int y = point.y / (m_Tileset->GetTileHeight() * m_ZoomFactor);

  int tile = (m_TopRow + y) * num_tiles_x + x;

  if (tile <= m_Tileset->GetNumTiles() -1)
  {
    CString tilenum;
    tilenum.Format("Tile (%i/%i)", tile, m_Tileset->GetNumTiles());
    GetStatusBar()->SetWindowText(tilenum);
  }
  else
    GetStatusBar()->SetWindowText("");

}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetView::OnRButtonUp(UINT flags, CPoint point)
{
  if (m_MenuShown)
    return;

  // select the tile
  OnLButtonDown(flags, point);

  // show pop-up menu
  ClientToScreen(&point);
  
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
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetView::OnInsertTile()
{
  // adjust map tile indices around
  m_Handler->TV_InsertedTiles(m_SelectedTile, 1);

  m_Tileset->InsertTiles(m_SelectedTile, 1);
  m_Handler->TV_TilesetChanged();
  UpdateScrollBar();
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetView::OnAppendTile()
{
  m_Tileset->AppendTiles(1);
  m_Handler->TV_TilesetChanged();
  UpdateScrollBar();
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetView::OnDeleteTile()
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
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetView::OnTileProperties()
{
  CTilePropertiesDialog dialog(m_Tileset, m_SelectedTile);
  if (dialog.DoModal() == IDOK)
  {
    m_Handler->TV_TilesetChanged();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetView::OnInsertTiles()
{
  CNumberDialog dialog("Insert Tiles", "Number of Tiles", 1, 1, 255);
  if (dialog.DoModal() == IDOK)
  {
    // adjust map tile indices around
    m_Handler->TV_InsertedTiles(m_SelectedTile, dialog.GetValue());

    m_Tileset->InsertTiles(m_SelectedTile, dialog.GetValue());
    m_Handler->TV_TilesetChanged();
    UpdateScrollBar();
    Invalidate();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetView::OnAppendTiles()
{
  CNumberDialog dialog("Append Tiles", "Number of Tiles", 1, 1, 255);
  if (dialog.DoModal() == IDOK)
  {
    m_Tileset->AppendTiles(dialog.GetValue());
    m_Handler->TV_TilesetChanged();
    UpdateScrollBar();
    Invalidate();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetView::OnDeleteTiles()
{
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
    Invalidate();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetView::OnInsertTileset()
{
  CTilesetFileDialog dialog(FDM_OPEN);
  if (dialog.DoModal() == IDOK) {

    // load the tileset
    sTileset tileset;
    if (!tileset.Load(dialog.GetPathName())) {
      MessageBox("Could not load tileset", "Insert Tileset");
      return;
    }

    // make sure it's compatible with the new one
    if (m_Tileset->GetTileWidth() != tileset.GetTileWidth() ||
        m_Tileset->GetTileHeight() != tileset.GetTileHeight()) {
      MessageBox("Tilesets do not have the same tile size", "Insert Tileset");
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
    Invalidate();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetView::OnAppendTileset()
{
  CTilesetFileDialog dialog(FDM_OPEN);
  if (dialog.DoModal() == IDOK) {

    // load the tileset
    sTileset tileset;
    if (!tileset.Load(dialog.GetPathName())) {
      MessageBox("Could not load tileset", "Insert Tileset");
      return;
    }

    // make sure it's compatible with the new one
    if (m_Tileset->GetTileWidth() != tileset.GetTileWidth() ||
        m_Tileset->GetTileHeight() != tileset.GetTileHeight()) {
      MessageBox("Tilesets do not have the same tile size", "Insert Tileset");
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
    Invalidate();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetView::OnZoom1x()
{
  m_ZoomFactor = 1;

  delete m_BlitTile;
  m_BlitTile = new CDIBSection(
    m_Tileset->GetTileWidth()  * m_ZoomFactor,
    m_Tileset->GetTileHeight() * m_ZoomFactor,
    32
  );

  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetView::OnZoom2x()
{
  m_ZoomFactor = 2;

  delete m_BlitTile;
  m_BlitTile = new CDIBSection(
    m_Tileset->GetTileWidth()  * m_ZoomFactor,
    m_Tileset->GetTileHeight() * m_ZoomFactor,
    32
  );

  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetView::OnZoom4x()
{
  m_ZoomFactor = 4;

  delete m_BlitTile;
  m_BlitTile = new CDIBSection(
    m_Tileset->GetTileWidth()  * m_ZoomFactor,
    m_Tileset->GetTileHeight() * m_ZoomFactor,
    32
  );

  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetView::OnZoom8x()
{
  m_ZoomFactor = 8;

  delete m_BlitTile;
  m_BlitTile = new CDIBSection(
    m_Tileset->GetTileWidth()  * m_ZoomFactor,
    m_Tileset->GetTileHeight() * m_ZoomFactor,
    32
  );

  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////