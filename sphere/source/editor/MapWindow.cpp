// identifier too long
#pragma warning(disable : 4786)


#include "MapWindow.hpp"
#include "Project.hpp"
#include "FileDialogs.hpp"
#include "ResizeDialog.hpp"
#include "NewMapDialog.hpp"
#include "TilesetSelectionDialog.hpp"
#include "MapPropertiesDialog.hpp"
#include "NumberDialog.hpp"
#include "TilePalette.hpp"
#include "../common/rgb.hpp"
#include "../common/minmax.hpp"
#include "resource.h"


#define IDC_TAB 1101
#define TAB_HEIGHT 24

#ifdef USE_SIZECBAR
IMPLEMENT_DYNAMIC(CMapWindow, CMDIChildWnd)
#endif

BEGIN_MESSAGE_MAP(CMapWindow, CSaveableDocumentWindow)

  ON_WM_SIZE()
  ON_WM_KEYDOWN()

  ON_COMMAND(ID_MAP_PROPERTIES,      OnMapProperties)
  ON_COMMAND(ID_MAP_CHANGETILESIZE,  OnChangeTileSize)
  ON_COMMAND(ID_MAP_RESCALETILESET,  OnRescaleTileset)
  ON_COMMAND(ID_MAP_RESIZEALLLAYERS, OnResizeAllLayers)
  ON_COMMAND(ID_MAP_EXPORTTILESET,   OnExportTileset)
  ON_COMMAND(ID_MAP_IMPORTTILESET,   OnImportTileset)
  ON_COMMAND(ID_MAP_PRUNETILESET,    OnPruneTileset)

  ON_COMMAND(ID_MAP_SLIDE_UP,        OnMapSlideUp)
  ON_COMMAND(ID_MAP_SLIDE_RIGHT,     OnMapSlideRight)
  ON_COMMAND(ID_MAP_SLIDE_DOWN,      OnMapSlideDown)
  ON_COMMAND(ID_MAP_SLIDE_LEFT,      OnMapSlideLeft)
  ON_COMMAND(ID_MAP_SLIDE_OTHER,     OnMapSlideOther)

  ON_NOTIFY(TCN_SELCHANGE, IDC_TAB,  OnTabChanged)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CMapWindow::CMapWindow(const char* map, const char* tileset)
: CSaveableDocumentWindow(map, IDR_MAP, CSize(400, 100))
, m_Created(false)
, m_TilePalette(NULL)
, m_TilePreviewPalette(NULL)
{
  // LoadMap() displays message boxes if something goes wrong
  // SetSaved() and SetModified() are called in here
  if (!LoadMap(map, tileset)) {
    delete this;
    return;
  }

  m_DocumentType = WA_MAP;

  Create();
}

////////////////////////////////////////////////////////////////////////////////

CMapWindow::~CMapWindow()
{
  // destroy all the child windows
  Destroy();
}

////////////////////////////////////////////////////////////////////////////////

void
CMapWindow::Create()
{
  // create the window
  CSaveableDocumentWindow::Create(AfxRegisterWndClass(0, NULL, NULL, AfxGetApp()->LoadIcon(IDI_MAP)));

  m_TabControl.Create(
    WS_CHILD | WS_VISIBLE,
    CRect(0, 0, 0, 0),
    this,
    IDC_TAB);
  m_TabControl.SetFont(CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT)));
  m_TabControl.InsertItem(0, "Map");
  m_TabControl.InsertItem(1, "Tileset");

  // create the views
  m_MapView.Create(this, this, this, &m_Map);
  m_LayerView.Create(this, this, &m_Map);
  m_TilesetEditView.Create(this, this, this, &m_Map.GetTileset());

	m_TilePreviewPalette = new CTilePreviewPalette(this, m_Map.GetTileset().GetTile(0));

  m_Created = true;  // the window and children are ready!

  // move things to their rightful places
  RECT ClientRect;
  GetClientRect(&ClientRect);
  OnSize(0, ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top);

  m_TilePalette = new CTilePalette(this, this, &m_Map.GetTileset());

