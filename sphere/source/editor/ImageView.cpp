#pragma warning(disable : 4786)

#include <stack>
#include "ImageView.hpp"
#include "Editor.hpp"
#include "../common/Filters.hpp"
#include "../common/minmax.hpp"
#include "resource.h"

#include "NumberDialog.hpp"

static int s_ImageViewID = 9000;

static UINT s_ClipboardFormat;


BEGIN_MESSAGE_MAP(CImageView, CWnd)

  ON_WM_PAINT()
  ON_WM_SIZE()

  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_RBUTTONUP()
  ON_WM_MOUSEMOVE()

  ON_COMMAND(ID_IMAGEVIEW_COLORPICKER,           OnColorPicker)
  ON_COMMAND(ID_IMAGEVIEW_UNDO,                  OnUndo)
  ON_COMMAND(ID_IMAGEVIEW_COPY,                  OnCopy)
  ON_COMMAND(ID_IMAGEVIEW_PASTE,                 OnPaste)
  ON_COMMAND(ID_IMAGEVIEW_PASTE_RGB,             OnPasteRGB)
  ON_COMMAND(ID_IMAGEVIEW_PASTE_ALPHA,           OnPasteAlpha)
  ON_COMMAND(ID_IMAGEVIEW_PASTE_INTOSELECTION,       OnPasteIntoSelection)
  ON_COMMAND(ID_IMAGEVIEW_VIEWGRID,              OnViewGrid)
  ON_COMMAND(ID_IMAGEVIEW_BLENDMODE_BLEND,       OnBlendModeBlend)
  ON_COMMAND(ID_IMAGEVIEW_BLENDMODE_REPLACE,     OnBlendModeReplace)
  ON_COMMAND(ID_IMAGEVIEW_BLENDMODE_RGBONLY,     OnBlendModeRGBOnly)
  ON_COMMAND(ID_IMAGEVIEW_BLENDMODE_ALPHAONLY,   OnBlendModeAlphaOnly)
  ON_COMMAND(ID_IMAGEVIEW_ROTATE_CW,             OnRotateCW)
  ON_COMMAND(ID_IMAGEVIEW_ROTATE_CCW,            OnRotateCCW)
  ON_COMMAND(ID_IMAGEVIEW_SLIDE_UP,              OnSlideUp)
  ON_COMMAND(ID_IMAGEVIEW_SLIDE_RIGHT,           OnSlideRight)
  ON_COMMAND(ID_IMAGEVIEW_SLIDE_DOWN,            OnSlideDown)
  ON_COMMAND(ID_IMAGEVIEW_SLIDE_LEFT,            OnSlideLeft)
  ON_COMMAND(ID_IMAGEVIEW_SLIDE_OTHER,           OnSlideOther)
  ON_COMMAND(ID_IMAGEVIEW_FLIP_HORIZONTALLY,     OnFlipHorizontally)
  ON_COMMAND(ID_IMAGEVIEW_FLIP_VERTICALLY,       OnFlipVertically)
  ON_COMMAND(ID_IMAGEVIEW_FILL_RGB,              OnFillRGB)
  ON_COMMAND(ID_IMAGEVIEW_FILL_ALPHA,            OnFillAlpha)
  ON_COMMAND(ID_IMAGEVIEW_FILL_BOTH,             OnFillBoth)
  ON_COMMAND(ID_IMAGEVIEW_REPLACE_RGBA,          OnReplaceRGBA)
  ON_COMMAND(ID_IMAGEVIEW_REPLACE_RGB,           OnReplaceRGB)
  ON_COMMAND(ID_IMAGEVIEW_REPLACE_ALPHA,         OnReplaceAlpha)
  ON_COMMAND(ID_IMAGEVIEW_FILTER_BLUR,           OnFilterBlur)
  ON_COMMAND(ID_IMAGEVIEW_FILTER_NOISE,          OnFilterNoise)
  ON_COMMAND(ID_IMAGEVIEW_FILTER_ADJUST_BRIGHTNESS, OnFilterAdjustBrightness)
  ON_COMMAND(ID_IMAGEVIEW_FILTER_NEGATIVE_IMAGE_RGB, OnFilterNegativeImageRGB)
  ON_COMMAND(ID_IMAGEVIEW_FILTER_NEGATIVE_IMAGE_ALPHA, OnFilterNegativeImageAlpha)
  ON_COMMAND(ID_IMAGEVIEW_FILTER_NEGATIVE_IMAGE_RGBA, OnFilterNegativeImageRGBA)
  ON_COMMAND(ID_IMAGEVIEW_FILTER_SOLARIZE,       OnFilterSolarize)
  ON_COMMAND(ID_IMAGEVIEW_SETCOLORALPHA,         OnSetColorAlpha)
  ON_COMMAND(ID_IMAGEVIEW_SCALEALPHA,            OnScaleAlpha)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CImageView::CImageView()
: m_Color(CreateRGBA(0, 0, 0, 255))
, m_SwatchPalette(NULL)
, m_ToolPalette(NULL)
, m_MouseDown(false)
, m_CurrentTool(Tool_Pencil)
, m_NumUndoImages(0)
, m_UndoImages(NULL)
, m_SelectionX(0)
, m_SelectionY(0)
, m_SelectionWidth(0)
, m_SelectionHeight(0)
, m_ShowGrid(false)
{
  m_Image.SetBlendMode(CImage32::REPLACE);
  m_ShowGrid = false;
  s_ClipboardFormat = RegisterClipboardFormat("FlatImage32");
}

////////////////////////////////////////////////////////////////////////////////

CImageView::~CImageView()
{
  if (m_SwatchPalette) {
    m_SwatchPalette->Destroy();
  }

  if (m_ToolPalette) {
    m_ToolPalette->Destroy();
  }

  DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CImageView::Create(CDocumentWindow* owner, IImageViewHandler* handler, CWnd* parent_window)
{
  m_Handler = handler;

  m_SwatchPalette = new CSwatchPalette(owner, this);
  m_ToolPalette   = new CImageToolPalette(owner, this);

  return CWnd::Create(
    AfxRegisterWndClass(0, LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)), NULL, LoadIcon(NULL, IDI_APPLICATION)),
    "ImageView",
    WS_CHILD | WS_VISIBLE,
    CRect(0, 0, 0, 0),
    parent_window,
    s_ImageViewID++);
}

