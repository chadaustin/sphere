#ifdef _MSC_VER
// identifier too long
#pragma warning(disable : 4786)
#endif


#include "MapWindow.hpp"
#include "Project.hpp"
#include "FileDialogs.hpp"
#include "ResizeDialog.hpp"
#include "NewMapDialog.hpp"
#include "TilesetSelectionDialog.hpp"
#include "MapPropertiesDialog.hpp"
#include "TilePalette.hpp"
#include "../common/rgb.hpp"
//#include "resource.h"
#include "IDs.hpp"


//#define IDC_TAB 1101
#define TAB_HEIGHT 24

/*
BEGIN_MESSAGE_MAP(CMapWindow, CSaveableDocumentWindow)

  ON_WM_SIZE()
  ON_COMMAND(ID_MAP_PROPERTIES,      OnMapProperties)
  ON_COMMAND(ID_MAP_CHANGETILESIZE,  OnChangeTileSize)
  ON_COMMAND(ID_MAP_RESCALETILESET,  OnRescaleTileset)
  ON_COMMAND(ID_MAP_RESIZEALLLAYERS, OnResizeAllLayers)
  ON_COMMAND(ID_MAP_EXPORTTILESET,   OnExportTileset)
  ON_COMMAND(ID_MAP_IMPORTTILESET,   OnImportTileset)
  ON_COMMAND(ID_MAP_PRUNETILESET,    OnPruneTileset)
  ON_NOTIFY(TCN_SELCHANGE, IDC_TAB,  OnTabChanged)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wMapWindow, wSaveableDocumentWindow)
  EVT_SIZE(wMapWindow::OnSize)
  EVT_MENU(wID_MAP_PROPERTIES,      wMapWindow::OnMapProperties)
  EVT_MENU(wID_MAP_CHANGETILESIZE,  wMapWindow::OnChangeTileSize)
  EVT_MENU(wID_MAP_RESCALETILESET,  wMapWindow::OnRescaleTileset)
  EVT_MENU(wID_MAP_RESIZEALLLAYERS, wMapWindow::OnResizeAllLayers)
  EVT_MENU(wID_MAP_EXPORTTILESET,   wMapWindow::OnExportTileset)
  EVT_MENU(wID_MAP_IMPORTTILESET,   wMapWindow::OnImportTileset)
  EVT_MENU(wID_MAP_PRUNETILESET,    wMapWindow::OnPruneTileset)

#ifdef USE_WXTABCTRL
  EVT_TAB_SEL_CHANGED(wID_MAP_TAB,  wMapWindow::OnTabChanged)
#else
  EVT_NOTEBOOK_PAGE_CHANGED(wID_MAP_TAB,  wMapWindow::OnNotebookChanged)
#endif

END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wMapWindow::wMapWindow(const char* map)
: wSaveableDocumentWindow(map, wID_MAP_base /*todo:IDR_MAP*/, wxSize(400, 100))
, m_Created(false)
, m_TilePalette(NULL)
{
  // LoadMap() displays message boxes if something goes wrong
  // SetSaved() and SetModified() are called in here
  if (!LoadMap(map)) {
    delete this;
    return;
  }

  Create();
}

////////////////////////////////////////////////////////////////////////////////

