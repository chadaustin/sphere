#include "ImageView.hpp"
#include "../common/Filters.hpp"
#include "resource.h"


static int s_ImageViewID = 9000;

static UINT s_ClipboardFormat;


BEGIN_MESSAGE_MAP(CImageView, CWnd)

  ON_WM_PAINT()
  ON_WM_SIZE()

  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_RBUTTONUP()
  ON_WM_MOUSEMOVE()

  ON_COMMAND(ID_IMAGEVIEW_COLORPICKER,         OnColorPicker)
  ON_COMMAND(ID_IMAGEVIEW_UNDO,                OnUndo)
  ON_COMMAND(ID_IMAGEVIEW_COPY,                OnCopy)
  ON_COMMAND(ID_IMAGEVIEW_PASTE,               OnPaste)
  ON_COMMAND(ID_IMAGEVIEW_BLENDMODE_BLEND,     OnBlendModeBlend)
  ON_COMMAND(ID_IMAGEVIEW_BLENDMODE_REPLACE,   OnBlendModeReplace)
  ON_COMMAND(ID_IMAGEVIEW_BLENDMODE_RGBONLY,   OnBlendModeRGBOnly)
  ON_COMMAND(ID_IMAGEVIEW_BLENDMODE_ALPHAONLY, OnBlendModeAlphaOnly)
  ON_COMMAND(ID_IMAGEVIEW_ROTATE_CW,           OnRotateCW)
  ON_COMMAND(ID_IMAGEVIEW_ROTATE_CCW,          OnRotateCCW)
  ON_COMMAND(ID_IMAGEVIEW_SLIDE_UP,            OnSlideUp)
  ON_COMMAND(ID_IMAGEVIEW_SLIDE_RIGHT,         OnSlideRight)
  ON_COMMAND(ID_IMAGEVIEW_SLIDE_DOWN,          OnSlideDown)
  ON_COMMAND(ID_IMAGEVIEW_SLIDE_LEFT,          OnSlideLeft)
  ON_COMMAND(ID_IMAGEVIEW_FLIP_HORIZONTALLY,   OnFlipHorizontally)
  ON_COMMAND(ID_IMAGEVIEW_FLIP_VERTICALLY,     OnFlipVertically)
  ON_COMMAND(ID_IMAGEVIEW_FILL_RGB,            OnFillRGB)
  ON_COMMAND(ID_IMAGEVIEW_FILL_ALPHA,          OnFillAlpha)
  ON_COMMAND(ID_IMAGEVIEW_FILL_BOTH,           OnFillBoth)
  ON_COMMAND(ID_IMAGEVIEW_FILTER_BLUR,         OnFilterBlur)
  ON_COMMAND(ID_IMAGEVIEW_FILTER_NOISE,        OnFilterNoise)
  ON_COMMAND(ID_IMAGEVIEW_SETCOLORALPHA,       OnSetColorAlpha)
  ON_COMMAND(ID_IMAGEVIEW_SCALEALPHA,          OnScaleAlpha)

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
{
  m_Image.SetBlendMode(CImage32::REPLACE);
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

  // ADD FLAT 32

  // copy the image as a flat 32-bit color image
  HGLOBAL memory = GlobalAlloc(GHND, 8 + width * height * 4);
  dword* ptr = (dword*)GlobalLock(memory);

  *ptr++ = width;
  *ptr++ = height;
  memcpy(ptr, m_Image.GetPixels(), width * height * sizeof(RGBA));

  // put the image on the clipboard
  GlobalUnlock(memory);
  SetClipboardData(s_ClipboardFormat, memory);

  // ADD DDB

  // create a pixel array to initialize the bitmap
  BGRA* pixels = new BGRA[width * height];
  RGBA* source = m_Image.GetPixels();
  for (int iy = 0; iy < height; iy++)
    for (int ix = 0; ix < width; ix++)
    {
      pixels[iy * width + ix].red   = source[iy * width + ix].red;
      pixels[iy * width + ix].green = source[iy * width + ix].green;
      pixels[iy * width + ix].blue  = source[iy * width + ix].blue;
    }

  // create the bitmap
  HBITMAP bitmap = CreateBitmap(width, height, 1, 32, pixels);

  // put the bitmap in the clipboard
  SetClipboardData(CF_BITMAP, bitmap);
  delete[] pixels;

  CloseClipboard();

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CImageView::Paste()
{
  if (OpenClipboard() == FALSE)
    return false;

  int iWidth = m_Image.GetWidth();
  int iHeight = m_Image.GetHeight();

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

    AddUndoState();

    int width = *ptr++;
    int height = *ptr++;
    RGBA* pixels = (RGBA*)ptr;
    RGBA* pImage = m_Image.GetPixels();

    // put them into the current view
    for (int iy = 0; iy < iHeight; iy++)
      for (int ix = 0; ix < iWidth; ix++)
      {
        if (ix < width && iy < height)
          pImage[iy * iWidth + ix] = pixels[iy * width + ix];
        else
          pImage[iy * iWidth + ix] = CreateRGBA(0, 0, 0, 255);
      }

    GlobalUnlock(memory);
    CloseClipboard();

    // things have changed
    Invalidate();
    m_Handler->IV_ImageChanged();
    
    return true;
  }

  // grab a bitmap out of the clipboard
  HBITMAP bitmap = (HBITMAP)GetClipboardData(CF_BITMAP);
  if (bitmap != NULL)
  {
    AddUndoState();

    BITMAP b;
    GetObject(bitmap, sizeof(b), &b);

    HDC dc = CreateCompatibleDC(NULL);
    HBITMAP oldbitmap = (HBITMAP)SelectObject(dc, bitmap);
    RGBA* pImage = m_Image.GetPixels();

    for (int iy = 0; iy < iHeight; iy++)
      for (int ix = 0; ix < iWidth; ix++)
      {
        COLORREF pixel = GetPixel(dc, ix, iy);
        if (pixel == CLR_INVALID)
          pixel = RGB(0, 0, 0);
        pImage[iy * iWidth + ix].red   = GetRValue(pixel);
        pImage[iy * iWidth + ix].green = GetGValue(pixel);
        pImage[iy * iWidth + ix].blue  = GetBValue(pixel);
        pImage[iy * iWidth + ix].alpha = 255;
      }

    SelectObject(dc, oldbitmap);
    DeleteDC(dc);

    CloseClipboard();

    // things have changed
    Invalidate();
    m_Handler->IV_ImageChanged();

    return true;
  }

  CloseClipboard();
  return false;
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
  int size = min(hsize, vsize);
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

  if (end.x < 0 ||
      end.y < 0 ||
      end.x >= m_Image.GetWidth() ||
      end.y >= m_Image.GetHeight()) {
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
  if (startPoint.x < 0 ||
      startPoint.y < 0 ||
      startPoint.x >= m_Image.GetWidth() ||
      startPoint.y >= m_Image.GetHeight())
    return;

  if (!memcmp(&m_Image.GetPixel(startPoint.x, startPoint.y), &m_Color, sizeof(RGBA)))
    return;

  FillMe(startPoint.x, startPoint.y, m_Image.GetPixel(startPoint.x, startPoint.y));

  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
CImageView::FillMe(int x, int y, RGBA colorToReplace)
{
  RGBA* pImage = m_Image.GetPixels();
  m_Image.SetPixel(x, y, m_Color);

  // fill up
  if (y > 0)
    if (!memcmp(pImage + (y - 1) * m_Image.GetWidth() + x, &colorToReplace, sizeof(RGBA)))
      FillMe(x, y-1, colorToReplace);
  // fill down
  if (y < m_Image.GetHeight() - 1)
    if (!memcmp(pImage + (y + 1) * m_Image.GetWidth() + x, &colorToReplace, sizeof(RGBA)))
      FillMe(x, y+1, colorToReplace);
  // fill left
  if (x > 0)
    if (!memcmp(pImage + y * m_Image.GetWidth() + x - 1, &colorToReplace, sizeof(RGBA)))
      FillMe(x-1, y, colorToReplace);  
  // fill right
  if (x < m_Image.GetWidth() - 1)
    if (!memcmp(pImage + y * m_Image.GetWidth() + x + 1, &colorToReplace, sizeof(RGBA)))
      FillMe(x+1, y, colorToReplace);
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
    if (start.x < 0 ||
        start.y < 0 ||
        start.x >= m_Image.GetWidth() ||
        start.y >= m_Image.GetHeight())
      return;
    if (end.x < 0 ||
        end.y < 0 ||
        end.x >= m_Image.GetWidth() ||
        end.y >= m_Image.GetHeight())
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
    if (start.x < 0 ||
        start.y < 0 ||
        start.x >= m_Image.GetWidth() ||
        start.y >= m_Image.GetHeight())
      return;
    if (end.x < 0 ||
        end.y < 0 ||
        end.x >= m_Image.GetWidth() ||
        end.y >= m_Image.GetHeight())
      return;

    m_Image.Rectangle(start.x, start.y, end.x, end.y, m_Color);
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
    if (start.x < 0 ||
        start.y < 0 ||
        start.x >= m_Image.GetWidth() ||
        start.y >= m_Image.GetHeight())
      return;
    if (end.x < 0 ||
        end.y < 0 ||
        end.x >= m_Image.GetWidth() ||
        end.y >= m_Image.GetHeight())
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
CImageView::GetColor(RGBA* color, int x, int y)
{
  // convert pixel coordinates to image coordinates
  POINT point = ConvertToPixel(m_CurPoint);

  // bounds check
  if (point.x < 0 ||
      point.y < 0 ||
      point.x >= m_Image.GetWidth() ||
      point.y >= m_Image.GetHeight())
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
  int size = min(hsize, vsize);
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
  if (start.x < 0 ||
      start.y < 0 ||
      start.x >= m_Image.GetWidth() ||
      start.y >= m_Image.GetHeight())
    return;
  if (end.x < 0 ||
      end.y < 0 ||
      end.x >= m_Image.GetWidth() ||
      end.y >= m_Image.GetHeight())
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
  if (start.x < 0 ||
      start.y < 0 ||
      start.x >= m_Image.GetWidth() ||
      start.y >= m_Image.GetHeight())
    return;
  if (end.x < 0 ||
      end.y < 0 ||
      end.x >= m_Image.GetWidth() ||
      end.y >= m_Image.GetHeight())
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
  if (start.x < 0 ||
      start.y < 0 ||
      start.x >= m_Image.GetWidth() ||
      start.y >= m_Image.GetHeight())
    return;
  if (end.x < 0 ||
      end.y < 0 ||
      end.x >= m_Image.GetWidth() ||
      end.y >= m_Image.GetHeight())
    return;

  if (abs(start.x - end.x) > abs(start.y - end.y))
    pImage.Circle(start.x, start.y, abs(start.x - end.x), m_Color);
  else
    pImage.Circle(start.x, start.y, abs(start.y - end.y), m_Color);
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

  // perform a normal click operation
  AddUndoState();

  switch (m_CurrentTool) {
    case Tool_Pencil:    Click(true); break;
    case Tool_Fill:      Fill();      break;
    case Tool_Line:      Line();      break;
    case Tool_Rectangle: Rectangle(); break;
    case Tool_Circle:    Circle();    break;
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

  // find out where we clicked in the image
  POINT p = ConvertToPixel(point);
  if (p.x < 0 ||
      p.y < 0 ||
      p.x >= m_Image.GetWidth() ||
      p.y >= m_Image.GetHeight()
  ) {
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

  switch (m_Image.GetBlendMode()) {
    case CImage32::BLEND:      CheckMenuItem(menu, ID_IMAGEVIEW_BLENDMODE_BLEND,     MF_BYCOMMAND | MF_CHECKED); break;
    case CImage32::REPLACE:    CheckMenuItem(menu, ID_IMAGEVIEW_BLENDMODE_REPLACE,   MF_BYCOMMAND | MF_CHECKED); break;
    case CImage32::RGB_ONLY:   CheckMenuItem(menu, ID_IMAGEVIEW_BLENDMODE_RGBONLY,   MF_BYCOMMAND | MF_CHECKED); break;
    case CImage32::ALPHA_ONLY: CheckMenuItem(menu, ID_IMAGEVIEW_BLENDMODE_ALPHAONLY, MF_BYCOMMAND | MF_CHECKED); break;
  }

  TrackPopupMenu(submenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, m_hWnd, NULL);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnMouseMove(UINT flags, CPoint point)
{
  m_LastPoint = m_CurPoint;
  m_CurPoint = point;

  if (!m_MouseDown)
    return;
  
  switch(m_CurrentTool)
  {
    case Tool_Pencil: 
      Click(false); 
      break;

    case Tool_Fill: 
      break;

    case Tool_Line: 
    case Tool_Rectangle: 
    case Tool_Circle: 
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

////////////////////////////////////////////////////////////////////////////////

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

  RGBA* pImage = m_Image.GetPixels();
  for (int i = 0; i < m_Image.GetWidth() * m_Image.GetHeight(); i++) {
    pImage[i].red   = m_Color.red;
    pImage[i].green = m_Color.green;
    pImage[i].blue  = m_Color.blue;
  }

  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnFillAlpha()
{
  AddUndoState();

  RGBA* pImage = m_Image.GetPixels();
  for (int i = 0; i < m_Image.GetWidth() * m_Image.GetHeight(); i++) {
    pImage[i].alpha = m_Color.alpha;
  }

  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CImageView::OnFillBoth()
{
  AddUndoState();

  RGBA* pImage = m_Image.GetPixels();
  for (int i = 0; i < m_Image.GetWidth() * m_Image.GetHeight(); i++) {
    pImage[i] = m_Color;
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
  for (int i = 0; i < m_Image.GetWidth() * m_Image.GetHeight(); i++) {
    pixels[i].alpha = (int)pixels[i].alpha * m_Color.alpha / 255;
  }

  Invalidate();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////