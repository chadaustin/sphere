#pragma warning(disable : 4786)

#include <algorithm>
#include "ImageView.hpp"
#include "Editor.hpp"
#include "../common/Filters.hpp"
//#include "resource.h"
#include "IDs.hpp"


//static int s_ImageViewID = 9000;

/*todo:
static UINT s_ClipboardFormat;
*/

/*
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
  ON_COMMAND(ID_IMAGEVIEW_REPLACE_RGBA,        OnReplaceRGBA)
  ON_COMMAND(ID_IMAGEVIEW_REPLACE_RGB,         OnReplaceRGB)
  ON_COMMAND(ID_IMAGEVIEW_REPLACE_ALPHA,       OnReplaceAlpha)
  ON_COMMAND(ID_IMAGEVIEW_FILTER_BLUR,         OnFilterBlur)
  ON_COMMAND(ID_IMAGEVIEW_FILTER_NOISE,        OnFilterNoise)
  ON_COMMAND(ID_IMAGEVIEW_SETCOLORALPHA,       OnSetColorAlpha)
  ON_COMMAND(ID_IMAGEVIEW_SCALEALPHA,          OnScaleAlpha)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wImageView, wxWindow)

  EVT_PAINT(wImageView::OnPaint)
  EVT_SIZE(wImageView::OnSize)

  EVT_LEFT_DOWN(wImageView::OnLButtonDown)
  EVT_LEFT_UP(wImageView::OnLButtonUp)
  EVT_RIGHT_UP(wImageView::OnRButtonUp)
  EVT_MOTION(wImageView::OnMouseMove)

  EVT_MENU(wID_IMAGEVIEW_COLORPICKER,         wImageView::OnColorPicker)
  EVT_MENU(wID_IMAGEVIEW_UNDO,                wImageView::OnUndo)
  EVT_MENU(wID_IMAGEVIEW_COPY,                wImageView::OnCopy)
  EVT_MENU(wID_IMAGEVIEW_PASTE,               wImageView::OnPaste)
  EVT_MENU(wID_IMAGEVIEW_BLENDMODE_BLEND,     wImageView::OnBlendModeBlend)
  EVT_MENU(wID_IMAGEVIEW_BLENDMODE_REPLACE,   wImageView::OnBlendModeReplace)
  EVT_MENU(wID_IMAGEVIEW_BLENDMODE_RGBONLY,   wImageView::OnBlendModeRGBOnly)
  EVT_MENU(wID_IMAGEVIEW_BLENDMODE_ALPHAONLY, wImageView::OnBlendModeAlphaOnly)
  EVT_MENU(wID_IMAGEVIEW_ROTATE_CW,           wImageView::OnRotateCW)
  EVT_MENU(wID_IMAGEVIEW_ROTATE_CCW,          wImageView::OnRotateCCW)
  EVT_MENU(wID_IMAGEVIEW_SLIDE_UP,            wImageView::OnSlideUp)
  EVT_MENU(wID_IMAGEVIEW_SLIDE_RIGHT,         wImageView::OnSlideRight)
  EVT_MENU(wID_IMAGEVIEW_SLIDE_DOWN,          wImageView::OnSlideDown)
  EVT_MENU(wID_IMAGEVIEW_SLIDE_LEFT,          wImageView::OnSlideLeft)
  EVT_MENU(wID_IMAGEVIEW_FLIP_HORIZONTALLY,   wImageView::OnFlipHorizontally)
  EVT_MENU(wID_IMAGEVIEW_FLIP_VERTICALLY,     wImageView::OnFlipVertically)
  EVT_MENU(wID_IMAGEVIEW_FILL_RGB,            wImageView::OnFillRGB)
  EVT_MENU(wID_IMAGEVIEW_FILL_ALPHA,          wImageView::OnFillAlpha)
  EVT_MENU(wID_IMAGEVIEW_FILL_BOTH,           wImageView::OnFillBoth)
  EVT_MENU(wID_IMAGEVIEW_REPLACE_RGBA,        wImageView::OnReplaceRGBA)
  EVT_MENU(wID_IMAGEVIEW_REPLACE_RGB,         wImageView::OnReplaceRGB)
  EVT_MENU(wID_IMAGEVIEW_REPLACE_ALPHA,       wImageView::OnReplaceAlpha)
  EVT_MENU(wID_IMAGEVIEW_FILTER_BLUR,         wImageView::OnFilterBlur)
  EVT_MENU(wID_IMAGEVIEW_FILTER_NOISE,        wImageView::OnFilterNoise)
  EVT_MENU(wID_IMAGEVIEW_SETCOLORALPHA,       wImageView::OnSetColorAlpha)
  EVT_MENU(wID_IMAGEVIEW_SCALEALPHA,          wImageView::OnScaleAlpha)

END_EVENT_TABLE()


////////////////////////////////////////////////////////////////////////////////

wImageView::wImageView(wxWindow* parent_window, wDocumentWindow* owner, IImageViewHandler* handler)
: wxWindow(parent_window, -1)
, m_Color(CreateRGBA(0, 0, 0, 255))
, m_SwatchPalette(NULL)
, m_ToolPalette(NULL)
, m_MouseDown(false)
, m_CurrentTool(Tool_Pencil)
, m_NumUndoImages(0)
, m_UndoImages(NULL)
{
  wxMenu *submenu;

  m_Image.SetBlendMode(CImage32::REPLACE);

  m_Handler = handler;

  m_SwatchPalette = new wSwatchPalette(owner, this);
  m_ToolPalette   = new wImageToolPalette(owner, this);

  m_Menu = new wxMenu();
  //submenu = m_Menu; /*todo: figure out what is wrong*/
  
  m_Menu->Append(wID_IMAGEVIEW_COLORPICKER, "Color Picker");
  m_Menu->AppendSeparator();
  m_Menu->Append(wID_IMAGEVIEW_UNDO, "Undo");
  m_Menu->AppendSeparator();
  m_Menu->Append(wID_IMAGEVIEW_COPY, "Copy");
  m_Menu->Append(wID_IMAGEVIEW_PASTE, "Paste");
  m_Menu->AppendSeparator();
  
  submenu = new wxMenu("Blend Mode");
  submenu->Append(wID_IMAGEVIEW_BLENDMODE_BLEND, "Blend", "", TRUE);
  submenu->Append(wID_IMAGEVIEW_BLENDMODE_REPLACE, "Replace", "", TRUE);
  submenu->Append(wID_IMAGEVIEW_BLENDMODE_RGBONLY, "RGB Only", "", TRUE);
  submenu->Append(wID_IMAGEVIEW_BLENDMODE_ALPHAONLY, "Alpha Only", "", TRUE);
  m_Menu->Append(wID_IMAGEVIEW_BLENDMODE_, "Blend Mode", submenu);

  submenu = new wxMenu("Rotate");
  submenu->Append(wID_IMAGEVIEW_ROTATE_CW, "Clockwise");
  submenu->Append(wID_IMAGEVIEW_ROTATE_CCW, "Counter-Clockwise");
  m_Menu->Append(wID_IMAGEVIEW_ROTATE_, "Rotate", submenu);

  submenu = new wxMenu("Slide");
  submenu->Append(wID_IMAGEVIEW_SLIDE_UP, "Up");
  submenu->Append(wID_IMAGEVIEW_SLIDE_RIGHT, "Right");
  submenu->Append(wID_IMAGEVIEW_SLIDE_DOWN, "Down");
  submenu->Append(wID_IMAGEVIEW_SLIDE_LEFT, "Left");
  m_Menu->Append(wID_IMAGEVIEW_SLIDE_, "Slide", submenu);

  submenu = new wxMenu("Flip");
  submenu->Append(wID_IMAGEVIEW_FLIP_HORIZONTALLY, "Horizontally");
  submenu->Append(wID_IMAGEVIEW_FLIP_VERTICALLY, "Vertically");
  m_Menu->Append(wID_IMAGEVIEW_FLIP_, "Flip", submenu);

  submenu = new wxMenu("Fill");
  submenu->Append(wID_IMAGEVIEW_FILL_RGB, "RGB");
  submenu->Append(wID_IMAGEVIEW_FILL_ALPHA, "Alpha");
  submenu->Append(wID_IMAGEVIEW_FILL_BOTH, "Both");
  m_Menu->Append(wID_IMAGEVIEW_FILL_, "Fill", submenu);

  submenu = new wxMenu("Replace");
  submenu->Append(wID_IMAGEVIEW_REPLACE_RGBA, "RGBA");
  submenu->Append(wID_IMAGEVIEW_REPLACE_RGB, "RGB");
  submenu->Append(wID_IMAGEVIEW_REPLACE_ALPHA, "Alpha");
  m_Menu->Append(wID_IMAGEVIEW_REPLACE_, "Replace", submenu);

  submenu = new wxMenu("Filter");
  submenu->Append(wID_IMAGEVIEW_FILTER_BLUR, "Blur");
  submenu->Append(wID_IMAGEVIEW_FILTER_NOISE, "Noise");
  m_Menu->Append(wID_IMAGEVIEW_FILTER_, "Filter", submenu);

  m_Menu->Append(wID_IMAGEVIEW_SETCOLORALPHA, "Set Color Alpha");
  m_Menu->Append(wID_IMAGEVIEW_SCALEALPHA, "Scale Alpha");
