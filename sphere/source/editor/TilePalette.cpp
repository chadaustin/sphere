#include "TilePalette.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"


BEGIN_MESSAGE_MAP(CTilePalette, CPaletteWindow)

  ON_WM_SIZE()

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CTilePalette::CTilePalette(CDocumentWindow* owner, ITilesetViewHandler* handler, sTileset* tileset)
: CPaletteWindow(owner, "Tiles",
  Configuration::Get(KEY_TILES_RECT),
  Configuration::Get(KEY_TILES_VISIBLE))
, m_Created(false)
{
// What is this?
//  CRect start_rect = Configuration::Get(KEY_TILES_RECT);
//  if (start_rect.left == -1 && start_rect.top == -1 && start_rect.right == -1 && start_rect.bottom == -1)
//    start_rect = CRect(0, 0, 80, 80);

  m_TilesetView.Create(handler, this, tileset);
  m_Created = true;
  
  RECT rect;
  GetClientRect(&rect);
  OnSize(0, rect.right, rect.bottom);
}

////////////////////////////////////////////////////////////////////////////////

void
CTilePalette::Destroy()
{
  // save state
  RECT rect;
  GetWindowRect(&rect);
  Configuration::Set(KEY_TILES_RECT, rect);

  Configuration::Set(KEY_TILES_VISIBLE, IsWindowVisible() != FALSE);

  // destroy window
  DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////

void
CTilePalette::TileChanged(int tile)
{
  m_TilesetView.TileChanged(tile);
}

////////////////////////////////////////////////////////////////////////////////

void
CTilePalette::TilesetChanged()
{
  m_TilesetView.TilesetChanged();
}                            

////////////////////////////////////////////////////////////////////////////////

void
CTilePalette::SelectTile(int tile)
{
  m_TilesetView.SetSelectedTile(tile);
}

////////////////////////////////////////////////////////////////////////////////

int
CTilePalette::GetSelectedTile() const
{
  return m_TilesetView.GetSelectedTile();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilePalette::OnSize(UINT type, int cx, int cy)
{
  if (m_Created)
    m_TilesetView.MoveWindow(0, 0, cx, cy);
}

////////////////////////////////////////////////////////////////////////////////
