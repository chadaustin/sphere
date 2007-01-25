#include <algorithm>
#include "Editor.hpp"
#include "SpriteBaseView.hpp"

//static int s_ViewID = 450;


/*
BEGIN_MESSAGE_MAP(CSpriteBaseView, CWnd)

  ON_WM_SIZE()  
  ON_WM_PAINT()

  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_MOUSEMOVE()

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wSpriteBaseView, wxWindow)
  EVT_SIZE(wSpriteBaseView::OnSize)
  EVT_PAINT(wSpriteBaseView::OnPaint)

  EVT_LEFT_DOWN(wSpriteBaseView::OnLButtonDown)
  EVT_LEFT_UP(wSpriteBaseView::OnLButtonUp)
  EVT_MOTION(wSpriteBaseView::OnMouseMove)
END_EVENT_TABLE()


////////////////////////////////////////////////////////////////////////////////

wSpriteBaseView::wSpriteBaseView(wxWindow* parent, ISpriteBaseViewHandler* handler, sSpriteset* spriteset)
: wxWindow(parent, -1)
, m_Handler(NULL)
, m_Spriteset(NULL)
, m_Sprite(NULL)
, m_MouseDown(false)
{
  m_Handler = handler;
  m_Spriteset = spriteset;
}

////////////////////////////////////////////////////////////////////////////////

wSpriteBaseView::~wSpriteBaseView()
{
  //DestroyWindow();
}

/*
////////////////////////////////////////////////////////////////////////////////

BOOL
CSpriteBaseView::Create(CWnd* parent, ISpriteBaseViewHandler* handler, sSpriteset* spriteset)
{
  m_Handler = handler;
  m_Spriteset = spriteset;

  return CWnd::Create(
    AfxRegisterWndClass(0, LoadCursor(NULL, IDC_ARROW), NULL, NULL),
    "",
    WS_CHILD | WS_VISIBLE,
    CRect(0, 0, 0, 0),
    parent,
    s_ViewID++);
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wSpriteBaseView::SetSprite(const CImage32* sprite)
{
  m_Sprite = sprite;
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpriteBaseView::BeginDrag(int x, int y)
{
  WindowToImage(&x, &y);

  m_Spriteset->SetBase(x, y, x, y);
  m_Handler->SBV_SpritesetModified();
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpriteBaseView::Drag(int x, int y)
{
  WindowToImage(&x, &y);

  int x1, y1, x2, y2;
  m_Spriteset->GetBase(x1, y1, x2, y2);

  m_Spriteset->SetBase(x1, y1, x, y);
  m_Handler->SBV_SpritesetModified();
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpriteBaseView::WindowToImage(int* x, int* y)
{
  // get client rectangle
  wxSize ClientSize = GetClientSize();

  // calculate size of pixel squares
  int hsize = ClientSize.GetWidth() / m_Sprite->GetWidth();
  int vsize = ClientSize.GetHeight() / m_Sprite->GetHeight();
  int size = std::min(hsize, vsize);
  if (size < 1)
    size = 1;

  int totalx = size * m_Sprite->GetWidth();
  int totaly = size * m_Sprite->GetHeight();
  int offsetx = (ClientSize.GetWidth() - totalx) / 2;
  int offsety = (ClientSize.GetHeight() - totaly) / 2;

  *x -= offsetx;
  *y -= offsety;

  *x /= size;
  *y /= size;

  // bounds check
  if (*x < 0)
    *x = 0;
  if (*y < 0)
    *y = 0;
  if (*x > m_Sprite->GetWidth() - 1)
    *x = m_Sprite->GetWidth() - 1;
  if (*y > m_Sprite->GetHeight() - 1)
    *y = m_Sprite->GetHeight() - 1;
}

////////////////////////////////////////////////////////////////////////////////

void
wSpriteBaseView::OnSize(wxSizeEvent &event)
{
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

// inclusive
inline bool between(int i, int min, int max)
{
  return ((i >= min && i <= max) ||
          (i <= min && i >= max));
}

////////////////////////////////////////////////////////////////////////////////

void
wSpriteBaseView::OnPaint(wxPaintEvent &event)
{
  wxPaintDC dc(this);

  int width  = m_Sprite->GetWidth();
  int height = m_Sprite->GetHeight();
  const RGBA* pixels = m_Sprite->GetPixels();

  // get client rectangle
  wxSize ClientSize = GetClientSize();

  // calculate size of pixel squares
  int hsize = ClientSize.GetWidth() / width;
  int vsize = ClientSize.GetHeight() / height;
  int size = std::min(hsize, vsize);
  if (size < 1)
    size = 1;

  int totalx = size * width;
  int totaly = size * height;
  int offsetx = (ClientSize.GetWidth() - totalx) / 2;
  int offsety = (ClientSize.GetHeight() - totaly) / 2;

  // draw black rectangles in the empty parts
  dc.SetBrush(*wxBLACK_BRUSH);
  dc.SetPen(*wxBLACK_PEN);

  // top
  dc.DrawRectangle(0, 0, ClientSize.GetWidth(), offsety);

  // bottom
  dc.DrawRectangle(0, offsety + totaly, ClientSize.GetWidth(), ClientSize.GetHeight() - totaly - offsety);

  // left
  dc.DrawRectangle(0, offsety, offsetx, totaly);

  // right
  dc.DrawRectangle(offsetx + totalx, offsety, ClientSize.GetWidth() - totalx - offsetx, totaly);

  dc.SetBrush(wxNullBrush);
  dc.SetPen(wxNullPen);

  // get the base rectangle
  int base_x1;
  int base_y1;
  int base_x2;
  int base_y2;
  m_Spriteset->GetBase(base_x1, base_y1, base_x2, base_y2);

  // draw the image
  for (int ix = 0; ix < width; ix++)
    for (int iy = 0; iy < height; iy++)
    {
      RGBA color = pixels[iy * width + ix];

      bool onbaseborder = false;
      if (iy == base_y1 || iy == base_y2)
        if (between(ix, base_x1, base_x2))
          onbaseborder = true;
      if (ix == base_x1 || ix == base_x2)
        if (between(iy, base_y1, base_y2))
          onbaseborder = true;

      // opaque
      if (color.alpha == 255 || onbaseborder)
      {
        wxColour color1;
        if (onbaseborder) {
          color1 = wxColour(0xff, 0x00, 0xff);
        } else {
          color1 = wxColour(color.red, color.green, color.blue);
        }
        dc.SetBrush(wxBrush(color1, wxSOLID));
        dc.SetPen(wxPen(color1, 1, wxSOLID));

        dc.DrawRectangle(offsetx + ix * size, offsety + iy * size, size, size);

        dc.SetBrush(wxNullBrush);
        dc.SetPen(wxNullPen);
      }
      // translucent
      else
      {
        // calculate background grid colors
        RGB Color1 = CreateRGB(255, 255, 255);
        RGB Color2 = CreateRGB(128, 128, 128);

        Color1.red   = (color.red   * color.alpha + Color1.red   * (256 - color.alpha)) / 256;
        Color1.green = (color.green * color.alpha + Color1.green * (256 - color.alpha)) / 256;
        Color1.blue  = (color.blue  * color.alpha + Color1.blue  * (256 - color.alpha)) / 256;

        Color2.red   = (color.red   * color.alpha + Color2.red   * (256 - color.alpha)) / 256;
        Color2.green = (color.green * color.alpha + Color2.green * (256 - color.alpha)) / 256;
        Color2.blue  = (color.blue  * color.alpha + Color2.blue  * (256 - color.alpha)) / 256;

        wxColour color1(Color1.red, Color1.green, Color1.blue);
        wxColour color2(Color2.red, Color2.green, Color2.blue);

        // draw rectangles


        dc.SetBrush(wxBrush(color1, wxSOLID));
        dc.SetPen(wxPen(color1, 1, wxSOLID));

        // upper left
        dc.DrawRectangle(offsetx + ix * size, offsety + iy * size, size / 2, size / 2);

        // lower right
        dc.DrawRectangle(offsetx + ix * size + size / 2, offsety + iy * size + size / 2, (size + 1) / 2, (size + 1) / 2);

       

        dc.SetBrush(wxBrush(color2, wxSOLID));
        dc.SetPen(wxPen(color2, 1, wxSOLID));

        // upper right
        dc.DrawRectangle(offsetx + ix * size + size / 2, offsety + iy * size, (size + 1) / 2, size / 2);

        // lower left
        dc.DrawRectangle(offsetx + ix * size, offsety + iy * size + size / 2, size / 2, (size + 1) / 2);

        dc.SetBrush(wxNullBrush);
        dc.SetPen(wxNullPen);
      }

    }

  // draw a white rectangle around the image
  // draw a white rectangle around the image
  dc.SetBrush(*wxTRANSPARENT_BRUSH);
  dc.SetPen(*wxWHITE_PEN);

  dc.DrawRectangle(offsetx - 1, offsety - 1, totalx + 2, totaly + 2);

  dc.SetBrush(wxNullBrush);
  dc.SetPen(wxNullPen);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpriteBaseView::OnLButtonDown(wxMouseEvent &event)
{
  BeginDrag(event.GetX(), event.GetY());
  m_MouseDown = true;
  CaptureMouse();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpriteBaseView::OnLButtonUp(wxMouseEvent &event)
{
  Drag(event.GetX(), event.GetY());
  m_MouseDown = false;
  if (HasCapture()) ReleaseMouse();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpriteBaseView::OnMouseMove(wxMouseEvent &event)
{
  if (!m_MouseDown)
    return;

  Drag(event.GetX(), event.GetY());
}

////////////////////////////////////////////////////////////////////////////////
