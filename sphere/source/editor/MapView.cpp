#pragma warning(disable : 4786)

#include "MapView.hpp"
#include "Editor.hpp"
#include "EntityPersonDialog.hpp"
#include "EntityTriggerDialog.hpp"
#include "ZoneEditDialog.hpp"
#include "../common/primitives.hpp"
#include "../common/minmax.hpp"
#include "resource.h"
#include "../common/spriteset.hpp"

#include "Configuration.hpp"
#include "Keys.hpp"
#include <stack>

static int s_MapViewID = 2000;
static int s_MapAreaClipboardFormat;
static int s_MapEntityClipboardFormat;
static int s_ClipboardFormat;

//static const int ANIMATION_TIMER = 9001;

BEGIN_MESSAGE_MAP(CMapView, CScrollWindow)

  ON_WM_DESTROY()
  ON_WM_PAINT()
  ON_WM_SIZE()
  ON_WM_LBUTTONDOWN()
  ON_WM_MOUSEMOVE()
  ON_WM_LBUTTONUP()
  ON_WM_RBUTTONUP()
//  ON_WM_TIMER()
  //ON_WM_ERASEBKGND()

END_MESSAGE_MAP()

/*
BOOL
CMapView::OnEraseBkgnd(CDC* pDC)
{
  return FALSE;
}
*/

////////////////////////////////////////////////////////////////////////////////

CMapView::CMapView()
: m_Handler(NULL)
//, m_ToolPalette(NULL)
, m_Map(NULL)
, m_BlitTile(NULL)
, m_ZoomFactor(1)
, m_ObstructionColor(CreateRGB(255, 0, 255))
, m_HighlightColor(CreateRGB(255, 255, 0))
, m_ZoneColor(CreateRGB(255, 0, 0))
, m_CurrentTool(tool_1x1Tile)
, m_StartX(0)
, m_StartY(0)
, m_CurrentX(0)
, m_CurrentY(0)
, m_CurrentCursorTileX(-1)
, m_CurrentCursorTileY(-1)
, m_StartCursorTileX(-1)
, m_StartCursorTileY(-1)
, m_SelectedTile(0)
, m_SelectedLayer(0)
, m_MoveIndex(-1)
, m_Clicked(false)
, m_ShowGrid(false)
, m_ShowTileObstructions(false)
//, m_ShowAnimations(false)
, m_PreviewLineOn(0)
, m_PreviewBoxOn(0)
, m_RedrawWindow(0)
, m_RedrawPreviewLine(0)
, m_MultiTileWidth(0)
, m_MultiTileHeight(0)
, m_MultiTileData(NULL)
{
  m_SpritesetDrawType = Configuration::Get(KEY_MAP_SPRITESET_DRAWTYPE);  
  m_ZoomFactor        = Configuration::Get(KEY_MAP_ZOOM_FACTOR);

  if (m_SpritesetDrawType != SDT_ICON
   || m_SpritesetDrawType != SDT_MINI_IMAGE
   || m_SpritesetDrawType != SDT_IMAGE) {
     m_SpritesetDrawType = SDT_ICON;
  }

  if (m_ZoomFactor < 0 || m_ZoomFactor > 8) {
    m_ZoomFactor = 1;
  }

  s_MapAreaClipboardFormat   = RegisterClipboardFormat("MapAreaSelection32");
  s_MapEntityClipboardFormat = RegisterClipboardFormat("MapEntitySelection32");
  s_ClipboardFormat = RegisterClipboardFormat("FlatImage32");
  m_Clipboard = new CClipboard();
}

////////////////////////////////////////////////////////////////////////////////

