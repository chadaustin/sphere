// identifier too long
#pragma warning(disable : 4786)


#include "TilesetEditView.hpp"
#include "TilePropertiesDialog.hpp"
#include "NumberDialog.hpp"
#include "FileDialogs.hpp"
#include "resource.h"


#define ID_TILESET_INSERTTILES      705
#define ID_TILESET_APPENDTILES      706
#define ID_TILESET_DELETETILE       707
#define ID_TILESET_REPLACEWITHIMAGE 708
#define ID_TILESET_INSERTIMAGE      709
#define ID_TILESET_APPENDIMAGE      710


BEGIN_MESSAGE_MAP(CTilesetEditView, CHScrollWindow)

  ON_WM_SIZE()

  ON_COMMAND(ID_TILESET_INSERTTILES, OnTilesetInsertTiles)
  ON_COMMAND(ID_TILESET_APPENDTILES, OnTilesetAppendTiles)

  ON_COMMAND(ID_TILESET_DELETETILE, OnTilesetDeleteTile)

  ON_COMMAND(ID_TILESET_REPLACEWITHIMAGE, OnTilesetReplaceWithImage)
  ON_COMMAND(ID_TILESET_INSERTIMAGE,      OnTilesetInsertImage)
  ON_COMMAND(ID_TILESET_APPENDIMAGE,      OnTilesetAppendImage)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CTilesetEditView::CTilesetEditView()
: m_Handler(NULL)
, m_Tileset(NULL)
, m_CurrentTile(0)
, m_Created(false)
{
}

////////////////////////////////////////////////////////////////////////////////

