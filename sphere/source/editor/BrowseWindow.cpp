// identifier too long
#pragma warning(disable : 4786)


#include "BrowseWindow.hpp"
#include "../common/minmax.hpp"
#include "Editor.hpp"
#include "resource.h"

#include "../common/Spriteset.hpp"
#include "../common/WindowStyle.hpp"
#include "../common/Map.hpp"

#ifdef USE_SIZECBAR
IMPLEMENT_DYNAMIC(CBrowseWindow, CDocumentWindow)
#endif

BEGIN_MESSAGE_MAP(CBrowseWindow, CDocumentWindow)

  ON_WM_SIZE()
  ON_WM_KEYDOWN()
  ON_WM_PAINT()
  ON_WM_MOUSEMOVE()

  ON_WM_LBUTTONDOWN()

  ON_COMMAND(ID_BROWSE_REFRESH, OnBrowseListRefresh)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CBrowseWindow::CBrowseWindow(const char* folder, const char* filter)
: CDocumentWindow(folder, IDR_BROWSE, CSize(400, 100))
, m_SelectedImage(0)
, m_BlitTile(NULL)
, m_ZoomFactor(1)
, m_TopRow(0)
, m_Created(false)
{
  m_Folder = folder;
  m_Filter = filter;

  if (!LoadImages(folder, filter)) {
    delete this;
    return;
  }

  m_BlitTile = new CDIBSection(
    100 * m_ZoomFactor,
    100 * m_ZoomFactor,
    32
  );

  Create();
}

////////////////////////////////////////////////////////////////////////////////

CBrowseWindow::~CBrowseWindow()
{
  // destroy all the child windows
  Destroy();
  delete m_BlitTile;
}

////////////////////////////////////////////////////////////////////////////////

void
CBrowseWindow::Create()
{
  // create the window
  CDocumentWindow::Create(AfxRegisterWndClass(0, NULL, NULL, AfxGetApp()->LoadIcon(IDI_BROWSE)));

  m_Created = true;  // the window and children are ready!

  // move things to their rightful places
  RECT ClientRect;
  GetClientRect(&ClientRect);
  OnSize(0, ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top);
}

////////////////////////////////////////////////////////////////////////////////

void
CBrowseWindow::Destroy()
{
  for (int i = 0; i < m_BrowseList.size(); i++)
    delete m_BrowseList[i];
  m_BrowseList.clear();
}

////////////////////////////////////////////////////////////////////////////////

#include "../wxeditor/system.cpp"

