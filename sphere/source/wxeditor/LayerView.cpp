#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include "LayerView.hpp"
#include "LayerPropertiesDialog.hpp"
#include "Editor.hpp"
#include "FileDialogs.hpp"
#include "../common/Map.hpp"
//#include "resource.h"
#include "IDs.hpp"

#include "icons/eye.xpm"
#include "icons/eyeclose.xpm"


const int LAYER_BUTTON_HEIGHT = 20;

/*
BEGIN_MESSAGE_MAP(CLayerView, CVScrollWindow)

  ON_WM_CREATE()
  ON_WM_SIZE()
  ON_WM_PAINT()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_RBUTTONUP()
  ON_WM_MOUSEMOVE()

  ON_COMMAND(ID_LAYERVIEW_INSERTLAYER,          OnInsertLayer)
  ON_COMMAND(ID_LAYERVIEW_INSERTLAYERFROMIMAGE, OnInsertLayerFromImage)
  ON_COMMAND(ID_LAYERVIEW_DELETELAYER,          OnDeleteLayer)
  ON_COMMAND(ID_LAYERVIEW_DUPLICATELAYER,       OnDuplicateLayer)
  ON_COMMAND(ID_LAYERVIEW_PROPERTIES,           OnLayerProperties)
  ON_COMMAND(ID_LAYERVIEW_EXPORTASIMAGE,        OnExportLayer)

END_MESSAGE_MAP()
*/
BEGIN_EVENT_TABLE(wLayerView, wVScrollWindow)
  EVT_SIZE(wLayerView::OnSize)
  EVT_PAINT(wLayerView::OnPaint)
  EVT_LEFT_DOWN(wLayerView::OnLButtonDown)
  EVT_LEFT_UP(wLayerView::OnLButtonUp)
  EVT_RIGHT_UP(wLayerView::OnRButtonUp)
  EVT_MOTION(wLayerView::OnMouseMove)

  EVT_MENU(wID_LAYERVIEW_INSERTLAYER,          wLayerView::OnInsertLayer)
  EVT_MENU(wID_LAYERVIEW_INSERTLAYERFROMIMAGE, wLayerView::OnInsertLayerFromImage)
  EVT_MENU(wID_LAYERVIEW_DELETELAYER,          wLayerView::OnDeleteLayer)
  EVT_MENU(wID_LAYERVIEW_DUPLICATELAYER,       wLayerView::OnDuplicateLayer)
  EVT_MENU(wID_LAYERVIEW_PROPERTIES,           wLayerView::OnLayerProperties)
  EVT_MENU(wID_LAYERVIEW_EXPORTASIMAGE,        wLayerView::OnExportLayer)
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wLayerView::wLayerView(wxWindow *parent, ILayerViewHandler* handler, sMap* map)
: wVScrollWindow(parent)
, m_Handler(NULL)
, m_Map(NULL)
, m_TopLayer(0)
, m_SelectedLayer(0)
, m_SelectedTile(0)
, m_IsDragging(false)
{
  m_Handler = handler;
  m_Map = map;

  m_Menu = new wxMenu();
  
  m_Menu->Append(wID_LAYERVIEW_INSERTLAYER,          "Insert");
  m_Menu->Append(wID_LAYERVIEW_INSERTLAYERFROMIMAGE, "Insert from Image");
  m_Menu->Append(wID_LAYERVIEW_DELETELAYER,          "Delete");
  m_Menu->Append(wID_LAYERVIEW_DUPLICATELAYER,       "Duplicate");
  m_Menu->AppendSeparator();
  m_Menu->Append(wID_LAYERVIEW_PROPERTIES,           "Properties");
  m_Menu->AppendSeparator();
  m_Menu->Append(wID_LAYERVIEW_EXPORTASIMAGE,        "Export");

  UpdateScrollBar();
}

////////////////////////////////////////////////////////////////////////////////

wLayerView::~wLayerView()
{
  delete m_Menu;
  //DestroyWindow();
}