CTilesetEditView::~CTilesetEditView()
{
  if (m_Created)
    DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////

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

void
CTilesetEditView::TilesetChanged()
{
  UpdateImageView();
  UpdateScrollBar();
  Invalidate(); 
}

////////////////////////////////////////////////////////////////////////////////

void
CTilesetEditView::SelectTile(int tile)
{
  m_CurrentTile = tile;
  UpdateImageView();
  UpdateScrollBar();
}

////////////////////////////////////////////////////////////////////////////////

void
CTilesetEditView::SP_ColorSelected(RGBA color)
{
  byte alpha = color.alpha;
  RGB  rgb   = { color.red, color.green, color.blue };

  m_ImageView.SetColor(color);
  m_ColorView.SetColor(rgb);
  m_AlphaView.SetAlpha(alpha);
}

////////////////////////////////////////////////////////////////////////////////

void
CTilesetEditView::UpdateImageView()
{
  sTile& tile = m_Tileset->GetTile(m_CurrentTile);
  m_ImageView.SetImage(tile.GetWidth(), tile.GetHeight(), tile.GetPixels(), true);
}

////////////////////////////////////////////////////////////////////////////////

void
CTilesetEditView::UpdateScrollBar()
{
  SetHScrollPosition(m_CurrentTile);
  SetHScrollRange(m_Tileset->GetNumTiles(), 1);
}

////////////////////////////////////////////////////////////////////////////////

void
CTilesetEditView::OnHScrollChanged(int x)
{
  m_CurrentTile = x;
  UpdateImageView();
  UpdateScrollBar();
  m_Handler->TEV_SelectedTileChanged(x);
}

////////////////////////////////////////////////////////////////////////////////

static inline void twMoveControl(CWnd& control, int x, int y, int w, int h)
{
  control.MoveWindow(x, y, w, h, FALSE);
  control.Invalidate();
}

afx_msg void
CTilesetEditView::OnSize(UINT type, int cx, int cy)
{
  const int PaletteWidth = 60;
  const int AlphaWidth = 32;

  if (m_Created) {
    
    twMoveControl(m_ImageView, 0, 0, cx - PaletteWidth - AlphaWidth, cy);
    twMoveControl(m_PaletteView, cx - PaletteWidth - AlphaWidth, 0, PaletteWidth, cy - PaletteWidth);
    twMoveControl(m_ColorView, cx - PaletteWidth - AlphaWidth, cy - PaletteWidth, PaletteWidth, PaletteWidth);
    twMoveControl(m_AlphaView, cx - AlphaWidth, 0, AlphaWidth, cy);

    UpdateScrollBar();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetEditView::OnTilesetInsertTiles()
{
  CNumberDialog NumberDialog("Insert Tiles", "How many tiles do you want to insert?", 1, 1, 256);
  if (NumberDialog.DoModal() == IDOK)
  {
    m_Tileset->InsertTiles(m_CurrentTile, NumberDialog.GetValue());
    UpdateScrollBar();
    UpdateImageView();
    
    m_Handler->TEV_TilesetModified();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetEditView::OnTilesetAppendTiles()
{
  CNumberDialog NumberDialog("Append Tiles", "How many tiles do you want to append?", 1, 1, 256);
  if (NumberDialog.DoModal() == IDOK)
  {
    m_Tileset->AppendTiles(NumberDialog.GetValue());
    UpdateScrollBar();
    UpdateImageView();
    m_Handler->TEV_TilesetModified();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetEditView::OnUpdateTilesetDeleteTile(CCmdUI* cmdui)
{
  if (m_Tileset->GetNumTiles() > 1)
    cmdui->Enable();
  else
    cmdui->Enable(FALSE);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetEditView::OnTilesetDeleteTile()
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

afx_msg void
CTilesetEditView::OnTilesetReplaceWithImage()
{
  if (MessageBox("Are you sure?", NULL, MB_YESNO) == IDNO)
    return;

  CImageFileDialog FileDialog(FDM_OPEN);
  if (FileDialog.DoModal() == IDOK)
  {
    if (!m_Tileset->Import_Image(FileDialog.GetPathName()))
    {
      MessageBox("Error: Could not import image");
      return;
    }

    m_Handler->TEV_TilesetModified();
    UpdateScrollBar();

    UpdateImageView();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetEditView::OnTilesetInsertImage()
{
  CImageFileDialog FileDialog(FDM_OPEN);
  if (FileDialog.DoModal() == IDOK)
  {
    if (!m_Tileset->InsertImage(m_CurrentTile, FileDialog.GetPathName()))
    {
      MessageBox("Error: Could not insert image");
      return;
    }

    m_Handler->TEV_TilesetModified();
    UpdateScrollBar();
    UpdateImageView();

    m_Handler->TEV_TilesetModified();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTilesetEditView::OnTilesetAppendImage()
{
  CImageFileDialog FileDialog(FDM_OPEN);
  if (FileDialog.DoModal() == IDOK)
  {
    if (!m_Tileset->AppendImage(FileDialog.GetPathName()))
    {
      MessageBox("Error: Could not append image");
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
CTilesetEditView::IV_ImageChanged()
{
  // store the old data
  memcpy(
    m_Tileset->GetTile(m_CurrentTile).GetPixels(),
    m_ImageView.GetPixels(),
    m_Tileset->GetTileWidth() * m_Tileset->GetTileHeight() * sizeof(RGBA));

  m_Handler->TEV_TileModified(m_CurrentTile);
}

////////////////////////////////////////////////////////////////////////////////

void
CTilesetEditView::IV_ColorChanged(RGBA color)
{
  RGBA rgba = m_ImageView.GetColor();
  RGB rgb = { rgba.red, rgba.green, rgba.blue };
  m_ColorView.SetColor(rgb);
  m_AlphaView.SetAlpha(rgba.alpha);
}

////////////////////////////////////////////////////////////////////////////////

void
CTilesetEditView::PV_ColorChanged(RGB color)
{
  m_ColorView.SetColor(color);
  byte alpha = m_AlphaView.GetAlpha();
  RGBA c = { color.red, color.green, color.blue, alpha };
  m_ImageView.SetColor(c);
}

////////////////////////////////////////////////////////////////////////////////

void
CTilesetEditView::CV_ColorChanged(RGB color)
{
  byte alpha = m_AlphaView.GetAlpha();
  RGBA rgba = { color.red, color.green, color.blue, alpha };
  m_ImageView.SetColor(rgba);
}

////////////////////////////////////////////////////////////////////////////////

void
CTilesetEditView::AV_AlphaChanged(byte alpha)
{
  RGBA rgba = m_ImageView.GetColor();
  rgba.alpha = alpha;
  m_ImageView.SetColor(rgba);
}

////////////////////////////////////////////////////////////////////////////////
