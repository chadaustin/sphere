#include "TilePalette.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"


/*
BEGIN_MESSAGE_MAP(CTilePalette, CPaletteWindow)

  ON_WM_SIZE()

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wTilePalette, wPaletteWindow)

  EVT_SIZE(wTilePalette::OnSize)

END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wTilePalette::wTilePalette(wDocumentWindow* owner, ITilesetViewHandler* handler, sTileset* tileset)
: wPaletteWindow(owner, "Tiles",
  Configuration::Get(KEY_TILES_RECT()),
  Configuration::Get(KEY_TILES_VISIBLE()))
, m_Created(false)
{
// What is this?
//  CRect start_rect = Configuration::Get(KEY_TILES_RECT);
//  if (start_rect.left == -1 && start_rect.top == -1 && start_rect.right == -1 && start_rect.bottom == -1)
//    start_rect = CRect(0, 0, 80, 80);

  //m_TilesetView.Create(handler, this, tileset);
  m_TilesetView = new wTilesetView(this, handler, tileset);
  m_Created = true;

  wxSizeEvent evt(GetSize());
  OnSize(evt);
}

////////////////////////////////////////////////////////////////////////////////

bool
wTilePalette::Destroy()
{
//  delete m_TilesetView;
  // save state
  wxRect rect = GetRect();
  Configuration::Set(KEY_TILES_RECT(), rect);

  Configuration::Set(KEY_TILES_VISIBLE(), true); /*todo:IsWindowVisible() != FALSE);*/

  // destroy window
  //DestroyWindow();

  return wPaletteWindow::Destroy();
}

////////////////////////////////////////////////////////////////////////////////

void
wTilePalette::TileChanged(int tile)
{
  m_TilesetView->TileChanged(tile);
}

////////////////////////////////////////////////////////////////////////////////

void
wTilePalette::TilesetChanged()
{
  m_TilesetView->TilesetChanged();
}                            

////////////////////////////////////////////////////////////////////////////////

void
wTilePalette::SelectTile(int tile)
{
  m_TilesetView->SetSelectedTile(tile);
}

////////////////////////////////////////////////////////////////////////////////

int
wTilePalette::GetSelectedTile() const
{
  return m_TilesetView->GetSelectedTile();
}

////////////////////////////////////////////////////////////////////////////////

void
wTilePalette::OnSize(wxSizeEvent &event)
{
  if (m_Created)
    m_TilesetView->SetSize(GetClientSize());
}

////////////////////////////////////////////////////////////////////////////////
