#ifdef _MSC_VER
// identifier too long
#pragma warning(disable : 4786)
#endif


#include "TilesetEditView.hpp"
#include "TilePropertiesDialog.hpp"
//#include "NumberDialog.hpp"
#include "FileDialogs.hpp"
//#include "resource.h"
#include "IDs.hpp"

/*
#define ID_TILESET_INSERTTILES      705
#define ID_TILESET_APPENDTILES      706
#define ID_TILESET_DELETETILE       707
#define ID_TILESET_REPLACEWITHIMAGE 708
#define ID_TILESET_INSERTIMAGE      709
#define ID_TILESET_APPENDIMAGE      710
*/

/*
BEGIN_MESSAGE_MAP(CTilesetEditView, CHScrollWindow)

  ON_WM_SIZE()

  ON_COMMAND(ID_TILESET_INSERTTILES, OnTilesetInsertTiles)
  ON_COMMAND(ID_TILESET_APPENDTILES, OnTilesetAppendTiles)

  ON_COMMAND(ID_TILESET_DELETETILE, OnTilesetDeleteTile)

  ON_COMMAND(ID_TILESET_REPLACEWITHIMAGE, OnTilesetReplaceWithImage)
  ON_COMMAND(ID_TILESET_INSERTIMAGE,      OnTilesetInsertImage)
  ON_COMMAND(ID_TILESET_APPENDIMAGE,      OnTilesetAppendImage)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wTilesetEditView, wHScrollWindow)

  EVT_SIZE(wTilesetEditView::OnSize)
  
  EVT_MENU(wID_TILESET_INSERTTILES,       wTilesetEditView::OnTilesetInsertTiles)
  EVT_MENU(wID_TILESET_APPENDTILES,       wTilesetEditView::OnTilesetAppendTiles)

  EVT_MENU(wID_TILESET_DELETETILE,        wTilesetEditView::OnTilesetDeleteTile)

  EVT_MENU(wID_TILESET_REPLACEWITHIMAGE,  wTilesetEditView::OnTilesetReplaceWithImage)
  EVT_MENU(wID_TILESET_INSERTIMAGE,       wTilesetEditView::OnTilesetInsertImage)
  EVT_MENU(wID_TILESET_APPENDIMAGE,       wTilesetEditView::OnTilesetAppendImage)

END_EVENT_TABLE()


////////////////////////////////////////////////////////////////////////////////

wTilesetEditView::wTilesetEditView(wxWindow* parent, wDocumentWindow* owner, ITilesetEditViewHandler* handler, sTileset* tileset)
: wHScrollWindow(parent, -1)
, m_Handler(handler)
, m_Tileset(tileset)
, m_CurrentTile(0)
, m_Created(false)
{
  m_Menu = new wxMenu();
  m_Created = true;

  m_Handler = handler;
  m_Tileset = tileset;


  m_ImageView = new wImageView(this, owner, this);
  m_PaletteView = new wPaletteView(this, this);
  m_ColorView = new wColorView(this, this);
  m_AlphaView = new wAlphaView(this, this);

  m_Created = true;

  UpdateImageView();
}

////////////////////////////////////////////////////////////////////////////////

wTilesetEditView::~wTilesetEditView()
{
  delete m_Menu;
  delete m_ImageView;
  delete m_PaletteView;
  delete m_ColorView;
  delete m_AlphaView;
}

////////////////////////////////////////////////////////////////////////////////
/*
BOOL
CTilesetEditView::Create(CWnd* parent, CDocumentWindow* owner, ITilesetEditViewHandler* handler, sTileset* tileset)
{
  CWnd::Create(
    AfxRegisterWndClass(0, LoadCursor(NULL, IDC_ARROW), NULL, NULL),
    "",
    WS_CHILD | WS_VISIBLE | WS_HSCROLL,
    CRect(0, 0, 0, 0),
    parent,
    1000);

  m_Handler = handler;
  m_Tileset = tileset;

  // create the views
  m_ImageView.Create(owner, this, this);
  m_PaletteView.Create(this, this);
  m_ColorView.Create(this, this);
  m_AlphaView.Create(this, this);

  m_Created = true;

  // put everything in the right place
  RECT rect;
  GetClientRect(&rect);
  OnSize(0, rect.right, rect.bottom);

  UpdateImageView();
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
*/
void
wTilesetEditView::TilesetChanged()
{
  UpdateImageView();
  UpdateScrollBar();
  Refresh(); 
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetEditView::SelectTile(int tile)
{
  m_CurrentTile = tile;
  UpdateImageView();
  UpdateScrollBar();
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetEditView::SP_ColorSelected(RGBA color)
{
  byte alpha = color.alpha;
  RGB  rgb   = { color.red, color.green, color.blue };

  m_ImageView->SetColor(color);
  m_ColorView->SetColor(rgb);
  m_AlphaView->SetAlpha(alpha);
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetEditView::UpdateImageView()
{
  sTile& tile = m_Tileset->GetTile(m_CurrentTile);
  m_ImageView->SetImage(tile.GetWidth(), tile.GetHeight(), tile.GetPixels());
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetEditView::UpdateScrollBar()
{
  SetHScrollPosition(m_CurrentTile);
  SetHScrollRange(m_Tileset->GetNumTiles(), 1);
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetEditView::OnHScrollChanged(int x)
{
  m_CurrentTile = x;
  UpdateImageView();
  UpdateScrollBar();
  m_Handler->TEV_SelectedTileChanged(x);
}

////////////////////////////////////////////////////////////////////////////////

static inline void twMoveControl(wxWindow* control, int x, int y, int w, int h)
{
  control->SetSize(x, y, w, h);
  control->Refresh();
}

void
wTilesetEditView::OnSize(wxSizeEvent &event)
{
  const int PaletteWidth = 60;
  const int AlphaWidth = 32;
  int cx = event.GetSize().GetWidth();
  int cy = event.GetSize().GetHeight();

  if (m_Created) {
    
    twMoveControl(m_ImageView, 0, 0, cx - PaletteWidth - AlphaWidth, cy);
    twMoveControl(m_PaletteView, cx - PaletteWidth - AlphaWidth, 0, PaletteWidth, cy - PaletteWidth);
    twMoveControl(m_ColorView, cx - PaletteWidth - AlphaWidth, cy - PaletteWidth, PaletteWidth, PaletteWidth);
    twMoveControl(m_AlphaView, cx - AlphaWidth, 0, AlphaWidth, cy);

    UpdateScrollBar();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetEditView::OnTilesetInsertTiles(wxCommandEvent &event)
{
  //CNumberDialog NumberDialog("Insert Tiles", "How many tiles do you want to insert?", 1, 1, 256);
  //if (NumberDialog.DoModal() == IDOK)
  int val = ::wxGetNumberFromUser("", "How many tiles do you want to insert?", "Insert Tiles", 1, 1, 256);
  if(val != -1)
  {
    m_Tileset->InsertTiles(m_CurrentTile, val);//NumberDialog.GetValue());
    UpdateScrollBar();
    UpdateImageView();
    
    m_Handler->TEV_TilesetModified();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetEditView::OnTilesetAppendTiles(wxCommandEvent &event)
{
  //CNumberDialog NumberDialog("Append Tiles", "How many tiles do you want to append?", 1, 1, 256);
  //if (NumberDialog.DoModal() == IDOK)
  int val = ::wxGetNumberFromUser("", "How many tiles do you want to append?", "Append Tiles", 1, 1, 256);
  if(val != -1)
  {
    m_Tileset->AppendTiles(val);//NumberDialog.GetValue());
    UpdateScrollBar();
    UpdateImageView();
    m_Handler->TEV_TilesetModified();
  }
}

/*todo:
////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetEditView::OnUpdateTilesetDeleteTile(CCmdUI* cmdui)
{
  if (m_Tileset->GetNumTiles() > 1)
    cmdui->Enable();
  else
    cmdui->Enable(FALSE);
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wTilesetEditView::OnTilesetDeleteTile(wxCommandEvent &event)
{
  m_Tileset->DeleteTiles(m_CurrentTile, 1);
  if (m_CurrentTile >= m_Tileset->GetNumTiles())
    m_CurrentTile--;

  m_Handler->TEV_TilesetModified();
  UpdateScrollBar();
  UpdateImageView();

  m_Handler->TEV_SelectedTileChanged(m_CurrentTile);
  m_Handler->TEV_TilesetModified();
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetEditView::OnTilesetReplaceWithImage(wxCommandEvent &event)
{
  if (wxMessageBox("Are you sure?", "Replace with image", wxYES_NO) == wxNO)
    return;

  wImageFileDialog FileDialog(this, FDM_OPEN);
  if (FileDialog.ShowModal() == wxID_OK)
  {
    if (!m_Tileset->Import_Image(FileDialog.GetPath()))
    {
      wxMessageBox("Error: Could not import image", "Error", wxOK);
      return;
    }

    m_Handler->TEV_TilesetModified();
    UpdateScrollBar();

    UpdateImageView();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetEditView::OnTilesetInsertImage(wxCommandEvent &event)
{
  wImageFileDialog FileDialog(this, FDM_OPEN);
  if (FileDialog.ShowModal() == wxID_OK)
  {
    if (!m_Tileset->InsertImage(m_CurrentTile, FileDialog.GetPath()))
    {
      wxMessageBox("Error: Could not insert image", "Error", wxOK);
      return;
    }

    m_Handler->TEV_TilesetModified();
    UpdateScrollBar();
    UpdateImageView();

    m_Handler->TEV_TilesetModified();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetEditView::OnTilesetAppendImage(wxCommandEvent &event)
{
  wImageFileDialog FileDialog(this, FDM_OPEN);
  if (FileDialog.ShowModal() == wxID_OK)
  {
    if (!m_Tileset->AppendImage(FileDialog.GetPath()))
    {
      wxMessageBox("Error: Could not append image", "Error", wxOK);
      return;
    }

    m_Handler->TEV_TilesetModified();
    UpdateScrollBar();
    UpdateImageView();

    m_Handler->TEV_TilesetModified();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetEditView::IV_ImageChanged()
{
  // store the old data
  memcpy(
    m_Tileset->GetTile(m_CurrentTile).GetPixels(),
    m_ImageView->GetPixels(),
    m_Tileset->GetTileWidth() * m_Tileset->GetTileHeight() * sizeof(RGBA));

  m_Handler->TEV_TileModified(m_CurrentTile);
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetEditView::IV_ColorChanged(RGBA color)
{
  RGBA rgba = m_ImageView->GetColor();
  RGB rgb = { rgba.red, rgba.green, rgba.blue };
  m_ColorView->SetColor(rgb);
  m_AlphaView->SetAlpha(rgba.alpha);
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetEditView::PV_ColorChanged(RGB color)
{
  m_ColorView->SetColor(color);
  byte alpha = m_AlphaView->GetAlpha();
  RGBA c = { color.red, color.green, color.blue, alpha };
  m_ImageView->SetColor(c);
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetEditView::CV_ColorChanged(RGB color)
{
  byte alpha = m_AlphaView->GetAlpha();
  RGBA rgba = { color.red, color.green, color.blue, alpha };
  m_ImageView->SetColor(rgba);
}

////////////////////////////////////////////////////////////////////////////////

void
wTilesetEditView::AV_AlphaChanged(byte alpha)
{
  RGBA rgba = m_ImageView->GetColor();
  rgba.alpha = alpha;
  m_ImageView->SetColor(rgba);
}

////////////////////////////////////////////////////////////////////////////////
