#include "TileObstructionView.hpp"
#include "../common/primitives.hpp"

/*
BEGIN_MESSAGE_MAP(CTileObstructionView, CWnd)

  ON_WM_PAINT()
  ON_WM_SIZE()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_MOUSEMOVE()
  ON_WM_RBUTTONDOWN()

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wTileObstructionView, wxWindow)
  EVT_PAINT(wTileObstructionView::OnPaint)
  EVT_SIZE(wTileObstructionView::OnSize)

  EVT_LEFT_DOWN(wTileObstructionView::OnLButtonDown)
  EVT_LEFT_UP(wTileObstructionView::OnLButtonUp)
  EVT_RIGHT_DOWN(wTileObstructionView::OnRButtonDown)
  EVT_MOTION(wTileObstructionView::OnMouseMove)
END_EVENT_TABLE()


struct Sclipper {
  int left;
  int top;
  int right;
  int bottom;
};


////////////////////////////////////////////////////////////////////////////////

wTileObstructionView::wTileObstructionView(wxWindow* parent, sTile* tile)
: wxWindow(parent, -1)
, m_tile(NULL)
, m_pixels(NULL)
, m_mouse_down(false)
{
  m_tile = tile;

  m_pixels = new RGBA[tile->GetWidth() * tile->GetHeight()];
}

////////////////////////////////////////////////////////////////////////////////

wTileObstructionView::~wTileObstructionView()
{
  delete[] m_pixels;
}

/*
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
*/

////////////////////////////////////////////////////////////////////////////////

void
wTileObstructionView::OnPaint(wxPaintEvent& event)
{
  wxPaintDC dc(this);

  // draw tile and obstruction lines onto backbuffer tile
  RenderTile();

  // get window size
  wxSize size = GetClientSize();

  // draw the pixels
  for (int iy = 0; iy < m_tile->GetHeight(); iy++) {
    for (int ix = 0; ix < m_tile->GetWidth(); ix++) {
      
      RGBA color = m_pixels[iy * m_tile->GetWidth() + ix];

      // calculate pixel rect
      int x = size.GetWidth()  * ix       / m_tile->GetWidth();
      int y = size.GetHeight() * iy       / m_tile->GetHeight();
      int w = x - size.GetWidth()  * (ix + 1) / m_tile->GetWidth();
      int h = y - size.GetHeight() * (iy + 1) / m_tile->GetHeight();
      
      // draw the pixel
      dc.SetBrush(wxBrush(wxColour(color.red, color.green, color.blue), wxSOLID));
      dc.SetPen(wxPen(wxColour(color.red, color.green, color.blue), 1, wxSOLID));
      dc.DrawRectangle(x, y, w, h);

      dc.SetBrush(wxNullBrush);
      dc.SetPen(wxNullPen);
    }
  }
/*
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
*/
}

////////////////////////////////////////////////////////////////////////////////

void
wTileObstructionView::OnSize(wxSizeEvent &event)
{
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wTileObstructionView::OnLButtonDown(wxMouseEvent &event)
{
  m_mouse_down = true;
  m_start_point = WindowToTile(event.GetPosition());
  CaptureMouse();
}

////////////////////////////////////////////////////////////////////////////////

void
wTileObstructionView::OnLButtonUp(wxMouseEvent &event)
{
  wxPoint end_point = WindowToTile(event.GetPosition());

  m_tile->GetObstructionMap().AddSegment(
    m_start_point.x,
    m_start_point.y,
    end_point.x,
    end_point.y
  );

  m_mouse_down = false;
  if (HasCapture()) ReleaseMouse();

  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wTileObstructionView::OnMouseMove(wxMouseEvent &event)
{
  m_current_point = WindowToTile(event.GetPosition());
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wTileObstructionView::OnRButtonDown(wxMouseEvent &event)
{
  wxPoint point = WindowToTile(event.GetPosition());
  m_tile->GetObstructionMap().RemoveSegmentByPoint(point.x, point.y);
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

struct Color {
    RGBA operator()(int, int) {
        return CreateRGBA(255, 0, 255, 255);
    }
};

static inline void CopyRGBA(RGBA& dest, RGBA src) {
    dest = src;
}

void
wTileObstructionView::RenderTile()
{
  // draw the tile's pixels
  memcpy(
    m_pixels,
    m_tile->GetPixels(),
    m_tile->GetWidth() * m_tile->GetHeight() * sizeof(RGBA)
  );

  // draw the obstruction segments
  Color c;
  Sclipper clipper = { 0, 0, m_tile->GetWidth() - 1, m_tile->GetHeight() - 1 };
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
      CopyRGBA
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
      CopyRGBA
    );
  }
}

////////////////////////////////////////////////////////////////////////////////

wxPoint
wTileObstructionView::WindowToTile(wxPoint p)
{
  wxSize size = GetSize();

  p.x = p.x * m_tile->GetWidth()  / size.GetWidth();
  p.y = p.y * m_tile->GetHeight() / size.GetHeight();
  return p;
}

////////////////////////////////////////////////////////////////////////////////
