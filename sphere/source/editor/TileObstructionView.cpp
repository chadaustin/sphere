#include "TileObstructionView.hpp"
#include "../common/primitives.hpp"


BEGIN_MESSAGE_MAP(CTileObstructionView, CWnd)

  ON_WM_PAINT()
  ON_WM_SIZE()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_MOUSEMOVE()
  ON_WM_RBUTTONDOWN()

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CTileObstructionView::CTileObstructionView()
: m_tile(NULL)
, m_pixels(NULL)
, m_mouse_down(false)
{
}

////////////////////////////////////////////////////////////////////////////////

CTileObstructionView::~CTileObstructionView()
{
  delete[] m_pixels;
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CTileObstructionView::Create(CWnd* parent, sTile* tile)
{
  m_tile = tile;

  m_pixels = new RGBA[tile->GetWidth() * tile->GetHeight()];
  
  return CWnd::Create(
    AfxRegisterWndClass(0, LoadCursor(NULL, IDC_ARROW), NULL, NULL),
    "tile obstruction view",
    WS_CHILD | WS_VISIBLE,
    CRect(0, 0, 0, 0),
    parent,
    0
  );
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTileObstructionView::OnPaint()
{
  CPaintDC dc_(this);
  HDC dc = dc_.m_hDC;

  // draw tile and obstruction lines onto backbuffer tile
  RenderTile();

  // get window size
  RECT rect;
  GetClientRect(&rect);

  // draw the pixels
  for (int iy = 0; iy < m_tile->GetHeight(); iy++) {
    for (int ix = 0; ix < m_tile->GetWidth(); ix++) {
      
      RGBA color = m_pixels[iy * m_tile->GetWidth() + ix];

      // calculate pixel rect
      RECT pixel_rect;
      pixel_rect.left   = rect.right  * ix       / m_tile->GetWidth();
      pixel_rect.top    = rect.bottom * iy       / m_tile->GetHeight();
      pixel_rect.right  = rect.right  * (ix + 1) / m_tile->GetWidth();
      pixel_rect.bottom = rect.bottom * (iy + 1) / m_tile->GetHeight();
      
      // draw the pixel
      HBRUSH brush = CreateSolidBrush(RGB(color.red, color.green, color.blue));
      FillRect(dc, &pixel_rect, brush);
      DeleteObject(brush);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTileObstructionView::OnSize(UINT type, int cx, int cy)
{
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTileObstructionView::OnLButtonDown(UINT flags, CPoint point)
{
  m_mouse_down = true;
  m_start_point = WindowToTile(point);
  SetCapture();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTileObstructionView::OnLButtonUp(UINT flags, CPoint point)
{
  CPoint end_point = WindowToTile(point);

  m_tile->GetObstructionMap().AddSegment(
    m_start_point.x,
    m_start_point.y,
    end_point.x,
    end_point.y
  );

  m_mouse_down = false;
  ReleaseCapture();

  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTileObstructionView::OnMouseMove(UINT flags, CPoint point)
{
  m_current_point = WindowToTile(point);
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CTileObstructionView::OnRButtonDown(UINT flags, CPoint point)
{
  point = WindowToTile(point);
  m_tile->GetObstructionMap().RemoveSegmentByPoint(point.x, point.y);
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void
CTileObstructionView::RenderTile()
{
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


  // draw the tile's pixels
  memcpy(
    m_pixels,
    m_tile->GetPixels(),
    m_tile->GetWidth() * m_tile->GetHeight() * sizeof(RGBA)
  );

  // draw the obstruction segments
  Local::Color c;
  RECT clipper = { 0, 0, m_tile->GetWidth() - 1, m_tile->GetHeight() - 1 };
  sObstructionMap& obs_map = m_tile->GetObstructionMap();
  for (int i = 0; i < obs_map.GetNumSegments(); i++) {
    
    const sObstructionMap::Segment& s = obs_map.GetSegment(i);

    primitives::Line(
      m_pixels,
      m_tile->GetWidth(),
      s.x1,
      s.y1,
      s.x2,
      s.y2,
      c,
      clipper,
      Local::CopyRGBA
    );
  }

  // draw current line
  if (m_mouse_down) {
    primitives::Line(
      m_pixels,
      m_tile->GetWidth(),
      m_start_point.x,
      m_start_point.y,
      m_current_point.x,
      m_current_point.y,
      c,
      clipper,
      Local::CopyRGBA
    );
  }
}

////////////////////////////////////////////////////////////////////////////////

CPoint
CTileObstructionView::WindowToTile(CPoint p)
{
  RECT rect;
  GetClientRect(&rect);

  p.x = p.x * m_tile->GetWidth()  / rect.right;
  p.y = p.y * m_tile->GetHeight() / rect.bottom;
  return p;
}

////////////////////////////////////////////////////////////////////////////////