CMapView::~CMapView()
{
  // destroy the blit DIB
  delete m_BlitTile;
  delete m_Clipboard;
  DestroyWindow();
  m_TileObstructions.clear();
  Configuration::Set(KEY_MAP_SPRITESET_DRAWTYPE, m_SpritesetDrawType);
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CMapView::Create(CDocumentWindow* /*owner*/, IMapViewHandler* handler, CWnd* parent, sMap* map)
{
  m_Handler = handler;
  m_Map = map;

  m_BlitTile = new CDIBSection(
    m_Map->GetTileset().GetTileWidth()  * m_ZoomFactor,
    m_Map->GetTileset().GetTileHeight() * m_ZoomFactor,
    32);

  // create tool palette
  // m_ToolPalette = new CMapToolPalette(owner, this);

  // create the window object
  BOOL retval = CWnd::Create(
    AfxRegisterWndClass(0, LoadCursor(NULL, IDC_ARROW), NULL, NULL),
    "",
    WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL,
    CRect(0, 0, 0, 0),
    parent,
    s_MapViewID++);

  UpdateScrollBars();
  UpdateObstructionTiles();

	//init the animation timer
	//m_FrameTick = 0;
  //m_Timer = SetTimer(ANIMATION_TIMER, Configuration::Get(KEY_MAP_ANIMATION_DELAY), NULL);
	//InitAnimations();

  return retval;
}

////////////////////////////////////////////////////////////////////////////////

/*
void 
CMapView::InitAnimations()
{
	//init animations in tileset
	int tileNum = m_Map->GetTileset().GetNumTiles();

	for (int i = 0; i < tileNum; i++)
	{
	//	m_Map->GetTileset().GetTile(i).InitAnimation(i, m_FrameTick);
	}
}
*/

////////////////////////////////////////////////////////////////////////////////

int
CMapView::GetZoomFactor()
{
  return m_ZoomFactor;
}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::SetZoomFactor(int factor)
{
  m_ZoomFactor = factor;

  delete m_BlitTile;
  m_BlitTile = new CDIBSection(
    m_Map->GetTileset().GetTileWidth()  * factor,
    m_Map->GetTileset().GetTileHeight() * factor,
    32);

  m_RedrawWindow = 1;
  Invalidate();
  UpdateScrollBars();

  Configuration::Set(KEY_MAP_ZOOM_FACTOR, m_ZoomFactor);
}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::UpdateObstructionTile(int tile) {

  struct Local {
    struct Color {
      RGBA operator()(int, int) {
        return CreateRGBA(255, 0, 255, 255);
      }
    };

    static inline void CopyRGBA(RGBA& dest, RGBA src) {
      dest = src;
    }
  };

  sTile& src_tile = m_Map->GetTileset().GetTile(tile);
  sTile& dest_tile = m_TileObstructions[tile] = src_tile;
  RGBA* dest_pixels = dest_tile.GetPixels();

  // draw the obstruction segments
  Local::Color c;

  RECT clipper = { 0, 0, (dest_tile.GetWidth()  - 1), (dest_tile.GetHeight()  - 1)  };

  const sObstructionMap& obs_map = src_tile.GetObstructionMap();
  for (int i = 0; i < obs_map.GetNumSegments(); ++i) {
    const sObstructionMap::Segment& s = obs_map.GetSegment(i);

    primitives::Line(
      (RGBA*) dest_pixels,
      dest_tile.GetWidth(),
      s.x1, s.y1, s.x2, s.y2,
      c,
      clipper,
      Local::CopyRGBA
    );
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::UpdateObstructionTiles() {
  if (m_ShowTileObstructions) {
    m_TileObstructions.resize(m_Map->GetTileset().GetNumTiles());
    for (int i = 0; i < int(m_TileObstructions.size()); ++i) {
      UpdateObstructionTile(i);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::TilesetChanged()
{
  delete m_BlitTile;
  m_BlitTile = new CDIBSection(
    m_Map->GetTileset().GetTileWidth()  * m_ZoomFactor,
    m_Map->GetTileset().GetTileHeight() * m_ZoomFactor,
    32);

  UpdateObstructionTiles();
	// InitAnimations();

  m_RedrawWindow = 1;
  Invalidate();
  UpdateScrollBars();
}

////////////////////////////////////////////////////////////////////////////////

int
CMapView::GetSelectedTile() {
  return m_SelectedTile;
}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::SelectTile(int tile)
{
  m_SelectedTile = tile;
}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::SelectLayer(int layer)
{
  m_SelectedLayer = layer;
  m_RedrawWindow = 1;
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::SetTileSelection(int width, int height, unsigned int* tiles)
{
  if (width > 0 && height > 0 && tiles != NULL) {
    m_MultiTileWidth = width;
    m_MultiTileHeight = height;
    m_MultiTileData = tiles;
  }
  else {
    m_MultiTileWidth  = 0;
    m_MultiTileHeight = 0;
    m_MultiTileData   = NULL;
  }
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::UpdateScrollBars()
{
  SetHScrollRange(GetTotalTilesX() + 1, GetPageSizeX());
  SetVScrollRange(GetTotalTilesY() + 1, GetPageSizeY());
}

////////////////////////////////////////////////////////////////////////////////

int
CMapView::GetPageSizeX()
{
  RECT ClientRect;
  GetClientRect(&ClientRect);
  return ClientRect.right / m_Map->GetTileset().GetTileWidth() / m_ZoomFactor;
}

////////////////////////////////////////////////////////////////////////////////

int
CMapView::GetPageSizeY()
{
  RECT ClientRect;
  GetClientRect(&ClientRect);
  return ClientRect.bottom / m_Map->GetTileset().GetTileHeight() / m_ZoomFactor;
}

////////////////////////////////////////////////////////////////////////////////

int
CMapView::GetTotalTilesX()
{
  int max_x = 0;
  for (int i = 0; i < m_Map->GetNumLayers(); i++)
    if (m_Map->GetLayer(i).GetWidth() - 1 > max_x)
      max_x = m_Map->GetLayer(i).GetWidth() - 1;
  return max_x;
}

////////////////////////////////////////////////////////////////////////////////

int
CMapView::GetTotalTilesY()
{
  int max_y = 0;
  for (int i = 0; i < m_Map->GetNumLayers(); i++)
    if (m_Map->GetLayer(i).GetHeight() - 1 > max_y)
      max_y = m_Map->GetLayer(i).GetHeight() - 1;
  return max_y;
}

////////////////////////////////////////////////////////////////////////////////

/*
#ifndef NO_TERRAFORMING
static bool IsTerraformable(int tile) {
  return (tile == 360     || tile == 361
       || tile == 360 + 6 || tile == 361 + 6);
}

////////////////////////////////////////////////////////////////////////////////

static enum {
   TF_NONE  = 0x00000000,
   TF_LEFT  = 0x00000001,
   TF_RIGHT = 0x00000010,
   TF_UP    = 0x00000100,
   TF_DOWN  = 0x00001000,
   TF_ALL   = TF_LEFT | TF_RIGHT | TF_DOWN | TF_UP,
   TF_HORZ  = TF_LEFT | TF_RIGHT,
   TF_VERT  = TF_UP | TF_DOWN,
   TF_UPPER_LEFT  = TF_LEFT | TF_UP,
   TF_UPPER_RIGHT = TF_LEFT | TF_UP,
   TF_LOWER_LEFT = TF_LEFT | TF_DOWN,
   TF_LOWER_RIGHT = TF_LEFT | TF_DOWN,
};

////////////////////////////////////////////////////////////////////////////////

static int GetTerraformSet(const int tile)
{
  int tile_offset = 0;
  int set = 0;

try_again:
  switch (tile - tile_offset) {
    case 360: break;
    case 361: break;
    case 362: break;
    
    case 390: break;
    case 391: break;
    case 392: break;
    
    case 420: break;
    case 421: break;  
    case 422: break;         
    
    case 450: break;
    case 451: break;
    case 452: break;
            
    case 480: break;
    case 481: break;
    case 482: break;

    case 510: break;
    case 511: break;
    case 512: break;

    default:
      if (tile_offset == 0) {
        tile_offset += 6;
        set += 1;
        goto try_again;
      }
      set = -1;
  }

  return set;
}

////////////////////////////////////////////////////////////////////////////////


static int GetTerraformTile(const int tile, const int type)
{
  int set = GetTerraformSet(tile);

  if (type & TF_ALL) {
    return set == 0 ? 421 : 421 + 6;
  }

  if (type & TF_NONE) {
    return set == 0 ? 361 : 361 + 6;
  }

  if (type & TF_VERT) {
    return set == 0 ? 481 : 481 + 6;
  }

  if (type & TF_HORZ) {
    return set == 0 ? 511 : 511 + 6;
  }

  ///////////////////////////

  if (type & TF_UPPER_LEFT) {
    return set == 0 ? 390 : 390 + 6;
  }

  if (type & TF_LOWER_LEFT) {
    return set == 0 ? 450 : 450 + 6;
  }

  if (type & TF_UPPER_RIGHT) {
    return set == 0 ? 392 : 392 + 6;
  }

  if (type & TF_LOWER_RIGHT) {
    return set == 0 ? 452 : 452 + 6;
  }

  ///////////////////////////

  if (type & TF_LEFT) {
    return set == 0 ? 510 : 510 + 6;
  }

  if (type & TF_RIGHT) {
    return set == 0 ? 512 : 512 + 6;
  }

  if (type & TF_UP) {
    return set == 0 ? 480 : 480 + 6;
  }

  if (type & TF_DOWN) {
    return set == 0 ? 482 : 482 + 6;
  }

  ///////////////////////////

  return tile;
}

int GetTerraformType(const int tile) {

  int type;
  int tile_offset = 0;

try_again:
  switch (tile - tile_offset) {
    case 360: type = TF_ALL; break;
    case 361: type = TF_NONE; break;
    case 362: type = TF_ALL; break;
    
    case 390: type = TF_UPPER_LEFT; break;
    case 391: type = TF_UP; break;
    case 392: type = TF_UPPER_RIGHT; break;
    
    case 420: type = TF_LEFT | TF_DOWN | TF_UP; break;
    case 421: type = TF_ALL; break;  
    case 422: type = TF_RIGHT | TF_DOWN | TF_UP; break;         
    
    case 450: type = TF_LOWER_LEFT; break;
    case 451: type = TF_DOWN; break;
    case 452: type = TF_LOWER_RIGHT; break;
            
    case 480: type = TF_DOWN; break;
    case 481: type = TF_VERT; break;
    case 482: type = TF_UP; break;

    case 510: type = TF_LEFT; break;
    case 511: type = TF_HORZ; break;
    case 512: type = TF_RIGHT; break;

    default:
      if (tile_offset == 0) {
        tile_offset += 6;
        goto try_again;
      }
      type = TF_NONE;
  }

  return type;
}

bool Terraform(const int tx, const int ty, const int m_SelectedTile)
{
  const int selected_tile = m_SelectedTile;
  const int original_tile = m_Map->GetLayer(m_SelectedLayer).GetTile(tx, ty);

  if (!IsTerraformable(selected_tile))
    return false;

  int terraform_grid[3][3] = {-1};
  int terraform_types[3][3] = {TF_NONE};
  const int terraform_delta = 3/2;
  const int terraform_size = 3;

  for (int y = 0; y < terraform_size; y++) {
    for (int x = 0; x < terraform_size; x++) {
      sLayer& layer = m_Map->GetLayer(m_SelectedLayer);
      int ix = tx + x + terraform_delta;
      int iy = ty + y + terraform_delta;
      if (ix < 0 || ix >= layer.GetWidth()
       || iy < 0 || iy >= layer.GetHeight())
        continue;
            
      int tile = terraform_grid[x][y] = m_Map->GetLayer(m_SelectedLayer).GetTile(ix, iy);
      terraform_types[x][y] = GetTerraformType(tile);
    }
  }

  terraform_grid[terraform_size/2][terraform_size/2] = selected_tile;
  terraform_types[terraform_size/2][terraform_size/2] = GetTerraformType(selected_tile);

  for (int y = terraform_size/2; y <= terraform_size/2; y++) {
    for (int x = terraform_size/2; x <= terraform_size/2; x++) {
      terraform_types[x][y] |= (int) terraform_types[x - 1][y - 0] & TF_LEFT;
      terraform_types[x][y] |= (int) terraform_types[x - 0][y - 1] & TF_UP;
      terraform_types[x][y] |= (int) terraform_types[x - 0][y + 1] & TF_DOWN;
      terraform_types[x][y] |= (int) terraform_types[x + 1][y - 0] & TF_RIGHT;
    }
  }

  for (int y = 0; y <= terraform_size; y++) {
    for (int x = 0; x <= terraform_size; x++) {
      int tile = m_SelectedTile;
      m_SelectedTile = GetTerraformTile(terraform_grid[x][y], terraform_types[x][y]);
      if (m_SelectedTile != -1) {
              map_changed |= SetTile(tx + x - terraform_delta, ty + y - terraform_delta);
            }
            m_SelectedTile = tile;
          }
        }

      }
      else {
        map_changed |= SetTile(tx, ty);
      }

      m_SelectedTile = selected_tile;
}
#endif
*/

void
CMapView::Click(CPoint point)
{
  int tx = m_CurrentX + (point.x / m_Map->GetTileset().GetTileWidth() / m_ZoomFactor);
  int ty = m_CurrentY + (point.y / m_Map->GetTileset().GetTileHeight() / m_ZoomFactor);

  // change the tile
  bool map_changed = false;

  if (m_MultiTileWidth && m_MultiTileHeight && m_MultiTileData) {
    int old_tile = m_SelectedTile;
    int num_times_x = 1;
    int num_times_y = 1;

    switch (m_CurrentTool) {
      case tool_3x3Tile: num_times_x = 3; num_times_y = 3; break;
      case tool_5x5Tile: num_times_x = 5; num_times_y = 5; break;
    }

    for (int i = 0; i < num_times_y; i++) {
      for (int j = 0; j < num_times_x; j++) {
        for (int ix = tx; ix < tx + m_MultiTileWidth; ix++) {
          for (int iy = ty; iy < ty + m_MultiTileHeight; iy++) {
            m_SelectedTile = m_MultiTileData[((iy - ty) * m_MultiTileWidth) + (ix - tx)];
            map_changed |= SetTile((j*m_MultiTileWidth)+ix, (i*m_MultiTileHeight)+iy);
          }
        }
      }
    }
    m_SelectedTile = old_tile;
  }
  else
  switch (m_CurrentTool) {
    case tool_1x1Tile: { // 1x1
      map_changed = SetTile(tx, ty);
      break;
    }

    case tool_3x3Tile: { // 3x3
      for (int ix = tx - 1; ix <= tx + 1; ix++) {
        for (int iy = ty - 1; iy <= ty + 1; iy++) {
          map_changed |= SetTile(ix, iy);
        }
      }
      break;
    }

    case tool_5x5Tile: { // 5x5
      for (int ix = tx - 2; ix <= tx + 2; ix++) {
        for (int iy = ty - 2; iy <= ty + 2; iy++) {
          map_changed |= SetTile(ix, iy);
        }
      }
      break;
    }
  }

  if (map_changed) {
    m_Handler->MV_MapChanged();
  }
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapView::SetTile(int tx, int ty)
{
  sLayer& Layer = m_Map->GetLayer(m_SelectedLayer);

  if (tx < 0 ||
      ty < 0 ||
      tx >= Layer.GetWidth() ||
      ty >= Layer.GetHeight()) {
    return false;
  }

  int oldtile = Layer.GetTile(tx, ty);
  Layer.SetTile(tx, ty, m_SelectedTile);

  // if the tile has changed, invalidate it
  if (oldtile != m_SelectedTile)
  {
    int tile_width  = m_Map->GetTileset().GetTileWidth();
    int tile_height = m_Map->GetTileset().GetTileHeight();
    
    RECT Rect =
    {
      (tx - m_CurrentX) * tile_width  * m_ZoomFactor,
      (ty - m_CurrentY) * tile_height * m_ZoomFactor,
      (tx - m_CurrentX) * tile_width  * m_ZoomFactor + tile_width  * m_ZoomFactor,
      (ty - m_CurrentY) * tile_height * m_ZoomFactor + tile_height * m_ZoomFactor,
    };
    m_RedrawWindow = 1;
    InvalidateRect(&Rect);

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::SelectTileUnderPoint(CPoint Point)
{
  int tx = m_CurrentX + (Point.x / m_Map->GetTileset().GetTileWidth() / m_ZoomFactor);
  int ty = m_CurrentY + (Point.y / m_Map->GetTileset().GetTileHeight() / m_ZoomFactor);

  // change the tile
  if (tx >= 0 &&
      ty >= 0 &&
      tx < m_Map->GetLayer(m_SelectedLayer).GetWidth() &&
      ty < m_Map->GetLayer(m_SelectedLayer).GetHeight())
  {
    int tile = m_Map->GetLayer(m_SelectedLayer).GetTile(tx, ty);
    SelectTile(tile);

    // change the current tile
    m_Handler->MV_SelectedTileChanged(tile);
  }
}

////////////////////////////////////////////////////////////////////////////////
//Aligns a point to a quarter tile margin on the X axis
//
//Note: it will align to the nearest pixel against the margin between pixel
//
//a 16 pixel wide tile would align to pixels marked "O" 
//("=" other pixels, "|" margin)
//
//  |O = = O|O = = O|O = = O|O = = O|
//
//Note: Margins will not align properly with tiles that have a width not 
//divisable by 4.
//
int
CMapView::RoundX(int x) {
  int w = m_Map->GetTileset().GetTileWidth() / 4;
  int v = x % w;
  x -= v;
  if(v > w / 2) {
    x += w - 1;
  }
  return x;
}

////////////////////////////////////////////////////////////////////////////////
//same as RoundX() but for the Y axis
int
CMapView::RoundY(int y) {
  int h = m_Map->GetTileset().GetTileWidth() / 4;
  int v = y % h;
  y -= v;
  if(v > h / 2) {
    y += h - 1;
  }
  return y;
}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::MapAreaCopy()
{
  // TODO: add Map(all layers) area selection copy
}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::LayerAreaCopy()
{
  if (OpenClipboard() == FALSE)
  {
    MessageBox("Cannot Open Clipboard!", NULL, MB_OK | MB_ICONERROR);
    return;
  }
  EmptyClipboard();

  // precalculate stuff
  int start_x = m_StartCursorTileX;
  int start_y = m_StartCursorTileY;
  int end_x = m_CurrentCursorTileX;
  int end_y = m_CurrentCursorTileY;
  if (end_x < start_x) std::swap(start_x, end_x); 
  if (end_y < start_y) std::swap(start_y, end_y);

  sLayer& l = m_Map->GetLayer(m_SelectedLayer);

  // clip end point within layer
  // this gives negative results if start_x/start_y are outside the layer
  if (end_x >= l.GetWidth())  end_x = l.GetWidth()  - start_x - 1; 
  if (end_y >= l.GetHeight()) end_y = l.GetHeight() - start_y - 1; 

  // grab the layer
  int width = end_x - start_x + 1;
  int height = end_y - start_y + 1;

  // nothing to copy
  if (width < 0 || width > 4096 || height < 0 || height > 4096) {
    CloseClipboard();
    return;
  }

  sLayer pLayer;
  sLayer newLayer;
  pLayer.Resize(width, height);
  newLayer.Resize(width, height);
  for (int y=start_y; y<=end_y; y++)
    for (int x=start_x; x<=end_x; x++)
      pLayer.SetTile(x-start_x, y-start_y, l.GetTile(x, y));

  // fun time, calculate the tiles needed
  sTileset& t = m_Map->GetTileset();
  sTileset newTileset;
  newTileset.SetTileSize(t.GetTileWidth(), t.GetTileHeight());
  std::vector<int> usedTiles;
  for (int y=0; y<height; y++)
    for (int x=0; x<width; x++)
    {
      int tileOffset = -1;

      // check for recurrance of the tile
      for (int z = 0; z < int(usedTiles.size()); z++)
        if (usedTiles[z] == pLayer.GetTile(x, y))
          tileOffset = z;

      // add tiles if needed.
      if (tileOffset < 0)
      {
        usedTiles.push_back(pLayer.GetTile(x, y));
        tileOffset = usedTiles.size() - 1;
        newTileset.AppendTiles(1);
        memcpy(newTileset.GetTile(tileOffset).GetPixels(), 
               t.GetTile(pLayer.GetTile(x, y)).GetPixels(), 
               t.GetTileHeight() * t.GetTileWidth() * 4);
      }
        
      newLayer.SetTile(x, y, tileOffset);
    }

  // memory and "header" allocation
  int mapMem = newLayer.GetWidth() * newLayer.GetHeight() * sizeof(int);
  int tileMem = newTileset.GetNumTiles() * newTileset.GetTileHeight() * newTileset.GetTileWidth() * 4;
  HGLOBAL memory = GlobalAlloc(GHND, 24 + mapMem + tileMem);
  if (memory == NULL) {
    CloseClipboard();
    return;
  }

  dword* ptr = (dword*)GlobalLock(memory);
  if (ptr == NULL) {
    CloseClipboard();
    return;
  }

  *ptr++ = 0;                          //number of layers
  *ptr++ = newTileset.GetNumTiles();   //numtiles
  *ptr++ = newTileset.GetTileWidth();  //tile width
  *ptr++ = newTileset.GetTileHeight(); //tile height

  // copy the map data into memory
  *ptr++ = newLayer.GetWidth();        //width
  *ptr++ = newLayer.GetHeight();       //height
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      *ptr++ = newLayer.GetTile(x, y);
    }
  }

  // copy the tile data into memory
  for (int z = 0; z < newTileset.GetNumTiles(); z++) {
    RGBA* dest = (RGBA*)ptr;
    RGBA* source = newTileset.GetTile(z).GetPixels();
    int tile_width = newTileset.GetTileWidth();
    int tile_height = newTileset.GetTileHeight();

    for (int y = 0; y<tile_height; y++) {
      for (int x = 0; x<tile_width; x++) {
        dest[(z * tile_width * tile_height) + (y * tile_width) + x].red   = source[y * tile_width + x].red;
        dest[(z * tile_width * tile_height) + (y * tile_width) + x].green = source[y * tile_width + x].green;
        dest[(z * tile_width * tile_height) + (y * tile_width) + x].blue  = source[y * tile_width + x].blue;
        dest[(z * tile_width * tile_height) + (y * tile_width) + x].alpha = source[y * tile_width + x].alpha;
      }
    }
  }


  // put the stuff on the clipboard
  GlobalUnlock(memory);
  SetClipboardData(s_MapAreaClipboardFormat, memory);



  //const int lw = l.GetWidth();
  //const int lh = l.GetHeight();
  const int tw = m_Map->GetTileset().GetTileWidth();
  const int th = m_Map->GetTileset().GetTileHeight();

  // ADD FLAT 32

  int sw = width * tw;
  int sh = height * th;

  // copy the image as a flat 32-bit color image
  memory = GlobalAlloc(GHND, 8 + sw * sh * 4);
  if (memory == NULL) {
    CloseClipboard();
    return;
  }

  ptr = (dword*)GlobalLock(memory);
  if (ptr == NULL) {
    CloseClipboard();
    return;
  }

  *ptr++ = sw;
  *ptr++ = sh;

  RGBA* flat_pixels = new RGBA[sw * sh];
  if (flat_pixels == NULL) {
    CloseClipboard();
    return;
  }

  for (int ty = start_y; ty <= end_y; ty++) {
    for (int tx = start_x; tx <= end_x; tx++) {

      const RGBA* source = m_Map->GetTileset().GetTile(l.GetTile(tx, ty)).GetPixels();

      for (int iy = 0; iy < th; iy++) {
        for (int ix = 0; ix < tw; ix++) {
          int counter = (((ty - start_y) * th) + iy) * (tw * width) + (((tx - start_x) * tw) + ix);
          flat_pixels[counter].red   = source[iy * tw + ix].red;
          flat_pixels[counter].green = source[iy * tw + ix].green;
          flat_pixels[counter].blue  = source[iy * tw + ix].blue;
          flat_pixels[counter].alpha = source[iy * tw + ix].alpha;
        }
      }
    }
  }

  memcpy(ptr, flat_pixels, sw * sh * sizeof(RGBA));
  delete[] flat_pixels;

  // put the image on the clipboard
  GlobalUnlock(memory);
  SetClipboardData(s_ClipboardFormat, memory);

  // ADD DIB
  // create a pixel array to initialize the bitmap
  BGRA* pixels = new BGRA[width * tw * height * th];
  if (pixels == NULL) {
    CloseClipboard();
    return;
  }

  for (int ty = start_y; ty <= end_y; ty++) {
    for (int tx = start_x; tx <= end_x; tx++) {

      const RGBA* source = m_Map->GetTileset().GetTile(l.GetTile(tx, ty)).GetPixels();

      for (int iy = 0; iy < th; iy++) {
        for (int ix = 0; ix < tw; ix++) {
          int counter = (height * th - (((ty - start_y) * th) + iy) - 1) * (tw * width) + (((tx - start_x) * tw) + ix);
          pixels[counter].red   = source[iy * tw + ix].red;
          pixels[counter].green = source[iy * tw + ix].green;
          pixels[counter].blue  = source[iy * tw + ix].blue;
          pixels[counter].alpha = source[iy * tw + ix].alpha;
        }
      }
    }
  }

	// create the bitmap  
	BITMAPINFOHEADER header;
	header.biSize = sizeof(header);
	header.biWidth = tw * width;
	header.biHeight = th * height;
	header.biPlanes = 1;
	header.biBitCount = 32;
	header.biCompression = BI_RGB;
	header.biSizeImage = 0;
	header.biXPelsPerMeter = 0;
	header.biYPelsPerMeter = 0;
	header.biClrUsed = 0;
	header.biClrImportant = 0;

	HGLOBAL hDIB = GlobalAlloc(GHND, sizeof(header) + tw * width * th * height * 4);
  if (hDIB == NULL) {
    CloseClipboard();
    return;
  }
  char* dibPtr = (char*)GlobalLock(hDIB);
  if (dibPtr == NULL) {
    CloseClipboard();
    return;
  }

	memcpy(dibPtr, &header, sizeof(header));
	memcpy(dibPtr+sizeof(header), pixels, tw * width * th * height * 4);
	GlobalUnlock(hDIB);

  // put the bitmap in the clipboard
  SetClipboardData(CF_DIB, hDIB);
	delete[] pixels;

  CloseClipboard();
}

////////////////////////////////////////////////////////////////////////////////

void PasteMapUnderPointFunc(sMap* m_Map, const sMap& tMap, int m_SelectedLayer, int tx, int ty)
{
  sTileset tTileset = tMap.GetTileset();
  std::vector<int> newTileLoc;
  const int tile_width = tTileset.GetTileWidth();
  const int tile_height = tTileset.GetTileHeight();

  // relocate all the tiles, add the tiles to the actual tileset if needed
  for (int i = 0; i < tTileset.GetNumTiles(); i++) {
    newTileLoc.push_back(i);
  }

  for (int i = 0; i < tTileset.GetNumTiles(); i++) {
    bool found = false;
    sTileset& cTileset = m_Map->GetTileset();

    for (int j = 0; j < cTileset.GetNumTiles(); j++) {
      if (memcmp(tTileset.GetTile(i).GetPixels(), cTileset.GetTile(j).GetPixels(), tile_width * tile_height * 4) == 0) {
        newTileLoc[i] = j;
        found = true;
      }
    }

    if (!found) {
      // add to the end
      m_Map->GetTileset().AppendTiles(1);
      newTileLoc[i] = m_Map->GetTileset().GetNumTiles() - 1;
      memcpy(m_Map->GetTileset().GetTile(newTileLoc[i]).GetPixels(), tTileset.GetTile(i).GetPixels(), tile_width * tile_height * 4);
    }
  }

  if (tMap.GetNumLayers() == 1)
  {
    sLayer tLayer = tMap.GetLayer(0);

    // update the map data offsets
    for (int y = 0; y < tLayer.GetHeight(); y++)
      for (int x = 0; x < tLayer.GetWidth(); x++)
        tLayer.SetTile(x, y, newTileLoc[tLayer.GetTile(x, y)]);

    // finally, overwrite the existing map data
    for (int y=ty; y<m_Map->GetLayer(m_SelectedLayer).GetHeight() && y-ty<tLayer.GetHeight(); y++)
      for (int x=tx; x<m_Map->GetLayer(m_SelectedLayer).GetWidth() && x-tx<tLayer.GetWidth(); x++)
        m_Map->GetLayer(m_SelectedLayer).SetTile(x, y, tLayer.GetTile(x-tx, y-ty));
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::PasteMapUnderPoint(CPoint point)
{
  //TODO: add map area selection paste
  if (!m_Clipboard)
    return;

  int tx = m_CurrentX + (point.x / m_Map->GetTileset().GetTileWidth() / m_ZoomFactor);
  int ty = m_CurrentY + (point.y / m_Map->GetTileset().GetTileHeight() / m_ZoomFactor);

  if (OpenClipboard() == FALSE)
  {
    MessageBox("Cannot Open Clipboard!", NULL, MB_OK | MB_ICONERROR);
    return;
  }

  sMap tMap;
  sTileset& tTileset = tMap.GetTileset();

  HGLOBAL memory = (HGLOBAL)GetClipboardData(s_MapAreaClipboardFormat);
  if (memory != NULL)
  {
    dword* ptr = (dword*)GlobalLock(memory);
    int num_layers = *ptr++;
    int num_tiles = *ptr++;
    int tile_width = *ptr++;
    int tile_height = *ptr++;

    std::vector<int> newTileLoc;

    if (m_Map->GetTileset().GetTileWidth() != tile_width ||
        m_Map->GetTileset().GetTileHeight() != tile_height)
    {
      CloseClipboard();
      MessageBox("Tile size being pasted does not match tile size in map.\nPaste Aborted.", NULL, MB_OK | MB_ICONEXCLAMATION);
      return;
    }

    // extract the map array from memory
    if (num_layers == 0)
    {
      sLayer tLayer;

      int width = *ptr++;
      int height = *ptr++;

			if (width  <= 0 || width  > 4096
			 || height <= 0 || height > 4096) {
				CloseClipboard();
				return;
			}

      tLayer.Resize(width, height);
      for (int y=0; y<height; y++)
        for (int x=0; x<width; x++)
          tLayer.SetTile(x, y, *ptr++);

      tMap.InsertLayer(0, tLayer);
    }

    // extract the tiles from memory
    tTileset.Create(num_tiles);
    tTileset.SetTileSize(tile_width, tile_height);
    RGBA* source = (RGBA*)ptr;
    for (int z=0; z<tTileset.GetNumTiles(); z++)
    {
      RGBA* dest = tTileset.GetTile(z).GetPixels();

      for (int i=0; i<tile_width * tile_height; i++)
      {
        dest[i].red = source[(z * tile_width * tile_height) + i].red;
        dest[i].green = source[(z * tile_width * tile_height) + i].green;
        dest[i].blue = source[(z * tile_width * tile_height) + i].blue;
        dest[i].alpha = source[(z * tile_width * tile_height) + i].alpha;
      }
    }

    PasteMapUnderPointFunc(m_Map, tMap, m_SelectedLayer, tx, ty);

    m_RedrawWindow = 1;
    Invalidate();
    m_Handler->MV_MapChanged();
    // m_Handler->MV_TilesetChanged();!!

    CloseClipboard();
  }
  else {

    int width;
    int height;
    RGBA* pixels = m_Clipboard->GetFlatImageFromClipboard(width, height);

    if (pixels == NULL)
      pixels = m_Clipboard->GetBitmapImageFromClipboard(width, height);

    CloseClipboard();

    if (pixels != NULL)
    {
      CImage32 image(width, height, pixels);
      delete[] pixels;

      if (width % m_Map->GetTileset().GetTileWidth() != 0)
      {  
        width += m_Map->GetTileset().GetTileWidth();
        width -= width % m_Map->GetTileset().GetTileWidth();
      }
      if (height % m_Map->GetTileset().GetTileHeight() != 0) {
        height += m_Map->GetTileset().GetTileHeight();
        height -= height % m_Map->GetTileset().GetTileHeight();
      }

      image.Resize(width, height); // make sure image is a multiple of tile_width and tile_height

      if (image.GetWidth() != width || image.GetHeight() != height) {
        return;
      }

      if (tTileset.BuildFromImage(image, m_Map->GetTileset().GetTileWidth(), m_Map->GetTileset().GetTileHeight(), true))
      {
        sLayer tLayer;
        int layer_width = width / m_Map->GetTileset().GetTileWidth();
        int layer_height = height / m_Map->GetTileset().GetTileHeight();

        tLayer.Resize(layer_width, layer_height);
        if (tLayer.GetWidth() != layer_width || tLayer.GetHeight() != layer_height) {
          return;
        }

        int i = 0;
        for (int y = 0; y < layer_height; y++)
          for (int x = 0; x < layer_width; x++)
            tLayer.SetTile(x, y, i++);

        tMap.InsertLayer(0, tLayer);
      }

      PasteMapUnderPointFunc(m_Map, tMap, m_SelectedLayer, tx, ty);

      m_RedrawWindow = 1;
      Invalidate();
      m_Handler->MV_MapChanged();
      // m_Handler->MV_TilesetChanged();!!
    }
  }

}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::FillRectArea()
{
  // precalculate stuff
  int start_x = m_StartCursorTileX;
  int start_y = m_StartCursorTileY;
  int end_x = m_CurrentCursorTileX;
  int end_y = m_CurrentCursorTileY;
  if (end_x < start_x) std::swap(start_x, end_x); 
  if (end_y < start_y) std::swap(start_y, end_y);

  int width = end_x - start_x + 1;
  int height = end_y - start_y + 1;

  for (int y=0; y<height && y+start_y<m_Map->GetLayer(m_SelectedLayer).GetHeight(); y++)
    for (int x=0; x<width && x+start_x<m_Map->GetLayer(m_SelectedLayer).GetWidth(); x++)
      m_Map->GetLayer(m_SelectedLayer).SetTile(x+start_x, y+start_y, m_SelectedTile);

  m_RedrawWindow = 1;
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

struct Point {
  Point(int x_, int y_) {
    x = x_;
    y = y_;
  }

  int x;
  int y;
};


void
CMapView::FillMe(int x, int y, int layer, int tileToReplace)
{
  int sx = 0; // GetSelectionLeftX();
  int sy = 0; // GetSelectionTopY();
  int sw = m_Map->GetLayer(layer).GetWidth(); // GetSelectionWidth();
  int sh = m_Map->GetLayer(layer).GetHeight(); //GetSelectionHeight();
  const int width  = sx + sw;
  const int height = sy + sh;

  std::stack<Point> q;
  q.push(    Point(x, y));
  m_Map->GetLayer(layer).SetTile(x, y, m_SelectedTile);

  while (!q.empty()) {
    Point p = q.top();
    q.pop();

    // fill up
    if (p.y > sy && m_Map->GetLayer(layer).GetTile(p.x, p.y - 1) == tileToReplace) {
      q.push(    Point(p.x, p.y - 1));
      m_Map->GetLayer(layer).SetTile(p.x, p.y - 1, m_SelectedTile);
    }
    // fill down
    if (p.y < height - 1 && m_Map->GetLayer(layer).GetTile(p.x, p.y + 1) == tileToReplace) {
      q.push(    Point(p.x, p.y + 1));
      m_Map->GetLayer(layer).SetTile(p.x, p.y + 1, m_SelectedTile);
    }
    // fill left
    if (p.x > sx && m_Map->GetLayer(layer).GetTile(p.x - 1, p.y) == tileToReplace) {
      q.push(    Point(p.x - 1, p.y));
      m_Map->GetLayer(layer).SetTile(p.x - 1, p.y, m_SelectedTile);
    }
    // fill right
    if (p.x < width - 1 && m_Map->GetLayer(layer).GetTile(p.x + 1, p.y) == tileToReplace) {
      q.push(    Point(p.x + 1, p.y));
      m_Map->GetLayer(layer).SetTile(p.x + 1, p.y, m_SelectedTile);
    }
  }
}


void
CMapView::FillArea()
{
  // precalculate stuff
  int start_x = m_StartCursorTileX;
  int start_y = m_StartCursorTileY;
  int end_x = m_CurrentCursorTileX;
  int end_y = m_CurrentCursorTileY;
  if (end_x < start_x) std::swap(start_x, end_x); 
  if (end_y < start_y) std::swap(start_y, end_y);

  if (m_Map->GetLayer(m_SelectedLayer).GetTile(start_x, start_y) == m_SelectedTile)
    return;

  FillMe(start_x, start_y, m_SelectedLayer, m_Map->GetLayer(m_SelectedLayer).GetTile(start_x, start_y));

  m_RedrawWindow = 1;
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void 
CMapView::EntityCopy(CPoint point)
{
  int tile_width  = m_Map->GetTileset().GetTileWidth();
  int tile_height = m_Map->GetTileset().GetTileHeight();
  int tx = m_CurrentX + (point.x / tile_width  / m_ZoomFactor);
  int ty = m_CurrentY + (point.y / tile_height / m_ZoomFactor);
  //int px = (tx * tile_width)  + (tile_width  / 2);
  //int py = (ty * tile_height) + (tile_height / 2);

  int entity_num = -1; // m_Map->FindEntity(px, py, m_SelectedLayer);

  for (int i = 0; i < m_Map->GetNumEntities(); i++) {
    if (m_Map->GetEntity(i).x / m_Map->GetTileset().GetTileWidth() == tx
     && m_Map->GetEntity(i).y / m_Map->GetTileset().GetTileHeight() == ty
     && m_Map->GetEntity(i).layer == m_SelectedLayer) {
      entity_num = i;
      break;
    }
  }  

  if (entity_num == -1) {
    return;
  }

  if (OpenClipboard() == FALSE) {
    MessageBox("Cannot Open Clipboard!", NULL, MB_OK | MB_ICONERROR);
    return;
  }
  EmptyClipboard();

  switch(m_Map->GetEntity(entity_num).GetEntityType()) {

    // person
    case sEntity::PERSON: {
/*
      sPersonEntity& person = (sPersonEntity&)m_Map->GetEntity(entity_num);
      int dialog_size = strlen(person.dialogue.c_str());
      int name_size = strlen(person.name.c_str());
      int spriteset_size = strlen(person.spriteset.c_str());

      // calculate memory needed
      int mem_needed = 0;
      mem_needed += 4;              // entity type
      mem_needed += 4 + 4 + 4 + 4;  // walk type, delay, duration, speed
      mem_needed += 4 + 4 + 4;      // dialog length, name length, spriteset length
      mem_needed += dialog_size;    // dialog text
      mem_needed += name_size;      // name
      mem_needed += spriteset_size; // spriteset

      // allocate memory and go!
      HGLOBAL memory = GlobalAlloc(GHND, mem_needed);
      dword* ptr = (dword*)GlobalLock(memory);
      *ptr++ = sEntity::PERSON;
      *ptr++ = person.movement_type;
      *ptr++ = person.walk_delay;
      *ptr++ = person.walk_duration;
      *ptr++ = person.walk_speed;
      *ptr++ = dialog_size;
      *ptr++ = name_size;
      *ptr++ = spriteset_size;
      char* c_ptr = (char*)ptr;
      memcpy(c_ptr, person.dialogue.c_str(), dialog_size);
      memcpy(c_ptr + dialog_size, person.name.c_str(), name_size);
      memcpy(c_ptr + (dialog_size + name_size), person.spriteset.c_str(), spriteset_size);

      GlobalUnlock(memory);
      SetClipboardData(s_MapEntityClipboardFormat, memory);
*/
    } break;

    // trigger
    case sEntity::TRIGGER: {
      sTriggerEntity& trigger = (sTriggerEntity&)m_Map->GetEntity(entity_num);
      int function_size = strlen(trigger.script.c_str());

      // calculate memory needed
      int mem_needed = 0;
      mem_needed += 4; // entity type
      mem_needed += 4; // function length
      mem_needed += function_size; // function

      // allocate memory and go!
      HGLOBAL memory = GlobalAlloc(GHND, mem_needed);
      if (memory != NULL) {
        dword* ptr = (dword*)GlobalLock(memory);
        *ptr++ = sEntity::TRIGGER;
        *ptr++ = function_size;
        char* c_ptr = (char*)ptr;
        memcpy(c_ptr, trigger.script.c_str(), function_size);
      
        GlobalUnlock(memory);
        SetClipboardData(s_MapEntityClipboardFormat, memory);
      }
    } break;
  }

  CloseClipboard();
}

////////////////////////////////////////////////////////////////////////////////

void 
CMapView::EntityPaste(CPoint point)
{
  int tile_width  = m_Map->GetTileset().GetTileWidth();
  int tile_height = m_Map->GetTileset().GetTileHeight();
  int tx = m_CurrentX + (point.x / tile_width  / m_ZoomFactor);
  int ty = m_CurrentY + (point.y / tile_height / m_ZoomFactor);

  if (OpenClipboard() == FALSE) {
    MessageBox("Unable to open the clipboard!");
    return;
  }

  // check if there's an entity already
  if (m_Map->FindEntity(tx * tile_width + tile_width / 2 , 
                        ty * tile_height + tile_height / 2, 
                        m_SelectedLayer) >= 0) {
    CloseClipboard();
    return;
  }

  HGLOBAL memory = (HGLOBAL)GetClipboardData(s_MapEntityClipboardFormat);
  if (memory != NULL) {

    dword* ptr = (dword*)GlobalLock(memory);
    switch (*ptr++) {

/*
      case sEntity::PERSON: {
        sPersonEntity person;

        // fill out the local info into the person
        person.x     = tx * m_Map->GetTileset().GetTileWidth();
        person.y     = ty * m_Map->GetTileset().GetTileHeight();
        person.layer = m_SelectedLayer;

        // fill in the info based on the clipboard
        person.movement_type = *ptr++;
        person.walk_delay    = *ptr++;
        person.walk_duration = *ptr++;
        person.walk_speed    = *ptr++;
        int dialog_size      = *ptr++;
        int name_size        = *ptr++;
        int spriteset_size   = *ptr++;
        char* dialog_text    = new char[dialog_size + 1];
        char* name_text      = new char[name_size + 1];
        char* spriteset_text = new char[spriteset_size + 1];

        // copy the varied-sized text into temporary places
        char* c_ptr = (char*)ptr;
        memcpy(dialog_text, c_ptr, dialog_size);
        memcpy(name_text, c_ptr + dialog_size, name_size);
        memcpy(spriteset_text, c_ptr + dialog_size + name_size, spriteset_size);
        dialog_text[dialog_size] = 0;
        name_text[name_size] = 0;
        spriteset_text[spriteset_size] = 0;

        // now slot it in
        person.dialogue = dialog_text;
        person.name = name_text;
        person.spriteset = spriteset_text;
        
        // pop the darn thing into the map
        m_Map->AddEntity(new sPersonEntity(person));

        delete[] dialog_text;
        delete[] name_text;
        delete[] spriteset_text;
      } break;
*/

      case sEntity::TRIGGER: {
        sTriggerEntity trigger;

        trigger.x = tx * m_Map->GetTileset().GetTileWidth();
        trigger.y = ty * m_Map->GetTileset().GetTileHeight();
        trigger.layer = m_SelectedLayer;

        int function_size = *ptr++;
        char* function_text = new char[function_size + 1];
        char* c_ptr = (char*)ptr;
        if (function_text) {
          memcpy(function_text, c_ptr, function_size);
          function_text[function_size] = 0;
          trigger.script = function_text;
        }

        // pop the darn thing into the map
        m_Map->AddEntity(new sTriggerEntity(trigger));

        delete[] function_text;
      } break;
    }

    m_Handler->MV_MapChanged();
  }
  
  CloseClipboard();
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapView::IsWithinSelectFillArea(int x, int y)
{
  int start_x = m_StartCursorTileX;
  int start_y = m_StartCursorTileY;
  int end_x = m_CurrentCursorTileX;
  int end_y = m_CurrentCursorTileY;

  // it should be top-left -> bottom right
  if (end_x < start_x) std::swap(start_x, end_x); 
  if (end_y < start_y) std::swap(start_y, end_y);

  if (x < start_x) return false;
  if (x > end_x)   return false;
  if (y < start_y) return false;
  if (y > end_y)   return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

int
CMapView::FindSpritesetImageIconsIndex(int person_index, std::string filename)
{
  int sprite_index = person_index;
  if (m_Map->GetNumEntities() != int(m_SpritesetImageIcons.size()))
    m_SpritesetImageIcons.resize(m_Map->GetNumEntities());

  if (sprite_index < 0 || sprite_index > int(m_SpritesetImageIcons.size()))
    sprite_index = -1;

  if (sprite_index != -1 && m_SpritesetImageIcons[sprite_index].filename != filename) {
    sSpriteset s;
    std::string path1 = std::string("../spritesets/" + filename);
    std::string path2 = std::string("spritesets/" + filename);
    // std::string path3 = std::string(SphereDirectory + "/" + ProjectDirectory "/spritesets/");
    m_SpritesetImageIcons[sprite_index].filename = filename;

    bool loaded = false;

    if (s.Load(path1.c_str()) || s.Load(path2.c_str())) {
      if (s.GetNumImages() > 0) {
        CImage32 image(s.GetImage(0));
        int x1, y1, x2, y2;
        s.GetBase(x1, y1, x2, y2);
        image.Rescale(m_Map->GetTileset().GetTileWidth(), m_Map->GetTileset().GetTileHeight());
        m_SpritesetImageIcons[sprite_index].x1 = x1;
        m_SpritesetImageIcons[sprite_index].y1 = y1;
        m_SpritesetImageIcons[sprite_index].x2 = x2;
        m_SpritesetImageIcons[sprite_index].y2 = y2;
        m_SpritesetImageIcons[sprite_index].icon = image;
        m_SpritesetImageIcons[sprite_index].image = s.GetImage(0);
        m_SpritesetImageIcons[sprite_index].created = true;
        loaded = true;
      }
    }

    if (!loaded)
      sprite_index = -1;
  }

  if (sprite_index != -1 && m_SpritesetImageIcons[sprite_index].created == false)
    sprite_index = -1;

  if (sprite_index < 0 || sprite_index > int(m_SpritesetImageIcons.size()))
    sprite_index = -1;

  return sprite_index;
}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::DrawTile(CDC& dc, const RECT& rect, int tx, int ty)
{
  int tile_width  = m_Map->GetTileset().GetTileWidth();
  int tile_height = m_Map->GetTileset().GetTileHeight();

  if (!m_BlitTile || m_BlitTile->GetPixels() == NULL) {
    dc.FillRect(&rect, CBrush::FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH)));
    return;
  }

  // clear the DIB
  memset(m_BlitTile->GetPixels(), 0,  m_ZoomFactor * m_ZoomFactor * tile_width * tile_height * 4);

  // draw the tile
  for (int i = 0; i < m_Map->GetNumLayers(); i++)
  {
    const sLayer& layer = m_Map->GetLayer(i);
    if (layer.IsVisible() == false) {
      continue;
    }

    if (tx >= 0 &&
        ty >= 0 &&
        tx < layer.GetWidth() &&
        ty < layer.GetHeight())
    {
      //int tile = m_Map->GetTileset().GetTile(layer.GetTile(tx, ty)).GetCurrentShown();
      int tile = layer.GetTile(tx, ty);
      const RGBA* src = m_Map->GetTileset().GetTile(tile).GetPixels();
      BGRA* dest = (BGRA*)m_BlitTile->GetPixels();

      if (m_ShowTileObstructions) {
        if (tile >= 0 && tile < int(m_TileObstructions.size())) {
          src = m_TileObstructions[tile].GetPixels();
        }
      }

      int counter = 0;
      for (int j=0; j<tile_height; j++)
      {
        for (int k=0; k<tile_width; k++)
        {
          for (int l=0; l<m_ZoomFactor; l++)
          {
            const RGBA s = src[j * tile_width + k];
            int alpha = src[j * tile_width + k].alpha;
            dest[counter].red   = (alpha * s.red   + (255 - alpha) * dest[counter].red)   / 256;
            dest[counter].green = (alpha * s.green + (255 - alpha) * dest[counter].green) / 256;
            dest[counter].blue  = (alpha * s.blue  + (255 - alpha) * dest[counter].blue)  / 256;


/*          THESE ARE TOO SLOW IN THIS INNER LOOP
            // additional draw rules
            // area fill
            if (m_CurrentTool == tool_FillRectArea && IsWithinSelectFillArea(tx, ty)) {
              if (m_Clicked)
              {
                RGBA p = m_Map->GetTileset().GetTile(m_SelectedTile).GetPixel(k, j);
                if (p.alpha > 30) Blend4(dest[counter], p, 100);
              }
            }
            else
            // area select
            if (m_CurrentTool == tool_CopyArea && IsWithinSelectFillArea(tx, ty))
              if (m_Clicked)
                Blend3(dest[counter], m_HighlightColor, 80);
*/

            counter++;
          }
        }

        for (int k=1; k<m_ZoomFactor; k++)
        {
          memcpy(dest + counter, dest + (counter - tile_width * m_ZoomFactor), tile_width * m_ZoomFactor * sizeof(RGBA));
          counter += tile_width * m_ZoomFactor;
        }
      }
    }
  }


  // render the tile
  dc.BitBlt(rect.left, rect.top, tile_width * m_ZoomFactor, tile_height * m_ZoomFactor,
            CDC::FromHandle(m_BlitTile->GetDC()), 0, 0, SRCCOPY);

  // draw start point
  if (tx == m_Map->GetStartX() / tile_width &&
      ty == m_Map->GetStartY() / tile_height &&
      m_Map->GetLayer(m_Map->GetStartLayer()).IsVisible())
  {
    // save the DC's state so there aren't any ill effects
    dc.SaveDC();
    dc.SetBkMode(TRANSPARENT);

    RECT r = rect;
    OffsetRect(&r, 1, 1);

    // draw text backdrop
    dc.SetTextColor(0x000000);
    dc.DrawText("ST", &r, DT_CENTER | DT_VCENTER);

    OffsetRect(&r, -1, -1);

    // draw white text
    dc.SetTextColor(0xFFFFFF);
    dc.DrawText("ST", &r, DT_CENTER | DT_VCENTER);

    dc.RestoreDC(-1);
  }

  bool should_render_tile = false; 

  if (m_SpritesetDrawType == SDT_ICON) {
    // draw entities
    for (int i = 0; i < m_Map->GetNumEntities(); ++i)
    {
      sEntity& entity = m_Map->GetEntity(i);
      if (tx == entity.x / tile_width &&
          ty == entity.y / tile_height &&
          m_Map->GetLayer(entity.layer).IsVisible())
      {
        HICON icon = NULL;
        switch (entity.GetEntityType())
        { 
          case sEntity::PERSON:  icon = AfxGetApp()->LoadIcon(IDI_ENTITY_PERSON); break;
          case sEntity::TRIGGER: icon = AfxGetApp()->LoadIcon(IDI_ENTITY_TRIGGER); break;
        }

        if (icon) {
          int tw = m_Map->GetTileset().GetTileWidth()  * m_ZoomFactor;
          int th = m_Map->GetTileset().GetTileHeight() * m_ZoomFactor;
          DrawIconEx(dc.m_hDC, rect.left, rect.top, icon, tw, th, 0, NULL, DI_NORMAL);
        }
      }
    }
  }
  else
  if (m_SpritesetDrawType == SDT_MINI_IMAGE) {

    // draw entities
    for (int i = 0; i < m_Map->GetNumEntities(); ++i)
    {
      sEntity& entity = m_Map->GetEntity(i);

      if (tx == entity.x / tile_width &&
          ty == entity.y / tile_height &&
          m_Map->GetLayer(entity.layer).IsVisible())
      {
        HICON icon = NULL;
        switch (entity.GetEntityType())
        {
          case sEntity::PERSON:  icon = AfxGetApp()->LoadIcon(IDI_ENTITY_PERSON); break;
          case sEntity::TRIGGER: icon = AfxGetApp()->LoadIcon(IDI_ENTITY_TRIGGER); break;
        }

        int tw = m_Map->GetTileset().GetTileWidth()  * m_ZoomFactor;
        int th = m_Map->GetTileset().GetTileHeight() * m_ZoomFactor;

        switch (entity.GetEntityType()) {

          case sEntity::PERSON: {
            sPersonEntity* person = (sPersonEntity*) &entity;
            int sprite_index = FindSpritesetImageIconsIndex(i, person->spriteset);

            if (sprite_index == -1) {
              if (icon) {
                DrawIconEx(dc.m_hDC, rect.left, rect.top, icon, tw, th, 0, NULL, DI_NORMAL);
              }
              continue;
            }
          
            const CImage32& image = m_SpritesetImageIcons[sprite_index].icon;
            const RGBA* src = image.GetPixels();
            BGRA* dest = (BGRA*)m_BlitTile->GetPixels();

            const int sprite_width = image.GetWidth();
            const int sprite_height = image.GetHeight();
            tile_width = std::min(tile_width, sprite_width);
            tile_height = std::min(tile_height, sprite_height);

            should_render_tile = true;

            int counter = 0;
            for (int j=0; j<tile_height; j++)
            {
              for (int k=0; k<tile_width; k++)
                for (int l=0; l<m_ZoomFactor; l++)
                {
                  RGBA s = src[j * sprite_width + k];
                  int alpha = src[j * sprite_width + k].alpha;
                  dest[counter].red   = (alpha * s.red   + (255 - alpha) * dest[counter].red)   / 256;
                  dest[counter].green = (alpha * s.green + (255 - alpha) * dest[counter].green) / 256;
                  dest[counter].blue  = (alpha * s.blue  + (255 - alpha) * dest[counter].blue)  / 256;
                  counter++;
                }

              for (int k=1; k<m_ZoomFactor; k++)
              {
                memcpy(dest + counter, dest + (counter - tile_width * m_ZoomFactor), tile_width * m_ZoomFactor * sizeof(RGBA));
                counter += tile_width * m_ZoomFactor;
              }
            }
          } 
          break;

          case sEntity::TRIGGER:
            DrawIconEx(dc.m_hDC, rect.left, rect.top, icon, tw, th, 0, NULL, DI_NORMAL);
          break;
        }
      }
    }
  }
  else
  if (m_SpritesetDrawType == SDT_IMAGE) {
    // draw entities
    for (int i = 0; i < m_Map->GetNumEntities(); ++i)
    {
      sEntity& entity = m_Map->GetEntity(i);

      int entity_width = tile_width;
      int entity_height = tile_height;
      int entity_x = entity.x;
      int entity_y = entity.y;

      switch (entity.GetEntityType()) {
        case sEntity::PERSON: {
          sPersonEntity* person = (sPersonEntity*) &entity;
          int sprite_index = FindSpritesetImageIconsIndex(i, person->spriteset);
          if (sprite_index != - 1) {
            int base_x1 = m_SpritesetImageIcons[sprite_index].x1;
            int base_y1 = m_SpritesetImageIcons[sprite_index].y1;
            int base_x2 = m_SpritesetImageIcons[sprite_index].x2;
            int base_y2 = m_SpritesetImageIcons[sprite_index].y2;
            int base_width = (base_x2 - base_x1);
            int base_height = (base_y2 - base_y1);

            entity_width = m_SpritesetImageIcons[sprite_index].image.GetWidth();
            entity_height = m_SpritesetImageIcons[sprite_index].image.GetHeight();

            entity_x = entity.x - entity_width/2 + base_width/2;
            entity_y = entity.y - entity_height/2 + base_height/2;

          }
        }
        break;
      }

      if (tx >= entity_x / tile_width  && tx < (entity_x + entity_width) / tile_width
       && ty >= entity_y / tile_height && ty < (entity_y + entity_height) / tile_height
       && m_Map->GetLayer(entity.layer).IsVisible())
      {
        HICON icon = NULL;
        switch (entity.GetEntityType())
        {
          case sEntity::PERSON:  icon = AfxGetApp()->LoadIcon(IDI_ENTITY_PERSON); break;
          case sEntity::TRIGGER: icon = AfxGetApp()->LoadIcon(IDI_ENTITY_TRIGGER); break;
        }

        int tw = m_Map->GetTileset().GetTileWidth()  * m_ZoomFactor;
        int th = m_Map->GetTileset().GetTileHeight() * m_ZoomFactor;

        switch (entity.GetEntityType()) {

          case sEntity::PERSON: {
            sPersonEntity* person = (sPersonEntity*) &entity;
            int sprite_index = FindSpritesetImageIconsIndex(i, person->spriteset);

            if (sprite_index == -1) {
              if (icon) {
                DrawIconEx(dc.m_hDC, rect.left, rect.top, icon, tw, th, 0, NULL, DI_NORMAL);
              }
              continue;
            }
          
            const CImage32& image = m_SpritesetImageIcons[sprite_index].image;
            const RGBA* src = image.GetPixels();
            BGRA* dest = (BGRA*)m_BlitTile->GetPixels();

            const int sprite_width = image.GetWidth();
            const int sprite_height = image.GetHeight();

            const int offset_x = (tx - (entity_x / tile_width)) * tile_width;
            const int offset_y = (ty - (entity_y / tile_height)) * tile_height;

            tile_width = std::min(tile_width, sprite_width);
            tile_height = std::min(tile_height, sprite_height);

            should_render_tile = true;

            int counter = 0;

            for (int j=0; j<tile_height; j++)
            {
              for (int k=0; k<tile_width; k++)
                for (int l=0; l<m_ZoomFactor; l++)
                {
                  RGBA s = src[(j + offset_y) * sprite_width + (k + offset_x)];
                  int alpha = src[(j + offset_y) * sprite_width + (k + offset_x)].alpha;
                  dest[counter].red   = (alpha * s.red   + (255 - alpha) * dest[counter].red)   / 256;
                  dest[counter].green = (alpha * s.green + (255 - alpha) * dest[counter].green) / 256;
                  dest[counter].blue  = (alpha * s.blue  + (255 - alpha) * dest[counter].blue)  / 256;
                  counter++;
                }

              for (int k=1; k<m_ZoomFactor; k++)
              {
                memcpy(dest + counter, dest + (counter - tile_width * m_ZoomFactor), tile_width * m_ZoomFactor * sizeof(RGBA));
                counter += tile_width * m_ZoomFactor;
              }
            }
          } 
          break;

          case sEntity::TRIGGER:
            DrawIconEx(dc.m_hDC, rect.left, rect.top, icon, tw, th, 0, NULL, DI_NORMAL);
          break;
        }
      }
    }
  }

  if (should_render_tile) {
    // render the tile
    dc.BitBlt(rect.left, rect.top, tile_width * m_ZoomFactor, tile_height * m_ZoomFactor,
    CDC::FromHandle(m_BlitTile->GetDC()), 0, 0, SRCCOPY);
  }

  // check if it's in fill or area select mode (not clicked)
  if ((m_CurrentTool == tool_FillRectArea && !m_Clicked) &&
      (m_CurrentTool == tool_CopyArea && !m_Clicked))
  {
  }
  else
  {
    int width;
    int height;
    int offset_x;
    int offset_y;
    GetRedrawRect(offset_x, offset_y, width, height);

    if (tx >= m_CurrentCursorTileX - (offset_x/(tile_width  * m_ZoomFactor)) && 
        ty >= m_CurrentCursorTileY - (offset_y/(tile_height * m_ZoomFactor)) &&
        tx < m_CurrentCursorTileX + (width/(tile_width * m_ZoomFactor)) -  (offset_x/(tile_width * m_ZoomFactor)) &&
        ty < m_CurrentCursorTileY + (height/(tile_height * m_ZoomFactor)) - (offset_y/(tile_height * m_ZoomFactor)) &&
        tx <= GetTotalTilesX() && ty <= GetTotalTilesY())
    {
      dc.SaveDC();

      CRect r(rect);
      CBrush brush;
        
      if (m_CurrentTool == tool_MoveEntity && m_MoveIndex != -1) {
        brush.CreateSolidBrush(RGB(255, 255, 255));
      }
      else {
        brush.CreateSolidBrush(RGB(m_HighlightColor.red, m_HighlightColor.green, m_HighlightColor.blue));
      }

      dc.FrameRect(r, &brush);
      brush.DeleteObject();

      dc.RestoreDC(-1);
    }
  }

}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::DrawObstructions(CDC& dc)
{
  const int tile_width  = m_Map->GetTileset().GetTileWidth();
  const int tile_height = m_Map->GetTileset().GetTileHeight();

  CPen pen(PS_SOLID, 1, RGB(m_ObstructionColor.red, m_ObstructionColor.green, m_ObstructionColor.blue));

  dc.SaveDC();
  dc.SelectObject(&pen);

  sLayer& l = m_Map->GetLayer(m_SelectedLayer);
  sObstructionMap& obs_map = l.GetObstructionMap();

  // for each segment
  for (int i = 0; i < obs_map.GetNumSegments(); i++) {

    const sObstructionMap::Segment& segment = obs_map.GetSegment(i);

    int x1 = (segment.x1 - m_CurrentX * tile_width)  * m_ZoomFactor;
    int y1 = (segment.y1 - m_CurrentY * tile_height) * m_ZoomFactor;
    int x2 = (segment.x2 - m_CurrentX * tile_width)  * m_ZoomFactor;
    int y2 = (segment.y2 - m_CurrentY * tile_height) * m_ZoomFactor;

    dc.MoveTo(x1, y1);
    dc.LineTo(x2, y2);
  }

  dc.RestoreDC(-1);
  pen.DeleteObject();
}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::DrawZones(CDC& dc)
{
  const int tile_width  = m_Map->GetTileset().GetTileWidth();
  const int tile_height = m_Map->GetTileset().GetTileHeight();

  CPen pen(PS_SOLID, 1, RGB(m_ZoneColor.red, m_ZoneColor.green, m_ZoneColor.blue));

  dc.SaveDC();
  dc.SelectObject(&pen);

  // for each segment
  for (int i = 0; i < m_Map->GetNumZones(); i++) {

    const sMap::sZone& zone = m_Map->GetZone(i);

    if(zone.layer == m_SelectedLayer) {
      int x1 = (zone.x1 - m_CurrentX * tile_width)  * m_ZoomFactor;
      int y1 = (zone.y1 - m_CurrentY * tile_height) * m_ZoomFactor;
      int x2 = (zone.x2 - m_CurrentX * tile_width)  * m_ZoomFactor;
      int y2 = (zone.y2 - m_CurrentY * tile_height) * m_ZoomFactor;

      dc.MoveTo(x1, y1);
      dc.LineTo(x2, y1);
      dc.LineTo(x2, y2);
      dc.LineTo(x1, y2);
      dc.LineTo(x1, y1);
    }
  }

  dc.RestoreDC(-1);
  pen.DeleteObject();
}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::DrawPreviewLine(CDC& dc, int x1, int y1, int x2, int y2)
{
  const int tile_width  = m_Map->GetTileset().GetTileWidth();
  const int tile_height = m_Map->GetTileset().GetTileHeight();

  CPen pen(PS_SOLID, 1, RGB(m_HighlightColor.red, m_HighlightColor.green, m_HighlightColor.blue));

  dc.SaveDC();
  dc.SetROP2(R2_XORPEN);
  dc.SelectObject(&pen);

  x1 = (x1 - m_CurrentX * tile_width) * m_ZoomFactor;
  y1 = (y1 - m_CurrentY * tile_height) * m_ZoomFactor;
  x2 = (x2 - m_CurrentX * tile_width) * m_ZoomFactor;
  y2 = (y2 - m_CurrentY * tile_height) * m_ZoomFactor;
  dc.MoveTo(x1, y1);
  dc.LineTo(x2, y2);

  dc.RestoreDC(-1);
  pen.DeleteObject();
}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::DrawPreviewBox(CDC& dc, int x1, int y1, int x2, int y2)
{
  const int tile_width  = m_Map->GetTileset().GetTileWidth();
  const int tile_height = m_Map->GetTileset().GetTileHeight();

  CPen pen(PS_SOLID, 1, RGB(m_HighlightColor.red, m_HighlightColor.green, m_HighlightColor.blue));

  dc.SaveDC();
  dc.SetROP2(R2_XORPEN);
  dc.SelectObject(&pen);

  x1 = (x1 - m_CurrentX * tile_width) * m_ZoomFactor;
  y1 = (y1 - m_CurrentY * tile_height) * m_ZoomFactor;
  x2 = (x2 - m_CurrentX * tile_width) * m_ZoomFactor;
  y2 = (y2 - m_CurrentY * tile_height) * m_ZoomFactor;
  dc.MoveTo(x1, y1);
  dc.LineTo(x2, y1);
  dc.LineTo(x2, y2);
  dc.LineTo(x1, y2);
  dc.LineTo(x1, y1);

  dc.RestoreDC(-1);
  pen.DeleteObject();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapView::OnDestroy()
{
  //if (m_ToolPalette) {
  //  m_ToolPalette->Destroy();
  //}
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapView::OnPaint()
{
  CPaintDC dc(this);

  if(!m_RedrawWindow && !m_RedrawPreviewLine) {
    //window was invalidated without setting any redraws. Assume a complete 
    //redraw is needed/intended
    m_RedrawWindow = 1;
  }

  if(m_RedrawWindow) {
    int NumTilesX = GetPageSizeX() + 1;
    int NumTilesY = GetPageSizeY() + 1;

    // draw all tiles visible in the client window
    for (int ix = 0; ix < NumTilesX; ix++) {
      for (int iy = 0; iy < NumTilesY; iy++) {

        // visibility check
        int tile_width  = m_Map->GetTileset().GetTileWidth();
        int tile_height = m_Map->GetTileset().GetTileHeight();
        RECT Rect = {
          ix * tile_width  * m_ZoomFactor,
          iy * tile_height * m_ZoomFactor,
          ix * tile_width  * m_ZoomFactor + tile_width  * m_ZoomFactor,
          iy * tile_height * m_ZoomFactor + tile_height * m_ZoomFactor,
        };

        if (dc.RectVisible(&Rect)) {

          // draw tile  
          int tx = ix + m_CurrentX;
          int ty = iy + m_CurrentY;
          DrawTile(dc, Rect, tx, ty);

        }
      }
    }
    DrawObstructions(dc);
    DrawZones(dc);

    m_RedrawWindow = 0;
    //force a redraw of the preview line after doing redraw.
    m_RedrawPreviewLine = 1;
  }

  if(m_RedrawPreviewLine) {
    if(m_PreviewLineOn) {
      if(m_RedrawPreviewLine == 2) {
        //erase previous line before drawing current line
        DrawPreviewLine(dc, m_StartX, m_StartY, m_PreviewOldX, m_PreviewOldY);
      }
      DrawPreviewLine(dc, m_StartX, m_StartY, m_PreviewX, m_PreviewY);
    }
    if(m_PreviewBoxOn) {
      if(m_RedrawPreviewLine == 2) {
        //erase previous line before drawing current line
        DrawPreviewBox(dc, m_StartX, m_StartY, m_PreviewOldX, m_PreviewOldY);
      }
      DrawPreviewBox(dc, m_StartX, m_StartY, m_PreviewX, m_PreviewY);
    }
    m_RedrawPreviewLine = 0;
  }

  if (m_ShowGrid && m_Map->GetNumLayers() > 0) {
    int tile_width = m_Map->GetTileset().GetTileWidth();
    int tile_height = m_Map->GetTileset().GetTileHeight();

    int width = m_Map->GetLayer(0).GetWidth();
    int height = m_Map->GetLayer(0).GetHeight();
    int size = 1 * GetZoomFactor();
    int offsetx = 0;
    int offsety = 0;

    // draw the grid if it is enabled
    if (m_ShowGrid) {
      HPEN linepen = CreatePen(PS_SOLID, 1, RGB(255, 0, 255));
      HPEN oldpen = (HPEN)SelectObject(dc, linepen);
      for (int ix = 0; ix <= width; ++ix) {
        MoveToEx(dc, offsetx + ix * size * tile_width, offsety, NULL);
        LineTo  (dc, offsetx + ix * size * tile_width, offsety + height * size * tile_height);
      }
      for (int iy = 0; iy <= height; ++iy) {
        MoveToEx(dc, offsetx,                offsety + iy * size * tile_height, NULL);
        LineTo  (dc, offsetx + width * size * tile_width, offsety + iy * size * tile_height);
      }
      SelectObject(dc, oldpen);
      DeleteObject(linepen);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapView::OnSize(UINT type, int cx, int cy)
{
  UpdateScrollBars();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapView::OnLButtonDown(UINT flags, CPoint point)
{
  int tile_width  = m_Map->GetTileset().GetTileWidth();
  int tile_height = m_Map->GetTileset().GetTileHeight();

  int zoom_tile_width  = m_Map->GetTileset().GetTileWidth()  * m_ZoomFactor;
  int zoom_tile_height = m_Map->GetTileset().GetTileHeight() * m_ZoomFactor;

  // if the shift key is down, select the current tile
  if (flags & MK_SHIFT) {

    SelectTileUnderPoint(point);

  } else {
    switch(m_CurrentTool) {
      case tool_1x1Tile:
      case tool_3x3Tile:
      case tool_5x5Tile: {
        Click(point); 
      } break;

      case tool_SelectTile: {
        SelectTileUnderPoint(point); 
      } break;

      case tool_CopyArea:
      case tool_FillRectArea:
      case tool_FillArea: {
        int x = point.x / zoom_tile_width  + m_CurrentX;
        int y = point.y / zoom_tile_height + m_CurrentY;
        m_StartCursorTileX = x;
        m_StartCursorTileY = y;
      } break;

      case tool_MoveEntity: {
        int tx = m_CurrentX + (point.x / m_Map->GetTileset().GetTileWidth() / m_ZoomFactor);
        int ty = m_CurrentY + (point.y / m_Map->GetTileset().GetTileHeight() / m_ZoomFactor);

        m_MoveIndex = -1;

        for (int i = 0; i < m_Map->GetNumEntities(); i++) {
          if (m_Map->GetEntity(i).x / m_Map->GetTileset().GetTileWidth() == tx
           && m_Map->GetEntity(i).y / m_Map->GetTileset().GetTileHeight() == ty
           && m_Map->GetEntity(i).layer == m_SelectedLayer) {
             m_MoveIndex = i;
             break;
           }
        }
      } break;

      case tool_ObsSegment: {
        m_StartX = point.x / m_ZoomFactor + m_CurrentX * tile_width;
        m_StartY = point.y / m_ZoomFactor + m_CurrentY * tile_height;
        if(flags & MK_CONTROL) {
          m_StartX = RoundX(m_StartX);
          m_StartY = RoundY(m_StartY);
        }
        m_PreviewX = m_StartX;
        m_PreviewY = m_StartY;
        m_PreviewLineOn = 1;
        m_RedrawPreviewLine = 1;
        Invalidate();
      } break;

      case tool_ObsDeleteSegment: {
        // delete the segment closest to this point (this is crap, FIXME)
        int x = point.x / m_ZoomFactor + m_CurrentX * tile_width;
        int y = point.y / m_ZoomFactor + m_CurrentY * tile_height;
        m_Map->GetLayer(m_SelectedLayer).GetObstructionMap().RemoveSegmentByPoint(x, y);

        m_RedrawWindow = 1;
        Invalidate();
        m_Handler->MV_MapChanged();
      } break;

      case tool_ObsMoveSegmentPoint: {
        //Moves a point on an obstruction segment by deleting the segment
        //and creating a new one with the start point being the point not
        //being moved.
        int x = point.x / m_ZoomFactor + m_CurrentX * tile_width;
        int y = point.y / m_ZoomFactor + m_CurrentY * tile_height;
        int seg = m_Map->GetLayer(m_SelectedLayer).GetObstructionMap().FindSegmentByPoint(x, y);
        if(seg != -1) {
          const sObstructionMap::Segment& segment = m_Map->GetLayer(m_SelectedLayer).GetObstructionMap().GetSegment(seg);
          if((segment.x1 - x) * (segment.x1 - x) + (segment.y1 - y) * (segment.y1 - y) < 
              (segment.x2 - x) * (segment.x2 - x) + (segment.y2 - y) * (segment.y2 - y)) {
            m_StartX = segment.x2;
            m_StartY = segment.y2;
          } else {
            m_StartX = segment.x1;
            m_StartY = segment.y1;
          }
          m_Map->GetLayer(m_SelectedLayer).GetObstructionMap().RemoveSegment(seg);
          m_RedrawWindow = 1;
          Invalidate();
		      m_Handler->MV_MapChanged();
          m_PreviewLineOn = 1;
		    } else {
          return;
		    }
      } break;

      case tool_ZoneAdd: {
        m_StartX = point.x / m_ZoomFactor + m_CurrentX * tile_width;
        m_StartY = point.y / m_ZoomFactor + m_CurrentY * tile_height;
        if(flags & MK_CONTROL) {
          m_StartX = RoundX(m_StartX);
          m_StartY = RoundY(m_StartY);
        }
        m_PreviewX = m_StartX;
        m_PreviewY = m_StartY;
        m_PreviewBoxOn = 1;
        m_RedrawPreviewLine = 1;
        Invalidate();
      } break;

      case tool_ZoneEdit:
      case tool_ZoneMove: {
        int x = point.x / m_ZoomFactor + m_CurrentX * tile_width;
        int y = point.y / m_ZoomFactor + m_CurrentY * tile_height;
        int z = m_Map->FindZone(x, y, m_SelectedLayer);
        m_MoveIndex = z;
        if (z == -1) {
          break;
        }
        sMap::sZone& zone = m_Map->GetZone(z);
        if (abs(x - zone.x1) < abs(x - zone.x2)) {
          m_StartX = zone.x2;
        } else {
          m_StartX = zone.x1;
        }
        if (abs(y - zone.y1) < abs(y - zone.y2)) {
          m_StartY = zone.y2;
        } else {
          m_StartY = zone.y1;
        }
        if(flags & MK_CONTROL) {
          m_PreviewX = RoundX(x);
          m_PreviewY = RoundY(y);
        } else {
          m_PreviewX = x;
          m_PreviewY = y;
        }
        m_PreviewBoxOn = 1;
        m_RedrawPreviewLine = 1;
        Invalidate();
      } break;

      case tool_ZoneDelete: {
        int x = point.x / m_ZoomFactor + m_CurrentX * tile_width;
        int y = point.y / m_ZoomFactor + m_CurrentY * tile_height;
        int z = m_Map->FindZone(x, y, m_SelectedLayer);
        if(z != -1) {
          m_Map->DeleteZone(z);
          m_RedrawWindow = 1;
          Invalidate();
          m_Handler->MV_MapChanged();
        }
      } break;

    }

    // grab all mouse events until the user releases the button
    SetCapture();
    m_Clicked = true;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::GetRedrawRect(int& offset_x, int& offset_y, int& width, int& height)
{
  const int tile_width  = m_Map->GetTileset().GetTileWidth()  * m_ZoomFactor;
  const int tile_height = m_Map->GetTileset().GetTileHeight() * m_ZoomFactor;

  int num_tiles_x = 1;
  int num_tiles_y = 1;
  offset_x = 0;
  offset_y = 0;

  switch (m_CurrentTool) {
    case tool_3x3Tile: num_tiles_x = 3; num_tiles_y = 3; break;
    case tool_5x5Tile: num_tiles_x = 5; num_tiles_y = 5; break;
  }

  switch (m_CurrentTool) {
    case tool_3x3Tile:
    case tool_5x5Tile:
     offset_x = ((tile_width  * num_tiles_x)/2);
     offset_y = ((tile_height * num_tiles_y)/2);
    break;
  }

  if (m_CurrentTool == tool_1x1Tile
   || m_CurrentTool == tool_3x3Tile
   || m_CurrentTool == tool_5x5Tile) {
    if (m_MultiTileWidth && m_MultiTileHeight && m_MultiTileData) {
      num_tiles_x = num_tiles_x * m_MultiTileWidth;
      num_tiles_y = num_tiles_y * m_MultiTileHeight;
      offset_x = 0;
      offset_y = 0;
    }
  }

  width = num_tiles_x * tile_width;
  height = num_tiles_y * tile_height;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapView::OnMouseMove(UINT flags, CPoint point)
{
  int tile_width  = m_Map->GetTileset().GetTileWidth()  * m_ZoomFactor;
  int tile_height = m_Map->GetTileset().GetTileHeight() * m_ZoomFactor;

  int x = point.x / tile_width  + m_CurrentX;
  int y = point.y / tile_height + m_CurrentY;
  if (x <= GetTotalTilesX() && y <= GetTotalTilesY())
  {
    CString Position;
    int pixel_x = (point.x + tile_width  * m_CurrentX) / m_ZoomFactor;
    int pixel_y = (point.y + tile_height * m_CurrentY) / m_ZoomFactor;
    Position.Format("Map tile:(%i,%i) pixel:(%i,%i)", x, y, pixel_x, pixel_y);
    GetStatusBar()->SetWindowText(Position);
  }
  else
    GetStatusBar()->SetWindowText("");


  if (m_Clicked) {
    switch (m_CurrentTool) {
      case tool_1x1Tile:
      case tool_3x3Tile:
      case tool_5x5Tile:
        Click(point);
      break;

      case tool_CopyArea:
      case tool_FillRectArea:
      case tool_FillArea:
        {
          // clear out the old area
          int old_x = (m_StartCursorTileX - m_CurrentX) * tile_width;
          int old_y = (m_StartCursorTileY - m_CurrentY) * tile_height;
          int new_x = (m_CurrentCursorTileX - m_CurrentX) * tile_width;
          int new_y = (m_CurrentCursorTileY - m_CurrentY) * tile_height;
          if (new_x < old_x) std::swap(new_x, old_x);
          if (new_y < old_y) std::swap(new_y, old_y);
          CRgn old_rgn;
          old_rgn.CreateRectRgn(old_x, old_y, new_x + tile_width, new_y + tile_height);

          m_CurrentCursorTileX = x;
          m_CurrentCursorTileY = y;
          //InvalidateRect(&rect);

          // draw the new area
          old_x = (m_StartCursorTileX - m_CurrentX) * tile_width;
          old_y = (m_StartCursorTileY - m_CurrentY) * tile_height;
          new_x = (m_CurrentCursorTileX - m_CurrentX) * tile_width;
          new_y = (m_CurrentCursorTileY - m_CurrentY) * tile_height;
          if (new_x < old_x) std::swap(new_x, old_x);
          if (new_y < old_y) std::swap(new_y, old_y);
          CRgn new_rgn;
          new_rgn.CreateRectRgn(old_x, old_y, new_x + tile_width, new_y + tile_height);
          //InvalidateRect(&new_rect);

          //CRgn total_rgn;
          new_rgn.CombineRgn(&old_rgn, &new_rgn, RGN_OR);
          InvalidateRgn(&new_rgn);
          m_RedrawWindow = 1;
          new_rgn.DeleteObject();
        }
        break;

      case tool_Paste:
        break;

      case tool_ObsMoveSegmentPoint:
      case tool_ObsSegment: {
        int tile_width  = m_Map->GetTileset().GetTileWidth();
        int tile_height = m_Map->GetTileset().GetTileHeight();
        m_PreviewOldX = m_PreviewX;
        m_PreviewOldY = m_PreviewY;
        m_PreviewX = point.x / m_ZoomFactor + m_CurrentX * tile_width;
        m_PreviewY = point.y / m_ZoomFactor + m_CurrentY * tile_height;
        if(flags & MK_CONTROL) {
          m_PreviewX = RoundX(m_PreviewX);
          m_PreviewY = RoundY(m_PreviewY);
        }
        //signal that a preview line erase and update is required.
        m_RedrawPreviewLine = 2;
        Invalidate();        
      } break;

      case tool_ZoneAdd:
      case tool_ZoneEdit: {
        //if(m_MoveIndex != -1)
        {
          int tile_width  = m_Map->GetTileset().GetTileWidth();
          int tile_height = m_Map->GetTileset().GetTileHeight();
          m_PreviewOldX = m_PreviewX;
          m_PreviewOldY = m_PreviewY;
          m_PreviewX = point.x / m_ZoomFactor + m_CurrentX * tile_width;
          m_PreviewY = point.y / m_ZoomFactor + m_CurrentY * tile_height;
          if(flags & MK_CONTROL) {
            m_PreviewX = RoundX(m_PreviewX);
            m_PreviewY = RoundY(m_PreviewY);
          }
          m_RedrawPreviewLine = 2;
          Invalidate();
        }
      } break;

      case tool_ZoneMove:
      {
        int tile_width  = m_Map->GetTileset().GetTileWidth();
        int tile_height = m_Map->GetTileset().GetTileHeight();
        m_PreviewOldX = m_PreviewX;
        m_PreviewOldY = m_PreviewY;
        m_PreviewX = point.x / m_ZoomFactor + m_CurrentX * tile_width;
        m_PreviewY = point.y / m_ZoomFactor + m_CurrentY * tile_height;
        if(flags & MK_CONTROL) {
          m_PreviewX = RoundX(m_PreviewX);
          m_PreviewY = RoundY(m_PreviewY);
        }
        m_RedrawPreviewLine = 2;
        Invalidate();
      }

      case tool_ZoneDelete: {
      } break;

    }
  }

  // do any additional updates here
  switch (m_CurrentTool)
  {
    case tool_1x1Tile:
    case tool_3x3Tile:
    case tool_5x5Tile:
    case tool_SelectTile:
    case tool_Paste:
    case tool_CopyEntity:
    case tool_PasteEntity:
    case tool_MoveEntity: {
      int width;
      int height;
      int offset_x;
      int offset_y;

      static int m_PreviousRedrawTool = m_CurrentTool;
      int temp_tool = m_CurrentTool;
      m_CurrentTool = m_PreviousRedrawTool;
      GetRedrawRect(offset_x, offset_y, width, height);
      m_CurrentTool = m_PreviousRedrawTool = temp_tool;

      int old_x = (m_CurrentCursorTileX - m_CurrentX) * tile_width;
      int old_y = (m_CurrentCursorTileY - m_CurrentY) * tile_height;

      RECT old_rect = { old_x - offset_x,
                        old_y - offset_y,
                        old_x + width,
                        old_y + height  };

      m_RedrawWindow = 1;
      InvalidateRect(&old_rect);
  
      m_CurrentCursorTileX = x;
      m_CurrentCursorTileY = y;

      GetRedrawRect(offset_x, offset_y, width, height);

      // refresh the new tile(s)
      int new_x = (x - m_CurrentX) * tile_width;
      int new_y = (y - m_CurrentY) * tile_height;

      RECT new_rect = { new_x - offset_x,
                        new_y - offset_y,
                        new_x + width,
                        new_y + height }; 
      
      m_RedrawWindow = 1;
      InvalidateRect(&new_rect, true);
    } break;

    case tool_CopyArea:
    case tool_FillRectArea:
    case tool_FillArea: {
      if (!m_Clicked) {
        int old_x = (m_CurrentCursorTileX - m_CurrentX) * tile_width;
        int old_y = (m_CurrentCursorTileY - m_CurrentY) * tile_height;
        RECT old_rect = { old_x, old_y, old_x + tile_width, old_y + tile_height };
        m_RedrawWindow = 1;
        InvalidateRect(&old_rect);
  
        m_CurrentCursorTileX = x;
        m_CurrentCursorTileY = y;

        // refresh the new tile
        int new_y = (y - m_CurrentY) * tile_height;
        int new_x = (x - m_CurrentX) * tile_width;
        RECT new_rect = { new_x, new_y, new_x + tile_width, new_y + tile_height };
        m_RedrawWindow = 1;
        InvalidateRect(&new_rect, true);
      } break;
    }

  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapView::OnLButtonUp(UINT flags, CPoint point)
{
  int tile_width  = m_Map->GetTileset().GetTileWidth();
  int tile_height = m_Map->GetTileset().GetTileHeight();

  int zoom_tile_width  = m_Map->GetTileset().GetTileWidth()  * m_ZoomFactor;
  int zoom_tile_height = m_Map->GetTileset().GetTileHeight() * m_ZoomFactor;

  switch (m_CurrentTool)
  {
    case tool_FillRectArea: {
      FillRectArea();
    } break;

    case tool_FillArea: {
      FillArea();
    } break;

    case tool_CopyEntity: {
      EntityCopy(point);
    } break;

    case tool_MoveEntity: {
      if (m_MoveIndex != -1) {
        int tx = m_CurrentX + (point.x / m_Map->GetTileset().GetTileWidth() / m_ZoomFactor);
        int ty = m_CurrentY + (point.y / m_Map->GetTileset().GetTileHeight() / m_ZoomFactor);

        // convert tile coordinates to pixel coordinates
        int px = tx * tile_width + (tile_width - 1) / 2;
        int py = ty * tile_height + (tile_height - 1) / 2;

        m_Map->GetEntity(m_MoveIndex).x = px;
        m_Map->GetEntity(m_MoveIndex).y = py;

        m_MoveIndex = -1;
        Invalidate();
        m_Handler->MV_MapChanged();
      }
    } break;

    case tool_CopyArea: {
      // clear out the old area
      int old_x = (m_StartCursorTileX - m_CurrentX)   * zoom_tile_width;
      int old_y = (m_StartCursorTileY - m_CurrentY)   * zoom_tile_height;
      int new_x = (m_CurrentCursorTileX - m_CurrentX) * zoom_tile_width;
      int new_y = (m_CurrentCursorTileY - m_CurrentY) * zoom_tile_height;
      if (new_x < old_x) std::swap(new_x, old_x);
      if (new_y < old_y) std::swap(new_y, old_y);
      RECT rect = { old_x, old_y, new_x + zoom_tile_width, new_y + zoom_tile_height };

      m_RedrawWindow = 1;
      InvalidateRect(&rect);
      LayerAreaCopy();
    } break;

    case tool_Paste: {
      PasteMapUnderPoint(point);
    } break;

    case tool_PasteEntity: {
      EntityPaste(point);
    } break;

    case tool_ObsMoveSegmentPoint:
    case tool_ObsSegment: {
      int x = point.x / m_ZoomFactor + m_CurrentX * tile_width;
      int y = point.y / m_ZoomFactor + m_CurrentY * tile_height;
      m_PreviewLineOn = 0;
      if(flags & MK_CONTROL) {
        x = RoundX(x);
        y = RoundY(y);
      }
      m_Map->GetLayer(m_SelectedLayer).GetObstructionMap().AddSegment(
        m_StartX, m_StartY, x, y
      );

      // we should just invalidate the area that the new line covers...
      m_RedrawWindow = 1;
      Invalidate();
      m_Handler->MV_MapChanged();
    } break;

    case tool_ObsDeleteSegment: {
    } break;

    case tool_ZoneAdd: {
      //if(m_MoveIndex != -1)
      {
        int x = point.x / m_ZoomFactor + m_CurrentX * tile_width;
        int y = point.y / m_ZoomFactor + m_CurrentY * tile_height;
        sMap::sZone zone;
        m_PreviewLineOn = 0;
        if(flags & MK_CONTROL) {
          x = RoundX(x);
          y = RoundY(y);
        }
        zone.x1 = m_StartX;
        zone.y1 = m_StartY;
        zone.x2 = x;
        zone.y2 = y;

        // don't allow zones that are only one pixel big, because
        // people can't see them.
        if (zone.x1 == zone.x2 || zone.y1 == zone.y2) {
          m_PreviewBoxOn = 0;
          m_RedrawWindow = 1;
          Invalidate();
          break;
        }

        zone.layer = m_SelectedLayer;
        zone.reactivate_in_num_steps = 8;
        zone.script = "";
        m_Map->AddZone(zone);
        m_PreviewBoxOn = 0;
        m_RedrawWindow = 1;
        Invalidate();
        m_Handler->MV_MapChanged();
      }
    } break;

    case tool_ZoneEdit: {
      if(m_MoveIndex != -1)
      {
        int x = point.x / m_ZoomFactor + m_CurrentX * tile_width;
        int y = point.y / m_ZoomFactor + m_CurrentY * tile_height;
        m_PreviewLineOn = 0;
        if(flags & MK_CONTROL) {
          x = RoundX(x);
          y = RoundY(y);
        }

        // ensure that the zone is atleast one pixel big
        if (m_StartX == x)
          x+=1;
        if (m_StartY == y)
          y+=1;

        m_Map->UpdateZone(m_MoveIndex, m_StartX, m_StartY, x, y);
        m_PreviewBoxOn = 0;
        m_RedrawWindow = 1;
        Invalidate();
        m_Handler->MV_MapChanged();
      }
    } break;

    case tool_ZoneMove: {
      if(m_MoveIndex != -1)
      {
        int x = point.x / m_ZoomFactor + m_CurrentX * tile_width;
        int y = point.y / m_ZoomFactor + m_CurrentY * tile_height;
        m_PreviewLineOn = 0;
        if(flags & MK_CONTROL) {
          x = RoundX(x);
          y = RoundY(y);
        }

        sMap::sZone& z = m_Map->GetZone(m_MoveIndex);
        int width = z.x2 - z.x1;
        int height = z.y2 - z.y1;
        z.x1 = x;
        z.y1 = y;
        z.x2 = x + width;
        z.y2 = y + height;

        m_PreviewBoxOn = 0;
        m_RedrawWindow = 1;
        Invalidate();
        m_Handler->MV_MapChanged();
      }
    } break;

    case tool_ZoneDelete: {
    } break;

/*
    case mObsRectangle: {
      int x = m_CurrentX * tile_width  + Point.x;
      int y = m_CurrentY * tile_height + Point.y;

      sObstructionMap::Rectangle rectangle;
      rectangle.p1.x = m_StartX;
      rectangle.p1.y = m_StartY;
      rectangle.p2.x = x;
      rectangle.p2.y = y;

      m_Map->GetLayer(m_SelectedLayer).GetObstructionMap().AddRectangle(rectangle);
      Invalidate();
      m_Handler->MV_MapChanged();

    } break;
*/
  }

  m_Clicked = false;
  ReleaseCapture();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapView::OnRButtonUp(UINT flags, CPoint point)
{
  // get a handle to the menu
  HMENU _menu = LoadMenu(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_MAPVIEW));
  HMENU menu = GetSubMenu(_menu, 0);

  int tile_width = m_Map->GetTileset().GetTileWidth();
  int tile_height = m_Map->GetTileset().GetTileHeight();

  // tile coordinates
  int tx = (point.x / m_Map->GetTileset().GetTileWidth()) / m_ZoomFactor + m_CurrentX;
  int ty = (point.y / m_Map->GetTileset().GetTileHeight()) / m_ZoomFactor + m_CurrentY;

  // map coordinates
  int x = point.x / m_ZoomFactor + m_CurrentX * tile_width;
  int y = point.y / m_ZoomFactor + m_CurrentY * tile_height;

  int z = m_Map->FindZone(x, y, m_SelectedLayer);

  // validate the menu items
  bool on_entity = false;
  for (int i = 0; i < m_Map->GetNumEntities(); i++)
    if (m_Map->GetEntity(i).x / m_Map->GetTileset().GetTileWidth() == tx &&
        m_Map->GetEntity(i).y / m_Map->GetTileset().GetTileHeight() == ty)
      on_entity = true;

  if (on_entity == false) {
    EnableMenuItem(menu, ID_MAPVIEW_DELETEENTITY, MF_BYCOMMAND | MF_GRAYED);
    EnableMenuItem(menu, ID_MAPVIEW_EDITENTITY,   MF_BYCOMMAND | MF_GRAYED);
  } else {
    EnableMenuItem(menu, ID_MAPVIEW_INSERTENTITY_PERSON,  MF_BYCOMMAND | MF_GRAYED);
    EnableMenuItem(menu, ID_MAPVIEW_INSERTENTITY_TRIGGER, MF_BYCOMMAND | MF_GRAYED);
  }

  switch (GetZoomFactor()) {
    case 1: CheckMenuItem(menu, ID_MAPVIEW_ZOOM_1X, MF_BYCOMMAND | MF_CHECKED); break;
    case 2: CheckMenuItem(menu, ID_MAPVIEW_ZOOM_2X, MF_BYCOMMAND | MF_CHECKED); break;
    case 4: CheckMenuItem(menu, ID_MAPVIEW_ZOOM_4X, MF_BYCOMMAND | MF_CHECKED); break;
    case 8: CheckMenuItem(menu, ID_MAPVIEW_ZOOM_8X, MF_BYCOMMAND | MF_CHECKED); break;
  }

  if(z == -1) {
    EnableMenuItem(menu, ID_MAPVIEW_ZONEEDIT,  MF_BYCOMMAND | MF_GRAYED);
  }

  if (m_ShowGrid) {
    CheckMenuItem(menu, ID_MAPVIEW_VIEWGRID, MF_BYCOMMAND | MF_CHECKED);
  }

  if (m_ShowTileObstructions) {
    CheckMenuItem(menu, ID_MAPVIEW_VIEWTILEOBSTRUCTIONS, MF_BYCOMMAND | MF_CHECKED);
  }

  /*
	if (m_ShowAnimations) {
		CheckMenuItem(menu, ID_MAPVIEW_VIEWANIMATIONS, MF_BYCOMMAND | MF_CHECKED);
	}
  */

  switch (m_SpritesetDrawType) {
    case SDT_ICON:       CheckMenuItem(menu, ID_MAPVIEW_VIEWPERSONS_ICON,      MF_BYCOMMAND | MF_CHECKED); break;
    case SDT_MINI_IMAGE: CheckMenuItem(menu, ID_MAPVIEW_VIEWPERSONS_MINIIMAGE, MF_BYCOMMAND | MF_CHECKED); break;
    case SDT_IMAGE:      CheckMenuItem(menu, ID_MAPVIEW_VIEWPERSONS_IMAGE,     MF_BYCOMMAND | MF_CHECKED); break;
  }

  // disable the select tile menu if out of range
  if ( (tx >= 0 && tx < m_Map->GetLayer(m_SelectedLayer).GetWidth()
   && ty >= 0 && ty < m_Map->GetLayer(m_SelectedLayer).GetHeight()) == false ) {
    EnableMenuItem(menu, ID_MAPVIEW_SELECTTILE,  MF_BYCOMMAND | MF_GRAYED);
  }

  // show the popup menu
  CPoint Screen = point;
  ClientToScreen(&Screen);
  BOOL retval = TrackPopupMenu(
    menu,
    TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
    Screen.x,
    Screen.y,
    0,
    m_hWnd,
    NULL);

  // convert tile coordinates to pixel coordinates
  int px = tx * tile_width + (tile_width - 1) / 2;
  int py = ty * tile_height + (tile_height - 1) / 2;

  // execute command
  switch (retval)
  {
    case ID_MAPVIEW_SELECTTILE:
    {
      SelectTileUnderPoint(point);
      break;
    }

    case ID_MAPVIEW_SETENTRYPOINT:
    {
      m_Map->SetStartX(px);
      m_Map->SetStartY(py);
      m_Map->SetStartLayer(m_SelectedLayer);

      m_RedrawWindow = 1;
      Invalidate();
      m_Handler->MV_MapChanged();
      break;
    }

    case ID_MAPVIEW_FILL: {
      if (MessageBox("Are you sure?", "Fill Layer", MB_YESNO) == IDYES) {
        
        sLayer& layer = m_Map->GetLayer(m_SelectedLayer);
        for (int iy = 0; iy < layer.GetHeight(); iy++) {
          for (int ix = 0; ix < layer.GetWidth(); ix++) {
            layer.SetTile(ix, iy, m_SelectedTile);
          }
        }
        m_RedrawWindow = 1;
        Invalidate();
        m_Handler->MV_MapChanged();

      }

      break;
    }

    case ID_MAPVIEW_INSERTENTITY_PERSON:
    {
      sPersonEntity person;
      person.x = px;
      person.y = py;
      person.layer = m_SelectedLayer;

      CEntityPersonDialog dialog(person, m_Map);
      if (dialog.DoModal() == IDOK)
      {
        // insert it into the map
        sPersonEntity* entity = new sPersonEntity(person);
        if (entity) {
          m_Map->AddEntity(entity);
          m_RedrawWindow = 1;
          Invalidate();
          m_Handler->MV_MapChanged();
        }
      }
      break;
    }

    case ID_MAPVIEW_INSERTENTITY_TRIGGER:
    {
      sTriggerEntity trigger;
      trigger.x = px;
      trigger.y = py;
      trigger.layer = m_SelectedLayer;

      CEntityTriggerDialog dialog(trigger, m_Map);
      if (dialog.DoModal() == IDOK)
      {
        // insert it into the map
        sTriggerEntity* entity = new sTriggerEntity(trigger);
        if (entity) {
          m_Map->AddEntity(entity);
          m_RedrawWindow = 1;
          Invalidate();
          m_Handler->MV_MapChanged();
        }
      }
      break;
    }

    case ID_MAPVIEW_DELETEENTITY:
    {
      int result = MessageBox("Delete selected entity?", "Delete Entity", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
      if (result == IDNO) {
        break;
      }

      for (int ie = 0; ie < m_Map->GetNumEntities(); ie++) {
        sEntity& e = m_Map->GetEntity(ie);
        if (e.x >= tx * tile_width && e.x < tx * tile_width + tile_width &&
            e.y >= ty * tile_height && e.y < ty * tile_height + tile_height) {

          m_Map->DeleteEntity(ie);

          m_RedrawWindow = 1;
          Invalidate();
          m_Handler->MV_MapChanged();
          break;
        }
      }
      break;
    }

    case ID_MAPVIEW_EDITENTITY:
    {
      for (int ie = 0; ie < m_Map->GetNumEntities(); ie++) {
        sEntity& e = m_Map->GetEntity(ie);
        if (e.x >= tx * tile_width && e.x < tx * tile_width + tile_width &&
            e.y >= ty * tile_height && e.y < ty * tile_height + tile_height) {

          switch (e.GetEntityType()) {
            case sEntity::PERSON: {
              CEntityPersonDialog dialog((sPersonEntity&)e, m_Map);
              if (dialog.DoModal() == IDOK) {
                m_Handler->MV_MapChanged();
              }
              break;
            }
          
            case sEntity::TRIGGER: {
              CEntityTriggerDialog dialog((sTriggerEntity&)e, m_Map);
              if (dialog.DoModal() == IDOK) {
                m_Handler->MV_MapChanged();
              }
              break;
            }

          } // end switch
        }
      }
      break;
    }

    case ID_MAPVIEW_ZONEEDIT:
      {
        CZoneEditDialog dialog(m_Map->GetZone(z), m_Map);
        if(dialog.DoModal() == IDOK) {
          m_Handler->MV_MapChanged();
          Invalidate();
        }
      }
      break;

    case ID_MAPVIEW_ZOOM_1X:
      SetZoomFactor(1);
      //OnHScrollChanged(tx - ((GetPageSizeX() + 1) / 2));
      //OnVScrollChanged(ty - ((GetPageSizeY() + 1) / 2));
      //Invalidate();
      break;

    case ID_MAPVIEW_ZOOM_2X:
      SetZoomFactor(2);
      //OnHScrollChanged(tx - ((GetPageSizeX() + 1) / 2));
      //OnVScrollChanged(ty - ((GetPageSizeY() + 1) / 2));
      //Invalidate();
      break;

    case ID_MAPVIEW_ZOOM_4X:
      SetZoomFactor(4);
      //OnHScrollChanged(tx - ((GetPageSizeX() + 1) / 2));
      //OnVScrollChanged(ty - ((GetPageSizeY() + 1) / 2));
      //Invalidate();
      break;

    case ID_MAPVIEW_ZOOM_8X:
      SetZoomFactor(8);
      //OnHScrollChanged(tx - ((GetPageSizeX() + 1) / 2));
      //OnVScrollChanged(ty - ((GetPageSizeY() + 1) / 2));
      //Invalidate();
      break;

    case ID_MAPVIEW_VIEWGRID:
      m_ShowGrid = !m_ShowGrid;
      m_RedrawWindow = 1;
      Invalidate();
      //m_Handler->MV_MapChanged();
      break;	  

    case ID_MAPVIEW_VIEWTILEOBSTRUCTIONS:
      m_ShowTileObstructions = !m_ShowTileObstructions;

      UpdateObstructionTiles();

      m_RedrawWindow = 1;
      Invalidate();
      //m_Handler->MV_MapChanged();
      break;

		case ID_MAPVIEW_VIEWANIMATIONS:
	    //m_ShowAnimations = !m_ShowAnimations;  
			//InitAnimations();
      //Invalidate();
      break;

     case ID_MAPVIEW_VIEWPERSONS_ICON:
       m_RedrawWindow = 1;
       Invalidate();
       m_SpritesetDrawType = SDT_ICON;
     break;

     case ID_MAPVIEW_VIEWPERSONS_MINIIMAGE:
       m_RedrawWindow = 1;
       Invalidate();
       m_SpritesetDrawType = SDT_MINI_IMAGE;
     break;

     case ID_MAPVIEW_VIEWPERSONS_IMAGE:
       m_RedrawWindow = 1;
       Invalidate();
       m_SpritesetDrawType = SDT_IMAGE;
     break;

  }

  DestroyMenu(_menu);
}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::OnHScrollChanged(int x)
{
  // do the scrolling thing
  int old_x = m_CurrentX;
  m_CurrentX = x;
  int new_x = x;
  CDC* dc_ = GetDC();
  HDC dc = dc_->m_hDC;

  HRGN region = CreateRectRgn(0, 0, 0, 0);
  int factor = m_ZoomFactor * m_Map->GetTileset().GetTileWidth();
  ScrollDC(dc, (old_x - new_x) * factor, 0, NULL, NULL, region, NULL);
  ::InvalidateRgn(m_hWnd, region, FALSE);
  m_RedrawWindow = 1;
  DeleteObject(region);

  ReleaseDC(dc_);
}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::OnVScrollChanged(int y)
{
  // do the scrolling thing
  int old_y = m_CurrentY;
  m_CurrentY = y;
  int new_y = y;
  CDC* dc_ = GetDC();
  HDC dc = dc_->m_hDC;

  HRGN region = CreateRectRgn(0, 0, 0, 0);
  int factor = m_ZoomFactor * m_Map->GetTileset().GetTileHeight();
  ScrollDC(dc, 0, (old_y - new_y) * factor, NULL, NULL, region, NULL);
  ::InvalidateRgn(m_hWnd, region, FALSE);
  m_RedrawWindow = 1;
  DeleteObject(region);

  ReleaseDC(dc_);
}

////////////////////////////////////////////////////////////////////////////////

void
CMapView::TP_ToolSelected(int tool)
{
  // do something
  m_CurrentTool = tool;
  Invalidate(); // we should just invalidate the tiles needed according to the old/new tools
}

////////////////////////////////////////////////////////////////////////////////

/*
afx_msg void
CMapView::OnTimer(UINT event)
{
	if (!m_ShowAnimations || m_MenuShown)
    return;

  //invalidate all visible animated tiles, changing their shown tile
  int NumTilesX = GetPageSizeX()+1;
  int NumTilesY = GetPageSizeY()+1;
  int tile_width  = m_Map->GetTileset().GetTileWidth();
  int tile_height = m_Map->GetTileset().GetTileHeight();
  int layers = m_Map->GetNumLayers();
  CDC * pDC = GetDC();
	
	m_FrameTick++;

	int numTiles = m_Map->GetTileset().GetNumTiles();
	for (int i = 0; i < numTiles; i++)
	{
		  m_Map->GetTileset().GetTile(i).
				UpdateAnimation(m_FrameTick, m_Map->GetTileset());
	}

  for (int ix = 0; ix < NumTilesX; ix++) 
  {
		for (int iy = 0; iy < NumTilesY; iy++) 
		{
			for (int i = 0; i < layers; i++)
			{
				const sLayer& layer = m_Map->GetLayer(i);
				
				if (layer.IsVisible() == false) 
				{
					continue;
				}

				if (m_CurrentX + ix < layer.GetWidth() &&
					  m_CurrentY + iy < layer.GetHeight()) 
				{
					int tile = layer.GetTile(m_CurrentX + ix,m_CurrentY + iy);
					if (m_Map->GetTileset().GetTile(tile).IsUpdated()) 
					{
						RECT Rect =
						{
							ix  * tile_width  * m_ZoomFactor,
							iy  * tile_height * m_ZoomFactor,
							ix  * tile_width  * m_ZoomFactor + tile_width  * m_ZoomFactor,
							iy  * tile_height * m_ZoomFactor + tile_height * m_ZoomFactor,
						};
						
						//This refers to the clipping region
						// (client area excluding hidden parts)
						// instead of the update region (area needed to be redrawn).
						if (pDC->RectVisible(&Rect))
						{	
							InvalidateRect(&Rect);
					  }
						break;
					}
				}
			}
		}
	}

  m_RedrawWindow = 1;
}
*/

////////////////////////////////////////////////////////////////////////////////

void
CMapView::OnToolChanged(UINT id)
{
  switch (id) {
    case IDI_MAPTOOL_1X1: m_CurrentTool = tool_1x1Tile; break;
    case IDI_MAPTOOL_3X3: m_CurrentTool = tool_3x3Tile; break;
    case IDI_MAPTOOL_5X5: m_CurrentTool = tool_5x5Tile; break;
    case IDI_MAPTOOL_SELECTTILE: m_CurrentTool = tool_SelectTile; break;
    case IDI_MAPTOOL_FILLRECTAREA: m_CurrentTool = tool_FillRectArea; break;
    case IDI_MAPTOOL_FILLAREA: m_CurrentTool = tool_FillArea; break;
    case IDI_MAPTOOL_COPYAREA: m_CurrentTool = tool_CopyArea; break;
    case IDI_MAPTOOL_PASTE: m_CurrentTool = tool_Paste; break;
    case IDI_MAPTOOL_COPYENTITY: m_CurrentTool = tool_CopyEntity; break;
    case IDI_MAPTOOL_PASTEENTITY: m_CurrentTool = tool_PasteEntity; break;
    case IDI_MAPTOOL_MOVEENTITY: m_CurrentTool = tool_MoveEntity; break;
    case IDI_MAPTOOL_OBS_SEGMENT: m_CurrentTool = tool_ObsSegment; break;
    case IDI_MAPTOOL_OBS_DELETE: m_CurrentTool = tool_ObsDeleteSegment; break;
    case IDI_MAPTOOL_OBS_MOVE_PT: m_CurrentTool = tool_ObsMoveSegmentPoint; break;
    case IDI_MAPTOOL_ZONEADD: m_CurrentTool = tool_ZoneAdd; break;
    case IDI_MAPTOOL_ZONEEDIT: m_CurrentTool = tool_ZoneEdit; break;
    case IDI_MAPTOOL_ZONEMOVE: m_CurrentTool = tool_ZoneMove; break;
    case IDI_MAPTOOL_ZONEDELETE: m_CurrentTool = tool_ZoneDelete; break;
  }

  TP_ToolSelected(m_CurrentTool);
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CMapView::IsToolAvailable(UINT id)
{
  BOOL available = FALSE;

  switch (id) {
    case IDI_MAPTOOL_1X1: available = TRUE; break;
    case IDI_MAPTOOL_3X3: available = TRUE; break;
    case IDI_MAPTOOL_5X5: available = TRUE; break;
    case IDI_MAPTOOL_SELECTTILE:   available = TRUE; break;
    case IDI_MAPTOOL_FILLRECTAREA: available = TRUE; break;
    case IDI_MAPTOOL_FILLAREA: available = TRUE; break;
    case IDI_MAPTOOL_COPYAREA: available = TRUE; break;
    case IDI_MAPTOOL_PASTE:
      if (m_Clipboard != NULL && OpenClipboard() != FALSE) {
        if (m_Clipboard->IsBitmapImageOnClipboard()
          || m_Clipboard->IsFlatImageOnClipbard()
          || IsClipboardFormatAvailable(s_MapAreaClipboardFormat)) {
          available = TRUE;
        }
        CloseClipboard();
      }      
    break;
    case IDI_MAPTOOL_COPYENTITY:  available = (m_Map->GetNumEntities() > 0) ? TRUE : FALSE; break;
    case IDI_MAPTOOL_PASTEENTITY: 
      if (OpenClipboard() != FALSE) {
        if (IsClipboardFormatAvailable(s_MapEntityClipboardFormat)) {
          available = TRUE;
        }
        CloseClipboard();
      }      
    break;
    case IDI_MAPTOOL_MOVEENTITY:  available = (m_Map->GetNumEntities() > 0) ? TRUE : FALSE; break;
    case IDI_MAPTOOL_OBS_SEGMENT: available = TRUE; break;
    case IDI_MAPTOOL_OBS_DELETE:  available = (m_Map->GetLayer(m_SelectedLayer).GetObstructionMap().GetNumSegments() > 0) ? TRUE : FALSE; break;
    case IDI_MAPTOOL_OBS_MOVE_PT: available = (m_Map->GetLayer(m_SelectedLayer).GetObstructionMap().GetNumSegments() > 0) ? TRUE : FALSE; break;
    case IDI_MAPTOOL_ZONEADD:  available = TRUE; break;
    case IDI_MAPTOOL_ZONEEDIT:   available = (m_Map->GetNumZones() > 0) ? TRUE : FALSE; break;
    case IDI_MAPTOOL_ZONEMOVE:   available = (m_Map->GetNumZones() > 0) ? TRUE : FALSE; break;
    case IDI_MAPTOOL_ZONEDELETE: available = (m_Map->GetNumZones() > 0) ? TRUE : FALSE; break;
  }

  return available;
}

////////////////////////////////////////////////////////////////////////////////