#ifdef USE_SIZECBAR
	LoadPaletteStates();
#endif

  TabChanged(0);
}

////////////////////////////////////////////////////////////////////////////////

void
CMapWindow::Destroy()
{
  m_TabControl.DestroyWindow();
  m_LayerView.DestroyWindow();
  m_MapView.DestroyWindow();
  m_TilesetEditView.DestroyWindow();

  if (m_TilePreviewPalette) {
		m_TilePreviewPalette->Destroy();
    m_TilePreviewPalette = NULL;
  }

  if (m_TilePalette) {
    m_TilePalette->Destroy();
    m_TilePalette = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapWindow::LoadMap(const char* szMap, const char* szTileset)
{
  SetSaved(szMap != NULL);
  SetModified(false);

  // try to load the map
  if (szMap == NULL || !m_Map.Load(szMap))
  {
    if (szMap) {
      int retval = AfxGetApp()->m_pMainWnd->MessageBox("Could not load map.\nCreate new?", NULL, MB_YESNO);
      if (retval != IDYES) {
        return false;
      }
    }

    // show the new map dialog box
    CNewMapDialog NewMapDialog(64, 64, szTileset);
    if (NewMapDialog.DoModal() != IDOK)
      return false;

    // allocate the map
    m_Map.Create(NewMapDialog.GetMapWidth(), NewMapDialog.GetMapHeight());

    // attempt to load the tileset
    if (strlen(NewMapDialog.GetTileset()) &&
        m_Map.GetTileset().Load(NewMapDialog.GetTileset()) == false)
    {
      MessageBox("Could not load tileset.\nUsing default.");
      m_Map.GetTileset().Create(1);
    }

    // set the initial name of the first layer
    m_Map.GetLayer(0).SetName("Base");

    SetModified(true);
  }

  // if the tileset has no tiles, ask user to choose a tileset
  if (m_Map.GetTileset().GetNumTiles() == 0)
  {
    CTilesetSelectionDialog dialog;
    if (dialog.DoModal() == IDCANCEL)
      return false;

    if (!m_Map.GetTileset().Load(dialog.GetTilesetPath()))
    {
      AfxGetApp()->m_pMainWnd->MessageBox("Error: Could not load tileset");
      return false;
    }
  }

  // check if map uses any tiles out of range of the tileset
  bool out_of_range;
  do {
    out_of_range = false;

    for (int il = 0; il < m_Map.GetNumLayers(); il++)
      for (int iy = 0; iy < m_Map.GetLayer(il).GetHeight(); iy++)
        for (int ix = 0; ix < m_Map.GetLayer(il).GetWidth(); ix++)
          if (m_Map.GetLayer(il).GetTile(ix, iy) >= m_Map.GetTileset().GetNumTiles())
            out_of_range = true;

    if (out_of_range)
    {
      if (MessageBox("Error: Map has some invalid tile references, remove them?", "Load Map", MB_YESNO) == IDNO)
        return false;

			int tile_index = 0;

			if (false) { // whether to show invalid references by using a red tile
  			m_Map.GetTileset().AppendTiles(1);
				tile_index = m_Map.GetTileset().GetNumTiles() - 1;
	  		sTile& tile = m_Map.GetTileset().GetTile(tile_index);
		  	tile.Rectangle(0, 0, 20, 20, CreateRGBA(255, 0, 0, 255));
			}

      for (int il = 0; il < m_Map.GetNumLayers(); il++)
        for (int iy = 0; iy < m_Map.GetLayer(il).GetHeight(); iy++)
          for (int ix = 0; ix < m_Map.GetLayer(il).GetWidth(); ix++)
            if (m_Map.GetLayer(il).GetTile(ix, iy) >= m_Map.GetTileset().GetNumTiles())
              m_Map.GetLayer(il).SetTile(ix, iy, tile_index);
    }

  } while (out_of_range);

  // check if base layer is out of range
  if (m_Map.GetStartLayer() >= m_Map.GetNumLayers())
  {
    m_Map.SetStartLayer(m_Map.GetNumLayers() - 1);
    SetModified(true);
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
CMapWindow::GetMapViewRect(RECT* pRect)
{
  const int SCROLLBAR_WIDTH  = GetSystemMetrics(SM_CXVSCROLL);
  const int SCROLLBAR_HEIGHT = GetSystemMetrics(SM_CYHSCROLL);

  RECT ClientRect;
  GetClientRect(&ClientRect);

  pRect->left   = 100 + SCROLLBAR_WIDTH;
  pRect->top    = 0;
  pRect->right  = ClientRect.right;
  pRect->bottom = ClientRect.bottom;
}

////////////////////////////////////////////////////////////////////////////////

void
CMapWindow::GetLayerViewRect(RECT* pRect)
{
  const int SCROLLBAR_WIDTH  = GetSystemMetrics(SM_CXVSCROLL);
  const int SCROLLBAR_HEIGHT = GetSystemMetrics(SM_CYHSCROLL);

  RECT ClientRect;
  GetClientRect(&ClientRect);

  pRect->left   = 0;
  pRect->top    = 0;
  pRect->right  = 100 + SCROLLBAR_WIDTH;
  pRect->bottom = ClientRect.bottom;
}

////////////////////////////////////////////////////////////////////////////////

void
CMapWindow::TabChanged(int tab)
{
  if (tab == 0)
  {
    m_MapView.ShowWindow(SW_SHOW);
    m_LayerView.ShowWindow(SW_SHOW);
    m_TilesetEditView.ShowWindow(SW_HIDE);
  }
  else
  {
    m_MapView.ShowWindow(SW_HIDE);
    m_LayerView.ShowWindow(SW_HIDE);
    m_TilesetEditView.ShowWindow(SW_SHOW);
  }

  //UpdateToolBars();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapWindow::OnSize(UINT uType, int cx, int cy)
{
  if (m_Created)
  {
    m_TabControl.MoveWindow(0, 0, cx, TAB_HEIGHT);

    // move the layer view
    RECT LayerViewRect;
    GetLayerViewRect(&LayerViewRect);
    m_LayerView.MoveWindow(
      LayerViewRect.left,
      LayerViewRect.top + TAB_HEIGHT,
      LayerViewRect.right - LayerViewRect.left,
      LayerViewRect.bottom - LayerViewRect.top - TAB_HEIGHT);

    // move the map view
    RECT MapViewRect;
    GetMapViewRect(&MapViewRect);
    m_MapView.MoveWindow(
      MapViewRect.left,
      MapViewRect.top + TAB_HEIGHT,
      MapViewRect.right - MapViewRect.left,
      MapViewRect.bottom - MapViewRect.top - TAB_HEIGHT);

    m_TilesetEditView.MoveWindow(0, TAB_HEIGHT, cx, cy - TAB_HEIGHT);
  }

  CSaveableDocumentWindow::OnSize(uType, cx, cy);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapWindow::OnKeyDown(UINT vk, UINT repeat, UINT flags)
{
  int tile = m_MapView.GetSelectedTile();
  if (vk == VK_RIGHT) {
    tile = std::min(tile + 1, m_Map.GetTileset().GetNumTiles() - 1);
  } else if (vk == VK_LEFT) {
    tile = std::max(tile - 1, 0);
  }

  m_MapView.SelectTile(tile);
  m_LayerView.SetSelectedTile(tile);
  m_TilesetEditView.SelectTile(tile);
  if (m_TilePalette) m_TilePalette->SelectTile(tile);

}

////////////////////////////////////////////////////////////////////////////////

afx_msg BOOL
CMapWindow::OnCommand(WPARAM wparam, LPARAM lparam)
{
  return m_TilesetEditView.SendMessage(WM_COMMAND, wparam, lparam);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapWindow::OnMapProperties()
{
  CMapPropertiesDialog Dialog(&m_Map);
  if (Dialog.DoModal() == IDOK)
    SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapWindow::OnChangeTileSize()
{
  int tile_width  = m_Map.GetTileset().GetTileWidth();
  int tile_height = m_Map.GetTileset().GetTileHeight();
  CResizeDialog dialog("Resize Tiles", tile_width, tile_height);
  if (dialog.DoModal() == IDOK)
  {
    if (dialog.GetWidth() > 0 && dialog.GetHeight() > 0) {
      m_Map.SetTileSize(dialog.GetWidth(), dialog.GetHeight());

      SetModified(true);
      m_MapView.TilesetChanged();
      m_TilesetEditView.TilesetChanged();
      if (m_TilePalette) m_TilePalette->TilesetChanged();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapWindow::OnRescaleTileset()
{
  int tile_width  = m_Map.GetTileset().GetTileWidth();
  int tile_height = m_Map.GetTileset().GetTileHeight();

  CResizeDialog dialog("Rescale Tiles", tile_width, tile_height);
  if (dialog.DoModal() == IDOK) {
    if (dialog.GetWidth() > 0 && dialog.GetHeight() > 0) {
      m_Map.SetTileSize(dialog.GetWidth(), dialog.GetHeight(), true);

      SetModified(true);
      m_MapView.TilesetChanged();
      m_TilesetEditView.TilesetChanged();
      if (m_TilePalette) m_TilePalette->TilesetChanged();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapWindow::OnResizeAllLayers()
{
  // calculate largest layer size
  int max_layer_width = 0;
  int max_layer_height = 0;

  for (int i = 0; i < m_Map.GetNumLayers(); i++) {
    sLayer& layer = m_Map.GetLayer(i);
    if (max_layer_width < layer.GetWidth()) {
      max_layer_width = layer.GetWidth();
    }
    if (max_layer_height < layer.GetHeight()) {
      max_layer_height = layer.GetHeight();
    }
  }

  // show the dialog
  CResizeDialog dialog("Resize All Layers", max_layer_width, max_layer_height);
	dialog.SetRange(1, 4906, 1, 4906);

  if (dialog.DoModal() == IDOK) {

    if (dialog.GetWidth() > 0 && dialog.GetWidth() <= 4096
		 && dialog.GetHeight() > 0 && dialog.GetHeight() <= 4096) {

      for (int i = 0; i < m_Map.GetNumLayers(); i++) {
        m_Map.GetLayer(i).Resize(dialog.GetWidth(), dialog.GetHeight());
      }

      SetModified(true);
      m_MapView.UpdateScrollBars();
      Invalidate();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapWindow::OnExportTileset()
{
  CTilesetFileDialog dialog(FDM_SAVE);
  if (dialog.DoModal() == IDOK)
  {
    if (!m_Map.GetTileset().Save(dialog.GetPathName()))
      MessageBox("Error: Could not export tileset");
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapWindow::OnImportTileset()
{
  CTilesetFileDialog dialog(FDM_OPEN);
  if (dialog.DoModal() == IDOK)
  {
    sTileset tileset;
    if (tileset.Load(dialog.GetPathName()))
    {
      m_Map.GetTileset() = tileset;
      m_Map.ValidateTileIndices();

      m_MapView.TilesetChanged();
      m_TilesetEditView.TilesetChanged();
      if (m_TilePalette) m_TilePalette->TilesetChanged();
      SetModified(true);
    }
    else
      MessageBox("Error: Could not import tileset");
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapWindow::OnPruneTileset()
{
  if (MessageBox("Are you sure you want to remove unused tiles?", "Prune Tileset", MB_ICONQUESTION | MB_YESNO) == IDYES) {
    m_Map.PruneTileset();

    SetModified(true);

    m_MapView.Invalidate();
    m_MapView.UpdateScrollBars();

    if (m_TilePalette) m_TilePalette->TilesetChanged();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapWindow::OnTabChanged(NMHDR* ns, LRESULT* result)
{
  if (ns->idFrom == IDC_TAB)
    TabChanged(m_TabControl.GetCurSel());
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapWindow::GetSavePath(char* path)
{
  CMapFileDialog Dialog(FDM_SAVE);
  if (Dialog.DoModal() != IDOK)
    return false;

  strcpy(path, Dialog.GetPathName());
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapWindow::SaveDocument(const char* path)
{
  return m_Map.Save(path);
}

////////////////////////////////////////////////////////////////////////////////

void
CMapWindow::MV_SelectedTileChanged(int tile)
{
  m_LayerView.SetSelectedTile(tile);
  m_TilesetEditView.SelectTile(tile);
  if (m_TilePalette) m_TilePalette->SelectTile(tile);
	if (m_TilePreviewPalette) m_TilePreviewPalette->OnImageChanged(m_Map.GetTileset().GetTile(tile));
}

////////////////////////////////////////////////////////////////////////////////

void
CMapWindow::MV_MapChanged()
{
  SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////

void
CMapWindow::LV_MapChanged()
{
  SetModified(true);

  m_MapView.Invalidate();
  m_MapView.UpdateScrollBars();

  // this might be called from a layerview delete layer operation, and the
  // currently selected tile might be out of range, so make sure TilesetChanged
  // verifies that.
  if (m_TilePalette) m_TilePalette->TilesetChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
CMapWindow::LV_SelectedLayerChanged(int layer)
{
  m_MapView.SelectLayer(layer);
}

////////////////////////////////////////////////////////////////////////////////

void
CMapWindow::TEV_SelectedTileChanged(int tile)
{
  m_LayerView.SetSelectedTile(tile);
  if (m_TilePalette) m_TilePalette->SelectTile(tile);
  m_MapView.SelectTile(tile);
	if (m_TilePreviewPalette) m_TilePreviewPalette->OnImageChanged(m_Map.GetTileset().GetTile(tile));
}

////////////////////////////////////////////////////////////////////////////////

void
CMapWindow::TEV_TileModified(int tile)
{
  SetModified(true);
  m_MapView.Invalidate();
  if (m_TilePalette) m_TilePalette->TileChanged(tile);
	if (m_TilePreviewPalette) m_TilePreviewPalette->OnImageChanged(m_Map.GetTileset().GetTile(tile));
}

////////////////////////////////////////////////////////////////////////////////

void
CMapWindow::TEV_TilesetModified()
{
  SetModified(true);
  m_MapView.Invalidate();
  if (m_TilePalette) m_TilePalette->Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void
CMapWindow::TV_SelectedTileChanged(int tile)
{
  m_LayerView.SetSelectedTile(tile);
  m_MapView.SelectTile(tile);
  m_TilesetEditView.SelectTile(tile);
	if (m_TilePreviewPalette) m_TilePreviewPalette->OnImageChanged(m_Map.GetTileset().GetTile(tile));
}

////////////////////////////////////////////////////////////////////////////////

void
CMapWindow::TV_TilesetChanged()
{
  m_MapView.Invalidate();
  m_MapView.TilesetChanged();
  m_TilesetEditView.TilesetChanged();
  if (m_TilePreviewPalette) m_TilePreviewPalette->OnImageChanged(m_Map.GetTileset().GetTile(m_MapView.GetSelectedTile()));
  SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////

void
CMapWindow::TV_InsertedTiles(int at, int numtiles)
{
  // go through the map and update the layers
  for (int l = 0; l < m_Map.GetNumLayers(); l++) {
    sLayer& layer = m_Map.GetLayer(l);
    for (int x = 0; x < layer.GetWidth(); x++) {
      for (int y = 0; y < layer.GetHeight(); y++) {
        
        if (layer.GetTile(x, y) >= at) {
          layer.SetTile(x, y, layer.GetTile(x, y) + numtiles);
        }

      }
    }
  }


  // now update the tileset animation indices
  sTileset& ts = m_Map.GetTileset();
  for (int i = at; i < ts.GetNumTiles(); i++) {
    ts.GetTile(i).SetNextTile(ts.GetTile(i).GetNextTile() + numtiles);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CMapWindow::TV_DeletedTiles(int at, int numtiles)
{
  // go through the map and update the layers
  for (int l = 0; l < m_Map.GetNumLayers(); l++) {
    sLayer& layer = m_Map.GetLayer(l);
    for (int x = 0; x < layer.GetWidth(); x++) {
      for (int y = 0; y < layer.GetHeight(); y++) {
        
        // if it was deleted, set it 0
        if (layer.GetTile(x, y) >= at &&
            layer.GetTile(x, y) < at + numtiles) {
          layer.SetTile(x, y, 0);
        } else if (layer.GetTile(x, y) >= at + numtiles) {
          layer.SetTile(x, y, layer.GetTile(x, y) - numtiles);
        }

      }
    }
  }


  // now update the tileset animation indices
  sTileset& ts = m_Map.GetTileset();
  for (int i = at + numtiles; i < ts.GetNumTiles(); i++) {
    ts.GetTile(i).SetNextTile(ts.GetTile(i).GetNextTile() - numtiles);
  }
}

////////////////////////////////////////////////////////////////////////////////

/**
  This takes two lists, where tile = list_b[i] if (tile == list_a[i]);
*/
void
CMapWindow::TV_SwapTiles(std::vector<int> list_a, std::vector<int> list_b) {
  for (int i = 0; i < m_Map.GetNumLayers(); i++) {
    for (int y = 0; y < m_Map.GetLayer(i).GetHeight(); y++) {
      for (int x = 0; x < m_Map.GetLayer(i).GetWidth(); x++) {
        int tile = m_Map.GetLayer(i).GetTile(x, y);
        for (int j = 0; ((j < list_a.size()) && (j < list_b.size())); j++) {
          if (tile == list_a[j]) {
            tile = list_b[j];
            m_Map.GetLayer(i).SetTile(x, y, tile);
            break;
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CMapWindow::TV_TilesetSelectionChanged(int width, int height, unsigned int* tiles) {

  if (width > 0 && height > 0 && tiles != NULL) {
    m_MapView.SetTileSelection(width, height, tiles);
    m_TilesetEditView.SetTileSelection(width, height, tiles);
  }
  else {
    m_MapView.SetTileSelection(0, 0, NULL);
    m_TilesetEditView.SetTileSelection(0, 0, NULL);
  }
}

////////////////////////////////////////////////////////////////////////////////

void 
CMapWindow::SP_ColorSelected(RGBA color)
{
  m_TilesetEditView.SP_ColorSelected(color);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapWindow::OnMapSlideUp()
{
  m_Map.Translate(0, -1);
  LV_MapChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapWindow::OnMapSlideRight()
{
  m_Map.Translate(1, 0);
  LV_MapChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapWindow::OnMapSlideDown()
{
  m_Map.Translate(0, 1);
  LV_MapChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapWindow::OnMapSlideLeft()
{
  m_Map.Translate(-1, 0);
  LV_MapChanged();
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CMapWindow::OnMapSlideOther()
{
  // find out the biggest width and height of layers
  int width = 0;
  int height = 0;
  
  for (int i = 0; i < m_Map.GetNumLayers(); i++) {
    if (m_Map.GetLayer(i).GetWidth() > width)
      width = m_Map.GetLayer(i).GetWidth();
    if (m_Map.GetLayer(i).GetHeight() > height)
      height = m_Map.GetLayer(i).GetHeight();
  }

  CNumberDialog dx("Slide Horizontally", "Value", 0, -width, width); 
  if (dx.DoModal() == IDOK) {
    CNumberDialog dy("Slide Vertically", "Value", 0, -height, height); 
    if (dy.DoModal() == IDOK) {
      if (dx.GetValue() != 0 || dy.GetValue() != 0) {
        m_Map.Translate(dx.GetValue(), dy.GetValue());
        LV_MapChanged();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CMapWindow::OnToolCommand(UINT id)
{
  if (m_Created) {
    m_MapView.OnToolChanged(id);
    m_TilesetEditView.OnToolChanged(id);
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CMapWindow::IsToolCommandAvailable(UINT id)
{
  BOOL available = FALSE;

  if (m_Created) {
    if (m_TabControl.GetCurSel() == 0) {
      available = m_MapView.IsToolAvailable(id);
    }
    else {
      available = m_TilesetEditView.IsToolAvailable(id);
    }
  }

  return available;
}

////////////////////////////////////////////////////////////////////////////////