wMapWindow::~wMapWindow()
{
  // destroy all the child windows
  //Destroy();
}

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::Create()
{
  // create the window
  //wSaveableDocumentWindow::Create(AfxRegisterWndClass(0, NULL, NULL, AfxGetApp()->LoadIcon(IDI_MAP)));

  // create the views
  m_MapView = new wMapView(this, this, this, &m_Map);
  m_LayerView = new wLayerView(this, this, &m_Map);
  m_TilesetEditView = new wTilesetEditView(this, this, this, &m_Map.GetTileset());

  m_TilePalette = new wTilePalette(this, this, &m_Map.GetTileset());

#ifdef USE_WXTABCTRL
  m_TabControl = new wxTabCtrl(this, wID_MAP_TAB); // wxDefaultPosition, wxDefaultSize);
  m_TabControl->InsertItem(0, "Map");
  m_TabControl->InsertItem(1, "Tileset");
#else
  wxNotebookPage* mapviewpage = new wxNotebookPage(this, -1);
  wxNotebookPage* tileviewpage = new wxNotebookPage(this, -1);
  m_NotebookControl = new wxNotebook(this, wID_MAP_TAB);
  m_NotebookControl->InsertPage(0, mapviewpage, "Map");
  m_NotebookControl->InsertPage(1, tileviewpage, "Tileset");
#endif

  m_Created = true;  // the window and children are ready!

/*todo:
  // move things to their rightful places
  RECT ClientRect;
  GetClientRect(&ClientRect);
  OnSize(0, ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top);
*/

  TabChanged(0);

  wxSizeEvent event(GetSize());
  OnSize(event);
}

/*
////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::Destroy()
{
  m_TabControl.DestroyWindow();
  m_LayerView.DestroyWindow();
  m_MapView.DestroyWindow();
  m_TilesetEditView.DestroyWindow();

  if (m_TilePalette)
    m_TilePalette->Destroy();
}
*/

////////////////////////////////////////////////////////////////////////////////