/*
////////////////////////////////////////////////////////////////////////////////

BOOL
CLayerView::Create(ILayerViewHandler* handler, CWnd* parent, sMap* map)
{
  m_Handler = handler;
  m_Map = map;

  return CWnd::Create(
    AfxRegisterWndClass(0, LoadCursor(NULL, IDC_ARROW), NULL, LoadIcon(NULL, IDI_APPLICATION)),
    "LayerView",
    WS_CHILD | WS_VISIBLE | WS_VSCROLL,
    CRect(0, 0, 0, 0),
    parent,
    0
  );
}
*/

////////////////////////////////////////////////////////////////////////////////

int
wLayerView::GetSelectedLayer() const
{
  return m_SelectedLayer;
}

////////////////////////////////////////////////////////////////////////////////

void
wLayerView::SetSelectedTile(int tile)
{
  m_SelectedTile = tile;
}

/*
////////////////////////////////////////////////////////////////////////////////

afx_msg int
CLayerView::OnCreate(CREATESTRUCT* createstruct)
{
  UpdateScrollBar();
  return 0;
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wLayerView::OnSize(wxSizeEvent &event)
{
  UpdateScrollBar();
  wVScrollWindow::OnSize(event);
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

static wxPen CreateShadedPen(RGB color1, RGB color2, int alpha)
{
  wxColor color(
    (color1.red   * alpha + color2.red   * (256 - alpha)) / 256,
    (color1.green * alpha + color2.green * (256 - alpha)) / 256,
    (color1.blue  * alpha + color2.blue  * (256 - alpha)) / 256
  );
  return wxPen(color, 1, wxSOLID);
}

static void DrawLayerButton(wxDC &dc, wxRect rect, const sLayer& layer, bool pushed)
{
  // get the menu color
  wxColour sys_color = wxSystemSettings::GetSystemColour(wxSYS_COLOUR_MENU);
  RGB menu_color = {
    sys_color.Red(),
    sys_color.Green(),
    sys_color.Blue(),
  };
  RGB white = { 255, 255, 255 };
  RGB black = { 0,   0,   0   };

  if (pushed) {
    std::swap(white, black);
  }

  wxPen very_light = CreateShadedPen(menu_color, white, 64);
  wxPen light      = CreateShadedPen(menu_color, white, 192);
  wxPen dark       = CreateShadedPen(menu_color, black, 192);
  wxPen very_dark  = CreateShadedPen(menu_color, black, 64);

  //SaveDC(dc);
  //SetROP2(dc, R2_COPYPEN);

  
  // draw the outer border
  dc.SetPen(very_light);
  dc.DrawLine(rect.x, rect.y, rect.x + rect.width - 1, rect.y);
  dc.DrawLine(rect.x, rect.y, rect.x, rect.y + rect.height - 1);

  dc.SetPen(very_dark);
  dc.DrawLine(rect.x + rect.width - 1, rect.y, rect.x + rect.width - 1, rect.y + rect.height - 1);
  dc.DrawLine(rect.x, rect.y + rect.height - 1, rect.x + rect.width - 1, rect.y + rect.height - 1);

  // draw the inner border
  dc.SetPen(light);
  dc.DrawLine(rect.x + 1, rect.y + 1, rect.x + rect.width - 2, rect.y + 1);
  dc.DrawLine(rect.x + 1, rect.y + 1, rect.x + 1, rect.y + rect.height - 2);

  dc.SetPen(dark);
  dc.DrawLine(rect.x + rect.width - 2, rect.y + 1, rect.x + rect.width - 2, rect.y + rect.height - 2);
  dc.DrawLine(rect.x + 1, rect.y + rect.height - 2, rect.x + rect.width - 2, rect.y + rect.height - 2);
  
  // draw the center of the button
  dc.SetBrush(wxBrush(sys_color, wxSOLID));
  dc.SetPen(wxPen(sys_color, 1, wxSOLID));

  dc.DrawRectangle(rect.x + 2, rect.y + 2, rect.width - 3, rect.height - 3);


  wxBitmap bitmap;
  if(layer.IsVisible()) {
    bitmap = wxBitmap((const char **)eye_xpm);
  } else {
    bitmap = wxBitmap((const char **)eyeclose_xpm);
  }
  dc.DrawBitmap(bitmap, rect.x + rect.height / 2 - 8, rect.y + rect.height / 2 - 8, TRUE);

  dc.SetTextForeground(*wxBLACK);
  dc.SetBackgroundMode(wxTRANSPARENT);
  dc.DrawText(layer.GetName(), rect.x + rect.height / 2 + 12, rect.y + rect.height / 2 - 8);
  dc.SetBrush(wxNullBrush);
  dc.SetPen(wxNullPen);
}

////////////////////////////////////////////////////////////////////////////////

void
wLayerView::OnPaint(wxPaintEvent &event)
{
  wxSize client_size = GetClientSize();

  wxPaintDC dc(this);
  wxRegion region = GetUpdateRegion();

  for (int i = 0; i < client_size.GetHeight() / LAYER_BUTTON_HEIGHT + 1; i++) {
    wxRect rect(
      0,
      i * LAYER_BUTTON_HEIGHT,
      client_size.GetWidth(),
      LAYER_BUTTON_HEIGHT
    );
    
    // visibility check
    if (region.Contains(rect) == wxOutRegion) {
      continue;
    }

    // if the layer isn't part of the map, draw an empty rectangle
    if ((i + m_TopLayer) >= m_Map->GetNumLayers()) {
      wxColour sys_color = wxSystemSettings::GetSystemColour(wxSYS_COLOUR_APPWORKSPACE);
      dc.SetBrush(wxBrush(sys_color, wxSOLID));
      dc.SetPen(wxPen(sys_color, 1, wxSOLID));
      dc.DrawRectangle(rect);
      dc.SetBrush(wxNullBrush);
      dc.SetPen(wxNullPen);
      continue;
    }

    // draw layers upside down!
    int layer = m_Map->GetNumLayers() - i - m_TopLayer - 1;
    DrawLayerButton(dc, rect, m_Map->GetLayer(layer), (m_SelectedLayer == layer));
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wLayerView::OnLButtonDown(wxMouseEvent &event)
{
  Click(event.GetX(), event.GetY(), true);
  if(!m_IsDragging) {
    m_IsDragging = true;
    CaptureMouse();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wLayerView::OnLButtonUp(wxMouseEvent &event)
{
  if(m_IsDragging) {
    m_IsDragging = false;
    if (HasCapture())  ReleaseMouse();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wLayerView::OnRButtonUp(wxMouseEvent &event)
{
  wxSize client_size = GetClientSize();

  // simulate a left click to set the selected layer
  Click(event.GetX(), event.GetY(), false);

  // make sure the right things are greyed and stuff
  if (event.GetX() >= 0 &&
      event.GetX() < client_size.GetWidth() &&
      event.GetY() >= 0 &&
      event.GetY() < LAYER_BUTTON_HEIGHT * m_Map->GetNumLayers())
  {
    // enable delete command if it's available
    if (m_Map->GetNumLayers() > 1) {
      m_Menu->Enable(wID_LAYERVIEW_DELETELAYER, TRUE);
    } else {
      m_Menu->Enable(wID_LAYERVIEW_DELETELAYER, FALSE);
    }
    m_Menu->Enable(wID_LAYERVIEW_PROPERTIES, TRUE);

    PopupMenu(m_Menu, event.GetPosition());
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wLayerView::OnMouseMove(wxMouseEvent &event)
{
  if (m_IsDragging) {

    // see if we're dragging up or down
    //point.y -= m_TopLayer * LAYER_BUTTON_HEIGHT;
    //int layer = (m_Map->GetNumLayers() * LAYER_BUTTON_HEIGHT - point.y) / LAYER_BUTTON_HEIGHT;
    int layer = (m_Map->GetNumLayers() * LAYER_BUTTON_HEIGHT + m_TopLayer * LAYER_BUTTON_HEIGHT - event.GetY()) / LAYER_BUTTON_HEIGHT;

    // drag down
    while (m_SelectedLayer > 0 && layer < m_SelectedLayer) {
      m_Map->SwapLayers(m_SelectedLayer, m_SelectedLayer - 1);
      m_SelectedLayer--;

      Refresh();
      m_Handler->LV_MapChanged();
      m_Handler->LV_SelectedLayerChanged(m_SelectedLayer);
    }

    // drag up
    while (m_SelectedLayer < m_Map->GetNumLayers() - 1 && layer > m_SelectedLayer) {
      m_Map->SwapLayers(m_SelectedLayer, m_SelectedLayer + 1);
      m_SelectedLayer++;

      Refresh();
      m_Handler->LV_MapChanged();
      m_Handler->LV_SelectedLayerChanged(m_SelectedLayer);
    }

  }

  int layer = event.GetY() / LAYER_BUTTON_HEIGHT;
#ifdef WIN32
  if (layer >= 0 && layer < m_Map->GetNumLayers()) {
    //CString str;
    //str.Format("Layer '%d'", m_Map->GetNumLayers() - 1 - layer);
    //GetStatusBar()->SetWindowText(str);
    wxString str;
    str.Format("Layer '%d'", m_Map->GetNumLayers() - 1 - layer);
    SetStatus(str);
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////

void
wLayerView::OnVScrollChanged(int y)
{
  m_TopLayer = y;
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wLayerView::OnInsertLayer(wxCommandEvent &event)
{
  sLayer& sl = m_Map->GetLayer(m_SelectedLayer);

  sLayer layer(sl.GetWidth(), sl.GetHeight());
  layer.SetName("Unnamed Layer");
  m_Map->InsertLayer(m_SelectedLayer + 1, layer); // m_SelectedLayer + 1 = above

  // update visibility buttons
  UpdateScrollBar();
  Refresh();
  m_Handler->LV_MapChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wLayerView::OnInsertLayerFromImage(wxCommandEvent &event)
{
  // get image filename
  wImageFileDialog dialog(this, FDM_OPEN);
  if (dialog.ShowModal() != wxID_OK) {
    return;
  }

  // load image
  CImage32 image;
  if (!image.Load(dialog.GetPath())) {
    ::wxMessageBox("Could not load image", "Insert Layer From Image");
    return;
  }

  // import image into the map
  sMap map;
  if (!map.BuildFromImage(
        image,
        m_Map->GetTileset().GetTileWidth(),
        m_Map->GetTileset().GetTileHeight())
  ) {
    ::wxMessageBox("Could not import layer from image", "Insert Layer From Image");
    return;
  }

  int where = m_SelectedLayer + 1;  // above
  m_Map->InsertLayerFromMap(where, map, 0);  
  m_Map->GetLayer(where).SetName(dialog.GetFilename());

  UpdateScrollBar();
  Refresh();
  m_Handler->LV_MapChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wLayerView::OnDeleteLayer(wxCommandEvent &event)
{
  int result = ::wxMessageBox("Delete associated tiles?", "Delete Layer", wxYES_NO | wxCANCEL);
  if (result == wxID_CANCEL) {
    return;
  }

  m_Map->DeleteLayer(m_SelectedLayer, (result == wxYES));

  // make sure the selected layer is still valid
  if (m_SelectedLayer > 0) {
    m_SelectedLayer--;
    m_Handler->LV_SelectedLayerChanged(m_SelectedLayer);
  }

  // make sure the base is a valid layer...
  if (m_Map->GetStartLayer() >= m_Map->GetNumLayers()) {
    m_Map->SetStartLayer(m_Map->GetNumLayers() - 1);
  }

  // make sure the start layer has parallax disabled
  m_Map->GetLayer(m_Map->GetStartLayer()).EnableParallax(false);

  UpdateScrollBar();
  Refresh();
  m_Handler->LV_MapChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wLayerView::OnDuplicateLayer(wxCommandEvent &event)
{
  m_Map->DuplicateLayer(m_SelectedLayer);

  UpdateScrollBar();
  Refresh();
  m_Handler->LV_MapChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wLayerView::OnLayerProperties(wxCommandEvent &event)
{
  // calculate if parallax should be allowed
  bool parallax_enabled = false;
  for (int i = 0; i < m_Map->GetNumLayers(); i++) {
    if (i != m_SelectedLayer && m_Map->GetLayer(i).HasParallax() == false) {
      parallax_enabled = true;
    }
  }

  parallax_enabled = parallax_enabled && (m_Map->GetStartLayer() != m_SelectedLayer);

  // show the dialog box
  wLayerPropertiesDialog Dialog(this, m_Map->GetLayer(m_SelectedLayer), parallax_enabled);
  if (Dialog.ShowModal() == wxID_OK) {
    Refresh();
    m_Handler->LV_MapChanged();
  }
}

////////////////////////////////////////////////////////////////////////////////

static bool ExportLayerAsImage(const char* filename, sLayer& layer, sTileset& tileset)
{
  // create image
  int tile_width  = tileset.GetTileWidth();
  int tile_height = tileset.GetTileHeight();
  int image_width  = layer.GetWidth()  * tile_width;
  int image_height = layer.GetHeight() * tile_height;

  CImage32 image(image_width, image_height);
  RGBA* image_pixels = image.GetPixels();

  // render the image
  for (int iy = 0; iy < layer.GetHeight(); iy++) {
    for (int ix = 0; ix < layer.GetWidth(); ix++) {

      RGBA* tile_pixels = tileset.GetTile(layer.GetTile(ix, iy)).GetPixels();

      int dsx = ix * tile_width;
      int dsy = iy * tile_height;

      // render this tile into the image
      for (int i = 0; i < tile_height; i++) {
        memcpy(
          image_pixels + (dsy + i) * image_width + dsx,
          tile_pixels + i * tile_width,
          sizeof(RGBA) * tile_width
        );
      }

    } // end for x
  } // end for y

  return image.Save(filename);
}

void
wLayerView::OnExportLayer(wxCommandEvent &event)
{
  // get file name to export to
  wImageFileDialog dialog(this, FDM_SAVE, "Export Layer as Image");
  if (dialog.ShowModal() == wxID_OK) {

    sLayer& layer = m_Map->GetLayer(m_SelectedLayer);
    sTileset& tileset = m_Map->GetTileset();

    // do the export
    if (!ExportLayerAsImage(dialog.GetPath(), layer, tileset)) {
      ::wxMessageBox("Could not save image", "Export Layer as Image");
    } else {
      ::wxMessageBox("Exported layer!", "Export Layer as Image");
    }

  }
}

////////////////////////////////////////////////////////////////////////////////

void
wLayerView::Click(int x, int y, bool left)
{
  wxSize client_size = GetClientSize();

  if (x >= 0 &&
      x < client_size.GetWidth() &&
      y >= 0 &&
      y < LAYER_BUTTON_HEIGHT * m_Map->GetNumLayers()
  ) {

    y -= m_TopLayer * LAYER_BUTTON_HEIGHT;
    int layer = (m_Map->GetNumLayers() * LAYER_BUTTON_HEIGHT - y) / LAYER_BUTTON_HEIGHT;

    // if x is less than 20, we clicked on the visibility button
    if (x < 20 && left == true) {

      sLayer& l = m_Map->GetLayer(layer);
      l.SetVisible(!l.IsVisible());
      Refresh();
      m_Handler->LV_MapChanged();

    } else {

      // layers are displayed upside down
      m_SelectedLayer = layer;
      Refresh();

      // tell parent that the layer changed
      m_Handler->LV_SelectedLayerChanged(m_SelectedLayer);

    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wLayerView::UpdateScrollBar()
{
  wxSize client_size = GetClientSize();
  int page_size = client_size.GetHeight() / LAYER_BUTTON_HEIGHT;

  // set the scroll range
  SetVScrollRange(m_Map->GetNumLayers(), page_size);
}

////////////////////////////////////////////////////////////////////////////////
