#include "PaletteView.hpp"
#include "DIBSection.hpp"
#include "../common/rgb.hpp"


//static int s_iPaletteViewID = 1000;

/*
BEGIN_MESSAGE_MAP(CPaletteView, CWnd)

  ON_WM_PAINT()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_MOUSEMOVE()
  ON_WM_SIZE()

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wPaletteView, wxWindow)

  EVT_SIZE(wPaletteView::OnSize)
  EVT_PAINT(wPaletteView::OnPaint)

  EVT_LEFT_DOWN(wPaletteView::OnLButtonDown)
  EVT_LEFT_UP(wPaletteView::OnLButtonUp)
  EVT_MOTION(wPaletteView::OnMouseMove)

END_EVENT_TABLE()



////////////////////////////////////////////////////////////////////////////////

wPaletteView::wPaletteView(wxWindow* pParentWindow, IPaletteViewHandler* pHandler)
: wxWindow(pParentWindow, -1)
, m_pPaletteDIB(NULL)
, m_Color(CreateRGB(0, 0, 0))
, m_bMouseDown(false)
{
  m_pHandler = pHandler;
}

////////////////////////////////////////////////////////////////////////////////

wPaletteView::~wPaletteView()
{
  delete m_pPaletteDIB;
  //DestroyWindow();
}

/*
////////////////////////////////////////////////////////////////////////////////

BOOL
CPaletteView::Create(IPaletteViewHandler* pHandler, CWnd* pParentWindow)
{
  m_pHandler = pHandler;

  return CWnd::Create(
    AfxRegisterWndClass(0, LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)), NULL, NULL),
    "PaletteView",
    WS_CHILD | WS_VISIBLE,
    CRect(0, 0, 0, 0),
    pParentWindow, s_iPaletteViewID++);
}
*/

////////////////////////////////////////////////////////////////////////////////

RGB
wPaletteView::GetColor() const
{
  return m_Color;
}

////////////////////////////////////////////////////////////////////////////////