bool
wMapWindow::LoadMap(const char* szMap)
{
  SetSaved(szMap != NULL);
  SetModified(false);

  // try to load the map
  if (szMap == NULL || !m_Map.Load(szMap))
  {
    if (szMap) {
      int retval = ::wxMessageBox("Could not load map.\nCreate new?", "New map?", wxYES_NO);
      if (retval != wxYES) {
        return false;
      }
    }

    // show the new map dialog box
    wNewMapDialog NewMapDialog(this);
    if (NewMapDialog.ShowModal() != wxID_OK)
      return false;

    // allocate the map
    sMap newMap(NewMapDialog.GetMapWidth(), NewMapDialog.GetMapHeight());
    m_Map = newMap;

    // attempt to load the tileset
    if (strlen(NewMapDialog.GetTileset()) != 0 &&
        m_Map.GetTileset().Load(NewMapDialog.GetTileset()) == false)
    {
      ::wxMessageBox("Could not load tileset.\nUsing default.");
      m_Map.GetTileset().Create(1);
    } else if(strlen(NewMapDialog.GetTileset()) == 0) {
      m_Map.GetTileset().Create(1);
    }

    // set the initial name of the first layer
    m_Map.GetLayer(0).SetName("Base");

    SetModified(true);
  }

  // if the tileset has no tiles, ask user to choose a tileset
  if (m_Map.GetTileset().GetNumTiles() == 0)
  {
    wTilesetSelectionDialog dialog(this);
    if (dialog.ShowModal() == wxID_CANCEL)
      return false;

    if (!m_Map.GetTileset().Load(dialog.GetTilesetPath()))
    {
      ::wxMessageBox("Error: Could not load tileset");
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
      if (::wxMessageBox("Error: Map has some invalid tile references, remove them?", "Load Map", wxYES_NO) == wxNO)
        return false;

      for (int il = 0; il < m_Map.GetNumLayers(); il++)
        for (int iy = 0; iy < m_Map.GetLayer(il).GetHeight(); iy++)
          for (int ix = 0; ix < m_Map.GetLayer(il).GetWidth(); ix++)
            if (m_Map.GetLayer(il).GetTile(ix, iy) >= m_Map.GetTileset().GetNumTiles())
              m_Map.GetLayer(il).SetTile(ix, iy, 0);
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
wMapWindow::GetMapViewRect(wxRect *Rect)
{
  const int SCROLLBAR_WIDTH  = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
  // const int SCROLLBAR_HEIGHT = wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y);

  wxSize ClientSize = GetClientSize();

  *Rect = wxRect(100 + SCROLLBAR_WIDTH, 0, ClientSize.GetWidth() - 100 - SCROLLBAR_WIDTH, ClientSize.GetHeight());
}

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::GetLayerViewRect(wxRect* Rect)
{
  const int SCROLLBAR_WIDTH  = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
  // const int SCROLLBAR_HEIGHT = wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y);

  wxSize ClientSize = GetClientSize();

  *Rect = wxRect(0, 0, 100 + SCROLLBAR_WIDTH, ClientSize.GetHeight());
}

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::TabChanged(int tab)
{
  if (tab == 0)
  {
    m_MapView->Show();
    m_LayerView->Show();
    m_TilesetEditView->Hide();
  }
  else
  {
    m_MapView->Hide();
    m_LayerView->Hide();
    m_TilesetEditView->Show();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::OnSize(wxSizeEvent& event)
{
  int cx = GetClientSize().GetWidth();
  int cy = GetClientSize().GetHeight();
  if (m_Created)
  {
#ifdef USE_WXTABCTRL
    m_TabControl->SetSize(0, 0, cx, TAB_HEIGHT);
#else
    m_NotebookControl->SetSize(0, 0, cx, TAB_HEIGHT);
#endif

    // move the layer view
    wxRect LayerViewRect;
    GetLayerViewRect(&LayerViewRect);
    m_LayerView->SetSize(
      LayerViewRect.x,
      LayerViewRect.y + TAB_HEIGHT,
      LayerViewRect.width,
      LayerViewRect.height - TAB_HEIGHT);

    // move the map view
    wxRect MapViewRect;
    GetMapViewRect(&MapViewRect);
    m_MapView->SetSize(
      MapViewRect.x,
      MapViewRect.y + TAB_HEIGHT,
      MapViewRect.width,
      MapViewRect.height - TAB_HEIGHT);

    m_TilesetEditView->SetSize(0, TAB_HEIGHT, cx, cy - TAB_HEIGHT);
  }

  wSaveableDocumentWindow::OnSize(event);
}

/*todo:
////////////////////////////////////////////////////////////////////////////////

void
CMapWindow::OnCommand(WPARAM wparam, LPARAM lparam)
{
  return m_TilesetEditView.SendMessage(WM_COMMAND, wparam, lparam);
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::OnMapProperties(wxCommandEvent &event)
{
  wMapPropertiesDialog Dialog(this, &m_Map);
  if (Dialog.ShowModal() == wxID_OK)
    SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::OnChangeTileSize(wxCommandEvent &event)
{
  int tile_width  = m_Map.GetTileset().GetTileWidth();
  int tile_height = m_Map.GetTileset().GetTileHeight();
  wResizeDialog dialog(this, "Resize Tiles", tile_width, tile_height);
  if (dialog.ShowModal() == wxID_OK)
  {
    m_Map.GetTileset().SetTileSize(dialog.GetWidth(), dialog.GetHeight());

    SetModified(true);
    m_MapView->TilesetChanged();
    m_TilesetEditView->TilesetChanged();
    m_TilePalette->TilesetChanged();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::OnRescaleTileset(wxCommandEvent &event)
{
    int tile_width  = m_Map.GetTileset().GetTileWidth();
    int tile_height = m_Map.GetTileset().GetTileHeight();

    wResizeDialog dialog(this, "Rescale Tiles", tile_width, tile_height);
    if (dialog.ShowModal() == wxID_OK) {
        m_Map.GetTileset().SetTileSize(dialog.GetWidth(), dialog.GetHeight(), true);

        SetModified(true);
        m_MapView->TilesetChanged();
        m_TilesetEditView->TilesetChanged();
        m_TilePalette->TilesetChanged();
    }
}

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::OnResizeAllLayers(wxCommandEvent &event)
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
  wResizeDialog dialog(this, "Resize All Layers", max_layer_width, max_layer_height);
  if (dialog.ShowModal() == wxID_OK) {

    for (int i = 0; i < m_Map.GetNumLayers(); i++) {
      m_Map.GetLayer(i).Resize(dialog.GetWidth(), dialog.GetHeight());
    }

    SetModified(true);
    m_MapView->UpdateScrollBars();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::OnExportTileset(wxCommandEvent &event)
{
  wTilesetFileDialog dialog(this, FDM_SAVE);
  if (dialog.ShowModal() == wxID_OK)
  {
    if (!m_Map.GetTileset().Save(dialog.GetPath()))
      ::wxMessageBox("Error: Could not export tileset");
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::OnImportTileset(wxCommandEvent &event)
{
  wTilesetFileDialog dialog(this, FDM_OPEN);
  if (dialog.ShowModal() == wxID_OK)
  {
    sTileset tileset;
    if (tileset.Load(dialog.GetPath()))
    {
      m_Map.GetTileset() = tileset;
      m_Map.ValidateTileIndices();

      m_MapView->TilesetChanged();
      m_TilesetEditView->TilesetChanged();
      m_TilePalette->TilesetChanged();
      SetModified(true);
    }
    else
      ::wxMessageBox("Error: Could not import tileset");
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::OnPruneTileset(wxCommandEvent &event)
{
  if (::wxMessageBox("Are you sure you want to remove unused tiles?", "Prune Tileset", wxYES_NO) == wxYES) {
    m_Map.PruneTileset();

    SetModified(true);

    m_MapView->Refresh();
    m_MapView->UpdateScrollBars();

    m_TilePalette->TilesetChanged();
  }
}

////////////////////////////////////////////////////////////////////////////////

#ifdef USE_WXTABCTRL
void
wMapWindow::OnTabChanged(wxTabEvent &event)
{
  TabChanged(0);// m_TabControl->GetCurFocus());
}
#else
void
wMapWindow::OnNotebookChanged(wxNotebookEvent &event)
{
  if (m_Created)
    TabChanged(m_NotebookControl->GetSelection());
}
#endif

////////////////////////////////////////////////////////////////////////////////

bool
wMapWindow::GetSavePath(char* path)
{
  wMapFileDialog Dialog(this, FDM_SAVE);
  if (Dialog.ShowModal() != wxID_OK)
    return false;

  strcpy(path, Dialog.GetPath());
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
wMapWindow::SaveDocument(const char* path)
{
  return m_Map.Save(path);
}

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::MV_SelectedTileChanged(int tile)
{
  m_LayerView->SetSelectedTile(tile);
  m_TilesetEditView->SelectTile(tile);
  m_TilePalette->SelectTile(tile);
}

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::MV_MapChanged()
{
  SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::LV_MapChanged()
{
  SetModified(true);

  m_MapView->Refresh();
  m_MapView->UpdateScrollBars();

  // this might be called from a layerview delete layer operation, and the
  // currently selected tile might be out of range, so make sure TilesetChanged
  // verifies that.
  m_TilePalette->TilesetChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::LV_SelectedLayerChanged(int layer)
{
  m_MapView->SelectLayer(layer);
}

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::TEV_SelectedTileChanged(int tile)
{
  m_LayerView->SetSelectedTile(tile);
  m_TilePalette->SelectTile(tile);
  m_MapView->SelectTile(tile);
}

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::TEV_TileModified(int tile)
{
  SetModified(true);
  m_MapView->Refresh();
  m_TilePalette->TileChanged(tile);
}

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::TEV_TilesetModified()
{
  SetModified(true);
  m_MapView->Refresh();
  m_TilePalette->Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::TV_SelectedTileChanged(int tile)
{
  m_LayerView->SetSelectedTile(tile);
  m_MapView->SelectTile(tile);
  m_TilesetEditView->SelectTile(tile);
}

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::TV_TilesetChanged()
{
  m_MapView->Refresh();
  m_TilesetEditView->TilesetChanged();
  SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////

void
wMapWindow::TV_InsertedTiles(int at, int numtiles)
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
wMapWindow::TV_DeletedTiles(int at, int numtiles)
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

void 
wMapWindow::SP_ColorSelected(RGBA color)
{
  m_TilesetEditView->SP_ColorSelected(color);
}

////////////////////////////////////////////////////////////////////////////////