/*todo:
  s_ClipboardFormat = RegisterClipboardFormat("FlatImage32");
  */
}

////////////////////////////////////////////////////////////////////////////////

wImageView::~wImageView()
{
/*
  if (m_SwatchPalette) {
    m_SwatchPalette->Destroy();
  }

  if (m_ToolPalette) {
    m_ToolPalette->Destroy();
  }
*/
  //DestroyWindow();
  /*todo: move to Destroy()*/
//  for(int i = 0; i < m_Submenu.size(); i++) {
//    delete m_Submenu[i];
//  }
  delete m_Menu;
}

/*
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
*/

////////////////////////////////////////////////////////////////////////////////

bool
wImageView::SetImage(int width, int height, const RGBA* pixels)
{
  ResetUndoStates();

  m_Image.Resize(width, height);
  memcpy(m_Image.GetPixels(), pixels, width * height * sizeof(RGBA));

  Refresh();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

int
wImageView::GetWidth() const
{
  return m_Image.GetWidth();
}

////////////////////////////////////////////////////////////////////////////////

int
wImageView::GetHeight() const
{
  return m_Image.GetHeight();
}

////////////////////////////////////////////////////////////////////////////////

RGBA*
wImageView::GetPixels()
{
  return m_Image.GetPixels();
}

////////////////////////////////////////////////////////////////////////////////

const RGBA*
wImageView::GetPixels() const
{
  return m_Image.GetPixels();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::SetColor(RGBA color)
{
  m_Color = color;
  m_SwatchPalette->SetColor(color);
}

////////////////////////////////////////////////////////////////////////////////

RGBA
wImageView::GetColor() const
{
  return m_Color;
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::FillRGB()
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

  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::FillAlpha()
{
  AddUndoState();

  for (int y = 0; y < m_Image.GetHeight(); y++)
    for (int x = 0; x < m_Image.GetWidth(); x++)
    {
      RGBA tColor = m_Image.GetPixel(x, y);
      tColor.alpha = m_Color.alpha;

      m_Image.SetPixel(x, y, tColor);
    }

  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

bool
wImageView::Copy()
{
/*todo:
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
*/
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
wImageView::Paste()
{
/*todo:
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
*/
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool
wImageView::CanUndo() const
{
  return m_NumUndoImages > 0;
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::Undo()
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

  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::SP_ColorSelected(RGBA color)
{
  m_Color = color;
  m_Handler->IV_ColorChanged(color);
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::TP_ToolSelected(int tool)
{
  // do something with the tool
  m_CurrentTool = tool;
}

////////////////////////////////////////////////////////////////////////////////

wxPoint
wImageView::ConvertToPixel(wxPoint point)
{
  // convert pixel coordinates to image coordinates
  wxPoint retPoint;
  retPoint.x = point.x;
  retPoint.y = point.y;

  // get client rectangle
  wxSize ClientSize(GetClientSize());

  // calculate size of pixel squares
  int width = m_Image.GetWidth();
  int height = m_Image.GetHeight();
  int hsize = ClientSize.GetWidth() / width;
  int vsize = ClientSize.GetHeight() / height;
  int size = std::_cpp_min(hsize, vsize);
  if (size < 1)
    size = 1;

  int totalx = size * width;
  int totaly = size * height;
  int offsetx = (ClientSize.GetWidth() - totalx) / 2;
  int offsety = (ClientSize.GetHeight() - totaly) / 2;

  retPoint.x -= offsetx;
  retPoint.y -= offsety;

  retPoint.x = (retPoint.x + size) / size - 1;
  retPoint.y = (retPoint.y + size) / size - 1;

  return retPoint;
}

////////////////////////////////////////////////////////////////////////////////

bool
wImageView::InImage(wxPoint p)
{
  return (p.x >= 0 && p.x < m_Image.GetWidth() &&
          p.y >= 0 && p.y < m_Image.GetHeight());
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::Click(bool force_draw)
{
  if (m_Image.GetWidth() == 0 || m_Image.GetHeight() == 0)
    return;

  // convert pixel coordinates to image coordinates
  wxPoint start = ConvertToPixel(m_LastPoint);
  wxPoint end = ConvertToPixel(m_CurPoint);

  // bounds check (why are we doing this?)
//  if (start.x < 0 ||
//      start.y < 0 ||
//      start.x >= m_Image.GetWidth() ||
//      start.y >= m_Image.GetHeight()) {
//    return;
//  }

  if (!InImage(end)) {
    return;
  }

  if (!force_draw && start.x == end.x && start.y == end.y) {
    return;
  }
  
  m_Image.SetPixel(end.x, end.y, m_Color);
  RefreshPixels(start.x, start.y, end.x, end.y);
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::Fill()
{
  if (m_MouseDown)
    return;

  // convert pixel coordinates to image coordinates
  wxPoint startPoint = ConvertToPixel(m_CurPoint);

  // bounds check
  if (!InImage(startPoint))
    return;

  if (!memcmp(&m_Image.GetPixel(startPoint.x, startPoint.y), &m_Color, sizeof(RGBA)))
    return;

  FillMe(startPoint.x, startPoint.y, m_Image.GetPixel(startPoint.x, startPoint.y));

  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::FillMe(int x, int y, RGBA colorToReplace)
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
wImageView::Line()
{
  if (!m_MouseDown)
  {
    m_StartPoint = m_CurPoint;
    wxPoint start = ConvertToPixel(m_StartPoint);
    RefreshPixels(start.x, start.y, start.x, start.y);
    m_Handler->IV_ImageChanged();
  }
  else
  {
    // convert pixel coordinates to image coordinates
    wxPoint start = ConvertToPixel(m_StartPoint);
    wxPoint end = ConvertToPixel(m_CurPoint);
 
    // bounds check
    if (!InImage(start) || !InImage(end))
      return;

    m_Image.Line(start.x, start.y, end.x, end.y, m_Color);
    RefreshPixels(start.x, start.y, end.x, end.y);
    m_Handler->IV_ImageChanged();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::Rectangle()
{
  if (!m_MouseDown)
  {
    m_StartPoint = m_CurPoint;
    wxPoint start = ConvertToPixel(m_StartPoint);
    RefreshPixels(start.x, start.y, start.x, start.y);
    m_Handler->IV_ImageChanged();
  }
  else
  {
    // convert pixel coordinates to image coordinates
    wxPoint start = ConvertToPixel(m_StartPoint);
    wxPoint end = ConvertToPixel(m_CurPoint);
 
    // bounds check
    if (!InImage(start) || !InImage(end))
      return;

    m_Image.Rectangle(start.x, start.y, end.x, end.y, m_Color);
    RefreshPixels(start.x, start.y, end.x, end.y);
    m_Handler->IV_ImageChanged();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::Circle()
{
  if (!m_MouseDown)
  {
    m_StartPoint = m_CurPoint;
    wxPoint start = ConvertToPixel(m_StartPoint);
    RefreshPixels(start.x, start.y, start.x, start.y);
    m_Handler->IV_ImageChanged();
  }
  else
  {
    // convert pixel coordinates to image coordinates
    wxPoint start = ConvertToPixel(m_StartPoint);
    wxPoint end = ConvertToPixel(m_CurPoint);
 
    // bounds check
    if (!InImage(start) || !InImage(end))
      return;

    int r;
    if (abs(start.x - end.x) > abs(start.y - end.y)) {
      r = abs(start.x - end.x);
    } else {
      r = abs(start.y - end.y);
    }

    m_Image.Circle(start.x, start.y, r, m_Color);
    RefreshPixels(start.x - r, start.y - r, start.x + r, start.y + r);
    m_Handler->IV_ImageChanged();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::GetColor(RGBA* color, int x, int y)
{
  // convert pixel coordinates to image coordinates
  wxPoint point = ConvertToPixel(m_CurPoint);

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
wImageView::AddUndoState()
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
wImageView::ResetUndoStates()
{
  for (int i = 0; i < m_NumUndoImages; i++)
    delete[] m_UndoImages[i].pixels;

  delete[] m_UndoImages;
  m_UndoImages = NULL;
  m_NumUndoImages = 0;
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::RefreshPixels(int x1, int y1, int x2, int y2) 
{
  wxSize ClientSize(GetClientSize());
  int width = m_Image.GetWidth();
  int height = m_Image.GetHeight();
  int hsize = ClientSize.GetWidth() / width;
  int vsize = ClientSize.GetHeight() / height;
  int size = std::_cpp_min(hsize, vsize);
  if (size < 1)
    size = 1;
  int totalx = size * width;
  int totaly = size * height;
  int offsetx = (ClientSize.GetWidth() - totalx) / 2;
  int offsety = (ClientSize.GetHeight() - totaly) / 2;

  if(x1 > x2) {
	  std::swap(x1, x2);
  }
  if(y1 > y2) {
	  std::swap(y1, y2);
  }
  x1--; y1--;
  x2++; y2++;
  wxRect rect(offsetx + x1 * size, offsety + y1 * size, (x2 - x1 + 1) * size, (y2 - y1 + 1) * size);
  Refresh(TRUE, &rect);
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnPaint(wxPaintEvent &event)
{
  wxPaintDC dc(this);
  //HDC dc = _dc.m_hDC;

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
    wxSize ClientSize(GetClientSize());
    dc.SetBrush(wxBrush(wxColour(0x00, 0x00, 0x00), wxSOLID));
    dc.SetPen(wxPen(wxColour(0x00, 0x00, 0x00), 1, wxSOLID));
    dc.DrawRectangle(wxRect(wxPoint(0, 0), ClientSize));
    dc.SetBrush(wxNullBrush);
    dc.SetPen(wxNullPen);
    return;
  }

  wxRegion region = GetUpdateRegion();

  // get client rectangle
  wxSize ClientSize(GetClientSize());

  // calculate size of pixel squares
  int hsize = ClientSize.GetWidth() / width;
  int vsize = ClientSize.GetHeight() / height;
  int size = std::_cpp_min(hsize, vsize);
  if (size < 1)
    size = 1;

  int totalx = size * width;
  int totaly = size * height;
  int offsetx = (ClientSize.GetWidth() - totalx) / 2;
  int offsety = (ClientSize.GetHeight() - totaly) / 2;

  // draw black rectangles in the empty parts
  dc.SetBrush(wxBrush(wxColour(0x00, 0x00, 0x00), wxSOLID));
  dc.SetPen(wxPen(wxColour(0x00, 0x00, 0x00), 1, wxSOLID));

  // top
  dc.DrawRectangle(0, 0,                        ClientSize.GetWidth(), offsety);

  // bottom
  dc.DrawRectangle(0, offsety + totaly,         ClientSize.GetWidth(), ClientSize.GetHeight() - offsety - totaly);

  // left
  dc.DrawRectangle(0, offsety,                  offsetx, totaly);

  // right
  dc.DrawRectangle(offsetx + totalx, offsety,   ClientSize.GetWidth() - offsetx - totalx, totaly);

  dc.SetBrush(wxNullBrush);
  dc.SetPen(wxNullPen);

  // draw the image
  for (int ix = 0; ix < width; ix++) {
    if(region.Contains(offsetx + ix * size, offsety, size, totaly)) {
      for (int iy = 0; iy < height; iy++)
      {
        if(region.Contains(offsetx + ix * size, offsety + iy * size, size, size)) {
          RGBA color = pImage[iy * width + ix];

          // opaque
          if (color.alpha == 255)
          {
            dc.SetBrush(wxBrush(wxColour(color.red, color.green, color.blue), wxSOLID));
            dc.SetPen(wxPen(wxColour(color.red, color.green, color.blue), 1, wxSOLID));

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
      }
    }
  }
  // draw a white rectangle around the image
  dc.SetBrush(wxBrush(wxColour(0xff, 0xff, 0xff), wxTRANSPARENT));
  dc.SetPen(wxPen(wxColour(0xff, 0xff, 0xff), 1, wxSOLID));

  dc.DrawRectangle(offsetx - 1, offsety - 1, totalx + 2, totaly + 2);

  dc.SetBrush(wxNullBrush);
  dc.SetPen(wxNullPen);
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::PaintLine(CImage32& pImage)
{
  if (!m_MouseDown)
    return;

  // convert pixel coordinates to image coordinates
  wxPoint start = ConvertToPixel(m_StartPoint);
  wxPoint end = ConvertToPixel(m_CurPoint);
 
  // bounds check
  if (!InImage(start) || !InImage(end))
    return;

  pImage.Line(start.x, start.y, end.x, end.y, m_Color);
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::PaintRectangle(CImage32& pImage)
{
  if (!m_MouseDown)
    return;

  // convert pixel coordinates to image coordinates
  wxPoint start = ConvertToPixel(m_StartPoint);
  wxPoint end = ConvertToPixel(m_CurPoint);
 
  // bounds check
  if (!InImage(start) || !InImage(end))
    return;

  pImage.Rectangle(start.x, start.y, end.x, end.y, m_Color);
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::PaintCircle(CImage32& pImage)
{
  if (!m_MouseDown)
    return;

  // convert pixel coordinates to image coordinates
  wxPoint start = ConvertToPixel(m_StartPoint);
  wxPoint end = ConvertToPixel(m_CurPoint);
 
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
wImageView::OnSize(wxSizeEvent &event)
{
  Refresh();
  //wxWindow::OnSize(event);
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnLButtonDown(wxMouseEvent &event)
{
  m_LastPoint = m_CurPoint;
  m_CurPoint = event.GetPosition();

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
  CaptureMouse();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnLButtonUp(wxMouseEvent &event)
{
  m_LastPoint = m_CurPoint;
  m_CurPoint = event.GetPosition();

  switch (m_CurrentTool) {
    case Tool_Pencil:    break;
    case Tool_Fill:      break;
    case Tool_Line:      Line();  break;
    case Tool_Rectangle: Rectangle(); break;
    case Tool_Circle:    Circle(); break;
  }

  m_MouseDown = false;
  ReleaseMouse();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnRButtonUp(wxMouseEvent &event)
{
  // show the image view menu

  // make sure we clicked in the image
  if (!InImage(ConvertToPixel(event.GetPosition()))) {
    return;
  }

  m_CurPoint = event.GetPosition();

  // disable menu items if they aren't available
  if (m_NumUndoImages == 0) {
    m_Menu->Enable(wID_IMAGEVIEW_UNDO, FALSE);
  } else {
    m_Menu->Enable(wID_IMAGEVIEW_UNDO, TRUE);
  }
  
  if (m_Image.GetWidth() != m_Image.GetHeight()) {
    m_Menu->Enable(wID_IMAGEVIEW_ROTATE_CW, FALSE);
    m_Menu->Enable(wID_IMAGEVIEW_ROTATE_CCW, FALSE);
  } else {
    m_Menu->Enable(wID_IMAGEVIEW_ROTATE_CW, TRUE);
    m_Menu->Enable(wID_IMAGEVIEW_ROTATE_CCW, TRUE);
  }
  
  m_Menu->Check(wID_IMAGEVIEW_BLENDMODE_BLEND, FALSE);
  m_Menu->Check(wID_IMAGEVIEW_BLENDMODE_REPLACE, FALSE);
  m_Menu->Check(wID_IMAGEVIEW_BLENDMODE_RGBONLY, FALSE);
  m_Menu->Check(wID_IMAGEVIEW_BLENDMODE_ALPHAONLY, FALSE);

  switch (m_Image.GetBlendMode()) {
    case CImage32::BLEND:      m_Menu->Check(wID_IMAGEVIEW_BLENDMODE_BLEND, TRUE); break;
    case CImage32::REPLACE:    m_Menu->Check(wID_IMAGEVIEW_BLENDMODE_REPLACE, TRUE); break;
    case CImage32::RGB_ONLY:   m_Menu->Check(wID_IMAGEVIEW_BLENDMODE_RGBONLY, TRUE); break;
    case CImage32::ALPHA_ONLY: m_Menu->Check(wID_IMAGEVIEW_BLENDMODE_ALPHAONLY, TRUE); break;
  }

  PopupMenu(m_Menu, event.GetPosition());
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnMouseMove(wxMouseEvent &event)
{
  wxPoint start;
  wxPoint end;
  wxPoint last;
  m_LastPoint = m_CurPoint;
  m_CurPoint = event.GetPosition();

  wxPoint current = ConvertToPixel(event.GetPosition());
  if (InImage(current)) {
    char str[80];
    sprintf(str, "(%d, %d)", current.x, current.y);
    SetStatus(wxString(str));
  } else {
    SetStatus(wxString(""));
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
      start = ConvertToPixel(m_StartPoint);
      end = ConvertToPixel(m_CurPoint);
      last = ConvertToPixel(m_LastPoint);
      RefreshPixels(start.x, start.y, end.x, end.y);
      RefreshPixels(start.x, start.y, last.x, last.y);
      break;
    case Tool_Circle: 
      start = ConvertToPixel(m_StartPoint);
      end = ConvertToPixel(m_CurPoint);
      last = ConvertToPixel(m_LastPoint);
      end.x = abs(start.x - end.x);
      end.y = abs(start.y - end.y);
      last.x = abs(start.x - last.x);
      last.y = abs(start.y - last.y);
      int r = end.x;
      if(end.y > r) {r = end.y;}
      if(last.x > r) {r = last.x;}
      if(last.y > r) {r = last.y;}

      RefreshPixels(start.x - r, start.y - r, start.x + r, start.y + r);
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnColorPicker(wxCommandEvent &event)
{
  GetColor(&m_Color, m_CurPoint.x, m_CurPoint.y);
  m_Handler->IV_ColorChanged(m_Color);
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnUndo(wxCommandEvent &event)
{
  Undo();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnCopy(wxCommandEvent &event)
{
  Copy();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnPaste(wxCommandEvent &event)
{
  Paste();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnBlendModeBlend(wxCommandEvent &event)
{
  m_Image.SetBlendMode(CImage32::BLEND);
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnBlendModeReplace(wxCommandEvent &event)
{
  m_Image.SetBlendMode(CImage32::REPLACE);
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnBlendModeRGBOnly(wxCommandEvent &event)
{
  m_Image.SetBlendMode(CImage32::RGB_ONLY);
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnBlendModeAlphaOnly(wxCommandEvent &event)
{
  m_Image.SetBlendMode(CImage32::ALPHA_ONLY);
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnRotateCW(wxCommandEvent &event)
{
  AddUndoState();
  RotateCW(m_Image.GetWidth(), m_Image.GetPixels());

  // things have changed
  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnRotateCCW(wxCommandEvent &event)
{
  AddUndoState();
  RotateCCW(m_Image.GetWidth(), m_Image.GetPixels());

  // things have changed
  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnSlideUp(wxCommandEvent &event)
{
  AddUndoState();
  Translate(m_Image.GetWidth(), m_Image.GetHeight(), m_Image.GetPixels(), 0, -1);

  // things have changed
  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnSlideRight(wxCommandEvent &event)
{
  AddUndoState();
  Translate(m_Image.GetWidth(), m_Image.GetHeight(), m_Image.GetPixels(), 1, 0);

  // things have changed
  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnSlideDown(wxCommandEvent &event)
{
  AddUndoState();
  Translate(m_Image.GetWidth(), m_Image.GetHeight(), m_Image.GetPixels(), 0, 1);

  // things have changed
  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnSlideLeft(wxCommandEvent &event)
{
  AddUndoState();
  Translate(m_Image.GetWidth(), m_Image.GetHeight(), m_Image.GetPixels(), -1, 0);

  // things have changed
  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnFlipHorizontally(wxCommandEvent &event)
{
  AddUndoState();
  FlipHorizontally(m_Image.GetWidth(), m_Image.GetHeight(), m_Image.GetPixels());

  // things have changed
  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnFlipVertically(wxCommandEvent &event)
{
  AddUndoState();
  FlipVertically(m_Image.GetWidth(), m_Image.GetHeight(), m_Image.GetPixels());

  // things have changed
  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnFillRGB(wxCommandEvent &event)
{
  AddUndoState();

  RGBA* pImage = m_Image.GetPixels();
  for (int i = 0; i < m_Image.GetWidth() * m_Image.GetHeight(); i++) {
    pImage[i].red   = m_Color.red;
    pImage[i].green = m_Color.green;
    pImage[i].blue  = m_Color.blue;
  }

  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnFillAlpha(wxCommandEvent &event)
{
  AddUndoState();

  RGBA* pImage = m_Image.GetPixels();
  for (int i = 0; i < m_Image.GetWidth() * m_Image.GetHeight(); i++) {
    pImage[i].alpha = m_Color.alpha;
  }

  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnFillBoth(wxCommandEvent &event)
{
  AddUndoState();

  RGBA* pImage = m_Image.GetPixels();
  for (int i = 0; i < m_Image.GetWidth() * m_Image.GetHeight(); i++) {
    pImage[i] = m_Color;
  }

  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnReplaceRGBA(wxCommandEvent &event)
{
  wxPoint p = ConvertToPixel(m_CurPoint);
  if (!InImage(p)) {
    return;
  }

  RGBA c = m_Image.GetPixel(p.x, p.y);
  for (int i = 0; i < m_Image.GetWidth() * m_Image.GetHeight(); ++i) {
    if (m_Image.GetPixels()[i] == c) {
      m_Image.GetPixels()[i] = GetColor();
    }
  }

  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnReplaceRGB(wxCommandEvent &event)
{
  wxPoint p = ConvertToPixel(m_CurPoint);
  if (!InImage(p)) {
    return;
  }

  RGBA c = m_Image.GetPixel(p.x, p.y);
  for (int i = 0; i < m_Image.GetWidth() * m_Image.GetHeight(); ++i) {
    if (m_Image.GetPixels()[i].red   == c.red &&
        m_Image.GetPixels()[i].green == c.green &&
        m_Image.GetPixels()[i].blue  == c.blue)
    {
      m_Image.GetPixels()[i].red   = GetColor().red;
      m_Image.GetPixels()[i].green = GetColor().green;
      m_Image.GetPixels()[i].blue  = GetColor().blue;
    }
  }

  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnReplaceAlpha(wxCommandEvent &event)
{
  wxPoint p = ConvertToPixel(m_CurPoint);
  if (!InImage(p)) {
    return;
  }

  RGBA c = m_Image.GetPixel(p.x, p.y);
  for (int i = 0; i < m_Image.GetWidth() * m_Image.GetHeight(); ++i) {
    if (m_Image.GetPixels()[i].alpha == c.alpha) {
      m_Image.GetPixels()[i].alpha = GetColor().alpha;
    }
  }

  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnFilterBlur(wxCommandEvent &event)
{
  AddUndoState();

  Blur(m_Image.GetWidth(), m_Image.GetHeight(), m_Image.GetPixels());

  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnFilterNoise(wxCommandEvent &event)
{
  AddUndoState();

  Noise(m_Image.GetWidth(), m_Image.GetHeight(), m_Image.GetPixels());

  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnSetColorAlpha(wxCommandEvent &event)
{
  AddUndoState();

  RGB c = { m_Color.red, m_Color.green, m_Color.blue };
  m_Image.SetColorAlpha(c, m_Color.alpha);

  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////

void
wImageView::OnScaleAlpha(wxCommandEvent &event)
{
  AddUndoState();

  RGBA* pixels = m_Image.GetPixels();
  for (int i = 0; i < m_Image.GetWidth() * m_Image.GetHeight(); i++) {
    pixels[i].alpha = (int)pixels[i].alpha * m_Color.alpha / 255;
  }

  Refresh();
  m_Handler->IV_ImageChanged();
}

////////////////////////////////////////////////////////////////////////////////
