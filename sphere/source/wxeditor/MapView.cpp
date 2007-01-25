#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include "MapView.hpp"
#include "Editor.hpp"
#include "EntityPersonDialog.hpp"
#include "EntityTriggerDialog.hpp"
#include "ZoneEditDialog.hpp"
#include "../common/primitives.hpp"
//#include "resource.h"
#include "IDs.hpp"

// because Windows sucks
#ifdef DrawText
#undef DrawText
#endif

//static int s_MapViewID = 2000;
/*todo:
static int s_MapAreaClipboardFormat;
static int s_MapEntityClipboardFormat;
*/

/*
BEGIN_MESSAGE_MAP(CMapView, CScrollWindow)

  ON_WM_DESTROY()
  ON_WM_PAINT()
  ON_WM_SIZE()
  ON_WM_LBUTTONDOWN()
  ON_WM_MOUSEMOVE()
  ON_WM_LBUTTONUP()
  ON_WM_RBUTTONUP()
  ON_WM_TIMER()

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wMapView, wScrollWindow)
  EVT_PAINT(wMapView::OnPaint)
  EVT_SIZE(wMapView::OnSize)
  EVT_LEFT_DOWN(wMapView::OnLButtonDown)
  EVT_LEFT_UP(wMapView::OnLButtonUp)
  EVT_RIGHT_UP(wMapView::OnRButtonUp)
  EVT_MOTION(wMapView::OnMouseMove)

  EVT_MENU(wID_MAPVIEW_SELECTTILE, wMapView::OnMenu)
  EVT_MENU(wID_MAPVIEW_SETENTRYPOINT, wMapView::OnMenu)
  EVT_MENU(wID_MAPVIEW_FILL, wMapView::OnMenu)
  EVT_MENU(wID_MAPVIEW_INSERTENTITY_PERSON, wMapView::OnMenu)
  EVT_MENU(wID_MAPVIEW_INSERTENTITY_TRIGGER, wMapView::OnMenu)
  EVT_MENU(wID_MAPVIEW_DELETEENTITY, wMapView::OnMenu)
  EVT_MENU(wID_MAPVIEW_EDITENTITY, wMapView::OnMenu)
  EVT_MENU(wID_MAPVIEW_ZONEEDIT, wMapView::OnMenu)
  EVT_MENU(wID_MAPVIEW_ZOOM_1X, wMapView::OnMenu)
  EVT_MENU(wID_MAPVIEW_ZOOM_2X, wMapView::OnMenu)
  EVT_MENU(wID_MAPVIEW_ZOOM_4X, wMapView::OnMenu)
  EVT_MENU(wID_MAPVIEW_ZOOM_8X, wMapView::OnMenu)
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wMapView::wMapView(wxWindow* parent, wDocumentWindow* owner, IMapViewHandler* handler, sMap* map)
: wScrollWindow(parent)
, m_Handler(NULL)

, m_ToolPalette(NULL)

, m_Map(NULL)

, m_BlitTile(NULL)
, m_ZoomFactor(1)
, m_ObstructionColor(CreateRGB(255, 0, 255))
, m_HighlightColor(CreateRGB(255, 255, 0))
, m_ZoneColor(CreateRGB(255, 0, 0))
, m_CurrentTool(tool_1x1Tile)

, m_CurrentX(0)
, m_CurrentY(0)
, m_CurrentCursorTileX(-1)
, m_CurrentCursorTileY(-1)
, m_StartCursorTileX(-1)
, m_StartCursorTileY(-1)
, m_PreviewLineOn(0)
, m_PreviewBoxOn(0)

, m_RedrawWindow(0)
, m_RedrawPreviewLine(0)

, m_SelectedTile(0)
, m_SelectedLayer(0)

, m_Clicked(false)

{
/*todo:
  s_MapAreaClipboardFormat = RegisterClipboardFormat("MapAreaSelection32");
  s_MapEntityClipboardFormat = RegisterClipboardFormat("MapEntitySelection32");
*/
  m_Handler = handler;
  m_Map = map;

  m_BlitTile = new wDIBSection(
    map->GetTileset().GetTileWidth(),
    map->GetTileset().GetTileHeight());

  // create tool palette
  m_ToolPalette = new wMapToolPalette(owner, this);

  UpdateScrollBars();

  m_Menu = new wxMenu();
  m_Menu->Append(wID_MAPVIEW_SELECTTILE, "Select Tile");
  m_Menu->AppendSeparator();
  m_Menu->Append(wID_MAPVIEW_SETENTRYPOINT, "Set Entry Point");
  m_Menu->AppendSeparator();
  m_Menu->Append(wID_MAPVIEW_FILL, "Fill Layer");
  m_Menu->Append(wID_MAPVIEW_INSERTENTITY_PERSON, "Insert Person");
  m_Menu->Append(wID_MAPVIEW_INSERTENTITY_TRIGGER, "Insert Trigger");
  m_Menu->Append(wID_MAPVIEW_DELETEENTITY, "Delete Entity");
  m_Menu->Append(wID_MAPVIEW_EDITENTITY, "Edit Entity");
  m_Menu->Append(wID_MAPVIEW_ZONEEDIT, "Edit Zone");
  m_Menu->AppendSeparator();
  m_Menu->Append(wID_MAPVIEW_ZOOM_1X, "Zoom x1", "", TRUE);
  m_Menu->Append(wID_MAPVIEW_ZOOM_2X, "Zoom x2", "", TRUE);
  m_Menu->Append(wID_MAPVIEW_ZOOM_4X, "Zoom x4", "", TRUE);
  m_Menu->Append(wID_MAPVIEW_ZOOM_8X, "Zoom x8", "", TRUE);
}

////////////////////////////////////////////////////////////////////////////////

wMapView::~wMapView()
{
  // destroy the blit DIB
  delete m_BlitTile;
  delete m_Menu;
  //DestroyWindow();
}