bool
CBrowseWindow::LoadImages(const char* szFolder, const char* szFilter)
{
  for (int i = 0; i < m_BrowseList.size(); i++)
    delete m_BrowseList[i];
  m_BrowseList.clear();

  std::vector<std::string> file_list = GetFileList(szFilter);

  for (int i = 0; i < file_list.size(); i++) {
    bool valid = false;

    CImage32 image;
    sSpriteset spriteset;
    sWindowStyle windowstyle;
    sMap map;

    if (image.Load(file_list[i].c_str())) {
      valid = true;
    }
    else if (spriteset.Load(file_list[i].c_str())) {
      image = spriteset.GetImage(0);
      valid = true;
    }
    else if (windowstyle.Load(file_list[i].c_str())) {
      CImage32 tl = windowstyle.GetBitmap(sWindowStyle::UPPER_LEFT);
      CImage32 tm = windowstyle.GetBitmap(sWindowStyle::TOP);
      CImage32 tr = windowstyle.GetBitmap(sWindowStyle::UPPER_RIGHT);
      CImage32 ml = windowstyle.GetBitmap(sWindowStyle::LEFT);
      CImage32 mm = windowstyle.GetBitmap(sWindowStyle::BACKGROUND);
      CImage32 mr = windowstyle.GetBitmap(sWindowStyle::RIGHT);
      CImage32 bl = windowstyle.GetBitmap(sWindowStyle::LOWER_LEFT);
      CImage32 bm = windowstyle.GetBitmap(sWindowStyle::BOTTOM);
      CImage32 br = windowstyle.GetBitmap(sWindowStyle::LOWER_RIGHT);

      tl.Rescale(100/3, 100/3);
      tm.Rescale(100/3, 100/3);
      tr.Rescale(100/3, 100/3);
      ml.Rescale(100/3, 100/3);
      mm.Rescale(100/3, 100/3);
      mr.Rescale(100/3, 100/3);
      bl.Rescale(100/3, 100/3);
      bm.Rescale(100/3, 100/3);
      br.Rescale(100/3, 100/3);

      image.Create(100, 100);

      image.BlitImage(tl, 100/3*0, 100/3*0);
      image.BlitImage(tm, 100/3*1, 100/3*0);
      image.BlitImage(tr, 100/3*2, 100/3*0);
      image.BlitImage(ml, 100/3*0, 100/3*1);
      image.BlitImage(mm, 100/3*1, 100/3*1);
      image.BlitImage(mr, 100/3*2, 100/3*1);
      image.BlitImage(bl, 100/3*0, 100/3*2);
      image.BlitImage(bm, 100/3*1, 100/3*2);
      image.BlitImage(br, 100/3*2, 100/3*2);

      valid = true;
    }
    else if (map.Load(file_list[i].c_str()) && map.GetNumLayers() > 0 && map.GetTileset().GetNumTiles() > 0) {
      int map_width = 0;
      int map_height = 0;
  
      for (int i = 0; i < map.GetNumLayers(); ++i) {
        map_width  = std::max(map_width, map.GetLayer(i).GetWidth());
        map_height = std::max(map_height, map.GetLayer(i).GetHeight());
      }

      map_width  = std::min(100, map_width);
      map_height = std::min(100, map_height);

      int xstep = 100 / map_width;
      int ystep = 100 / map_height;

      map.GetTileset().SetTileSize(xstep, ystep, true);
      image.Create(map_width, map_height);

      for (int layer = 0; layer < map.GetNumLayers(); ++layer) {
        int layer_width  = map.GetLayer(layer).GetWidth();
        int layer_height = map.GetLayer(layer).GetHeight();

        for (int y = 0; y < layer_height; y += ystep) {
          for (int x = 0; x < layer_width; x += xstep) {
            image.BlitImage(map.GetTileset().GetTile(map.GetLayer(layer).GetTile(x, y)), x, y);
          }
        }
      }

      valid = true;
    }

    if (valid) {
      image.Rescale(100, 100);
      if (image.GetWidth() != 100 && image.GetHeight() != 100) {
        valid = false;
      }
   }

   if (valid) {
     CBrowseInfo* b = new CBrowseInfo();
     if (b) {
       b->filename = file_list[i];
       b->image = image;
       m_BrowseList.push_back(b);
     }
   }
  }


  return true;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CBrowseWindow::OnSize(UINT uType, int cx, int cy)
{
  CDocumentWindow::OnSize(uType, cx, cy);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CBrowseWindow::OnKeyDown(UINT vk, UINT repeat, UINT flags)
{
  std::vector<std::string> m_ImageList;

  int image = 0;
  if (vk == VK_RIGHT) {
    image = std::min(image + 1, (int)m_ImageList.size() - 1);
  } else if (vk == VK_LEFT) {
    image = std::max(image - 1, 0);
  }
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CBrowseWindow::OnPaint()
{
  CPaintDC dc(this);
  
  RECT client_rect;
  GetClientRect(&client_rect);

  int blit_width  = m_BlitTile->GetWidth();
  int blit_height = m_BlitTile->GetHeight();

  for (int iy = 0; iy < client_rect.bottom / blit_height + 1; iy++)
    for (int ix = 0; ix < client_rect.right / blit_width + 1; ix++)
    {
      RECT Rect = {
        ix       * blit_width,
        iy       * blit_height,
        (ix + 1) * blit_width,
        (iy + 1) * blit_height,
      };
      if (dc.RectVisible(&Rect) == FALSE)
        continue;
      
      int num_tiles_x = client_rect.right / blit_width;

      int it = (iy + m_TopRow) * (client_rect.right / blit_width) + ix;
      if (ix < num_tiles_x && (it >= 0 && it < m_BrowseList.size()))
      {
        // draw the tile
        // fill the DIB section
        BGRA* pixels = (BGRA*)m_BlitTile->GetPixels();
        
        // make a checkerboard
        for (int iy = 0; iy < blit_height; iy++)
          for (int ix = 0; ix < blit_width; ix++)
          {
            pixels[iy * blit_width + ix] = 
              ((ix / 8 + iy / 8) % 2 ?
                CreateBGRA(255, 255, 255, 255) :
                CreateBGRA(255, 192, 192, 255)
              );
          }        

        // draw the tile into it
        const RGBA* tilepixels = m_BrowseList[it]->GetPixels();
        for (int iy = 0; iy < blit_height; iy++)
          for (int ix = 0; ix < blit_width; ix++)
          {
            int ty = iy / m_ZoomFactor;
            int tx = ix / m_ZoomFactor;
            int t = ty * m_BrowseList[it]->GetWidth() + tx;
            
            int d = iy * blit_width + ix;

            int alpha = tilepixels[t].alpha;
            pixels[d].red   = (tilepixels[t].red   * alpha + pixels[d].red   * (255 - alpha)) / 256;
            pixels[d].green = (tilepixels[t].green * alpha + pixels[d].green * (255 - alpha)) / 256;
            pixels[d].blue  = (tilepixels[t].blue  * alpha + pixels[d].blue  * (255 - alpha)) / 256;
          }
        
        // blit the tile
        CDC* tile = CDC::FromHandle(m_BlitTile->GetDC());
        dc.BitBlt(Rect.left, Rect.top, Rect.right - Rect.left, Rect.bottom - Rect.top, tile, 0, 0, SRCCOPY);

        // if the tile is selected, draw a pink rectangle around it
        if (it == m_SelectedImage)
        {
          HBRUSH newbrush = (HBRUSH)GetStockObject(NULL_BRUSH);
          CBrush* oldbrush = dc.SelectObject(CBrush::FromHandle(newbrush));
          HPEN newpen = (HPEN)CreatePen(PS_SOLID, 1, RGB(0xFF, 0x00, 0xFF));
          CPen* oldpen = dc.SelectObject(CPen::FromHandle(newpen));

          dc.Rectangle(&Rect);

          dc.SelectObject(oldbrush);
          DeleteObject(newbrush);
          dc.SelectObject(oldpen);
          DeleteObject(newpen);
        }

      }
      else
      {
        // draw black rectangle
        dc.FillRect(&Rect, CBrush::FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH)));
      }      
    }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CBrowseWindow::OnLButtonDown(UINT flags, CPoint point)
{
  //if (m_MenuShown)
  //  return;

  RECT client_rect;
  GetClientRect(&client_rect);
  int num_tiles_x = client_rect.right / m_BlitTile->GetWidth();

  int col = point.x / m_BlitTile->GetWidth();
  int row = point.y / m_BlitTile->GetHeight();

  // don't let user select tile off the right edge (and go to the next row)
  if (col >= num_tiles_x) {
    return;
  }

  int tile = (m_TopRow + row) * num_tiles_x + col;

  if (tile >= 0 && tile < m_BrowseList.size())
    m_SelectedImage = tile;

  Invalidate();
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CBrowseWindow::OnMouseMove(UINT flags, CPoint point)
{
  RECT client_rect;
  GetClientRect(&client_rect);
  int num_tiles_x = client_rect.right / m_BlitTile->GetWidth();

  int x = point.x / (m_BlitTile->GetWidth()  * m_ZoomFactor);
  int y = point.y / (m_BlitTile->GetHeight() * m_ZoomFactor);

  int tile = (m_TopRow + y) * num_tiles_x + x;

  if (tile >= 0 && tile <= m_BrowseList.size() - 1)
  {
    CString browse_info;
    browse_info.Format("%s", m_BrowseList[tile]->filename.c_str());
    GetStatusBar()->SetWindowText(browse_info);
  }
  else
    GetStatusBar()->SetWindowText("");

}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CBrowseWindow::OnBrowseListRefresh() {
  LoadImages(m_Folder.c_str(), m_Filter.c_str());
  Invalidate();
}

///////////////////////////////////////////////////////////////////////////////