////////////////////////////////////////////////////////////////////////////////

bool
CImageView::SetImage(int width, int height, const RGBA* pixels)
{
  ResetUndoStates();

  m_Image.Resize(width, height);
  memcpy(m_Image.GetPixels(), pixels, width * height * sizeof(RGBA));

  Invalidate();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

int
CImageView::GetWidth() const
{
  return m_Image.GetWidth();
}

////////////////////////////////////////////////////////////////////////////////

int
CImageView::GetHeight() const
{
  return m_Image.GetHeight();
}

////////////////////////////////////////////////////////////////////////////////

RGBA*
CImageView::GetPixels()
{
  return m_Image.GetPixels();
}

////////////////////////////////////////////////////////////////////////////////

const RGBA*
CImageView::GetPixels() const
{
  return m_Image.GetPixels();
}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::SetColor(RGBA color)
{
  m_Color = color;
  m_SwatchPalette->SetColor(color);
}

////////////////////////////////////////////////////////////////////////////////

RGBA
CImageView::GetColor() const
{
  return m_Color;
}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::FillRGB()
{
  AddUndoState();

  for (int y = 0; y < m_Image.GetHeight(); y++)
    for (int x = 0; x < m_Image.GetWidth(); x++)
    {
      RGBA tColor = m_Image.GetPixel(x, y);
      tColor.red   = m_Color.red;
      tColor.green = m_Color.green;
      tColor.blue  = m_Color.blue;

      m_Image.SetPixel(x, y, tColor);
    }

  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::FillAlpha()
{
  AddUndoState();

  for (int y = 0; y < m_Image.GetHeight(); y++)
    for (int x = 0; x < m_Image.GetWidth(); x++)
    {
      RGBA tColor = m_Image.GetPixel(x, y);
      tColor.alpha = m_Color.alpha;

      m_Image.SetPixel(x, y, tColor);
    }

  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

bool
CImageView::Copy()
{
  if (OpenClipboard() == FALSE)
    return false;

  int width = m_Image.GetWidth();
  int height = m_Image.GetHeight();

  // clear the previous contents of the clipboard
  EmptyClipboard();

  int sx, sy, sw, sh;
  GetSelectionArea(sx, sy, sw, sh);
  RGBA* source = m_Image.GetPixels();

  // ADD FLAT 32

  // copy the image as a flat 32-bit color image
  HGLOBAL memory = GlobalAlloc(GHND, 8 + width * height * 4);
  dword* ptr = (dword*)GlobalLock(memory);

  *ptr++ = sw; // *ptr++ = width;
  *ptr++ = sh; // *ptr++ = height;

  RGBA* flat_pixels = new RGBA[sw * sh];
  if (flat_pixels == NULL)
    return false;

  for (int iy = sy; iy < (sy + sh); iy++)
    for (int ix = sx; ix < (sx + sw); ix++)
    {
      flat_pixels[(iy - sy) * (sw) + ix - sx].red   = source[iy * width + ix].red;
      flat_pixels[(iy - sy) * (sw) + ix - sx].green = source[iy * width + ix].green;
      flat_pixels[(iy - sy) * (sw) + ix - sx].blue  = source[iy * width + ix].blue;
      flat_pixels[(iy - sy) * (sw) + ix - sx].alpha = source[iy * width + ix].alpha;
    }

  // memcpy(ptr, m_Image.GetPixels(), width * height * sizeof(RGBA));
  memcpy(ptr, flat_pixels, sw * sh * sizeof(RGBA));
  delete[] flat_pixels;

  // put the image on the clipboard
  GlobalUnlock(memory);
  SetClipboardData(s_ClipboardFormat, memory);

  // ADD DDB
  // create a pixel array to initialize the bitmap
  BGRA* pixels = new BGRA[sw * sh];
  if (pixels == NULL)
    return false;

  for (int iy = sy; iy < (sy + sh); iy++)
    for (int ix = sx; ix < (sx + sw); ix++)
    {
      pixels[(iy - sy) * (sw) + ix - sx].red   = source[iy * width + ix].red;
      pixels[(iy - sy) * (sw) + ix - sx].green = source[iy * width + ix].green;
      pixels[(iy - sy) * (sw) + ix - sx].blue  = source[iy * width + ix].blue;
      pixels[(iy - sy) * (sw) + ix - sx].alpha = source[iy * width + ix].alpha;
    }

  // create the bitmap
  HBITMAP bitmap = CreateBitmap(sw, sh, 1, 32, pixels);

  // put the bitmap in the clipboard
  SetClipboardData(CF_BITMAP, bitmap);
  delete[] pixels;

  CloseClipboard();

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CImageView::PasteChannels(bool red, bool green, bool blue, bool alpha, int merge_method) {
  if (OpenClipboard() == FALSE)
    return false;

  int iWidth = m_Image.GetWidth();
  int iHeight = m_Image.GetHeight();

  int cwidth = 0;
  int cheight = 0;
  RGBA* cpixels = NULL;

  // see if the flat image is in the clipboard
  HGLOBAL memory = (HGLOBAL)GetClipboardData(s_ClipboardFormat);
  if (memory != NULL)
  {
    // get the height and pixels from the clipboard
    dword* ptr = (dword*)GlobalLock(memory);
    if (ptr == NULL) {
      CloseClipboard();
      return false;
    }

    cwidth = *ptr++;
    cheight = *ptr++;
    RGBA* pixels = (RGBA*)ptr;

    cpixels = new RGBA[cwidth * cheight];
    if (cpixels == NULL)
      return false;

    for (int y = 0; y < cheight; y++) {
      for (int x = 0; x < cwidth; x++) {
        cpixels[y * cwidth + x] = pixels[y * cwidth + x];
      }
    }

    GlobalUnlock(memory);
    CloseClipboard();
  }

  HBITMAP bitmap = NULL;
  if (cpixels == NULL) {
    bitmap = (HBITMAP)GetClipboardData(CF_BITMAP);
  }

  // grab a bitmap out of the clipboard
  if (bitmap != NULL)
  {
    BITMAP b;
    GetObject(bitmap, sizeof(b), &b);

    HDC dc = CreateCompatibleDC(NULL);
    HBITMAP oldbitmap = (HBITMAP)SelectObject(dc, bitmap);
    cwidth = iWidth;
    cheight = iHeight;

    cpixels = new RGBA[cwidth * cheight];
    if (cpixels == NULL)
      return false;

    for (int iy = 0; iy < cheight; iy++)
      for (int ix = 0; ix < cwidth; ix++)
      {
        COLORREF pixel = GetPixel(dc, ix, iy);
        if (pixel == CLR_INVALID)
          pixel = RGB(0, 0, 0);

        cpixels[iy * cwidth + ix].red   = GetRValue(pixel);
        cpixels[iy * cwidth + ix].green = GetGValue(pixel);
        cpixels[iy * cwidth + ix].blue  = GetBValue(pixel);
        cpixels[iy * cwidth + ix].alpha = 255;  // there is no alpha so we use a default
      }

    SelectObject(dc, oldbitmap);
    DeleteDC(dc);

    CloseClipboard();
  }

  if (cpixels != NULL) {

    AddUndoState();

    // and now we merge the clipboard image with the current image
    RGBA* iPixels = m_Image.GetPixels();
    int xoffset = 0;
    int yoffset = 0;

    if (merge_method == Merge_IntoSelection) {
      int sx, sy, sw, sh;
      GetSelectionArea(sx, sy, sw, sh);
      xoffset = sx;
      yoffset = sy;
      // rescale cpixels
      CImage32 tmp(cwidth, cheight, cpixels);
      delete[] cpixels;
      cpixels = new RGBA[sw * sh];
      if (cpixels == NULL)
        return false;

      tmp.Rescale(sw, sh);
      RGBA* pixels = tmp.GetPixels();

      for (int i = 0; i < sw * sh; i++)
        cpixels[i] = pixels[i];

      cwidth = sw;
      cheight = sh;
    }

    for (int iy = 0; iy < cheight; iy++)
    {
      for (int ix = 0; ix < cwidth; ix++)
      {
        if (red) iPixels[(iy + yoffset) * iWidth + (ix + xoffset)].red = cpixels[iy * cwidth + ix].red;
        if (green) iPixels[(iy + yoffset) * iWidth + (ix + xoffset)].green = cpixels[iy * cwidth + ix].green;
        if (blue) iPixels[(iy + yoffset) * iWidth + (ix + xoffset)].blue = cpixels[iy * cwidth + ix].blue;
        if (alpha) iPixels[(iy + yoffset) * iWidth + (ix + xoffset)].alpha = cpixels[iy * cwidth + ix].alpha;
      }
    }

    // things have changed
    Invalidate();
    m_Handler->IV_ImageChanged();

    delete[] cpixels;
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool
CImageView::Paste()
{
  return PasteChannels(true, true, true, true);
}

////////////////////////////////////////////////////////////////////////////////

bool
CImageView::CanUndo() const
{
  return m_NumUndoImages > 0;
}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::Undo()
{
  Image* i = m_UndoImages + m_NumUndoImages - 1;
  memcpy(m_Image.GetPixels(), i->pixels, m_Image.GetWidth() * m_Image.GetHeight() * sizeof(RGBA));
  delete[] i->pixels;

  Image* new_images = new Image[m_NumUndoImages - 1];
  for (int i = 0; i < m_NumUndoImages - 1; i++)
    new_images[i] = m_UndoImages[i];

  m_NumUndoImages--;
  delete[] m_UndoImages;
  m_UndoImages = new_images;

  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::SP_ColorSelected(RGBA color)
{
  m_Color = color;
  m_Handler->IV_ColorChanged(color);
}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::TP_ToolSelected(int tool)
{
  // do something with the tool
  m_CurrentTool = tool;
}

////////////////////////////////////////////////////////////////////////////////

POINT
CImageView::ConvertToPixel(POINT point)
{
  // convert pixel coordinates to image coordinates
  POINT retPoint;
  retPoint.x = point.x;
  retPoint.y = point.y;

  // get client rectangle
  RECT ClientRect;
  GetClientRect(&ClientRect);

  // calculate size of pixel squares
  int width = m_Image.GetWidth();
  int height = m_Image.GetHeight();
  int hsize = ClientRect.right / width;
  int vsize = ClientRect.bottom / height;
  int size = std::min(hsize, vsize);
  if (size < 1)
    size = 1;

  int totalx = size * width;
  int totaly = size * height;
  int offsetx = (ClientRect.right - totalx) / 2;
  int offsety = (ClientRect.bottom - totaly) / 2;

  retPoint.x -= offsetx;
  retPoint.y -= offsety;

  retPoint.x = (retPoint.x + size) / size - 1;
  retPoint.y = (retPoint.y + size) / size - 1;

  return retPoint;
}

////////////////////////////////////////////////////////////////////////////////

bool
CImageView::InImage(POINT p)
{
  return (p.x >= 0 && p.x < m_Image.GetWidth() &&
          p.y >= 0 && p.y < m_Image.GetHeight());
}

////////////////////////////////////////////////////////////////////////////////

bool
CImageView::InSelection(POINT p)
{
  if (m_SelectionWidth <= 0 && m_SelectionHeight <= 0)
    return InImage(p);
  else
  if (p.x >= m_SelectionX && p.x < m_SelectionX + m_SelectionWidth &&
      p.y >= m_SelectionY && p.y < m_SelectionY + m_SelectionHeight) {
    return InImage(p);
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////

void
CImageView::GetSelectionArea(int& x, int& y, int& w, int& h) {
  if (m_SelectionWidth <= 0 && m_SelectionHeight <= 0) {
     x = 0;
     y = 0;
     w = m_Image.GetWidth();
     h = m_Image.GetHeight();
  }
  else {
    x = m_SelectionX;
    y = m_SelectionY;
    w = m_SelectionWidth;
    h = m_SelectionHeight;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::Click(bool force_draw)
{
  if (m_Image.GetWidth() == 0 || m_Image.GetHeight() == 0)
    return;

  // convert pixel coordinates to image coordinates
  POINT start = ConvertToPixel(m_LastPoint);
  POINT end = ConvertToPixel(m_CurPoint);

  // bounds check (why are we doing this?)
//  if (start.x < 0 ||
//      start.y < 0 ||
//      start.x >= m_Image.GetWidth() ||
//      start.y >= m_Image.GetHeight()) {
//    return;
//  }

  if (!InImage(end) || !InSelection(end)) {
    return;
  }

  if (!force_draw && start.x == end.x && start.y == end.y) {
    return;
  }

  m_Image.SetPixel(end.x, end.y, m_Color);
  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::Fill()
{
  if (m_MouseDown)
    return;

  // convert pixel coordinates to image coordinates
  POINT startPoint = ConvertToPixel(m_CurPoint);

  // bounds check
  if (!InImage(startPoint) || !InSelection(startPoint))
    return;

  if (!memcmp(&m_Image.GetPixel(startPoint.x, startPoint.y), &m_Color, sizeof(RGBA)))
    return;

  FillMe(startPoint.x, startPoint.y, m_Image.GetPixel(startPoint.x, startPoint.y));

  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

struct Point {
  Point(int x_, int y_) {
    x = x_;
    y = y_;
  }

  int x;
  int y;
};

void
CImageView::FillMe(int x, int y, RGBA colorToReplace)
{
  int sx, sy, sw, sh;
  GetSelectionArea(sx, sy, sw, sh);
  const int width  = sx + sw;
  const int height = sy + sh;

  std::stack<Point> q;
  q.push(    Point(x, y));
  m_Image.SetPixel(x, y, m_Color);

  while (!q.empty()) {
    Point p = q.top();
    q.pop();

    // fill up
    if (p.y > sy && m_Image.GetPixel(p.x, p.y - 1) == colorToReplace) {
      q.push(    Point(p.x, p.y - 1));
      m_Image.SetPixel(p.x, p.y - 1, m_Color);
    }
    // fill down
    if (p.y < height - 1 && m_Image.GetPixel(p.x, p.y + 1) == colorToReplace) {
      q.push(    Point(p.x, p.y + 1));
      m_Image.SetPixel(p.x, p.y + 1, m_Color);
    }
    // fill left
    if (p.x > sx && m_Image.GetPixel(p.x - 1, p.y) == colorToReplace) {
      q.push(    Point(p.x - 1, p.y));
      m_Image.SetPixel(p.x - 1, p.y, m_Color);
    }
    // fill right
    if (p.x < width - 1 && m_Image.GetPixel(p.x + 1, p.y) == colorToReplace) {
      q.push(    Point(p.x + 1, p.y));
      m_Image.SetPixel(p.x + 1, p.y, m_Color);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::Line()
{
  if (!m_MouseDown)
  {
    m_StartPoint = m_CurPoint;
    Invalidate();
    m_Handler->IV_ImageChanged();
  }
  else
  {
    // convert pixel coordinates to image coordinates
    POINT start = ConvertToPixel(m_StartPoint);
    POINT end = ConvertToPixel(m_CurPoint);

    // bounds check
    if (!InImage(start) || !InImage(end))
      return;

    m_Image.Line(start.x, start.y, end.x, end.y, m_Color);
    Invalidate();
    m_Handler->IV_ImageChanged();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::Rectangle()
{
  if (!m_MouseDown)
  {
    m_StartPoint = m_CurPoint;
    Invalidate();
    m_Handler->IV_ImageChanged();
  }
  else
  {
    // convert pixel coordinates to image coordinates
    POINT start = ConvertToPixel(m_StartPoint);
    POINT end = ConvertToPixel(m_CurPoint);

    // bounds check
    if (!InImage(start) || !InImage(end))
      return;

    m_Image.Rectangle(start.x, start.y, end.x, end.y, m_Color);
    Invalidate();
    m_Handler->IV_ImageChanged();
  }
}


////////////////////////////////////////////////////////////////////////////////

void
CImageView::Selection()
{
  if (!m_MouseDown)
  {
    m_StartPoint = m_CurPoint;
    Invalidate();
    m_Handler->IV_ImageChanged();
  }
  else
  {
    UpdateSelection();
    Invalidate();
    m_Handler->IV_ImageChanged();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::Circle()
{
  if (!m_MouseDown)
  {
    m_StartPoint = m_CurPoint;
    Invalidate();
    m_Handler->IV_ImageChanged();
  }
  else
  {
    // convert pixel coordinates to image coordinates
    POINT start = ConvertToPixel(m_StartPoint);
    POINT end = ConvertToPixel(m_CurPoint);

    // bounds check
    if (!InImage(start) || !InImage(end))
      return;

    if (abs(start.x - end.x) > abs(start.y - end.y))
      m_Image.Circle(start.x, start.y, abs(start.x - end.x), m_Color);
    else
      m_Image.Circle(start.x, start.y, abs(start.y - end.y), m_Color);
    Invalidate();
    m_Handler->IV_ImageChanged();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::Ellipse()
{
  if (!m_MouseDown)
  {
    m_StartPoint = m_CurPoint;
    Invalidate();
    m_Handler->IV_ImageChanged();
  }
  else
  {
    // convert pixel coordinates to image coordinates
    POINT start = ConvertToPixel(m_StartPoint);
    POINT end = ConvertToPixel(m_CurPoint);

    // bounds check
    if (!InImage(start) || !InImage(end))
      return;

    m_Image.Ellipse(start.x, start.y, abs(start.x - end.x), abs(start.y - end.y), m_Color);
    Invalidate();
    m_Handler->IV_ImageChanged();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::GetColor(RGBA* color, int x, int y)
{
  // convert pixel coordinates to image coordinates
  POINT point = ConvertToPixel(m_CurPoint);

  // bounds check
  if (!InImage(point))
    return;

  RGBA* pImage = m_Image.GetPixels();

  // now that we have image coordinates, we can update the image
  if (memcmp(pImage + point.y * m_Image.GetWidth() + point.x, &m_Color, sizeof(RGBA)) != 0)
  {
    m_Color = pImage[point.y * m_Image.GetWidth() + point.x];
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::AddUndoState()
{
  Image* new_images = new Image[m_NumUndoImages + 1];
  for (int i = 0; i < m_NumUndoImages; i++)
    new_images[i] = m_UndoImages[i];

  int width = m_Image.GetWidth();
  int height = m_Image.GetHeight();
  new_images[m_NumUndoImages].width = width;
  new_images[m_NumUndoImages].height = height;
  new_images[m_NumUndoImages].pixels = new RGBA[width * height];
  memcpy(new_images[m_NumUndoImages].pixels, m_Image.GetPixels(), width * height * sizeof(RGBA));

  m_NumUndoImages++;
  delete[] m_UndoImages;
  m_UndoImages = new_images;
}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::ResetUndoStates()
{
  for (int i = 0; i < m_NumUndoImages; i++)
    delete[] m_UndoImages[i].pixels;

  delete[] m_UndoImages;
  m_UndoImages = NULL;
  m_NumUndoImages = 0;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnPaint()
{
  CPaintDC _dc(this);
  HDC dc = _dc.m_hDC;

  CImage32 drawImage(m_Image);
  switch(m_CurrentTool)
  {
    case Tool_Pencil:    break;
    case Tool_Fill:      break;
    case Tool_Line:      PaintLine(drawImage); break;
    case Tool_Rectangle: PaintRectangle(drawImage); break;
    case Tool_Circle:    PaintCircle(drawImage); break;
    case Tool_Ellipse:   PaintEllipse(drawImage); break;
    case Tool_Selection: UpdateSelection(); break;
  }

  int width = drawImage.GetWidth();
  int height = drawImage.GetHeight();
  RGBA* pImage = drawImage.GetPixels();

  if (width == 0 || height == 0 || pImage == NULL)
  {
    RECT ClientRect;
    GetClientRect(&ClientRect);
    FillRect(dc, &ClientRect, (HBRUSH)GetStockObject(BLACK_BRUSH));
    return;
  }

  // get client rectangle
  RECT ClientRect;
  GetClientRect(&ClientRect);

  // calculate size of pixel squares
  int hsize = ClientRect.right / width;
  int vsize = ClientRect.bottom / height;
  int size = std::min(hsize, vsize);
  if (size < 1)
    size = 1;

  int totalx = size * width;
  int totaly = size * height;
  int offsetx = (ClientRect.right - totalx) / 2;
  int offsety = (ClientRect.bottom - totaly) / 2;

  // draw black rectangles in the empty parts
  HBRUSH black_brush = (HBRUSH)GetStockObject(BLACK_BRUSH);
  RECT Rect;

  // top
  SetRect(&Rect, 0, 0, ClientRect.right, offsety - 1);
  FillRect(dc, &Rect, black_brush);

  // bottom
  SetRect(&Rect, 0, offsety + totaly + 1, ClientRect.right, ClientRect.bottom);
  FillRect(dc, &Rect, black_brush);

  // left
  SetRect(&Rect, 0, offsety - 1, offsetx - 1, offsety + totaly + 1);
  FillRect(dc, &Rect, black_brush);

  // right
  SetRect(&Rect, offsetx + totalx + 1, offsety - 1, ClientRect.right, offsety + totaly + 1);
  FillRect(dc, &Rect, black_brush);

  // draw the image
  for (int ix = 0; ix < width; ix++)
    for (int iy = 0; iy < height; iy++)
    {
      RGBA color = pImage[iy * width + ix];

      // opaque
      if (color.alpha == 255)
      {
        HBRUSH brush = CreateSolidBrush(RGB(color.red, color.green, color.blue));

        RECT Rect = { ix * size, iy * size, ix * size + size, iy * size + size };
        OffsetRect(&Rect, offsetx, offsety);
        FillRect(dc, &Rect, brush);

        DeleteObject(brush);
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

        HBRUSH Brush1 = CreateSolidBrush(RGB(Color1.red, Color1.green, Color1.blue));
        HBRUSH Brush2 = CreateSolidBrush(RGB(Color2.red, Color2.green, Color2.blue));

        RECT Rect;

        // draw rectangles

        // upper left
        SetRect(&Rect,
          ix * size,
          iy * size,
          ix * size + size / 2,
          iy * size + size / 2);
        OffsetRect(&Rect, offsetx, offsety);
        FillRect(dc, &Rect, Brush1);

        // upper right
        SetRect(&Rect,
          ix * size + size / 2,
          iy * size,
          ix * size + size,
          iy * size + size / 2);
        OffsetRect(&Rect, offsetx, offsety);
        FillRect(dc, &Rect, Brush2);

        // lower left
        SetRect(&Rect,
          ix * size,
          iy * size + size / 2,
          ix * size + size / 2,
          iy * size + size);
        OffsetRect(&Rect, offsetx, offsety);
        FillRect(dc, &Rect, Brush2);

        // lower right
        SetRect(&Rect,
          ix * size + size / 2,
          iy * size + size / 2,
          ix * size + size,
          iy * size + size);
        OffsetRect(&Rect, offsetx, offsety);
        FillRect(dc, &Rect, Brush1);

        DeleteObject(Brush1);
        DeleteObject(Brush2);
      }

    }

  // draw the grid if it is enabled
  if (size >= 3 && m_ShowGrid) {
    HPEN linepen = CreatePen(PS_SOLID, 1, RGB(255, 0, 255));
    HPEN oldpen = (HPEN)SelectObject(dc, linepen);
    for (int ix = 0; ix <= width; ++ix) {
      MoveToEx(dc, offsetx + ix * size, offsety, NULL);
      LineTo  (dc, offsetx + ix * size, offsety + height * size);
    }
    for (int iy = 0; iy <= height; ++iy) {
      MoveToEx(dc, offsetx,                offsety + iy * size, NULL);
      LineTo  (dc, offsetx + width * size, offsety + iy * size);
    }
    SelectObject(dc, oldpen);
    DeleteObject(linepen);
  }

  // draw the selection box if it exists
  if (m_SelectionWidth > 0 && m_SelectionHeight > 0) {
    HPEN linepen = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
    HPEN oldpen = (HPEN)SelectObject(dc, linepen);
    MoveToEx(dc, offsetx + m_SelectionX * size, offsety + m_SelectionY * size, NULL);
    LineTo  (dc, offsetx + m_SelectionX * size, offsety + (m_SelectionY + m_SelectionHeight) * size);
    LineTo  (dc, offsetx + (m_SelectionX + m_SelectionWidth) * size, offsety + (m_SelectionY + m_SelectionHeight) * size);
    LineTo  (dc, offsetx + (m_SelectionX + m_SelectionWidth) * size, offsety + m_SelectionY * size);
    LineTo  (dc, offsetx + m_SelectionX * size, offsety + m_SelectionY * size);
    SelectObject(dc, oldpen);
    DeleteObject(linepen);
  }

  // draw a white rectangle around the image
  SetRect(&Rect, offsetx - 1, offsety - 1, offsetx + totalx + 1, offsety + totaly + 1);

  HPEN   white_pen = CreatePen(PS_SOLID, 1, RGB(0xFF, 0xFF, 0xFF));
  HBRUSH old_brush = (HBRUSH)SelectObject(dc, GetStockObject(NULL_BRUSH));
  HPEN   old_pen   = (HPEN)  SelectObject(dc, white_pen);

  ::Rectangle(dc, Rect.left, Rect.top, Rect.right, Rect.bottom);

  SelectObject(dc, old_pen);
  SelectObject(dc, old_brush);
  DeleteObject(white_pen);
}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::PaintLine(CImage32& pImage)
{
  if (!m_MouseDown)
    return;

  // convert pixel coordinates to image coordinates
  POINT start = ConvertToPixel(m_StartPoint);
  POINT end = ConvertToPixel(m_CurPoint);

  // bounds check
  if (!InImage(start) || !InImage(end))
    return;

  pImage.Line(start.x, start.y, end.x, end.y, m_Color);
}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::PaintRectangle(CImage32& pImage)
{
  if (!m_MouseDown)
    return;

  // convert pixel coordinates to image coordinates
  POINT start = ConvertToPixel(m_StartPoint);
  POINT end = ConvertToPixel(m_CurPoint);

  // bounds check
  if (!InImage(start) || !InImage(end))
    return;

  pImage.Rectangle(start.x, start.y, end.x, end.y, m_Color);
}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::PaintCircle(CImage32& pImage)
{
  if (!m_MouseDown)
    return;

  // convert pixel coordinates to image coordinates
  POINT start = ConvertToPixel(m_StartPoint);
  POINT end = ConvertToPixel(m_CurPoint);

  // bounds check
  if (!InImage(start) || !InImage(end))
    return;

  if (abs(start.x - end.x) > abs(start.y - end.y))
    pImage.Circle(start.x, start.y, abs(start.x - end.x), m_Color);
  else
    pImage.Circle(start.x, start.y, abs(start.y - end.y), m_Color);
}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::PaintEllipse(CImage32& pImage)
{
  if (!m_MouseDown)
    return;

  // convert pixel coordinates to image coordinates
  POINT start = ConvertToPixel(m_StartPoint);
  POINT end = ConvertToPixel(m_CurPoint);

  // bounds check
  if (!InImage(start) || !InImage(end))
    return;

  pImage.Ellipse(start.x, start.y, abs(start.x - end.x), abs(start.y - end.y), m_Color);

}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::UpdateSelection()
{
  if (!m_MouseDown)
    return;

  // convert pixel coordinates to image coordinates
  POINT start = ConvertToPixel(m_StartPoint);
  POINT end = ConvertToPixel(m_CurPoint);

  // bounds check
  if (!InImage(start) || !InImage(end))
    return;

  m_SelectionX = std::min(start.x, end.x);
  m_SelectionY = std::min(start.y, end.y);
  m_SelectionWidth  = std::max(start.x, end.x) - m_SelectionX;
  m_SelectionHeight = std::max(start.y, end.y) - m_SelectionY;  

}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnSize(UINT type, int cx, int cy)
{
  Invalidate();
  CWnd::OnSize(type, cx, cy);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnLButtonDown(UINT flags, CPoint point)
{
  m_LastPoint = m_CurPoint;
  m_CurPoint = point;

  if (m_CurrentTool != Tool_Selection) {
    // perform a normal click operation
    AddUndoState();
  }

  switch (m_CurrentTool) {
    case Tool_Pencil:    Click(true); break;
    case Tool_Fill:      Fill();      break;
    case Tool_Line:      Line();      break;
    case Tool_Rectangle: Rectangle(); break;
    case Tool_Circle:    Circle();    break;
    case Tool_Ellipse:   Ellipse();   break;
    case Tool_Selection: Selection(); break;
  }

  m_MouseDown = true;
  SetCapture();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnLButtonUp(UINT flags, CPoint point)
{
  m_LastPoint = m_CurPoint;
  m_CurPoint = point;

  switch (m_CurrentTool) {
    case Tool_Pencil:    break;
    case Tool_Fill:      break;
    case Tool_Line:      Line();  break;
    case Tool_Rectangle: Rectangle(); break;
    case Tool_Circle:    Circle(); break;
    case Tool_Ellipse:   Ellipse(); break;
    case Tool_Selection: Selection(); break;
  }

  m_MouseDown = false;
  ReleaseCapture();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnRButtonUp(UINT flags, CPoint point)
{
  // show the image view menu
  HMENU menu = LoadMenu(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_IMAGEVIEW));
  HMENU submenu = GetSubMenu(menu, 0);

  // make sure we clicked in the image
  if (!InImage(ConvertToPixel(point))) {
    return;
  }

  m_CurPoint = point;
  ClientToScreen(&point);

  // disable menu items if they aren't available
  if (m_NumUndoImages == 0) {
    EnableMenuItem(submenu, ID_IMAGEVIEW_UNDO, MF_BYCOMMAND | MF_GRAYED);
  }

  if (m_Image.GetWidth() != m_Image.GetHeight()) {
    EnableMenuItem(menu, ID_IMAGEVIEW_ROTATE_CW,  MF_BYCOMMAND | MF_GRAYED);
    EnableMenuItem(menu, ID_IMAGEVIEW_ROTATE_CCW, MF_BYCOMMAND | MF_GRAYED);
  }

  if (m_ShowGrid) {
    CheckMenuItem(menu, ID_IMAGEVIEW_VIEWGRID, MF_BYCOMMAND | MF_CHECKED);
  }

  switch (m_Image.GetBlendMode()) {
    case CImage32::BLEND:      CheckMenuItem(menu, ID_IMAGEVIEW_BLENDMODE_BLEND,     MF_BYCOMMAND | MF_CHECKED); break;
    case CImage32::REPLACE:    CheckMenuItem(menu, ID_IMAGEVIEW_BLENDMODE_REPLACE,   MF_BYCOMMAND | MF_CHECKED); break;
    case CImage32::RGB_ONLY:   CheckMenuItem(menu, ID_IMAGEVIEW_BLENDMODE_RGBONLY,   MF_BYCOMMAND | MF_CHECKED); break;
    case CImage32::ALPHA_ONLY: CheckMenuItem(menu, ID_IMAGEVIEW_BLENDMODE_ALPHAONLY, MF_BYCOMMAND | MF_CHECKED); break;
  }

  TrackPopupMenu(submenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, m_hWnd, NULL);
  DestroyMenu(menu);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnMouseMove(UINT flags, CPoint point)
{
  m_LastPoint = m_CurPoint;
  m_CurPoint = point;

  POINT current = ConvertToPixel(point);
  if (InImage(current)) {
    char str[80];
    sprintf(str, "(%d, %d)", current.x, current.y);
    GetStatusBar()->SetWindowText(str);
  } else {
    GetStatusBar()->SetWindowText("");
  }

  if (!m_MouseDown)
    return;

  switch (m_CurrentTool)
  {
    case Tool_Pencil:
      Click(false);
      break;

    case Tool_Fill:
      break;

    case Tool_Line:
    case Tool_Rectangle:
    case Tool_Circle:
    case Tool_Ellipse:
    case Tool_Selection:
      Invalidate();
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnColorPicker()
{
  GetColor(&m_Color, m_CurPoint.x, m_CurPoint.y);
  m_Handler->IV_ColorChanged(m_Color);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnUndo()
{
  Undo();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnCopy()
{
  Copy();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnPaste()
{
  Paste();
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnPasteRGB()
{
  PasteChannels(true, true, true, false);
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnPasteAlpha()
{
  PasteChannels(false, false, false, true);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnPasteIntoSelection()
{
  PasteChannels(true, true, true, true, Merge_IntoSelection);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnViewGrid()
{
  m_ShowGrid = !m_ShowGrid;
  Invalidate();
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnBlendModeBlend()
{
  m_Image.SetBlendMode(CImage32::BLEND);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnBlendModeReplace()
{
  m_Image.SetBlendMode(CImage32::REPLACE);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnBlendModeRGBOnly()
{
  m_Image.SetBlendMode(CImage32::RGB_ONLY);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnBlendModeAlphaOnly()
{
  m_Image.SetBlendMode(CImage32::ALPHA_ONLY);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnRotateCW()
{
  AddUndoState();
  RotateCW(m_Image.GetWidth(), m_Image.GetPixels());

  // things have changed
  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnRotateCCW()
{
  AddUndoState();
  RotateCCW(m_Image.GetWidth(), m_Image.GetPixels());

  // things have changed
  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnSlideUp()
{
  AddUndoState();
  Translate(m_Image.GetWidth(), m_Image.GetHeight(), m_Image.GetPixels(), 0, -1);

  // things have changed
  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnSlideRight()
{
  AddUndoState();
  Translate(m_Image.GetWidth(), m_Image.GetHeight(), m_Image.GetPixels(), 1, 0);

  // things have changed
  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnSlideDown()
{
  AddUndoState();
  Translate(m_Image.GetWidth(), m_Image.GetHeight(), m_Image.GetPixels(), 0, 1);

  // things have changed
  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnSlideLeft()
{
  AddUndoState();
  Translate(m_Image.GetWidth(), m_Image.GetHeight(), m_Image.GetPixels(), -1, 0);

  // things have changed
  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnSlideOther()
{
  CNumberDialog dx("Slide Horizontally", "Value", 0, 0, m_Image.GetWidth()); 
  if (dx.DoModal() == IDOK) {
    CNumberDialog dy("Slide Vertically", "Value", 0, 0, m_Image.GetHeight()); 
    if (dy.DoModal() == IDOK) {
      if (dx.GetValue() != 0 || dy.GetValue() != 0) {
        AddUndoState();
        Translate(m_Image.GetWidth(), m_Image.GetHeight(), m_Image.GetPixels(), dx.GetValue(), dy.GetValue());

        // things have changed
        Invalidate();
        m_Handler->IV_ImageChanged();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnFlipHorizontally()
{
  AddUndoState();
  FlipHorizontally(m_Image.GetWidth(), m_Image.GetHeight(), m_Image.GetPixels());

  // things have changed
  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnFlipVertically()
{
  AddUndoState();
  FlipVertically(m_Image.GetWidth(), m_Image.GetHeight(), m_Image.GetPixels());

  // things have changed
  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnFillRGB()
{
  AddUndoState();

  int sx, sy, sw, sh;
  int width = m_Image.GetWidth();
  RGBA* pImage = m_Image.GetPixels();

  GetSelectionArea(sx, sy, sw, sh);

  for (int dx = sx; dx < (sx + sw); dx++) {
    for (int dy = sy; dy < sy + sh; dy++) {
      pImage[dy * width + dx].red   = m_Color.red;
      pImage[dy * width + dx].green = m_Color.green;
      pImage[dy * width + dx].blue  = m_Color.blue;
    } 
  }

  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnFillAlpha()
{
  AddUndoState();
  int sx, sy, sw, sh;
  int width = m_Image.GetWidth();
  RGBA* pImage = m_Image.GetPixels();

  GetSelectionArea(sx, sy, sw, sh);

  for (int dx = sx; dx < (sx + sw); dx++) {
    for (int dy = sy; dy < sy + sh; dy++) {
      pImage[dy * width + dx].alpha = m_Color.alpha;
    } 
  }

  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnFillBoth()
{
  AddUndoState();

  int sx, sy, sw, sh;
  int width = m_Image.GetWidth();
  RGBA* pImage = m_Image.GetPixels();

  GetSelectionArea(sx, sy, sw, sh);

  for (int dx = sx; dx < (sx + sw); dx++) {
    for (int dy = sy; dy < (sy + sh); dy++) {
      pImage[dy * width + dx] = m_Color;
    } 
  }

  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnReplaceRGBA()
{
  POINT p = ConvertToPixel(m_CurPoint);
  if (!InImage(p)) {
    return;
  }

  AddUndoState();

  m_Image.ReplaceColor(m_Image.GetPixel(p.x, p.y), GetColor());

  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnReplaceRGB()
{
  POINT p = ConvertToPixel(m_CurPoint);
  if (!InImage(p)) {
    return;
  }

  AddUndoState();

  RGBA c = m_Image.GetPixel(p.x, p.y);
  int sx, sy, sw, sh;
  int width = m_Image.GetWidth();
  RGBA* pImage = m_Image.GetPixels();

  GetSelectionArea(sx, sy, sw, sh);


  for (int dx = sx; dx < (sx + sw); dx++) {
    for (int dy = sy; dy < (sy + sh); dy++) {
      if (pImage[dy * width + dx].red   == c.red &&
          pImage[dy * width + dx].green == c.green &&
          pImage[dy * width + dx].blue  == c.blue)
      {
        pImage[dy * width + dx].red   = GetColor().red;
        pImage[dy * width + dx].green = GetColor().green;
        pImage[dy * width + dx].blue  = GetColor().blue;
      }
    }
  }

  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnReplaceAlpha()
{
  POINT p = ConvertToPixel(m_CurPoint);
  if (!InImage(p)) {
    return;
  }

  AddUndoState();

  RGBA c = m_Image.GetPixel(p.x, p.y);

  int sx, sy, sw, sh;
  int width = m_Image.GetWidth();
  RGBA* pImage = m_Image.GetPixels();

  GetSelectionArea(sx, sy, sw, sh);


  for (int dx = sx; dx < (sx + sw); ++dx) {
    for (int dy = sy; dy < (sy + sh); ++dy) {
      if (pImage[dy * width + dx].alpha == c.alpha) {
        pImage[dy * width + dx].alpha = GetColor().alpha;
      }
    }
  }

  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnFilterBlur()
{
  AddUndoState();

  Blur(m_Image.GetWidth(), m_Image.GetHeight(), m_Image.GetPixels());

  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnFilterNoise()
{
  AddUndoState();

  Noise(m_Image.GetWidth(), m_Image.GetHeight(), m_Image.GetPixels());

  Invalidate();
  m_Handler->IV_ImageChanged();
}


////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnFilterNegativeImage(bool red, bool green, bool blue, bool alpha)
{
  AddUndoState();

  NegativeImage(m_Image.GetWidth(), m_Image.GetHeight(), red, green, blue, alpha, m_Image.GetPixels());

  Invalidate();
  m_Handler->IV_ImageChanged();
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnFilterNegativeImageRGB()
{
  OnFilterNegativeImage(true, true, true, false);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnFilterNegativeImageAlpha()
{
  OnFilterNegativeImage(false, false, false, true);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnFilterNegativeImageRGBA()
{
  OnFilterNegativeImage(true, true, true, true);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnFilterSolarize()
{
  CNumberDialog dialog("Solarize Value", "Value", 128, 0, 255);

  if (dialog.DoModal() == IDOK) {
    int value = dialog.GetValue();
    AddUndoState();

    Solarize(m_Image.GetWidth(), m_Image.GetHeight(), value, m_Image.GetPixels());

    Invalidate();
    m_Handler->IV_ImageChanged();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnFilterAdjustBrightness()
{
  CNumberDialog dialog("Adjustment Value", "Value", 0, -255, 255);

  if (dialog.DoModal() == IDOK) {
    int value = dialog.GetValue();
    if (value != 0) {
      AddUndoState();

      AdjustBrightness(m_Image.GetWidth(), m_Image.GetHeight(), m_Image.GetPixels(), value, value, value);

      Invalidate();
      m_Handler->IV_ImageChanged();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnSetColorAlpha()
{
  AddUndoState();

  RGB c = { m_Color.red, m_Color.green, m_Color.blue };
  m_Image.SetColorAlpha(c, m_Color.alpha);

  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnScaleAlpha()
{
  AddUndoState();

  RGBA* pixels = m_Image.GetPixels();
  int sx, sy, sw, sh;
  int width = m_Image.GetWidth();

  GetSelectionArea(sx, sy, sw, sh);

  for (int dx = sx; dx < (sx + sw); dx++) {
    for (int dy = sy; dy < (sy + sh); dy++) {
      pixels[dy * width + dx].alpha = (int) pixels[dy * width + dx].alpha * m_Color.alpha / 255;
    }  
  }

  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////