/*
////////////////////////////////////////////////////////////////////////////////

BOOL
CMapView::Create(CDocumentWindow* owner, IMapViewHandler* handler, CWnd* parent, sMap* map)
{
  m_Handler = handler;
  m_Map = map;

  m_BlitTile = new CDIBSection(
    map->GetTileset().GetTileWidth(),
    map->GetTileset().GetTileHeight(),
    32);

  // create tool palette
  m_ToolPalette = new CMapToolPalette(owner, this);

  // create the window object
  BOOL retval = CWnd::Create(
    AfxRegisterWndClass(0, LoadCursor(NULL, IDC_ARROW), NULL, NULL),
    "",
    WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL,
    CRect(0, 0, 0, 0),
    parent,
    s_MapViewID++);

  UpdateScrollBars();

  return retval;
}
*/

////////////////////////////////////////////////////////////////////////////////

int
wMapView::GetZoomFactor()
{
  return m_ZoomFactor;
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::SetZoomFactor(int factor)
{
  m_ZoomFactor = factor;

  delete m_BlitTile;
  m_BlitTile = new wDIBSection(
    m_Map->GetTileset().GetTileWidth()  * factor,
    m_Map->GetTileset().GetTileHeight() * factor
  );
  m_RedrawWindow = 1;
  Refresh();
  UpdateScrollBars();
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::TilesetChanged()
{
  delete m_BlitTile;
  m_BlitTile = new wDIBSection(
    m_Map->GetTileset().GetTileWidth()  * m_ZoomFactor,
    m_Map->GetTileset().GetTileHeight() * m_ZoomFactor
  );

  m_RedrawWindow = 1;
  Refresh();
  UpdateScrollBars();
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::SelectTile(int tile)
{
  m_SelectedTile = tile;
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::SelectLayer(int layer)
{
  m_SelectedLayer = layer;
  m_RedrawWindow = 1;
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::UpdateScrollBars()
{
  SetHScrollRange(GetTotalTilesX() + 1, GetPageSizeX());
  SetVScrollRange(GetTotalTilesY() + 1, GetPageSizeY());
}

////////////////////////////////////////////////////////////////////////////////

int
wMapView::GetPageSizeX()
{
  wxSize ClientSize = GetClientSize();
  return ClientSize.GetWidth() / m_Map->GetTileset().GetTileWidth() / m_ZoomFactor;
}

////////////////////////////////////////////////////////////////////////////////

int
wMapView::GetPageSizeY()
{
  wxSize ClientSize = GetClientSize();
  return ClientSize.GetHeight() / m_Map->GetTileset().GetTileHeight() / m_ZoomFactor;
}

////////////////////////////////////////////////////////////////////////////////

int
wMapView::GetTotalTilesX()
{
  int max_x = 0;
  for (int i = 0; i < m_Map->GetNumLayers(); i++)
    if (m_Map->GetLayer(i).GetWidth() - 1 > max_x)
      max_x = m_Map->GetLayer(i).GetWidth() - 1;
  return max_x;
}

////////////////////////////////////////////////////////////////////////////////

int
wMapView::GetTotalTilesY()
{
  int max_y = 0;
  for (int i = 0; i < m_Map->GetNumLayers(); i++)
    if (m_Map->GetLayer(i).GetHeight() - 1 > max_y)
      max_y = m_Map->GetLayer(i).GetHeight() - 1;
  return max_y;
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::Click(wxPoint point)
{
  int tx = m_CurrentX + (point.x / m_Map->GetTileset().GetTileWidth() / m_ZoomFactor);
  int ty = m_CurrentY + (point.y / m_Map->GetTileset().GetTileHeight() / m_ZoomFactor);

  // change the tile
  bool map_changed = false;

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
wMapView::SetTile(int tx, int ty)
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
    
    wxRect Rect(
      (tx - m_CurrentX) * tile_width  * m_ZoomFactor,
      (ty - m_CurrentY) * tile_height * m_ZoomFactor,
      tile_width  * m_ZoomFactor,
      tile_height * m_ZoomFactor
    );
    m_RedrawWindow = 1;
    Refresh(TRUE, &Rect);

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::SelectTileUnderPoint(wxPoint Point)
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
wMapView::RoundX(int x) {
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
wMapView::RoundY(int y) {
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
wMapView::MapAreaCopy()
{
  // TODO: add Map(all layers) area selection copy
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::LayerAreaCopy()
{
/*todo:
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

  // grab the layer
  int width = end_x - start_x + 1;
  int height = end_y - start_y + 1;
  sLayer& l = m_Map->GetLayer(m_SelectedLayer);
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
      for (int z=0; z<usedTiles.size(); z++)
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
  dword* ptr = (dword*)GlobalLock(memory);

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
  CloseClipboard();
*/
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::PasteMapUnderPoint(wxPoint point)
{
/*todo:
  //TODO: add map area selection paste

  int tx = m_CurrentX + (point.x / m_Map->GetTileset().GetTileWidth() / m_ZoomFactor);
  int ty = m_CurrentY + (point.y / m_Map->GetTileset().GetTileHeight() / m_ZoomFactor);

  if (OpenClipboard() == FALSE)
  {
    MessageBox("Cannot Open Clipboard!", NULL, MB_OK | MB_ICONERROR);
    return;
  }

  HGLOBAL memory = (HGLOBAL)GetClipboardData(s_MapAreaClipboardFormat);
  if (memory != NULL)
  {
    dword* ptr = (dword*)GlobalLock(memory);
    int num_layers = *ptr++;
    int num_tiles = *ptr++;
    int tile_width = *ptr++;
    int tile_height = *ptr++;
    sLayer tLayer;
    sMap tMap;
    sTileset tTileset;
    std::vector<int> newTileLoc;

    if (m_Map->GetTileset().GetTileWidth() != tile_width ||
        m_Map->GetTileset().GetTileHeight() != tile_height)
    {
      MessageBox("Tile size being pasted does not match tile size in map.\nPaste Aborted.", NULL, MB_OK | MB_ICONEXCLAMATION);
      return;
    }

    // extract the map array from memory
    if (num_layers == 0)
    {
      int width = *ptr++;
      int height = *ptr++;

      tLayer.Resize(width, height);
      for (int y=0; y<height; y++)
        for (int x=0; x<width; x++)
          tLayer.SetTile(x, y, *ptr++);
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

    // relocate all the tiles, add the tiles to the actual tileset if needed
    for (int i = 0; i < num_tiles; i++) {
      newTileLoc.push_back(i);
    }

    for (int i = 0; i < num_tiles; i++) {
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

    if (num_layers == 0)
    {
      // update the map data offsets
      for (int y=0; y<tLayer.GetHeight(); y++)
        for (int x=0; x<tLayer.GetWidth(); x++)
          tLayer.SetTile(x, y, newTileLoc[tLayer.GetTile(x, y)]);

      // finally, overwrite the existing map data
      for (int y=ty; y<m_Map->GetLayer(m_SelectedLayer).GetHeight() && y-ty<tLayer.GetHeight(); y++)
        for (int x=tx; x<m_Map->GetLayer(m_SelectedLayer).GetWidth() && x-tx<tLayer.GetWidth(); x++)
          m_Map->GetLayer(m_SelectedLayer).SetTile(x, y, tLayer.GetTile(x-tx, y-ty));
    }

    m_RedrawWindow = 1;
    Invalidate();
  }

  CloseClipboard();

  m_Handler->MV_MapChanged();
*/
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::FillArea()
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
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void 
wMapView::EntityCopy(wxPoint point)
{
/*todo:
  int tile_width  = m_Map->GetTileset().GetTileWidth();
  int tile_height = m_Map->GetTileset().GetTileHeight();
  int tx = m_CurrentX + (point.x / tile_width  / m_ZoomFactor);
  int ty = m_CurrentY + (point.y / tile_height / m_ZoomFactor);
  int px = (tx * tile_width)  + (tile_width  / 2);
  int py = (ty * tile_height) + (tile_height / 2);
  
  int entity_num = m_Map->FindEntity(px, py, m_SelectedLayer);
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
/ *
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
* /
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
      dword* ptr = (dword*)GlobalLock(memory);
      *ptr++ = sEntity::TRIGGER;
      *ptr++ = function_size;
      char* c_ptr = (char*)ptr;
      memcpy(c_ptr, trigger.script.c_str(), function_size);
      
      GlobalUnlock(memory);
      SetClipboardData(s_MapEntityClipboardFormat, memory);
    } break;
  }

  CloseClipboard();
*/
}

////////////////////////////////////////////////////////////////////////////////

void 
wMapView::EntityPaste(wxPoint point)
{
/*todo:
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

/ *
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
* /

      case sEntity::TRIGGER: {
        sTriggerEntity trigger;

        trigger.x = tx * m_Map->GetTileset().GetTileWidth();
        trigger.y = ty * m_Map->GetTileset().GetTileHeight();
        trigger.layer = m_SelectedLayer;

        int function_size = *ptr++;
        char* function_text = new char[function_size + 1];
        char* c_ptr = (char*)ptr;
        memcpy(function_text, c_ptr, function_size);
        function_text[function_size] = 0;
        trigger.script = function_text;

        // pop the darn thing into the map
        m_Map->AddEntity(new sTriggerEntity(trigger));

        delete[] function_text;
      } break;
    }

    m_Handler->MV_MapChanged();
  }
  
  CloseClipboard();
*/
}

////////////////////////////////////////////////////////////////////////////////

bool
wMapView::IsWithinSelectFillArea(int x, int y)
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

void
wMapView::DrawTile(wxDC& dc, const wxRect& rect, int tx, int ty)
{
  int tile_width  = m_Map->GetTileset().GetTileWidth();
  int tile_height = m_Map->GetTileset().GetTileHeight();

  // clear the DIB
  memset(m_BlitTile->GetPixels(), 0,  m_ZoomFactor * m_ZoomFactor * tile_width * tile_height * 3);


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
      int tile = layer.GetTile(tx, ty);
      const RGBA* src = m_Map->GetTileset().GetTile(tile).GetPixels();      
      RGB* dest = (RGB*)m_BlitTile->GetPixels();

      int counter = 0;
      for (int j=0; j<tile_height; j++)
      {
        for (int k=0; k<tile_width; k++)
          for (int l=0; l<m_ZoomFactor; l++)
          {
            RGBA s = src[j * tile_width + k];
            int alpha = src[j * tile_width + k].alpha;
            dest[counter].red   = (alpha * s.red   + (255 - alpha) * dest[counter].red)   / 256;
            dest[counter].green = (alpha * s.green + (255 - alpha) * dest[counter].green) / 256;
            dest[counter].blue  = (alpha * s.blue  + (255 - alpha) * dest[counter].blue)  / 256;

/*          THESE ARE TOO SLOW IN THIS INNER LOOP
            // additional draw rules
            // area fill
            if (m_CurrentTool == tool_FillArea && IsWithinSelectFillArea(tx, ty))
              if (m_Clicked)
              {
                RGBA p = m_Map->GetTileset().GetTile(m_SelectedTile).GetPixel(k, j);
                if (p.alpha > 30) Blend4(dest[counter], p, 100);
              }
            // area select
            if (m_CurrentTool == tool_CopyArea && IsWithinSelectFillArea(tx, ty))
              if (m_Clicked)
                Blend3(dest[counter], m_HighlightColor, 80);
*/
            counter++;
          }

        for (int k=1; k<m_ZoomFactor; k++)
        {
          memcpy(dest + counter, dest + (counter - tile_width * m_ZoomFactor), tile_width * m_ZoomFactor * sizeof(RGB));
          counter += tile_width * m_ZoomFactor;
        }
      }
    }
  }

  // render the tile
  wxBitmap bitmap = m_BlitTile->GetImage()->ConvertToBitmap();
  dc.DrawBitmap(bitmap, rect.GetX(), rect.GetY());
  //dc.BitBlt(rect.left, rect.top, tile_width * m_ZoomFactor, tile_height * m_ZoomFactor,
  //          CDC::FromHandle(m_BlitTile->GetDC()), 0, 0, SRCCOPY);



  // check if it's in fill or area select mode (not clicked)
  if ((m_CurrentTool == tool_FillArea && !m_Clicked) &&
      (m_CurrentTool == tool_CopyArea && !m_Clicked))
  {
  }
  else
  {
    if (tx == m_CurrentCursorTileX && 
        ty == m_CurrentCursorTileY &&
        tx <= GetTotalTilesX() &&
        ty <= GetTotalTilesY())
      {
        /*
        dc.SaveDC();

        wxRect r(rect);
        CBrush brush;
        brush.CreateSolidBrush(RGB(m_HighlightColor.red, 
                               m_HighlightColor.green, 
                               m_HighlightColor.blue));
        dc.FrameRect(r, &brush);
        brush.DeleteObject();

        dc.RestoreDC(-1);
        */
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(wxPen(wxColour(m_HighlightColor.red, 
                                      m_HighlightColor.green, 
                                      m_HighlightColor.blue), 1, wxSOLID));
        dc.DrawRectangle(rect);
        dc.SetBrush(wxNullBrush);
        dc.SetPen(wxNullPen);
      }
  }

  // draw start point
  if (tx == m_Map->GetStartX() / tile_width &&
      ty == m_Map->GetStartY() / tile_height &&
      m_Map->GetLayer(m_Map->GetStartLayer()).IsVisible())
  {
    /*
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
    */
    
    /*todo: accurate placement*/
    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.SetTextForeground(*wxBLACK);
    dc.DrawText("ST", rect.x + 4, rect.y + 4);
    dc.SetTextForeground(*wxWHITE);
    dc.DrawText("ST", rect.x + 3, rect.y + 3);
  }

  // draw entities
  for (int i = 0; i < m_Map->GetNumEntities(); i++)
  {
    sEntity& entity = m_Map->GetEntity(i);
    if (tx == entity.x / tile_width &&
        ty == entity.y / tile_height &&
        m_Map->GetLayer(entity.layer).IsVisible())
    {
      /*
      HICON icon;
      switch (entity.GetEntityType())
      {
        case sEntity::PERSON:  icon = AfxGetApp()->LoadIcon(IDI_ENTITY_PERSON); break;
        case sEntity::TRIGGER: icon = AfxGetApp()->LoadIcon(IDI_ENTITY_TRIGGER); break;
      }

      int tw = m_Map->GetTileset().GetTileWidth()  * m_ZoomFactor;
      int th = m_Map->GetTileset().GetTileHeight() * m_ZoomFactor;
      DrawIconEx(dc.m_hDC, rect.left, rect.top, icon, tw, th, 0, NULL, DI_NORMAL);
      */

      switch (entity.GetEntityType())
      {
        case sEntity::PERSON:  
          /*todo: Use proper icons*/
          dc.SetBackgroundMode(wxTRANSPARENT);
          dc.SetTextForeground(*wxBLACK);
          dc.DrawText("P", rect.x + 4, rect.y + 4);
          dc.SetTextForeground(*wxWHITE);
          dc.DrawText("P", rect.x + 3, rect.y + 3);
          break;
        case sEntity::TRIGGER: 
          /*todo: Use proper icons*/
          dc.SetBackgroundMode(wxTRANSPARENT);
          dc.SetTextForeground(*wxBLACK);
          dc.DrawText("T", rect.x + 4, rect.y + 4);
          dc.SetTextForeground(*wxWHITE);
          dc.DrawText("T", rect.x + 3, rect.y + 3);
          break;

        // prevent unhandled enum warning
        default:
          break;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::DrawObstructions(wxDC& dc)
{
  const int tile_width  = m_Map->GetTileset().GetTileWidth();
  const int tile_height = m_Map->GetTileset().GetTileHeight();

  dc.SetPen(wxPen(wxColour(m_ObstructionColor.red, m_ObstructionColor.green, m_ObstructionColor.blue), 1, wxSOLID));

  sLayer& l = m_Map->GetLayer(m_SelectedLayer);
  sObstructionMap& obs_map = l.GetObstructionMap();

  // for each segment
  for (int i = 0; i < obs_map.GetNumSegments(); i++) {

    const sObstructionMap::Segment& segment = obs_map.GetSegment(i);

    int x1 = (segment.x1 - m_CurrentX * tile_width)  * m_ZoomFactor;
    int y1 = (segment.y1 - m_CurrentY * tile_height) * m_ZoomFactor;
    int x2 = (segment.x2 - m_CurrentX * tile_width)  * m_ZoomFactor;
    int y2 = (segment.y2 - m_CurrentY * tile_height) * m_ZoomFactor;

    dc.DrawLine(x1, y1, x2, y2);
  }

  dc.SetPen(wxNullPen);
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::DrawZones(wxDC& dc)
{
  const int tile_width  = m_Map->GetTileset().GetTileWidth();
  const int tile_height = m_Map->GetTileset().GetTileHeight();

  dc.SetBrush(*wxTRANSPARENT_BRUSH);
  dc.SetPen(wxPen(wxColour(m_ZoneColor.red, m_ZoneColor.green, m_ZoneColor.blue), 1, wxSOLID));

  // for each segment
  for (int i = 0; i < m_Map->GetNumZones(); i++) {

    const sMap::sZone& zone = m_Map->GetZone(i);

    if(zone.layer == m_SelectedLayer) {
      int x1 = (zone.x1 - m_CurrentX * tile_width)  * m_ZoomFactor;
      int y1 = (zone.y1 - m_CurrentY * tile_height) * m_ZoomFactor;
      int x2 = (zone.x2 - m_CurrentX * tile_width)  * m_ZoomFactor;
      int y2 = (zone.y2 - m_CurrentY * tile_height) * m_ZoomFactor;

      dc.DrawRectangle(x1, y1, x2 - x1, y2 - y1);
    }
  }

  dc.SetBrush(wxNullBrush);
  dc.SetPen(wxNullPen);
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::DrawPreviewLine(wxDC& dc, int x1, int y1, int x2, int y2)
{
  const int tile_width  = m_Map->GetTileset().GetTileWidth();
  const int tile_height = m_Map->GetTileset().GetTileHeight();

  int oldfunc = dc.GetLogicalFunction();

  dc.SetBrush(*wxTRANSPARENT_BRUSH);
  dc.SetPen(wxPen(wxColour(m_HighlightColor.red, m_HighlightColor.green, m_HighlightColor.blue), 1, wxSOLID));
  dc.SetLogicalFunction(wxXOR);

  x1 = (x1 - m_CurrentX * tile_width) * m_ZoomFactor;
  y1 = (y1 - m_CurrentY * tile_height) * m_ZoomFactor;
  x2 = (x2 - m_CurrentX * tile_width) * m_ZoomFactor;
  y2 = (y2 - m_CurrentY * tile_height) * m_ZoomFactor;
  dc.DrawLine(x1, y1, x2, y2);


  dc.SetLogicalFunction(oldfunc);
  dc.SetBrush(wxNullBrush);
  dc.SetPen(wxNullPen);
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::DrawPreviewBox(wxDC& dc, int x1, int y1, int x2, int y2)
{
  const int tile_width  = m_Map->GetTileset().GetTileWidth();
  const int tile_height = m_Map->GetTileset().GetTileHeight();

  int oldfunc = dc.GetLogicalFunction();

  dc.SetBrush(*wxTRANSPARENT_BRUSH);
  dc.SetPen(wxPen(wxColour(m_HighlightColor.red, m_HighlightColor.green, m_HighlightColor.blue), 1, wxSOLID));
  dc.SetLogicalFunction(wxXOR);

  x1 = (x1 - m_CurrentX * tile_width) * m_ZoomFactor;
  y1 = (y1 - m_CurrentY * tile_height) * m_ZoomFactor;
  x2 = (x2 - m_CurrentX * tile_width) * m_ZoomFactor;
  y2 = (y2 - m_CurrentY * tile_height) * m_ZoomFactor;
  dc.DrawRectangle(x1, y1, x2 - x1, y2 - y1);

  dc.SetLogicalFunction(oldfunc);
  dc.SetBrush(wxNullBrush);
  dc.SetPen(wxNullPen);
}

/*todo:
////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapView::OnDestroy()
{
  if (m_ToolPalette) {
    m_ToolPalette->Destroy();
  }
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wMapView::OnPaint(wxPaintEvent &event)
{
  wxPaintDC dc(this);
  wxRegion region = GetUpdateRegion();
  dc.SetClippingRegion(wxPoint(0, 0), GetClientSize());

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
        wxRect Rect(
          ix * tile_width  * m_ZoomFactor,
          iy * tile_height * m_ZoomFactor,
          //ix * tile_width  * m_ZoomFactor + 
          tile_width  * m_ZoomFactor,
          //iy * tile_height * m_ZoomFactor + 
          tile_height * m_ZoomFactor
        );

        if (region.Contains(Rect) != wxOutRegion) {

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
  dc.DestroyClippingRegion();
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::OnSize(wxSizeEvent &event)
{
  UpdateScrollBars();
  wScrollWindow::OnSize(event);
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::OnLButtonDown(wxMouseEvent &event)
{
  int tile_width  = m_Map->GetTileset().GetTileWidth();
  int tile_height = m_Map->GetTileset().GetTileHeight();

  int zoom_tile_width  = m_Map->GetTileset().GetTileWidth()  * m_ZoomFactor;
  int zoom_tile_height = m_Map->GetTileset().GetTileHeight() * m_ZoomFactor;
  
  wxPoint point = event.GetPosition();

  // if the shift key is down, select the current tile
  if (event.ShiftDown()) {

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
      case tool_FillArea: {
        int x = point.x / zoom_tile_width  + m_CurrentX;
        int y = point.y / zoom_tile_height + m_CurrentY;
        m_StartCursorTileX = x;
        m_StartCursorTileY = y;
      } break;

      case tool_ObsSegment: {
        m_StartX = point.x / m_ZoomFactor + m_CurrentX * tile_width;
        m_StartY = point.y / m_ZoomFactor + m_CurrentY * tile_height;
        if(event.ControlDown()) {
          m_StartX = RoundX(m_StartX);
          m_StartY = RoundY(m_StartY);
        }
        m_PreviewX = m_StartX;
        m_PreviewY = m_StartY;
        m_PreviewLineOn = 1;
        m_RedrawPreviewLine = 1;
        Refresh(FALSE);
      } break;

      case tool_ObsDeleteSegment: {
        // delete the segment closest to this point (this is crap, FIXME)
        int x = point.x / m_ZoomFactor + m_CurrentX * tile_width;
        int y = point.y / m_ZoomFactor + m_CurrentY * tile_height;
        m_Map->GetLayer(m_SelectedLayer).GetObstructionMap().RemoveSegmentByPoint(x, y);

        m_RedrawWindow = 1;
        Refresh(FALSE);
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
          Refresh(FALSE);
		      m_Handler->MV_MapChanged();
          m_PreviewLineOn = 1;
		    } else {
          return;
		    }
      } break;
      case tool_ZoneAdd: {
        m_StartX = point.x / m_ZoomFactor + m_CurrentX * tile_width;
        m_StartY = point.y / m_ZoomFactor + m_CurrentY * tile_height;
        if(event.ControlDown()) {
          m_StartX = RoundX(m_StartX);
          m_StartY = RoundY(m_StartY);
        }
        m_PreviewX = m_StartX;
        m_PreviewY = m_StartY;
        m_PreviewBoxOn = 1;
        m_RedrawPreviewLine = 1;
        Refresh(FALSE);
      } break;
      case tool_ZoneEdit: {
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
        if(event.ControlDown()) {
          m_PreviewX = RoundX(x);
          m_PreviewY = RoundY(y);
        } else {
          m_PreviewX = x;
          m_PreviewY = y;
        }
        m_PreviewBoxOn = 1;
        m_RedrawPreviewLine = 1;
        Refresh(FALSE);
      } break;
      case tool_ZoneDelete: {
        int x = point.x / m_ZoomFactor + m_CurrentX * tile_width;
        int y = point.y / m_ZoomFactor + m_CurrentY * tile_height;
        int z = m_Map->FindZone(x, y, m_SelectedLayer);
        if(z != -1) {
          m_Map->DeleteZone(z);
          m_RedrawWindow = 1;
          Refresh(FALSE);
          m_Handler->MV_MapChanged();
        }
      } break;
    }

    // grab all mouse events until the user releases the button
    CaptureMouse();
    m_Clicked = true;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::OnMouseMove(wxMouseEvent &event)
{
  int tile_width  = m_Map->GetTileset().GetTileWidth()  * m_ZoomFactor;
  int tile_height = m_Map->GetTileset().GetTileHeight() * m_ZoomFactor;

  wxPoint point = event.GetPosition();

  int x = point.x / tile_width  + m_CurrentX;
  int y = point.y / tile_height + m_CurrentY;

  if (x >= 0 && y >= 0 && x <= GetTotalTilesX() && y <= GetTotalTilesY())
  {
    wxString Position;
    int pixel_x = (point.x + tile_width  * m_CurrentX) / m_ZoomFactor;
    int pixel_y = (point.y + tile_height * m_CurrentY) / m_ZoomFactor;
    Position.Format("Map tile:(%i,%i) pixel:(%i,%i)", x, y, pixel_x, pixel_y);
    SetStatus(Position);
  }
  else
    SetStatus(wxString(""));

  if (m_Clicked) {
    switch (m_CurrentTool) {
      case tool_1x1Tile:
      case tool_3x3Tile:
      case tool_5x5Tile:
        Click(point);
        break;

      case tool_CopyArea:
      case tool_FillArea:
        {
          // clear out the old area
          int old_x = (m_StartCursorTileX - m_CurrentX) * tile_width;
          int old_y = (m_StartCursorTileY - m_CurrentY) * tile_height;
          int new_x = (m_CurrentCursorTileX - m_CurrentX) * tile_width;
          int new_y = (m_CurrentCursorTileY - m_CurrentY) * tile_height;
          if (new_x < old_x) std::swap(new_x, old_x);
          if (new_y < old_y) std::swap(new_y, old_y);
          //CRgn old_rgn;
          //old_rgn.CreateRectRgn(old_x, old_y, new_x + tile_width, new_y + tile_height);

          m_CurrentCursorTileX = x;
          m_CurrentCursorTileY = y;
          //InvalidateRect(&rect);
          wxRect refreshRect(old_x, old_y, new_x - old_x + tile_width, new_y - old_y + tile_height);
          Refresh(TRUE, &refreshRect);

          // draw the new area
          old_x = (m_StartCursorTileX - m_CurrentX) * tile_width;
          old_y = (m_StartCursorTileY - m_CurrentY) * tile_height;
          new_x = (m_CurrentCursorTileX - m_CurrentX) * tile_width;
          new_y = (m_CurrentCursorTileY - m_CurrentY) * tile_height;
          if (new_x < old_x) std::swap(new_x, old_x);
          if (new_y < old_y) std::swap(new_y, old_y);
          //CRgn new_rgn;
          //new_rgn.CreateRectRgn(old_x, old_y, new_x + tile_width, new_y + tile_height);
          //InvalidateRect(&new_rect);
          wxRect refreshRect2(old_x, old_y, new_x - old_x + tile_width, new_y - old_y + tile_height);
          Refresh(TRUE, &refreshRect2);

          //CRgn total_rgn;
          //new_rgn.CombineRgn(&old_rgn, &new_rgn, RGN_OR);
          //InvalidateRgn(&new_rgn);
          m_RedrawWindow = 1;
          //new_rgn.DeleteObject();
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
        if(event.ControlDown()) {
          m_PreviewX = RoundX(m_PreviewX);
          m_PreviewY = RoundY(m_PreviewY);
        }
        //signal that a preview line erase and update is required.
        m_RedrawPreviewLine = 2;
        Refresh(FALSE);        
      } break;

      case tool_ZoneAdd:
      case tool_ZoneEdit: {
        if(m_MoveIndex != -1) {
          int tile_width  = m_Map->GetTileset().GetTileWidth();
          int tile_height = m_Map->GetTileset().GetTileHeight();
          m_PreviewOldX = m_PreviewX;
          m_PreviewOldY = m_PreviewY;
          m_PreviewX = point.x / m_ZoomFactor + m_CurrentX * tile_width;
          m_PreviewY = point.y / m_ZoomFactor + m_CurrentY * tile_height;
          if(event.ControlDown()) {
            m_PreviewX = RoundX(m_PreviewX);
            m_PreviewY = RoundY(m_PreviewY);
          }
          m_RedrawPreviewLine = 2;
          Refresh(FALSE);
        }
      } break;
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
    case tool_PasteEntity: {
      int old_x = (m_CurrentCursorTileX - m_CurrentX) * tile_width;
      int old_y = (m_CurrentCursorTileY - m_CurrentY) * tile_height;
      wxRect old_rect = wxRect( old_x, old_y, tile_width, tile_height);
      m_RedrawWindow = 1;
      //InvalidateRect(&old_rect);
      Refresh(FALSE, &old_rect);
  
      m_CurrentCursorTileX = x;
      m_CurrentCursorTileY = y;

      // refresh the new tile
      int new_y = (y - m_CurrentY) * tile_height;
      int new_x = (x - m_CurrentX) * tile_width;
      wxRect new_rect = wxRect(new_x, new_y, tile_width, tile_height);
      m_RedrawWindow = 1;
      Refresh(FALSE, &new_rect);
    } break;

    case tool_CopyArea:
    case tool_FillArea: {
      if (!m_Clicked) {
        int old_x = (m_CurrentCursorTileX - m_CurrentX) * tile_width;
        int old_y = (m_CurrentCursorTileY - m_CurrentY) * tile_height;
        wxRect old_rect = wxRect( old_x, old_y, tile_width, tile_height);
        m_RedrawWindow = 1;
        Refresh(FALSE, &old_rect);
  
        m_CurrentCursorTileX = x;
        m_CurrentCursorTileY = y;

        // refresh the new tile
        int new_y = (y - m_CurrentY) * tile_height;
        int new_x = (x - m_CurrentX) * tile_width;
        wxRect new_rect = wxRect(new_x, new_y, tile_width, tile_height);
        m_RedrawWindow = 1;
        Refresh(FALSE, &new_rect);
      } break;
    }

  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::OnLButtonUp(wxMouseEvent &event)
{
  int tile_width  = m_Map->GetTileset().GetTileWidth();
  int tile_height = m_Map->GetTileset().GetTileHeight();

  int zoom_tile_width  = m_Map->GetTileset().GetTileWidth()  * m_ZoomFactor;
  int zoom_tile_height = m_Map->GetTileset().GetTileHeight() * m_ZoomFactor;

  wxPoint point = event.GetPosition();

  switch (m_CurrentTool)
  {
    case tool_FillArea: {
      FillArea();
    } break;

    case tool_CopyEntity: {
      EntityCopy(point);
    } break;

    case tool_CopyArea: {
      // clear out the old area
      int old_x = (m_StartCursorTileX - m_CurrentX)   * zoom_tile_width;
      int old_y = (m_StartCursorTileY - m_CurrentY)   * zoom_tile_height;
      int new_x = (m_CurrentCursorTileX - m_CurrentX) * zoom_tile_width;
      int new_y = (m_CurrentCursorTileY - m_CurrentY) * zoom_tile_height;
      if (new_x < old_x) std::swap(new_x, old_x);
      if (new_y < old_y) std::swap(new_y, old_y);
      wxRect rect = wxRect(old_x, old_y, new_x + zoom_tile_width, new_y + zoom_tile_height);

      m_RedrawWindow = 1;
      Refresh(TRUE, &rect);
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
      if(event.ControlDown()) {
        x = RoundX(x);
        y = RoundY(y);
      }
      m_Map->GetLayer(m_SelectedLayer).GetObstructionMap().AddSegment(
        m_StartX, m_StartY, x, y
      );

      // we should just invalidate the area that the new line covers...
      /*todo:*/
      m_RedrawWindow = 1;
      Refresh();
      m_Handler->MV_MapChanged();
    } break;

    case tool_ObsDeleteSegment: {
    } break;
    case tool_ZoneAdd: {
      if(m_MoveIndex != -1) {
        int x = point.x / m_ZoomFactor + m_CurrentX * tile_width;
        int y = point.y / m_ZoomFactor + m_CurrentY * tile_height;
        sMap::sZone zone;
        m_PreviewLineOn = 0;
        if(event.ControlDown()) {
          x = RoundX(x);
          y = RoundY(y);
        }
        zone.x1 = m_StartX;
        zone.y1 = m_StartY;
        zone.x2 = x;
        zone.y2 = y;
        zone.layer = m_SelectedLayer;
        zone.reactivate_in_num_steps = 8;
        zone.script = "";
        m_Map->AddZone(zone);
        m_PreviewBoxOn = 0;
        m_RedrawWindow = 1;
        Refresh();
        m_Handler->MV_MapChanged();
      }
    } break;
    case tool_ZoneEdit: {
      if(m_MoveIndex != -1) {
        int x = point.x / m_ZoomFactor + m_CurrentX * tile_width;
        int y = point.y / m_ZoomFactor + m_CurrentY * tile_height;
        m_PreviewLineOn = 0;
        if(event.ControlDown()) {
          x = RoundX(x);
          y = RoundY(y);
        }
        m_Map->UpdateZone(m_MoveIndex, m_StartX, m_StartY, x, y);
        m_PreviewBoxOn = 0;
        m_RedrawWindow = 1;
        Refresh();
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
  if (HasCapture()) ReleaseMouse();
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::OnRButtonUp(wxMouseEvent &event)
{
  m_MenuPoint = event.GetPosition();
  wxPoint point = event.GetPosition();

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
    m_Menu->Enable(wID_MAPVIEW_DELETEENTITY, FALSE);
    m_Menu->Enable(wID_MAPVIEW_EDITENTITY, FALSE);
    m_Menu->Enable(wID_MAPVIEW_INSERTENTITY_PERSON, TRUE);
    m_Menu->Enable(wID_MAPVIEW_INSERTENTITY_TRIGGER, TRUE);
  } else {
    m_Menu->Enable(wID_MAPVIEW_DELETEENTITY, TRUE);
    m_Menu->Enable(wID_MAPVIEW_EDITENTITY, TRUE);
    m_Menu->Enable(wID_MAPVIEW_INSERTENTITY_PERSON, FALSE);
    m_Menu->Enable(wID_MAPVIEW_INSERTENTITY_TRIGGER, FALSE);
  }

  m_Menu->Check(wID_MAPVIEW_ZOOM_1X, FALSE);
  m_Menu->Check(wID_MAPVIEW_ZOOM_2X, FALSE);
  m_Menu->Check(wID_MAPVIEW_ZOOM_4X, FALSE);
  m_Menu->Check(wID_MAPVIEW_ZOOM_8X, FALSE);
  switch (GetZoomFactor()) {
    case 1: m_Menu->Check(wID_MAPVIEW_ZOOM_1X, TRUE); break;
    case 2: m_Menu->Check(wID_MAPVIEW_ZOOM_2X, TRUE); break;
    case 4: m_Menu->Check(wID_MAPVIEW_ZOOM_4X, TRUE); break;
    case 8: m_Menu->Check(wID_MAPVIEW_ZOOM_8X, TRUE); break;
  }

  if(z == -1) {
    m_Menu->Enable(wID_MAPVIEW_ZONEEDIT, FALSE);
  } else {
    m_Menu->Enable(wID_MAPVIEW_ZONEEDIT, TRUE);
  }

  // show the popup menu
  PopupMenu(m_Menu, point);
}

void
wMapView::OnMenu(wxCommandEvent &event)
{
  wxPoint point = m_MenuPoint;

  int tile_width = m_Map->GetTileset().GetTileWidth();
  int tile_height = m_Map->GetTileset().GetTileHeight();

  // tile coordinates
  int tx = (point.x / m_Map->GetTileset().GetTileWidth()) / m_ZoomFactor + m_CurrentX;
  int ty = (point.y / m_Map->GetTileset().GetTileHeight()) / m_ZoomFactor + m_CurrentY;

  // map coordinates
  int x = point.x / m_ZoomFactor + m_CurrentX * tile_width;
  int y = point.y / m_ZoomFactor + m_CurrentY * tile_height;

  int z = m_Map->FindZone(x, y, m_SelectedLayer);

  // convert tile coordinates to pixel coordinates
  int px = tx * tile_width + tile_width / 2;
  int py = ty * tile_height + tile_height / 2;

  // execute command
  switch (event.GetId())
  {
    case wID_MAPVIEW_SELECTTILE:
    {
      int tile = m_Map->GetLayer(m_SelectedLayer).GetTile(tx, ty);
      m_SelectedTile = tile;
      m_Handler->MV_SelectedTileChanged(tile);
      break;
    }

    case wID_MAPVIEW_SETENTRYPOINT:
    {
      m_Map->SetStartX(px);
      m_Map->SetStartY(py);
      m_Map->SetStartLayer(m_SelectedLayer);

      m_RedrawWindow = 1;
      Refresh();
      m_Handler->MV_MapChanged();
      break;
    }

    case wID_MAPVIEW_FILL: {
      if (::wxMessageBox("Are you sure?", "Fill Layer", wxYES_NO) == wxYES) {
        
        sLayer& layer = m_Map->GetLayer(m_SelectedLayer);
        for (int iy = 0; iy < layer.GetHeight(); iy++) {
          for (int ix = 0; ix < layer.GetWidth(); ix++) {
            layer.SetTile(ix, iy, m_SelectedTile);
          }
        }
        m_RedrawWindow = 1;
        Refresh();
        m_Handler->MV_MapChanged();

      }

      break;
    }

    case wID_MAPVIEW_INSERTENTITY_PERSON:
    {
      sPersonEntity person;
      person.x = px;
      person.y = py;
      person.layer = m_SelectedLayer;

      wEntityPersonDialog dialog(this, person);
      if (dialog.ShowModal() == wxID_OK)
      {
        // insert it into the map
        m_Map->AddEntity(new sPersonEntity(person));
        m_RedrawWindow = 1;
        Refresh();
        m_Handler->MV_MapChanged();
      }
      break;
    }

    case wID_MAPVIEW_INSERTENTITY_TRIGGER:
    {
      sTriggerEntity trigger;
      trigger.x = px;
      trigger.y = py;
      trigger.layer = m_SelectedLayer;

      wEntityTriggerDialog dialog(this, trigger);
      if (dialog.ShowModal() == wxID_OK)
      {
        // insert it into the map
        m_Map->AddEntity(new sTriggerEntity(trigger));
        m_RedrawWindow = 1;
        Refresh();
        m_Handler->MV_MapChanged();
      }
      break;
    }

    case wID_MAPVIEW_DELETEENTITY:
    {
      for (int ie = 0; ie < m_Map->GetNumEntities(); ie++) {
        sEntity& e = m_Map->GetEntity(ie);
        if (e.x >= tx * tile_width && e.x < tx * tile_width + tile_width &&
            e.y >= ty * tile_height && e.y < ty * tile_height + tile_height) {

          m_Map->DeleteEntity(ie);

          m_RedrawWindow = 1;
          Refresh();
          m_Handler->MV_MapChanged();
          break;
        }
      }
      break;
    }

    case wID_MAPVIEW_EDITENTITY:
    {
      for (int ie = 0; ie < m_Map->GetNumEntities(); ie++) {
        sEntity& e = m_Map->GetEntity(ie);
        if (e.x >= tx * tile_width && e.x < tx * tile_width + tile_width &&
            e.y >= ty * tile_height && e.y < ty * tile_height + tile_height) {

          switch (e.GetEntityType()) {
            case sEntity::PERSON: {
              wEntityPersonDialog dialog(this, (sPersonEntity&)e);
              if (dialog.ShowModal() == wxID_OK) {
                m_Handler->MV_MapChanged();
              }
              break;
            }
          
            case sEntity::TRIGGER: {
              wEntityTriggerDialog dialog(this, (sTriggerEntity&)e);
              if (dialog.ShowModal() == wxID_OK) {
                m_Handler->MV_MapChanged();
              }
              break;
            }
            
            default: // gcc 'unhandled enum' warning
              break;

          } // end switch
        }
      }
      break;
    }
    case wID_MAPVIEW_ZONEEDIT:
      {
        wZoneEditDialog dialog(this, m_Map->GetZone(z));
        if(dialog.ShowModal() == wxID_OK) {
          m_Handler->MV_MapChanged();
        }
      }
      break;

    case wID_MAPVIEW_ZOOM_1X:
      SetZoomFactor(1);
      break;

    case wID_MAPVIEW_ZOOM_2X:
      SetZoomFactor(2);
      break;

    case wID_MAPVIEW_ZOOM_4X:
      SetZoomFactor(4);
      break;

    case wID_MAPVIEW_ZOOM_8X:
      SetZoomFactor(8);
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::OnHScrollChanged(int x)
{
  // do the scrolling thing
  int old_x = m_CurrentX;
  m_CurrentX = x;
  int new_x = x;
  if(old_x == new_x) {
    return;
  }
  /*todo:
  CDC* dc_ = GetDC();
  HDC dc = dc_->m_hDC;

  HRGN region = CreateRectRgn(0, 0, 0, 0);
  int factor = m_ZoomFactor * m_Map->GetTileset().GetTileWidth();
  ScrollDC(dc, (old_x - new_x) * factor, 0, NULL, NULL, region, NULL);
  ::InvalidateRgn(m_hWnd, region, FALSE);
  m_RedrawWindow = 1;
  DeleteObject(region);

  ReleaseDC(dc_);
  */
  //Refresh(FALSE);
  ScrollArea((old_x - new_x) * (m_ZoomFactor * m_Map->GetTileset().GetTileWidth()), 0);
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::OnVScrollChanged(int y)
{
  // do the scrolling thing
  int old_y = m_CurrentY;
  m_CurrentY = y;
  int new_y = y;
  if(old_y == new_y) {
    return;
  }
  /*todo:
  CDC* dc_ = GetDC();
  HDC dc = dc_->m_hDC;

  HRGN region = CreateRectRgn(0, 0, 0, 0);
  int factor = m_ZoomFactor * m_Map->GetTileset().GetTileHeight();
  ScrollDC(dc, 0, (old_y - new_y) * factor, NULL, NULL, region, NULL);
  ::InvalidateRgn(m_hWnd, region, FALSE);
  m_RedrawWindow = 1;
  DeleteObject(region);

  ReleaseDC(dc_);
  */
  //Refresh(FALSE);
  ScrollArea(0, (old_y - new_y) * (m_ZoomFactor * m_Map->GetTileset().GetTileHeight()));
}

////////////////////////////////////////////////////////////////////////////////

void
wMapView::TP_ToolSelected(int tool)
{
  // do something
  m_CurrentTool = tool;
}

////////////////////////////////////////////////////////////////////////////////
