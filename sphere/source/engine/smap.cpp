#include <stddef.h>
#include "smap.hpp"


////////////////////////////////////////////////////////////////////////////////

SMAP::SMAP()
{
  m_MaxLayerWidth = 0;
  m_MaxLayerHeight = 0;
}

////////////////////////////////////////////////////////////////////////////////

SMAP::~SMAP()
{
  for (int i = 0; i < m_Tiles.size(); i++) {
    DestroyImage(m_Tiles[i]);
  }
  for (int i = 0; i < m_SolidTiles.size(); i++) {
    DestroyImage(m_SolidTiles[i]);
  }
}

////////////////////////////////////////////////////////////////////////////////

bool
SMAP::Load(const char* filename, IFileSystem& fs)
{
  // load map
  if (m_Map.Load(filename, fs) == false)
    return false;

  // load tileset
  if (m_Map.GetTileset().GetNumTiles() == 0)
    return false;

  // initialize layer times array
  m_LayerTimes.resize(m_Map.GetNumLayers());
  m_LayerAlphas.resize(m_Map.GetNumLayers());
  std::fill(m_LayerAlphas.begin(), m_LayerAlphas.end(), 255);

  InitializeAnimation();

  for (int i = 0; i < m_Tiles.size(); i++) {
    DestroyImage(m_Tiles[i]);
  }
  for (int i = 0; i < m_SolidTiles.size(); i++) {
    DestroyImage(m_SolidTiles[i]);
  }

  // create the image array
  m_Tiles.resize(m_Map.GetTileset().GetNumTiles());
  for (int i = 0; i < m_Map.GetTileset().GetNumTiles(); i++) {
    sTile& tile = m_Map.GetTileset().GetTile(i);
    m_Tiles[i] = CreateImage(tile.GetWidth(), tile.GetHeight(), tile.GetPixels());
  }

  // create the solid image array
  m_SolidTiles.resize(m_Map.GetTileset().GetNumTiles());
  for (int i = 0; i < m_Map.GetTileset().GetNumTiles(); i++) {

    sTile tile = m_Map.GetTileset().GetTile(i);
    // make the tile completely opaque
    for (int i = 0; i < tile.GetWidth() * tile.GetHeight(); i++) {
      tile.GetPixels()[i].alpha = 0;
    }

    m_SolidTiles[i] = CreateImage(tile.GetWidth(), tile.GetHeight(), tile.GetPixels());
  }

  // calculate maximum non-parallax layer dimensions
  for (int i = 0; i < m_Map.GetNumLayers(); i++) {
    sLayer& layer = m_Map.GetLayer(i);
    if (layer.HasParallax() == false) {
      if (layer.GetWidth() > m_MaxLayerWidth) {
        m_MaxLayerWidth = layer.GetWidth();
      }
      if (layer.GetHeight() > m_MaxLayerHeight) {
        m_MaxLayerHeight = layer.GetHeight();
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
SMAP::UpdateMap()
{
  // update layer times for autoscrolling
  for (int i = 0; i < m_LayerTimes.size(); i++) {
    m_LayerTimes[i]++;
  }

  // update animations
  sTileset& tileset = m_Map.GetTileset();
  for (int i = 0; i < tileset.GetNumTiles(); i++) {

    sTile& tile = tileset.GetTile(i);
    if (tile.IsAnimated()) {
      if (--m_AnimationMap[i].delay < 0) {
        int next = m_AnimationMap[i].next;

        // !!!! HACK - make sure the next tile is in bounds
        if (next >= 0 && next < tileset.GetNumTiles()) {
          m_AnimationMap[i].current = next;
          m_AnimationMap[i].delay   = tileset.GetTile(next).GetDelay();
          m_AnimationMap[i].next    = tileset.GetTile(next).GetNextTile();
        }
      }
    }

  }
}

////////////////////////////////////////////////////////////////////////////////

void
SMAP::RenderLayer(int i, bool solid, int camera_x, int camera_y, int& offset_x, int& offset_y)
{
  sLayer& layer = m_Map.GetLayer(i);
  if (layer.IsVisible() == false) {
    return;
  }

  const int tile_width = m_Map.GetTileset().GetTileWidth();
  const int tile_height = m_Map.GetTileset().GetTileHeight();

  const int cx = GetScreenWidth()  / 2;
  const int cy = GetScreenHeight() / 2;

  // calculate camera offsets
  offset_x = 0;
  offset_y = 0;

  // if map is wider than the screen...
  if (tile_width * m_MaxLayerWidth > GetScreenWidth()) {
    if (camera_x < cx) {
      offset_x = cx - camera_x;
    } else if (camera_x > m_MaxLayerWidth * tile_width - cx) {
      offset_x = m_MaxLayerWidth * tile_width - camera_x - cx;
    }
  } else {
    offset_x = cx - camera_x;
  }

  // if map is higher than the screen...
  if (tile_height * m_MaxLayerHeight > GetScreenHeight()) {
    if (camera_y < cy) {
      offset_y = cy - camera_y;
    } else if (camera_y > m_MaxLayerHeight * tile_height - cy) {
      offset_y = m_MaxLayerHeight * tile_height - camera_y - cy;
    }
  } else {
    offset_y = cy - camera_y;
  }

  // calculate parallax/autoscrolling
  int parallax_x = 0;
  int parallax_y = 0;
  if (layer.HasParallax()) {
    // autoscrolling
    parallax_x = m_LayerTimes[i] * layer.GetXScrolling();
    parallax_y = m_LayerTimes[i] * layer.GetYScrolling();

    // parallax
    parallax_x -= (camera_x + offset_x - cx) * (layer.GetXParallax() - 1);
    parallax_y -= (camera_y + offset_y - cy) * (layer.GetYParallax() - 1);
  }

  // calculate the tile to start with on the upper-left side
  int numerator_x = camera_x - cx + offset_x - parallax_x;
  int numerator_y = camera_y - cy + offset_y - parallax_y;
  while (numerator_x < 0) {
    numerator_x += tile_width * layer.GetWidth();
  }
  while (numerator_y < 0) {
    numerator_y += tile_height * layer.GetHeight();
  }

  // t[x,y] = indices into the layer
  // o[x,y] = rendering offsets
  // i[x,y] = number of rows/columns to render

  int ty = numerator_y / tile_height;
  int oy = -(numerator_y % tile_height);

  std::vector<IMAGE>& tiles = (solid ? m_SolidTiles : m_Tiles);

  // !!!! Warning!  Repeated code!  Please fix!
  if (m_LayerAlphas[i] == 255) {

    // how many rows/columns to blit
    int iy = GetScreenHeight() / tile_height + 2;
    while (iy--) {

      int tx = numerator_x / tile_width;
      int ox = -(numerator_x % tile_width);

      int ix = GetScreenWidth() / tile_width + 2;
      while (ix--) {

        tx %= layer.GetWidth();
        ty %= layer.GetHeight();
        IMAGE image = tiles[m_AnimationMap[layer.GetTile(tx, ty)].current];
            
        BlitImage(image, ox, oy);
      
        tx++;
        ox += tile_width;
      }

      ty++;
      oy += tile_height;
    }

  } else if (m_LayerAlphas[i] != 0) {

    RGBA mask = { 255, 255, 255, m_LayerAlphas[i] };

    // how many rows/columns to blit
    int iy = GetScreenHeight() / tile_height + 2;
    while (iy--) {

      int tx = numerator_x / tile_width;
      int ox = -(numerator_x % tile_width);

      int ix = GetScreenWidth() / tile_width + 2;
      while (ix--) {

        tx %= layer.GetWidth();
        ty %= layer.GetHeight();
        IMAGE image = tiles[m_AnimationMap[layer.GetTile(tx, ty)].current];
            
        BlitImageMask(image, ox, oy, mask);
      
        tx++;
        ox += tile_width;
      }

      ty++;
      oy += tile_height;
    }

  }
}

////////////////////////////////////////////////////////////////////////////////

void
SMAP::SetLayerAlpha(int layer, int alpha)
{
  m_LayerAlphas[layer] = alpha;
}

////////////////////////////////////////////////////////////////////////////////

int
SMAP::GetLayerAlpha(int layer)
{
  return m_LayerAlphas[layer];
}

////////////////////////////////////////////////////////////////////////////////

void
SMAP::InitializeAnimation()
{
  sTileset& tileset = m_Map.GetTileset();
  m_AnimationMap.resize(tileset.GetNumTiles());

  for (int i = 0; i < tileset.GetNumTiles(); i++) {
    m_AnimationMap[i].current = i;
    m_AnimationMap[i].delay = tileset.GetTile(i).GetDelay();
    m_AnimationMap[i].next = tileset.GetTile(i).GetNextTile();
  }
}

////////////////////////////////////////////////////////////////////////////////