void
wPaletteView::UpdatePalette(int w, int h, int pitch, RGB* pixels)
{
  // fill it with the rainbow
  static const int NumColorSections = 6;
  static const RGB Rainbow[NumColorSections] =
  {
    { 255, 0,   0,   }, // red
    { 255, 255, 0,   }, // yellow
    { 0,   255, 0,   }, // green
    { 0,   255, 255, }, // cyan
    { 0,   0,   255, }, // blue
    { 255, 0,   255, }, // magenta
  };

  // calculate section boundaries
  int SectionBoundaries[NumColorSections + 1];
  for (int i = 0; i <= NumColorSections; i++)
    SectionBoundaries[i] = i * h / NumColorSections;

  // fill the DIB section with colors
  for (int iy = 0; iy < h; iy++)
  {
    // figure out which section (and weight!) we're in
    int section = 0;
    int weight  = 0; // from 0 to 255
    for (int i = NumColorSections - 1; i >= 0; i--)
    {
      if (iy >= SectionBoundaries[i])
      {
        section = i;

        int height = (SectionBoundaries[i + 1] - SectionBoundaries[i]);
        weight  = 256 * (iy - SectionBoundaries[i]) / height;

        break;
      }
    }

    RGB color1 = Rainbow[(section + 0) % NumColorSections];
    RGB color2 = Rainbow[(section + 1) % NumColorSections];
    
    // calculate average color
    RGB rgb;
    rgb.red   = (color1.red   * (255 - weight) + color2.red   * weight) / 256;
    rgb.green = (color1.green * (255 - weight) + color2.green * weight) / 256;
    rgb.blue  = (color1.blue  * (255 - weight) + color2.blue  * weight) / 256;

    const RGB color = rgb;

    for (int ix = 0; ix < w; ix++)
    {
      rgb = color;

      // now calculate luminosity (0..127 = darker, 128..255 = brighter)
      int luminosity = ix * 256 / w;
      if (luminosity < 128)  // average color with black
      {
        int weight = (127 - luminosity) * 255 / 127;
        rgb.red   = (rgb.red   * (255 - weight) + 0 * weight) / 256;
        rgb.green = (rgb.green * (255 - weight) + 0 * weight) / 256;
        rgb.blue  = (rgb.blue  * (255 - weight) + 0 * weight) / 256;
      }
      else                   // average color with white
      {
        int weight = (luminosity - 128) * 255 / 127;
        rgb.red   = (rgb.red   * (255 - weight) + 255 * weight) / 256;
        rgb.green = (rgb.green * (255 - weight) + 255 * weight) / 256;
        rgb.blue  = (rgb.blue  * (255 - weight) + 255 * weight) / 256;
      }

      // draw pixels
      pixels[iy * pitch + ix] = rgb;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wPaletteView::OnPaint(wxPaintEvent &event)
{
  wxPaintDC dc(this);

  if (m_pPaletteDIB)
  {
    wxSize ClientSize(GetClientSize());
    wxBitmap bitmap = m_pPaletteDIB->GetImage()->ConvertToBitmap();
    dc.DrawBitmap(bitmap, 0, 0, FALSE);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wPaletteView::OnLButtonDown(wxMouseEvent &event)
{
  // bounds check
  wxSize ClientSize(GetClientSize());
  if (event.GetX() < 0 ||
      event.GetY() < 0 ||
      event.GetX() >= ClientSize.GetWidth() ||
      event.GetY() >= ClientSize.GetHeight())
    return;

  // get color
  RGB* pixels = (RGB*)m_pPaletteDIB->GetPixels();
  RGB color = pixels[event.GetY() * m_pPaletteDIB->GetWidth() + event.GetX()];

  m_Color.red   = color.red;
  m_Color.green = color.green;
  m_Color.blue  = color.blue;

  m_bMouseDown = true;
  CaptureMouse();

  // tell parent window that the color has changed
  m_pHandler->PV_ColorChanged(m_Color);
}

////////////////////////////////////////////////////////////////////////////////

void
wPaletteView::OnLButtonUp(wxMouseEvent &event)
{
  m_bMouseDown = false;
  ReleaseMouse();
}

////////////////////////////////////////////////////////////////////////////////

void
wPaletteView::OnMouseMove(wxMouseEvent &event)
{
  // bounds check
  if (!m_bMouseDown)
    return;

  wxSize ClientSize(GetClientSize());
  if (event.GetX() < 0 ||
      event.GetY() < 0 ||
      event.GetX() >= ClientSize.GetWidth() ||
      event.GetY() >= ClientSize.GetHeight())
    return;

  // get color
  RGB* pixels = (RGB*)m_pPaletteDIB->GetPixels();
  RGB color = pixels[event.GetY() * m_pPaletteDIB->GetWidth() + event.GetX()];

  m_Color.red   = color.red;
  m_Color.green = color.green;
  m_Color.blue  = color.blue;

  // tell parent window that the color has changed
  m_pHandler->PV_ColorChanged(m_Color);
}

////////////////////////////////////////////////////////////////////////////////

void
wPaletteView::OnSize(wxSizeEvent &event)
{
  int cx = event.GetSize().GetWidth();
  int cy = event.GetSize().GetHeight();
  // lock the cw and ch to boundaries to a multiple of cSize, but make sure they're bigger than cx and cy
  const int cSize = 80;
  int cw = (cx + cSize - 1) / cSize * cSize;
  int ch = (cy + cSize - 1) / cSize * cSize;

  // this stops invalid cw and ch values appearing
  if (cw < cSize)
    cw = cSize;
  if (ch < cSize)
    ch = cSize;

  if (m_pPaletteDIB == NULL ||
    cw != m_pPaletteDIB->GetWidth() ||
    ch != m_pPaletteDIB->GetHeight())
  {
    delete m_pPaletteDIB;
    m_pPaletteDIB = new wDIBSection(cw, ch);
  }

  RGB* pixels = (RGB*)m_pPaletteDIB->GetPixels();
  UpdatePalette(cx, cy, cw, pixels);

  Refresh();
}

////////////////////////////////////////////////////////////////////////////////
