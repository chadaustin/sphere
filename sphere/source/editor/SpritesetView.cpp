#include "SpritesetView.hpp"
#include "ResizeDialog.hpp"
#include "SpritePropertiesDialog.hpp"
#include "StringDialog.hpp"
#include "FileDialogs.hpp"
#include "resource.h"

#include "NumberDialog.hpp"
#include "FontGradientDialog.hpp"

#include "EditRange.hpp"
#include "../common/strcmp_ci.hpp"
#include "../common/sphere_version.h"

// #include "../common/Filters.hpp"  added for CountColorsUsed

#define LABEL_WIDTH 80

static int s_SpritesetViewID = 100;


BEGIN_MESSAGE_MAP(CSpritesetView, CWnd)

  ON_WM_SIZE()
  ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONUP()
  ON_WM_LBUTTONDBLCLK()
  ON_WM_HSCROLL()
  ON_WM_VSCROLL()
  ON_WM_PAINT()

  ON_COMMAND(ID_SPRITESETVIEWDIRECTIONS_INSERT,     OnInsertDirection)
  ON_COMMAND(ID_SPRITESETVIEWDIRECTIONS_DELETE,     OnDeleteDirection)
  ON_COMMAND(ID_SPRITESETVIEWDIRECTIONS_APPEND,     OnAppendDirection)
  ON_COMMAND(ID_SPRITESETVIEWDIRECTIONS_PROPERTIES, OnDirectionProperties)
  ON_COMMAND(ID_SPRITESETVIEWDIRECTIONS_IMPORT_FROM_IMAGE, OnInsertDirectionFromImage)
  ON_COMMAND(ID_SPRITESETVIEWDIRECTIONS_IMPORT_FROM_ANIMATION, OnInsertDirectionFromAnimation)
  ON_COMMAND(ID_SPRITESETVIEWDIRECTIONS_EXPORT_AS_ANIMATION, OnExportDirectionAsAnimation)
  ON_COMMAND(ID_SPRITESETVIEWDIRECTIONS_EXPORT_AS_IMAGE, OnExportDirectionAsImage)

  ON_COMMAND(ID_SPRITESETVIEWDIRECTIONS_FILLDELAY,  OnFillDelay)

  ON_COMMAND(ID_SPRITESETVIEWFRAMES_INSERT,     OnInsertFrame)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_DELETE,     OnDeleteFrame)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_APPEND,     OnAppendFrame)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_COPY,       OnCopyFrame)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_PASTE,      OnPasteFrame)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_PROPERTIES, OnFrameProperties)

//  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_ROTATE_CW,             OnEditRangeRotateCW)
//  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_ROTATE_CCW,            OnEditRangeRotateCCW)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_SLIDE_UP,              OnEditRange)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_SLIDE_RIGHT,           OnEditRange)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_SLIDE_DOWN,            OnEditRange)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_SLIDE_LEFT,            OnEditRange)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_SLIDE_OTHER,           OnEditRange)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_FLIP_HORIZONTALLY,     OnEditRange)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_FLIP_VERTICALLY,       OnEditRange)
//  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_FILL_RGB,              OnEditRangeFillRGB)
//  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_FILL_ALPHA,            OnEditRangeFillAlpha)
//  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_FILL_BOTH,             OnEditRangeFillBoth)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_REPLACE_RGBA,            OnEditRange)
//  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_REPLACE_RGB,           OnEditRangeReplaceRGB)
//  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_REPLACE_ALPHA,         OnEditRangeReplaceAlpha)
//  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_FLT_BLUR,              OnEditRangeFilterBlur)
//  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_FLT_NOISE,              OnEditRangeFilterNoise)
//  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_FLT_ADJUST_BRIGHTNESS,  OnEditRangeFilterAdjustBrightness)
//  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_FLT_ADJUST_GAMMA,       OnEditRangeFilterAdjustGamma)
//  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_FLT_NEGATIVE_IMAGE_RGB, OnEditRangeFilterNegativeImageRGB)
//  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_FLT_NEGATIVE_IMAGE_ALPHA, OnEditRangeFilterNegativeImageAlpha)
//  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_FLT_NEGATIVE_IMAGE_RGBA,  OnEditRangeFilterNegativeImageRGBA)
//  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_FLT_SOLARIZE,             OnEditRangeFilterSolarize)
//  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_SETCOLORALPHA,         OnEditRangeSetColorAlpha)
//  ON_COMMAND(ID_SPRITESETVIEWFRAMES_ER_SCALEALPHA,            OnEditRangeScaleAlpha)
	
	ON_MESSAGE(WM_GETACCELERATOR, OnGetAccelerator)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CSpritesetView::CSpritesetView()
: m_Handler(NULL)
, m_Spriteset(NULL)
, m_CurrentDirection(0)
, m_CurrentFrame(0)

, m_DrawBitmap(NULL)
, m_ZoomFactor(1)
, m_MaxFrameWidth(0)
, m_MaxFrameHeight(0)

, m_TopDirection(0)
, m_LeftFrame(0)

, m_MenuDisplayed(false)
{
}

////////////////////////////////////////////////////////////////////////////////

CSpritesetView::~CSpritesetView()
{
  delete m_DrawBitmap;
  DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CSpritesetView::Create(CWnd* parent, ISpritesetViewHandler* handler, sSpriteset* spriteset)
{
  m_Handler = handler;
  m_Spriteset = spriteset;

  UpdateMaxSizes();

  CWnd::Create(
    AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)), NULL, NULL),
    "",
    WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL,
    CRect(0, 0, 0, 0),
    parent,
    s_SpritesetViewID++);

  // everything should start out in the right place
  RECT rect;
  GetClientRect(&rect);
  OnSize(0, rect.right, rect.bottom);

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetView::SetDirection(int direction)
{
  m_CurrentDirection = direction;
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetView::SetFrame(int frame)
{
  m_CurrentFrame = frame;
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetView::SetZoomFactor(double zoom)
{
  m_ZoomFactor = zoom;
	m_Handler->SV_ZoomFactorChanged(zoom);
  UpdateMaxSizes();
  Invalidate();
  UpdateScrollBars();
}

////////////////////////////////////////////////////////////////////////////////

double
CSpritesetView::GetZoomFactor() const
{
  return m_ZoomFactor;
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetView::SpritesetResized()
{
  UpdateMaxSizes();
  UpdateScrollBars();
  m_Handler->SV_SpritesetModified();
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetView::UpdateMaxSizes()
{
  // apply the zoom ratio
  m_MaxFrameWidth  = m_Spriteset->GetFrameWidth()  * m_ZoomFactor;
  m_MaxFrameHeight = m_Spriteset->GetFrameHeight() * m_ZoomFactor;

  // update the draw bitmap
  delete m_DrawBitmap;
  m_DrawBitmap = new CDIBSection(m_MaxFrameWidth, m_MaxFrameHeight, 32);
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetView::DrawDirection(HDC dc, int direction, int y)
{
  HBRUSH brush = CreateSolidBrush(0xFFFFFF);
  HBRUSH old_brush = (HBRUSH)SelectObject(dc, brush);
  HFONT old_font = (HFONT)SelectObject(dc, GetStockObject(DEFAULT_GUI_FONT));

  RECT r = { 0, y, LABEL_WIDTH, y + m_MaxFrameHeight };
  FillRect(dc, &r, brush);
  DrawText(
    dc,
    m_Spriteset->GetDirectionName(direction),
    -1,
    &r,
    DT_CENTER | DT_VCENTER | DT_SINGLELINE
  );

  SelectObject(dc, old_font);
  SelectObject(dc, old_brush);
  DeleteObject(brush);

  RECT rect;
  GetClientRect(&rect);
  for (int i = 0; i < rect.right / m_MaxFrameWidth + 1; i++)
  {
    RECT c;
    c.left   = LABEL_WIDTH + i * m_MaxFrameWidth;
    c.top    = y;
    c.right  = c.left + m_MaxFrameWidth;
    c.bottom = y + m_MaxFrameHeight;

    if (RectVisible(dc, &c))
    {
      if (i + m_LeftFrame < m_Spriteset->GetNumFrames(direction))
      {
        UpdateDrawBitmap(direction, i + m_LeftFrame);
        DrawFrame(dc, LABEL_WIDTH + i * m_MaxFrameWidth, y, direction, i + m_LeftFrame);
      }
      else
        FillRect(dc, &c, (HBRUSH)GetStockObject(BLACK_BRUSH));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetView::DrawFrame(HDC dc, int x, int y, int direction, int frame)
{
  int index = m_Spriteset->GetFrameIndex(direction, frame);
  CImage32& sprite = m_Spriteset->GetImage(index);
  BitBlt(dc, x, y, m_MaxFrameWidth, m_MaxFrameHeight, m_DrawBitmap->GetDC(), 0, 0, SRCCOPY);

  if (direction == m_CurrentDirection && frame == m_CurrentFrame)
  {
    HPEN new_pen = CreatePen(PS_SOLID, 1, 0xFF00FF);
    HBRUSH brush = (HBRUSH)GetStockObject(NULL_BRUSH);
    HPEN old_pen = (HPEN)SelectObject(dc, new_pen);
    HBRUSH old_brush = (HBRUSH)SelectObject(dc, brush);

    Rectangle(dc, x, y, x + sprite.GetWidth() * m_ZoomFactor, y + sprite.GetHeight() * m_ZoomFactor);

    SelectObject(dc, old_pen);
    SelectObject(dc, old_brush);
    DeleteObject(new_pen);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetView::UpdateDrawBitmap(int direction, int frame)
{
  int index = m_Spriteset->GetFrameIndex(direction, frame);
  CImage32& sprite = m_Spriteset->GetImage(index);
  int   src_width  = sprite.GetWidth();
  int   src_height = sprite.GetHeight();
  RGBA* src_pixels = sprite.GetPixels();

  int   dst_width  = m_DrawBitmap->GetWidth();
  int   dst_height = m_DrawBitmap->GetHeight();
  BGRA* dst_pixels = (BGRA*)m_DrawBitmap->GetPixels();

  for (int iy = 0; iy < dst_height; iy++)
    for (int ix = 0; ix < dst_width; ix++)
    {
      if (ix < src_width * m_ZoomFactor && iy < src_height * m_ZoomFactor)
      {
        int sx = ix / m_ZoomFactor;
        int sy = iy / m_ZoomFactor;
        dst_pixels[iy * dst_width + ix].red   = src_pixels[sy * src_width + sx].red;
        dst_pixels[iy * dst_width + ix].green = src_pixels[sy * src_width + sx].green;
        dst_pixels[iy * dst_width + ix].blue  = src_pixels[sy * src_width + sx].blue;
      }
      else
        dst_pixels[iy * dst_width + ix] = CreateBGRA(0, 0, 0, 255);
    }

  // draw a white border around the frame
  HDC dc = m_DrawBitmap->GetDC();
  HPEN   old_pen   = (HPEN)SelectObject(dc, GetStockObject(WHITE_PEN));
  HBRUSH old_brush = (HBRUSH)SelectObject(dc, GetStockObject(NULL_BRUSH));
  Rectangle(dc, 0, 0, src_width * m_ZoomFactor, src_height* m_ZoomFactor);
  SelectObject(dc, old_pen);
  SelectObject(dc, old_brush);
}

////////////////////////////////////////////////////////////////////////////////

static int GetMaxNumFrames(sSpriteset* spriteset)
{
  int max = 0;
  for (int i = 0; i < spriteset->GetNumDirections(); i++)
    if (spriteset->GetNumFrames(i) > max)
      max = spriteset->GetNumFrames(i);
  return max;
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetView::UpdateScrollBars()
{
  if (m_MaxFrameWidth == 0 || m_MaxFrameHeight == 0)
    return;

  RECT client_rect;
  GetClientRect(&client_rect);
  int num_x_sprites = GetMaxNumFrames(m_Spriteset);
  int x_page_size   = (client_rect.right - LABEL_WIDTH) / m_MaxFrameWidth;
  int num_y_sprites = m_Spriteset->GetNumDirections();
  int y_page_size   = client_rect.bottom / m_MaxFrameHeight;

  SCROLLINFO si;
  si.cbSize = sizeof(si);
  si.fMask = SIF_ALL;
  si.nMin  = 0;

  // horizontal scrollbar (frames)
  if (x_page_size < num_x_sprites)
  {
    si.nMax  = num_x_sprites - 1;
    si.nPage = x_page_size;
    si.nPos  = m_LeftFrame;
  }
  else
  {
    si.nMax  = 0xFFFF;
    si.nPage = 0xFFFE;
    si.nPos  = 0;
  }

  SetScrollInfo(SB_HORZ, &si);

  // vertical scrollbar (directions)
  if (y_page_size < num_y_sprites)
  {
    si.nMax  = num_y_sprites - 1;
    si.nPage = y_page_size;
    si.nPos  = m_TopDirection;
  }
  else
  {
    si.nMax  = 0xFFFF;
    si.nPage = 0xFFFE;
    si.nPos  = 0;
  }

  SetScrollInfo(SB_VERT, &si);
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetView::InvalidateFrame(int direction, int frame)
{
  direction -= m_TopDirection;
  frame     -= m_LeftFrame;

  RECT r;
  r.left   = LABEL_WIDTH + frame * m_MaxFrameWidth;
  r.top    = direction * m_MaxFrameHeight;
  r.right  = r.left + m_MaxFrameWidth;
  r.bottom = r.top + m_MaxFrameHeight;
  InvalidateRect(&r);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnSize(UINT type, int cx, int cy)
{
  if (m_MaxFrameWidth == 0 || m_MaxFrameHeight == 0)
    return;

  if (cx != 0 && cy != 0)
  {
    int x_page_size = (cx - LABEL_WIDTH) / m_MaxFrameWidth;
    int y_page_size = cy / m_MaxFrameHeight;
    int x_num_frames = GetMaxNumFrames(m_Spriteset);
    int y_num_frames = m_Spriteset->GetNumDirections();

    // update m_LeftFrame
    if (m_LeftFrame > x_num_frames - x_page_size)
    {
      m_LeftFrame = x_num_frames - x_page_size;
      if (m_LeftFrame < 0)
        m_LeftFrame = 0;
    }

    // update m_TopDirection
    if (m_TopDirection > y_num_frames - y_page_size)
    {
      m_TopDirection = y_num_frames - y_page_size;
      if (m_TopDirection < 0)
        m_TopDirection = 0;
    }
  }

  UpdateScrollBars();
  CWnd::OnSize(type, cx, cy);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnLButtonDown(UINT flags, CPoint point)
{
  if (m_MenuDisplayed)
    return;

  if (point.x < LABEL_WIDTH)
  {
    m_CurrentDirection = point.y / m_MaxFrameHeight + m_TopDirection;
    if (m_CurrentDirection >= m_Spriteset->GetNumDirections())
      return;
    if (m_CurrentFrame > m_Spriteset->GetNumFrames(m_CurrentDirection) - 1)
      m_CurrentFrame = m_Spriteset->GetNumFrames(m_CurrentDirection) - 1;
    
    m_Handler->SV_CurrentFrameChanged(m_CurrentDirection, m_CurrentFrame);
    Invalidate();
    return;
  }

  int direction = point.y / m_MaxFrameHeight + m_TopDirection;
  int frame = (point.x - LABEL_WIDTH) / m_MaxFrameWidth + m_LeftFrame;

  if (direction < 0 || direction >= m_Spriteset->GetNumDirections())
    return;
  if (frame < 0 || frame >= m_Spriteset->GetNumFrames(direction))
    return;

  InvalidateFrame(m_CurrentDirection, m_CurrentFrame);

  // set the new selection values
  m_CurrentDirection = direction;
  m_CurrentFrame = frame;

  // tell the handler that things have changed
  m_Handler->SV_CurrentFrameChanged(direction, frame);

  InvalidateFrame(m_CurrentDirection, m_CurrentFrame);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnRButtonUp(UINT flags, CPoint point)
{
  if (m_MenuDisplayed)
    return;

  // select the frame
  OnLButtonDown(flags, point);

  // if user right-clicked on a direction
  if (point.x < LABEL_WIDTH)
  {
    // load the menu
    HMENU base_menu = LoadMenu(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_SPRITESETVIEW));
    HMENU menu = GetSubMenu(base_menu, 0);

    if (m_Spriteset->GetNumDirections() < 2)
      EnableMenuItem(menu, ID_SPRITESETVIEWDIRECTIONS_DELETE, MF_BYCOMMAND | MF_GRAYED);

    m_MenuDisplayed = true;

    // display it
    POINT p;
    GetCursorPos(&p);
    TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, p.x, p.y, 0, m_hWnd, NULL);

    m_MenuDisplayed = false;

    DestroyMenu(base_menu);
  }

  if (point.x >= LABEL_WIDTH)
  {
    int direction = point.y / m_MaxFrameHeight + m_TopDirection;
    int frame = (point.x - LABEL_WIDTH) / m_MaxFrameWidth;
    if (frame >= m_Spriteset->GetNumFrames(direction))
      return;

    // load the menu
    HMENU base_menu = LoadMenu(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_SPRITESETVIEW));
    HMENU menu = GetSubMenu(base_menu, 1);

    // enable/disable the menu items
    if (m_Spriteset->GetNumFrames(m_CurrentDirection) < 2)
      EnableMenuItem(menu, ID_SPRITESETVIEWFRAMES_DELETE, MF_BYCOMMAND | MF_GRAYED);

    m_MenuDisplayed = true;

    // display it
    POINT p;
    GetCursorPos(&p);
    TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, p.x, p.y, 0, m_hWnd, NULL);

    m_MenuDisplayed = false;

    DestroyMenu(base_menu);
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnLButtonDblClk(UINT flags, CPoint point)
{
  if (m_MenuDisplayed)
    return;

  if (point.x - LABEL_WIDTH < 0)
    return;

  int direction = point.y / m_MaxFrameHeight + m_TopDirection;
  int frame = (point.x - LABEL_WIDTH) / m_MaxFrameWidth + m_LeftFrame;

  if (direction < 0 || direction >= m_Spriteset->GetNumDirections())
    return;
  if (frame < 0 || frame >= m_Spriteset->GetNumFrames(direction))
    return;

  InvalidateFrame(m_CurrentDirection, m_CurrentFrame);

  m_CurrentDirection = direction;
  m_CurrentFrame = frame;

  m_Handler->SV_CurrentFrameChanged(direction, frame);
  m_Handler->SV_EditFrame();

  InvalidateFrame(m_CurrentDirection, m_CurrentFrame);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnHScroll(UINT code, UINT pos, CScrollBar* scroll_bar)
{
  RECT client_rect;
  GetClientRect(&client_rect);
  int page_width = (client_rect.right - LABEL_WIDTH) / m_MaxFrameWidth;
  int x_num_frames = GetMaxNumFrames(m_Spriteset);

  switch (code)
  {
    case SB_LEFT:       m_LeftFrame = 0; break;
    case SB_RIGHT:      m_LeftFrame = x_num_frames - page_width; break;
    case SB_PAGELEFT:   m_LeftFrame -= page_width; break;
    case SB_PAGERIGHT:  m_LeftFrame += page_width; break;
    case SB_LINELEFT:   m_LeftFrame--; break;
    case SB_LINERIGHT:  m_LeftFrame++; break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK: m_LeftFrame = pos; break;
  }

  if (m_LeftFrame > x_num_frames - page_width)
    m_LeftFrame = x_num_frames - page_width;
  if (m_LeftFrame < 0)
    m_LeftFrame = 0;

  UpdateScrollBars();
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnVScroll(UINT code, UINT pos, CScrollBar* scroll_bar)
{
  RECT client_rect;
  GetClientRect(&client_rect);
  int page_width = client_rect.bottom / m_MaxFrameHeight;
  int y_num_frames = m_Spriteset->GetNumDirections();

  switch (code)
  {
    case SB_TOP:        m_TopDirection = 0; break;
    case SB_BOTTOM:     m_TopDirection = y_num_frames - page_width; break;
    case SB_PAGEUP:     m_TopDirection -= page_width; break;
    case SB_PAGEDOWN:   m_TopDirection += page_width; break;
    case SB_LINEUP:     m_TopDirection--; break;
    case SB_LINEDOWN:   m_TopDirection++; break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK: m_TopDirection = pos; break;
  }

  if (m_TopDirection > y_num_frames - page_width)
    m_TopDirection = y_num_frames - page_width;
  if (m_TopDirection < 0)
    m_TopDirection = 0;

  UpdateScrollBars();
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnPaint()
{
  CPaintDC dc_(this);
  HDC dc = dc_.m_hDC;

  RECT rect;
  GetClientRect(&rect);

  for (int i = 0; i < rect.bottom / m_MaxFrameHeight + 1; i++)
  {
    if (m_TopDirection + i < m_Spriteset->GetNumDirections())
    {
      DrawDirection(dc, m_TopDirection + i, i * m_MaxFrameHeight);
    }
    else
    {
      RECT r;
      r.left = 0;
      r.top = i * m_MaxFrameHeight;
      r.right = rect.right;
      r.bottom = i * m_MaxFrameHeight + m_MaxFrameHeight;
      if (RectVisible(dc, &r))
        FillRect(dc, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnInsertDirection()
{
  m_Spriteset->InsertDirection(m_CurrentDirection);
  if (m_CurrentDirection > m_Spriteset->GetNumDirections() - 1)
    m_CurrentDirection = m_Spriteset->GetNumDirections() - 1;

  UpdateMaxSizes();
  UpdateScrollBars();
  Invalidate();
  m_Handler->SV_CurrentFrameChanged(m_CurrentDirection, m_CurrentFrame);
  m_Handler->SV_SpritesetModified();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnDeleteDirection()
{
  m_Spriteset->DeleteDirection(m_CurrentDirection);
  if (m_CurrentDirection > m_Spriteset->GetNumDirections() - 1)
    m_CurrentDirection = m_Spriteset->GetNumDirections() - 1;

  UpdateMaxSizes();
  UpdateScrollBars();
  Invalidate();
  m_Handler->SV_CurrentFrameChanged(m_CurrentDirection, m_CurrentFrame);
  m_Handler->SV_SpritesetModified();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnAppendDirection()
{
  m_Spriteset->InsertDirection(m_Spriteset->GetNumDirections());

  UpdateMaxSizes();
  UpdateScrollBars();
  Invalidate();
  m_Handler->SV_CurrentFrameChanged(m_CurrentDirection, m_CurrentFrame);
  m_Handler->SV_SpritesetModified();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnDirectionProperties()
{
  CStringDialog dialog(
    "Direction Name",
    m_Spriteset->GetDirectionName(m_CurrentDirection)
  );

  if (dialog.DoModal() == IDOK) {
    m_Spriteset->SetDirectionName(m_CurrentDirection, dialog.GetValue());
    Invalidate();
    m_Handler->SV_SpritesetModified();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnInsertFrame()
{
  m_Spriteset->InsertFrame(m_CurrentDirection, m_CurrentFrame);
  UpdateMaxSizes(); 
  UpdateScrollBars();
  Invalidate();
  m_Handler->SV_SpritesetModified();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnDeleteFrame()
{
  m_Spriteset->DeleteFrame(m_CurrentDirection, m_CurrentFrame);
  if (m_CurrentFrame > m_Spriteset->GetNumFrames(m_CurrentDirection) - 1)
    m_CurrentFrame = m_Spriteset->GetNumFrames(m_CurrentDirection) - 1;

  UpdateMaxSizes();
  UpdateScrollBars();
  Invalidate();
  m_Handler->SV_CurrentFrameChanged(m_CurrentDirection, m_CurrentFrame);
  m_Handler->SV_SpritesetModified();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnAppendFrame()
{
  m_Spriteset->InsertFrame(m_CurrentDirection, m_Spriteset->GetNumFrames(m_CurrentDirection));
  UpdateMaxSizes(); 
  UpdateScrollBars();
  Invalidate();
  m_Handler->SV_SpritesetModified();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnCopyFrame()
{
  m_Handler->SV_CopyCurrentFrame();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnPasteFrame()
{
  m_Handler->SV_PasteCurrentFrame();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnFillDelay()
{
  CNumberDialog dialog("Spriteset Delay", "Delay", 8, 1, 4096);

  if (dialog.DoModal() == IDOK) {

    bool modified = false;
    
    if (m_CurrentDirection >= 0 && m_CurrentDirection < m_Spriteset->GetNumDirections()) {

      for (int j = 0; j < m_Spriteset->GetNumFrames(m_CurrentDirection); j++) {

        if (m_Spriteset->GetFrameDelay(m_CurrentDirection, j) != dialog.GetValue()) {
          modified = true;
        }

        m_Spriteset->SetFrameDelay(m_CurrentDirection, j, dialog.GetValue());
      }
    }

    if (modified) {
      m_Handler->SV_SpritesetModified();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnFrameProperties()
{
  CSpritePropertiesDialog dialog(m_Spriteset, m_CurrentDirection, m_CurrentFrame);
  if (dialog.DoModal() == IDOK)
    m_Handler->SV_SpritesetModified();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnInsertDirectionFromImage()
{
  CImageFileDialog dialog(FDM_OPEN, "Insert Direction From Image");
  if (dialog.DoModal() == IDOK) {

    int frame_width = m_Spriteset->GetFrameWidth();
    int frame_height = m_Spriteset->GetFrameHeight();

    CImage32 image;
    if ( !image.Load(dialog.GetPathName()) ) {
      MessageBox("Error loading image.");
      return;
    }

    if (image.GetWidth() % frame_width > 0
     || image.GetHeight() % frame_height > 0) {
      MessageBox("Invalid image width or height.");
      return;
    }

    int num_frames = image.GetWidth() / frame_width;
    int old_current_frame = m_CurrentFrame;

    int x = 0;
    int y = 0;

    int current_direction = m_CurrentDirection;
    m_Spriteset->InsertDirection(current_direction);

    for (int i = 0; i < num_frames; i++) {
      int current_image = m_Spriteset->GetNumImages();
      int current_frame = old_current_frame + i;
      
      m_Spriteset->InsertImage(current_image);

      if (current_image < m_Spriteset->GetNumImages()) {

        CImage32& frame = m_Spriteset->GetImage(current_image);
        // having to set the blend mode seems weird to me
        frame.SetBlendMode(CImage32::BlendMode::REPLACE);

        for (int sy = 0; sy < frame.GetHeight(); sy++) {
          for (int sx = 0; sx < frame.GetWidth(); sx++) {
            frame.SetPixel(sx, sy, image.GetPixel(x + sx, y + sy));
          }
        }

        x += frame.GetWidth();

        if (current_direction < m_Spriteset->GetNumDirections())
        {
          int __num_frames__ = m_Spriteset->GetNumFrames(current_direction) + 1;
          m_Spriteset->InsertFrame(current_direction, current_frame);
          if (__num_frames__ == m_Spriteset->GetNumFrames(current_direction)) {
            m_Spriteset->SetFrameIndex(current_direction, current_frame, current_image);
          }
        }

      }
    }

    if (m_Spriteset->GetNumFrames(current_direction) > 0)
      m_Spriteset->DeleteFrame(current_direction, m_Spriteset->GetNumFrames(current_direction) - 1);

  }

  UpdateMaxSizes(); 
  UpdateScrollBars();
  Invalidate();
  m_Handler->SV_SpritesetModified();
}

///////////////////////////////////////////////////////////////////////////////

#include "../common/AnimationFactory.hpp"

afx_msg void
CSpritesetView::OnInsertDirectionFromAnimation()
{
  CAnimationFileDialog dialog(FDM_OPEN, "Insert Direction From Animation");
  if (dialog.DoModal() == IDOK) {

    int frame_width = m_Spriteset->GetFrameWidth();
    int frame_height = m_Spriteset->GetFrameHeight();

    IAnimation* animation = LoadAnimation(dialog.GetPathName());
    if ( !animation ) {
      MessageBox("Error loading animation.");
      return;
    }

    /*
    if (animation->GetWidth() != frame_width
     || animation->GetHeight() != frame_height) {
      char message[1000];
      sprintf(message, "Invalid animation width or height: %d %d",
        animation->GetWidth(), animation->GetHeight());
      MessageBox(message);
      return;
    }
    */

    int current_direction = m_CurrentDirection;
    m_Spriteset->InsertDirection(current_direction);
    m_Spriteset->SetDirectionName(current_direction, dialog.GetFileName());

    int max_frames = animation->GetNumFrames() == 0 ? 255 : animation->GetNumFrames();

    for (int frame_number = 0; frame_number < max_frames; frame_number++) {

      if (animation->GetWidth()  < 0 || animation->GetWidth() > 4096
        || animation->GetHeight() < 0 || animation->GetHeight() > 4096) {
        break;
      }

      if (animation->IsEndOfAnimation()) {       
        break;
      }

      RGBA* pixels = new RGBA[animation->GetWidth() * animation->GetHeight()];
      if ( !pixels )
        return;

      if (animation->ReadNextFrame((RGBA*) pixels) == false) {
        delete[] pixels;
        break;
      }

      CImage32 image;
      if (!image.Create(m_Spriteset->GetFrameWidth(), m_Spriteset->GetFrameHeight())) {
        delete[] pixels;
        break;
      }
 
      for (int y = 0; (y < image.GetHeight() && y < animation->GetHeight()); y++) {
        for (int x = 0; (x < image.GetWidth() && x < animation->GetWidth()); x++) {
          image.SetPixel(x, y, pixels[y * animation->GetWidth() + x]);
        }
      }

      delete[] pixels;

      int current_image = -1;
      if (1) {
        for (int i = 0; i < m_Spriteset->GetNumImages(); i++) {
          const CImage32& img = m_Spriteset->GetImage(i);
          if (image == img) {
            current_image = i;
            break;
          }
        }
      }

      if (current_image == -1) {
        current_image = m_Spriteset->GetNumImages();
        m_Spriteset->InsertImage(current_image);

        if (m_Spriteset->GetNumImages() != current_image + 1) {
          return;
        }

        m_Spriteset->GetImage(current_image) = image;
      }

      int __num_frames__ = m_Spriteset->GetNumFrames(current_direction) + 1;
      m_Spriteset->InsertFrame(current_direction, frame_number);
      if (m_Spriteset->GetNumFrames(current_direction) == __num_frames__) {
        int delay = animation->GetDelay();
        if (delay)
          m_Spriteset->SetFrameDelay(current_direction, frame_number, delay / 10);
        m_Spriteset->SetFrameIndex(current_direction, frame_number, current_image);
      }
    }

    if (m_Spriteset->GetNumFrames(current_direction) > 0)
      m_Spriteset->DeleteFrame(current_direction, m_Spriteset->GetNumFrames(current_direction) - 1);
  }

  UpdateMaxSizes(); 
  UpdateScrollBars();
  Invalidate();
  m_Handler->SV_SpritesetModified();
}

///////////////////////////////////////////////////////////////////////////////

#include <libmng.h>

const char* mng_get_error_message(mng_retcode code) {
  switch (code) {
    case (MNG_NOERROR):          return "er.. indicates all's well";    break;
    case (MNG_OUTOFMEMORY):      return "oops, buy some megabytes!";    break;
    case (MNG_INVALIDHANDLE):    return "call mng_initialize first";    break;
    case (MNG_NOCALLBACK):       return "set the callbacks please";     break;
    case (MNG_UNEXPECTEDEOF):    return "what'd ya do with the data?";  break;
    case (MNG_ZLIBERROR):        return "zlib burped";                  break;
    case (MNG_JPEGERROR):        return "jpglib complained";            break;
    case (MNG_LCMSERROR):        return "little cms stressed out";      break;
    case (MNG_NOOUTPUTPROFILE):  return "no output-profile defined";    break;
    case (MNG_NOSRGBPROFILE):    return "no sRGB-profile defined";      break;
    case (MNG_BUFOVERFLOW):      return "zlib output-buffer overflow";  break;
    case (MNG_FUNCTIONINVALID):  return "ay, totally inappropriate";    break;
    case (MNG_OUTPUTERROR):      return "disk full ?";                  break;
    case (MNG_JPEGBUFTOOSMALL):  return "can't handle buffer overflow"; break;
    case (MNG_NEEDMOREDATA):     return "I'm hungry, give me more";     break;
    case (MNG_NEEDTIMERWAIT):    return "Sleep a while then wake me";   break;
    case (MNG_NEEDSECTIONWAIT):  return "just processed a SEEK";        break;
    case (MNG_APPIOERROR):       return "application I/O error";        break;
    case (MNG_APPTIMERERROR):    return "application timing error";     break;
    case (MNG_APPCMSERROR):      return "application CMS error";        break;
    case (MNG_APPMISCERROR):     return "application other error";      break;
    case (MNG_APPTRACEABORT):    return "application aborts on trace";  break;
    case (MNG_INTERNALERROR):    return "internal inconsistancy";       break;
    case (MNG_INVALIDSIG):       return "invalid graphics file";        break;
    case (MNG_INVALIDCRC):       return "crc check failed";             break;
    case (MNG_INVALIDLENGTH):    return "chunklength mystifies me";     break;
    case (MNG_SEQUENCEERROR):    return "invalid chunk sequence";       break;
    case (MNG_CHUNKNOTALLOWED):  return "completely out-of-place";      break;
    case (MNG_MULTIPLEERROR):    return "only one occurence allowed";   break;
    case (MNG_PLTEMISSING):      return "indexed-color requires PLTE";  break;
    case (MNG_IDATMISSING):      return "IHDR-block requires IDAT";     break;
    case (MNG_CANNOTBEEMPTY):    return "must contain some data";       break;
    case (MNG_GLOBALLENGTHERR):  return "global data incorrect";        break;
    case (MNG_INVALIDBITDEPTH):  return "bitdepth out-of-range";        break;
    case (MNG_INVALIDCOLORTYPE): return "colortype out-of-range";       break;
    case (MNG_INVALIDCOMPRESS):  return "compression method invalid";   break;
    case (MNG_INVALIDFILTER):    return "filter method invalid";        break;
    case (MNG_INVALIDINTERLACE): return "interlace method invalid";     break;
    case (MNG_NOTENOUGHIDAT):    return "ran out of compressed data";   break;
    case (MNG_PLTEINDEXERROR):   return "palette-index out-of-range";   break;
    case (MNG_NULLNOTFOUND):     return "couldn't find null-separator"; break;
    case (MNG_KEYWORDNULL):      return "keyword cannot be empty";      break;
    case (MNG_OBJECTUNKNOWN):    return "the object can't be found";    break;
    case (MNG_OBJECTEXISTS):     return "the object already exists";    break;
    case (MNG_TOOMUCHIDAT):      return "got too much compressed data"; break;
    case (MNG_INVSAMPLEDEPTH):   return "sampledepth out-of-range";     break;
    case (MNG_INVOFFSETSIZE):    return "invalid offset-size";          break;
    case (MNG_INVENTRYTYPE):     return "invalid entry-type";           break;
    case (MNG_ENDWITHNULL):      return "may not end with NULL";        break;
    case (MNG_INVIMAGETYPE):     return "invalid image_type";           break;
    case (MNG_INVDELTATYPE):     return "invalid delta_type";           break;
    case (MNG_INVALIDINDEX):     return "index-value invalid";          break;
    case (MNG_TOOMUCHJDAT):      return "got too much compressed data"; break;
    case (MNG_JPEGPARMSERR):     return "JHDR/JPEG parms do not match"; break;
    case (MNG_INVFILLMETHOD):    return "invalid fill_method";          break;
    case (MNG_OBJNOTCONCRETE):   return "object must be concrete";      break;
    case (MNG_TARGETNOALPHA):    return "object has no alpha-channel";  break;
    case (MNG_MNGTOOCOMPLEX):    return "can't handle complexity";      break;
    case (MNG_UNKNOWNCRITICAL):  return "unknown critical chunk found"; break;
    case (MNG_UNSUPPORTEDNEED):  return "nEED requirement unsupported"; break;
    case (MNG_INVALIDDELTA):     return "Delta operation illegal";      break;
    case (MNG_INVALIDMETHOD):    return "invalid MAGN method";          break;
    case (MNG_INVALIDCNVSTYLE):  return "can't make anything of this";  break;
    case (MNG_WRONGCHUNK):       return "accessing the wrong chunk";    break;
    case (MNG_INVALIDENTRYIX):   return "accessing the wrong entry";    break;
    case (MNG_NOHEADER):         return "must have had header first";   break;
    case (MNG_NOCORRCHUNK):      return "can't find parent chunk";      break;
    case (MNG_NOMHDR):           return "no MNG header available";      break;
    case (MNG_IMAGETOOLARGE):    return "input-image way too big";      break;
    case (MNG_NOTANANIMATION):   return "file not a MNG";               break;
    case (MNG_FRAMENRTOOHIGH):   return "frame-nr out-of-range";        break;
    case (MNG_LAYERNRTOOHIGH):   return "layer-nr out-of-range";        break;
    case (MNG_PLAYTIMETOOHIGH):  return "playtime out-of-range";        break;
    case (MNG_FNNOTIMPLEMENTED): return "function not yet available";   break;
    case (MNG_IMAGEFROZEN):      return "stopped displaying";           break;
  }
  return "Unknown error code";
}

///////////////////////////////////////////////////////////

typedef struct userdata {
  FILE* file;
  char filename[MAX_PATH];
} userdata;

typedef userdata* userdatap;

mng_ptr MNG_DECL mng_alloc (mng_size_t iLen) { return (mng_ptr)calloc (1, iLen); }
void MNG_DECL mng_free (mng_ptr pPtr, mng_size_t iLen) { if (iLen) free (pPtr); }

mng_bool MNG_DECL
mng_write_stream (mng_handle mng, mng_ptr buffer, mng_uint32 size, mng_uint32p iWritten)
{
	userdatap userdata = (userdatap) mng_get_userdata(mng);
	*iWritten = fwrite(buffer, 1, size, userdata->file);
	return MNG_TRUE;
}

mng_bool MNG_DECL mng_open_stream (mng_handle mng)
{
  userdatap userdata = (userdatap) mng_get_userdata(mng);
  userdata->file = fopen(userdata->filename, "wb+");
  return (userdata->file) ? MNG_TRUE : MNG_FALSE;
}

mng_bool MNG_DECL mng_close_stream (mng_handle mng)
{
  userdatap userdata = (userdatap) mng_get_userdata(mng);
  if (userdata->file)
    fclose(userdata->file);
	return MNG_TRUE;
}

mng_bool MNG_DECL mng_trace(mng_handle  mng,
                            mng_int32   iFuncnr,
                            mng_int32   iFuncseq,
                            mng_pchar   zFuncname) {
  /*
  FILE* file = NULL;
  static bool once = true;
  file = fopen("c:\\windows\\desktop\\mnglog.txt", once ? "wb+" : "a+");
  once = false;
  if (file) {
    fclose(file);
  }
  */
  return MNG_TRUE;
}

///////////////////////////////////////////////////////////

void rgba_image_add_filter_byte(const RGBA* pixels, int sx, int sy, int sw, int sh, const int width, const int height, unsigned char* filtered)
{
  unsigned char* ptr = filtered;
  int x;
  int y;

  for (y = sy; y < sy + sh; y++) {
    for (x = sx; x < sx + sw; x++)
    {
      if (x == sx) {
  			*ptr++ = 0;
      }

      *ptr++ = pixels[(y * width) + x].red;
      *ptr++ = pixels[(y * width) + x].green;
  	  *ptr++ = pixels[(y * width) + x].blue;
      *ptr++ = pixels[(y * width) + x].alpha;
		}
	}
}

void rgb_image_add_filter_byte(const RGBA* pixels,
                               const int sx, const int sy,
                               const int sw, const int sh,
                               const int width, const int height,
                               unsigned char* filtered)
{
  int x;
	int y;
  unsigned char* ptr = filtered;

  for (y = sy; y < sy + sh; y++) {
    for (x = sx; x < sx + sw; x++)
    {
      if (x == sx) {
  			*ptr++ = 0;
      }

      *ptr++ = pixels[(y * width) + x].red;
      *ptr++ = pixels[(y * width) + x].green;
  	  *ptr++ = pixels[(y * width) + x].blue;
		}
	}
}

void gray_image_add_filter_byte(const RGBA* pixels,
                               const int sx, const int sy,
                               const int sw, const int sh,
                               const int width, const int height,
                               unsigned char* filtered)
{
  int x;
	int y;
  unsigned char* ptr = filtered;

  for (y = sy; y < sy + sh; y++) {
    for (x = sx; x < sx + sw; x++)
    {
      if (x == sx) {
  			*ptr++ = 0;
      }
  	  *ptr++ = pixels[(y * width) + x].red;
		}
	}
}

void grayalpha_image_add_filter_byte(const RGBA* pixels,
                               const int sx, const int sy,
                               const int sw, const int sh,
                               const int width, const int height,
                               unsigned char* filtered)
{
  int x;
	int y;
  unsigned char* ptr = filtered;

  for (y = sy; y < sy + sh; y++) {
    for (x = sx; x < sx + sw; x++)
    {
      if (x == sx) {
  			*ptr++ = 0;
      }
  	  *ptr++ = pixels[(y * width) + x].red;
  	  *ptr++ = pixels[(y * width) + x].alpha;
		}
	}
}


int findcolorfrompalette(const mng_palette8 palette,
                         int palette_size, int red, int green, int blue) {
  for (int i = 0; i < palette_size; i++) {
    if (palette[i].iRed   == red
     && palette[i].iGreen == green
     && palette[i].iBlue  == blue)
       return i;
  }

  return -1;
}

void rgb_palette_image_add_filter_byte(const mng_palette8 palette, int palette_size,
                                       const RGBA* pixels,
                                       const int sx, const int sy, const int sw, const int sh,
                                       const int width, const int height,
                                       unsigned char* filtered)
{
  int x;
	int y;
  unsigned char* ptr = filtered;

  for (y = sy; y < sy + sh; y++) {
    for (x = sx; x < sx + sw; x++)
    {
      if (x == sx) {
  			*ptr++ = 0;
      }

      const RGBA* p = &pixels[y * width + x];
      *ptr++ = findcolorfrompalette(palette, palette_size, p->red, p->green, p->blue);
		}
	}
}

///////////////////////////////////////////////////////////

bool image_to_palette(mng_palette8 palette, int* palette_size,
                      const int width, const int height, const RGBA* pixels) {
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++)
    {
      const RGBA* p = &pixels[y * width + x];
      int index = findcolorfrompalette(palette, *palette_size, p->red, p->green, p->blue);
      if (index == -1) {
        if (*palette_size <= 255) {
          palette[*palette_size].iRed   = p->red;
          palette[*palette_size].iGreen = p->green;
          palette[*palette_size].iBlue  = p->blue;
          *palette_size += 1;
        }
        else
          return false;
      }
    }
  }

  return true;
}

///////////////////////////////////////////////////////////

void calc_different_area(const RGBA* one, const RGBA* two, const int width, const int height, int* x, int* y, int* w, int* h)
{
  int done;
  *x = 0;
  *y = 0;
  *w = width;
  *h = height;

  done = 0;
  for (int py = 0; !done && py < height; py++) {
    for (int px = 0; px < width; px++) {
      const RGBA* p1 = &one[py * width + px];
      const RGBA* p2 = &two[py * width + px];

      if (p1->red   != p2->red
       || p1->green != p2->green
       || p1->blue  != p2->blue
       || p1->alpha != p2->alpha) {
         done = 1;
         break;
      }
    }
    if (!done)
      *y += 1;
  }

  done = 0;
  for (int px = 0; !done && px < width; px++) {
    for (int py = 0; py < height; py++) {
      const RGBA* p1 = &one[py * width + px];
      const RGBA* p2 = &two[py * width + px];

      if (p1->red   != p2->red
       || p1->green != p2->green
       || p1->blue  != p2->blue
       || p1->alpha != p2->alpha) {
        done = 1;
        break;
      }
    }
    if (!done)
      *x += 1;
  }

  *w = width - *x;
  *h = height - *y;

  done = 0;
  for (int px = width - 1; !done && px > *x; px--) {
    for (int py = height - 1; py > *y; py--) {
      const RGBA* p1 = &one[py * width + px];
      const RGBA* p2 = &two[py * width + px];

      if (p1->red   != p2->red
       || p1->green != p2->green
       || p1->blue  != p2->blue
       || p1->alpha != p2->alpha) {
        done = 1;
        break;
      }
    }
    if (!done)
      *w -= 1;
  }

  done = 0;
  for (int py = height - 1; !done && py > *y; py--) {
    for (int px = width - 1; px > *x; px--) {
      const RGBA* p1 = &one[py * width + px];
      const RGBA* p2 = &two[py * width + px];

      if (p1->red   != p2->red
       || p1->green != p2->green
       || p1->blue  != p2->blue
       || p1->alpha != p2->alpha) {
        done = 1;
        break;
      }
    }
    if (!done)
      *h -= 1;
  }

}


/*
mng_retcode mng_putjpgimage(mng_handle hMNG, const RGBA* pixels, const int width, const int height,
                            const int x, const int y, const int w, const int h) {

  mng_retcode iRC = mng_putchunk_jhdr(hMNG,
                                      w, // mng_uint32       iWidth,
                                      h, // mng_uint32       iHeight,
                                      MNG_COLORTYPE_JPEGCOLOR,   // mng_uint8        iColortype,
                                      MNG_BITDEPTH_JPEG8,        // mng_uint8        iImagesampledepth,
                                      MNG_COMPRESSION_BASELINEJPEG,   // mng_uint8        iImagecompression,
                                      0, // MNG_INTERLACE_SEQUENTIAL,  // mng_uint8        iImageinterlace,
                                      0, // MNG_BITDEPTH_JPEG8,        // mng_uint8        iAlphasampledepth,
                                      0, // MNG_COMPRESSION_DEFLATE,   // mng_uint8        iAlphacompression,
                                      0, // MNG_FILTER_ADAPTIVE,       // mng_uint8        iAlphafilter,
                                      0); // MNG_INTERLACE_SEQUENTIAL); // mng_uint8        iAlphainterlace);
  if (iRC != 0)
    return iRC;

  int pixel_size = sizeof(RGB);

  if (1) {
    mng_memalloc __mng_alloc__ = mng_getcb_memalloc(hMNG);
    mng_memfree  __mng_free__  = mng_getcb_memfree(hMNG);
    
    unsigned char* buffer;
    unsigned char* compressed;

    mng_uint32 filter_len     = (pixel_size * w * h) + h;
    mng_uint32 compressed_len = (pixel_size * w * h) + h;
               compressed_len += compressed_len / 100 + 12 + 8;	// extra 8 for safety

    buffer = (unsigned char*) __mng_alloc__(filter_len);
    if (buffer == NULL)
      return MNG_OUTOFMEMORY;

    compressed = (unsigned char*) __mng_alloc__(compressed_len);
    if (compressed == NULL) {
      __mng_free__(buffer, filter_len);
      return MNG_OUTOFMEMORY;
    }

    rgb_image_add_filter_byte(pixels, x, y, w, h, width, height, buffer);

    uLong dstLen = compressed_len;
    uLong srcLen = filter_len;
    if (compress2(compressed, &dstLen, buffer, srcLen, 9) != Z_OK) {
      __mng_free__(buffer, filter_len);
      __mng_free__(compressed, compressed_len);
      return MNG_ZLIBERROR;
    } 

    iRC = mng_putchunk_idat(hMNG, dstLen, compressed);

    __mng_free__(buffer, filter_len);
    __mng_free__(compressed, compressed_len);

    if (iRC != 0) return iRC;
  }

  if (iRC != 0)
    return iRC;

  iRC = mng_putchunk_iend(hMNG);

  return iRC;
}
*/

static bool image_has_alpha(const RGBA* pixels, const int width, const int height,
                         const int x, const int y, const int w, const int h) {
  bool has_alpha = false;
  for (int j = y; j < y + h; j++) {
    for (int i = x; i < x + w; i++) {
      if (pixels[j * width + i].alpha != 255) {
        has_alpha = true;
        break;
      }
    }
  }
  return has_alpha;
}

static bool image_is_grayscale(const RGBA* pixels, const int width, const int height,
                         const int x, const int y, const int w, const int h) {
  bool grayscale = true;
  for (int j = y; j < y + h; j++) {
    for (int i = x; i < x + w; i++) {
      if (pixels[j * width + i].red != pixels[j * width + i].green
       && pixels[j * width + i].green != pixels[j * width + i].blue) {
        grayscale = false;
        break;
      }
    }
  }
  return grayscale;
}

/**
 * palette_size of -1 for no palette
 */
mng_retcode mng_putpngimage(mng_handle hMNG, const RGBA* pixels, const int width, const int height,
                         const int x, const int y, const int w, const int h,
                         mng_palette8 palette, const int palette_size, bool global_palette)
{
  mng_retcode iRC;
  int pixel_size = palette_size == -1 ? sizeof(RGBA) : sizeof(RGB);
  bool grayscale = image_is_grayscale(pixels, width, height, x, y, w, h);
  bool has_alpha = image_has_alpha(pixels, width, height, x, y, w, h);

  /*
  // If the number of colors could be MNG_BITDEPTH_1, MNG_BITDEPTH_2 or MNG_BITDEPTH_4
  // we could probably fit more colors per pixel, but I only support MNG_BITDEPTH_8 for now
  unsigned long num_colors = CountColorsUsed(pixels, width, height, x, y, w, h);
  mng_int8 bit_depth = MNG_BITDEPTH_8; 

  if (num_colors != 0) {
    if (num_colors <= 16) {
      bit_depth = MNG_BITDEPTH_4;
    }
    if (num_colors <= 4) {
//      bit_depth = MNG_BITDEPTH_2;
    }
    if (num_colors <= 2) {
//      bit_depth = MNG_BITDEPTH_1;
    }
  }
  */

  if (grayscale) {
    pixel_size = has_alpha ? 2 : 1;
  }
 
  if (pixel_size == sizeof(RGBA) && !has_alpha) {
    pixel_size = sizeof(RGB);
  }

  if (pixel_size == sizeof(RGBA)) {
    iRC = mng_putchunk_ihdr (hMNG, w, h,
	      		MNG_BITDEPTH_8, MNG_COLORTYPE_RGBA, MNG_COMPRESSION_DEFLATE,
		      	MNG_FILTER_NONE, MNG_INTERLACE_NONE);
    if (iRC != 0) return iRC;
  }
  else
  if (pixel_size == sizeof(RGB) && palette_size != -1) {
    iRC = mng_putchunk_ihdr (hMNG, w, h,
					  MNG_BITDEPTH_8, MNG_COLORTYPE_INDEXED, MNG_COMPRESSION_DEFLATE,
					  MNG_FILTER_ADAPTIVE, MNG_INTERLACE_NONE);
    if (iRC != 0) return iRC;

    iRC = mng_putchunk_plte (hMNG, global_palette ? 0 : palette_size, palette);
    if (iRC != 0) return iRC;

    //RGBA palettewithalpha[256];
    //int palettewithalpha_size = 0;
    //if (image_to_palettewithalpha(palettewithalpha, &palettewithalpha_size, width, height, pixels)) {

      if (has_alpha) {
        mng_uint8arr alpha_palette;
        int alpha_palette_size = 1;
        for (int i = 0; i < alpha_palette_size; i++) {
          alpha_palette[i] = 255;
        }

        mng_uint8arr raw_data;

        // write transparency stuff
        mng_putchunk_trns(hMNG,
                        MNG_FALSE,
                        MNG_FALSE,
                        0,
                        alpha_palette_size, // mng_uint32       iCount,
                        alpha_palette, //  mng_uint8arr     aAlphas,
                        0, // mng_uint16       iGray,
                        0, // mng_uint16       iRed,
                        0, // mng_uint16       iGreen,
                        0, // mng_uint16       iBlue,
                        0, // mng_uint32       iRawlen,
                        raw_data); // mng_uint8arr     aRawdata);
        if (iRC != 0) return iRC;
//      }
    }
  
  }
  else
  if (pixel_size == sizeof(RGB)) {
    iRC = mng_putchunk_ihdr (hMNG, w, h,
	        	MNG_BITDEPTH_8, MNG_COLORTYPE_RGB, MNG_COMPRESSION_DEFLATE,
	      	  MNG_FILTER_NONE, MNG_INTERLACE_NONE);
    if (iRC != 0) return iRC;
  }
  else
  if (grayscale) {
    if (has_alpha) {
      iRC = mng_putchunk_ihdr(hMNG, w, h, 
              MNG_BITDEPTH_8, MNG_COLORTYPE_GRAYA, MNG_COMPRESSION_DEFLATE,
              MNG_FILTER_NONE, MNG_INTERLACE_NONE);
      if (iRC != 0) return iRC;
    }
    else {
      iRC = mng_putchunk_ihdr(hMNG, w, h, 
              MNG_BITDEPTH_8, MNG_COLORTYPE_GRAY, MNG_COMPRESSION_DEFLATE,
              MNG_FILTER_NONE, MNG_INTERLACE_NONE);
      if (iRC != 0) return iRC;
    }
  }
  
  if (1) {
    mng_memalloc __mng_alloc__ = mng_getcb_memalloc(hMNG);
    mng_memfree  __mng_free__  = mng_getcb_memfree(hMNG);
    
    unsigned char* buffer;
    unsigned char* compressed;

    mng_uint32 filter_len     = (pixel_size * w * h) + h;
    mng_uint32 compressed_len = (pixel_size * w * h) + h;
               compressed_len += compressed_len / 100 + 12 + 8;	// extra 8 for safety

    buffer = (unsigned char*) __mng_alloc__(filter_len);
    if (buffer == NULL)
      return MNG_OUTOFMEMORY;

    compressed = (unsigned char*) __mng_alloc__(compressed_len);
    if (compressed == NULL) {
      __mng_free__(buffer, filter_len);
      return MNG_OUTOFMEMORY;
    }

    if (pixel_size == sizeof(RGBA))
      rgba_image_add_filter_byte(pixels, x, y, w, h, width, height, buffer);
    else
    if (pixel_size == sizeof(RGB) && palette_size != -1) {
//      RGBA palette[256];
//      int palettewithalpha_size = 0;
//      if (image_to_palettewithalpha(palette, &palettewithalpha_size, width, height, pixels)) {
      rgb_palette_image_add_filter_byte(palette, palette_size, pixels, x, y, w, h, width, height, buffer);
//        rgb_palette_with_alpha_image_add_filter_byte(palette, palettewithalpha_size, pixels, x, y, w, h, width, height, buffer);
//      }
    }
    else
    if (pixel_size == sizeof(RGB))
      rgb_image_add_filter_byte(pixels, x, y, w, h, width, height, buffer);
    else
    if (grayscale) {
      if (has_alpha) {
        grayalpha_image_add_filter_byte(pixels, x, y, w, h, width, height, buffer);
      }
      else {
        gray_image_add_filter_byte(pixels, x, y, w, h, width, height, buffer);
      }
    }

    uLong dstLen = compressed_len;
    uLong srcLen = filter_len;
    if (compress2(compressed, &dstLen, buffer, srcLen, 9) != Z_OK) {
      __mng_free__(buffer, filter_len);
      __mng_free__(compressed, compressed_len);
      return MNG_ZLIBERROR;
    } 

    iRC = mng_putchunk_idat(hMNG, dstLen, compressed);

    __mng_free__(buffer, filter_len);
    __mng_free__(compressed, compressed_len);

    if (iRC != 0) return iRC;
  }

  iRC = mng_putchunk_iend (hMNG);
  if (iRC != 0)
    return iRC;

  return iRC;
}


typedef bool (*GetImage)(int index, CImage32&, void*);
typedef mng_uint32  (*GetDelay)(int index, void*);
typedef bool (*ContinueProcessingImages)(int, int);

#include "Editor.hpp"

bool ContinueProcessing(int index, int total) {
  bool ret = true;

  if (1) {
    char string[255] = {0};
    if (index == -1) {
      sprintf (string, "%d...", total);
    }
    else {
      int percent = (int)( ((double)index / (double)total) * 100);
      sprintf (string, "%3d%% Complete", percent);
    }
    GetStatusBar()->SetWindowText(string);
  }

  /*
  MSG msg;
  int count = 0;
  while (count++ < 4 && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
  {
    if (msg.message != WM_QUIT) {
      DispatchMessage(&msg);
    }
    else {
      ret = false;
      break;
    }
  }
  */

  return ret;
}

struct userwritedata {
  sSpriteset spriteset;
  int direction;
};

bool GetImageFromSpriteset(int index, CImage32& image, void* data) {
  userwritedata* s = (userwritedata*) data;
  if (index < 0 || index >= s->spriteset.GetNumFrames(s->direction))
    return false;
  image = s->spriteset.GetImage(s->spriteset.GetFrameIndex(s->direction, index));
  return true;
}

bool GetNextImageFromFileList(int index, CImage32& image, void* data) {
  // std::vector<CImage32>& images = (std::vector<CImage32>&) data;
  std::vector<std::string>* filelist = (std::vector<std::string>*) data;
  if (index < 0 || index >= filelist->size())
    return false;

  return image.Load((*filelist)[index].c_str());
}

mng_uint32 GetDelayFromImageFileList(int index, void* data) {
  return 10 * 1000;
}

mng_uint32 GetDelayFromSpriteset(int index, void* data) {
  userwritedata* s = (userwritedata*) data;
  return 10 * s->spriteset.GetFrameDelay(s->direction, index);
  // spritesets use a frame rate and this is converting it into a sort of hashed time based system
}

///////////////////////////////////////////////////////////////////////////////

static mng_retcode
____SaveMNGAnimationFromImages____(mng_handle hMNG,
                           GetImage get_image,
                           GetDelay get_delay,
                           ContinueProcessingImages should_continue,
                           void* data) {
  int max_frame_width = 0;
  int max_frame_height = 0;

  //if (!(images.size() >= 1))
  //  return -1;

  /*
  for (int i = 0; i < images.size(); i++) {
    if (max_frame_width < images[i].GetWidth())
      max_frame_width = images[i].GetWidth();
    if (max_frame_height < images[i].GetHeight())
      max_frame_height = images[i].GetHeight();
  }
  */

  mng_palette8 GlobalPalette;
  int GlobalPaletteSize = 0;
  bool can_use_global_palette = true;

  int max_images = -1;
  int num_images = 0;

  mng_uint32 totaldelay = 0;
  bool playtime_known = true;

  CImage32 __temp__;
  while (get_image(num_images, __temp__, data) == true && (max_images == -1 || num_images < max_images)) {

    if (max_frame_width < __temp__.GetWidth())
      max_frame_width = __temp__.GetWidth();
    if (max_frame_height < __temp__.GetHeight())
      max_frame_height = __temp__.GetHeight();

    if (playtime_known) {
      mng_uint32 delay = get_delay(num_images, data);
      if (delay && (delay + totaldelay > totaldelay)) {
        totaldelay += delay;
      }
      else {
        playtime_known = false;
        totaldelay = 0;
      }
    }

    if (can_use_global_palette
     && !image_to_palette(GlobalPalette, &GlobalPaletteSize, __temp__.GetWidth(), __temp__.GetHeight(), __temp__.GetPixels())) {
      can_use_global_palette = false;
    }
    num_images++;

    if (!should_continue(-1, num_images))
      return -1;

  }

  if (max_frame_width <= 0 || max_frame_height <= 0) {
    return -1;
  }

  mng_retcode iRC = mng_setcb_writedata(hMNG, mng_write_stream);
  if (iRC != 0) return iRC;
  iRC = mng_setcb_openstream(hMNG, mng_open_stream);
	if (iRC != 0) return iRC;
  iRC = mng_setcb_closestream(hMNG, mng_close_stream);
  if (iRC != 0) return iRC;
  
  iRC = mng_create (hMNG);
  if (iRC != 0) return iRC;
 
  iRC = mng_putchunk_mhdr (hMNG, max_frame_width, max_frame_height,
          1000, 0, num_images, totaldelay, MNG_SIMPLICITY_TRANSPARENCY);

  if (iRC != 0) return iRC;

  bool repeating = true;
  mng_uint32 repeat_count = (repeating) ? 0x7fffffff : 0;
  mng_putchunk_term(hMNG, MNG_TERMACTION_REPEAT, MNG_ITERACTION_FIRSTFRAME, 0, repeat_count);
  if (iRC != 0) return iRC;

  const char* software = "Sphere - http://sphere.sf.net/";
  iRC = mng_putchunk_text(hMNG,
                           strlen(MNG_TEXT_SOFTWARE), MNG_TEXT_SOFTWARE,
                           strlen(software), (char*) software);
  if (iRC != 0) return iRC;

  iRC = mng_putchunk_text(hMNG,
                          strlen("Version"), "Version",
                          strlen(SPHERE_VERSION), SPHERE_VERSION);
  if (iRC != 0) return iRC;

#if 1
  char text[100] = {0};
  sprintf (text, "%s", repeating ? "Infinite" : "Once");
  iRC = mng_putchunk_text(hMNG,
                          strlen("RepeatType"), "RepeatType",
                          strlen(text), text);
  if (iRC != 0) return iRC;
  sprintf (text, "%d", repeating ? 1 : repeat_count);
  iRC = mng_putchunk_text(hMNG,
                          strlen("RepeatCount"), "RepeatCount",
                          strlen(text), text);
  if (iRC != 0) return iRC;
  iRC = mng_putchunk_text(hMNG,
                          strlen("BuildDate"), "BuildDate",
                          strlen(__DATE__), __DATE__);
  if (iRC != 0) return iRC;
  iRC = mng_putchunk_text(hMNG,
                          strlen("BuildTime"), "BuildTime",
                          strlen(__TIME__), __TIME__);
  if (iRC != 0) return iRC;
  
  iRC = mng_putchunk_text(hMNG, strlen("libmng_version"), "libmng_version",
                                strlen(MNG_VERSION_TEXT), MNG_VERSION_TEXT);
  if (iRC != 0) return iRC;
#endif

  iRC = mng_set_srgb(hMNG, true);
  if (iRC != 0) return iRC;

  //iRC = mng_putchunk_back (hMNG, 255, 255, 255, 0, 0, MNG_BACKGROUNDIMAGE_NOTILE);
  //if (iRC != 0) return iRC;

  iRC = mng_putchunk_fram  (hMNG,  MNG_FALSE,  MNG_FRAMINGMODE_3,
                            0,  NULL,  MNG_CHANGEDELAY_DEFAULT, 0,  0, 0, 1000, 0,  0,  0,  0, 0, 0, 0, 0);
  if (iRC != 0) return iRC;

  if (can_use_global_palette) {
    iRC = mng_putchunk_plte(hMNG, GlobalPaletteSize, GlobalPalette);
    if (iRC != 0) return iRC;
  }

  CImage32 image;
  CImage32 last;

  int i = 0;
  
  while (get_image(i, image, data) && (max_images == -1 || i < max_images))
  {
    int x = 0, y = 0, w = image.GetWidth(), h = image.GetHeight();

    if (i > 0) {
      if (last.GetWidth() == image.GetWidth()
       && last.GetHeight() == image.GetHeight())
      {
        calc_different_area(last.GetPixels(), image.GetPixels(), image.GetWidth(), image.GetHeight(), &x, &y, &w, &h);

        iRC = mng_putchunk_fram(hMNG,  MNG_FALSE,  MNG_FRAMINGMODE_3,
                            0,  NULL,  MNG_CHANGEDELAY_NEXTSUBFRAME, MNG_CHANGETIMOUT_NO,
                            MNG_CHANGECLIPPING_NO, MNG_CHANGESYNCID_NO,
                            get_delay(i, data), 0,  0,  
                            0,  0, 0, 0,
                            0, 0);
        mng_putchunk_move(hMNG, 0, 0, 0, 0, 0);

        if (x != 0 || y != 0 || w != image.GetWidth() || h != image.GetHeight()) {
          iRC = mng_putchunk_fram(hMNG,  MNG_FALSE,  MNG_FRAMINGMODE_4,
                          0,  NULL,  MNG_CHANGEDELAY_NO, MNG_CHANGETIMOUT_NO,
                          MNG_CHANGECLIPPING_NEXTSUBFRAME, MNG_CHANGESYNCID_NO,
                          0, 0,  0,
                          x,  x+w, y, y+h,
                          0, 0);
          if (iRC != 0) return iRC;
          iRC = mng_putchunk_move(hMNG, 0, 0, 0, x, y);
          if (iRC != 0) return iRC;
        }
      }
      else {
        iRC = mng_putchunk_fram(hMNG,  MNG_FALSE,  MNG_FRAMINGMODE_3,
                          0,  NULL,  MNG_CHANGEDELAY_NO, MNG_CHANGETIMOUT_NO,
                          MNG_CHANGECLIPPING_NO, MNG_CHANGESYNCID_NO,
                          0, 0,  0,  
                          0,  0, 0, 0,
                          0, 0);
        mng_putchunk_move(hMNG, 0, 0, 0, 0, 0);
      }
    }

    if (can_use_global_palette) {
      iRC = mng_putpngimage(hMNG, image.GetPixels(), image.GetWidth(), image.GetHeight(), x, y, w, h,
                 GlobalPalette, GlobalPaletteSize, true);
    }
    else {
      mng_palette8 palette;
      int palette_size = 0;
      if (image_to_palette(palette, &palette_size, image.GetWidth(), image.GetHeight(), image.GetPixels())) { 
        iRC = mng_putpngimage(hMNG, image.GetPixels(), image.GetWidth(), image.GetHeight(), x, y, w, h,
                 palette, palette_size, false); 
      }
      else {
        iRC = mng_putpngimage(hMNG, image.GetPixels(), image.GetWidth(), image.GetHeight(), x, y, w, h,
                 GlobalPalette, -1, false);
      }
    }

    if (iRC != 0)
      return iRC;

    i++;

    if (!should_continue(i, num_images))
      return -1;

    last = image;
  }

  iRC = mng_putchunk_mend (hMNG);
  if (iRC != 0) return iRC;

  iRC = mng_write(hMNG);
  if (iRC != 0) return iRC;

  return iRC;
}

///////////////////////////////////////////////////////////////////////////////

static mng_retcode
__SaveMNGAnimationFromImages__(mng_handle hMNG, const char* filename,
                           GetImage get_image,
                           GetDelay get_delay,
                           ContinueProcessingImages should_continue,
                           void* data)
{
  userdatap pMydata = (userdatap)calloc (1, sizeof (userdata));
  if (!pMydata)
    return -1;

  strcpy(pMydata->filename, filename);
  mng_set_userdata(hMNG, pMydata);

  mng_retcode iRC = ____SaveMNGAnimationFromImages____(hMNG,
                           get_image,
                           get_delay,
                           should_continue,
                           data);

  free(pMydata);
  pMydata = NULL;

  mng_set_userdata(hMNG, NULL);

  return iRC;
}

///////////////////////////////////////////////////////////////////////////////

static mng_retcode 
SaveMNGAnimationFromImages(const char* filename,
                           GetImage get_image,
                           GetDelay get_delay,
                           ContinueProcessingImages should_continue,
                           void* data) {

  mng_retcode iRC;

  mng_handle hMNG = mng_initialize (MNG_NULL, mng_alloc, mng_free, mng_trace);
  if (!hMNG)
    return -1;

  iRC = __SaveMNGAnimationFromImages__(hMNG, filename, get_image, get_delay, should_continue, data);

  if (iRC == 0)
    iRC = mng_cleanup(&hMNG);
  else
    mng_cleanup(&hMNG);

  return iRC;
}

///////////////////////////////////////////////////////////////////////////////

#include "../common/system.hpp"

mng_retcode TestAnimationCode() {
  std::vector<std::string> filelist = GetFileList("*");
  std::vector<std::string> imagefilelist;

  for (int i = 0; i < filelist.size(); i++) {
    //CImage32 image;
    const char* filename = filelist[i].c_str();
    if (filelist[i].rfind(".jpg") == strlen(filename) - 4) {
      imagefilelist.push_back(filename);
    }
    else {
    //if (image.Load(filename)) {
//      imagefilelist.push_back(filename);
    //}
    }
  }

  return SaveMNGAnimationFromImages("comics.mng", GetNextImageFromFileList, GetDelayFromImageFileList, ContinueProcessing, (void*) &imagefilelist);
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnExportDirectionAsAnimation()
{
  //SetCurrentDirectory("c:\\windows\\desktop\\8bit");
  //mng_retcode ret = TestAnimationCode();
  //MessageBox(mng_get_error_message(ret), "Export Comics As Animation", MB_OK);
  //return;

  CAnimationFileDialog dialog(FDM_SAVE, "Export Direction As Animation");
  if (dialog.DoModal() == IDOK) {

    /*
    std::vector<CImage32> images;
    for (int i = 0; i < m_Spriteset->GetNumFrames(m_CurrentDirection); i++) {
      images.push_back(m_Spriteset->GetImage(m_Spriteset->GetFrameIndex(m_CurrentDirection, i)));
    }
    */

    bool is_mng = strcmp_ci(dialog.GetFileExt(), "mng") == 0;
    bool is_fli = strcmp_ci(dialog.GetFileExt(), "flic") == 0
               || strcmp_ci(dialog.GetFileExt(), "flc")  == 0
               || strcmp_ci(dialog.GetFileExt(), "fli")  == 0;

    if (is_mng) {
      struct userwritedata data;
      data.spriteset = *m_Spriteset;
      data.direction = m_CurrentDirection;

      mng_retcode iRC = SaveMNGAnimationFromImages(dialog.GetPathName(), GetImageFromSpriteset, GetDelayFromSpriteset, ContinueProcessing, (void*) &data);
      if (iRC == 0) {
        MessageBox("Exported Animation!", "Export Direction As Animation", MB_OK);
      }
      else {
        MessageBox(mng_get_error_message(iRC), "Error Exporting Direction As Animation", MB_OK);
      } 
    }
    else
    if (is_fli) {
      MessageBox("Unsupported save mode", "Error Exporting Direction As Animation", MB_OK);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnExportDirectionAsImage()
{
  // get file name to export to
  CImageFileDialog dialog(FDM_SAVE, "Export Direction As Image");
  if (dialog.DoModal() == IDOK) {
    int image_width = 0;
    int image_height = 0;

    for (int i = 0; i < m_Spriteset->GetNumFrames(m_CurrentDirection); i++) {
      int height = m_Spriteset->GetImage(m_Spriteset->GetFrameIndex(m_CurrentDirection, i)).GetHeight();
      image_height = image_height > height ? image_height : height;

      image_width  += m_Spriteset->GetImage(m_Spriteset->GetFrameIndex(m_CurrentDirection, i)).GetWidth();
    }

    if (image_width <= 0 || image_height <= 0) {
      MessageBox("Nothing to export!", "Export Direction As Image", MB_OK);
      return;
    }

    CImage32 image(image_width, image_height);
    image.SetBlendMode(CImage32::BlendMode::REPLACE);

    int x = 0;
    int y = 0;

    for (int i = 0; i < m_Spriteset->GetNumFrames(m_CurrentDirection); ++i) {
      CImage32& frame = m_Spriteset->GetImage(m_Spriteset->GetFrameIndex(m_CurrentDirection, i));
      image.BlitImage(frame, x, y);
      x += frame.GetWidth();
    }

    if( !image.Save(dialog.GetPathName()) ) {
      MessageBox("Could not save image", "Export Direction As Image", MB_OK);
    } else {
      MessageBox("Exported direction!", "Export Direction As Image", MB_OK);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnEditRange()
{
  const unsigned int id = GetCurrentMessage()->wParam;
  if( EditRange::OnEditRange("spriteset", id, false, (void*) m_Spriteset, m_CurrentDirection) ) {
    m_Handler->SV_SpritesetModified();
    Invalidate();
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOL CSpritesetView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
	{
		// use document specific accelerator table over m_hAccelTable
		HACCEL hAccel = ((CFrameWnd*)AfxGetMainWnd())->GetDefaultAccelerator();
		return hAccel != NULL &&
		   ::TranslateAccelerator(m_hWnd, hAccel, pMsg);
	}

	return CWnd::PreTranslateMessage(pMsg);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg LRESULT 
CSpritesetView::OnGetAccelerator(WPARAM wParam, LPARAM lParam)
{
	// If you want to provide a custom accelerator, copy from CImageView::OnGetAccelerator!
	// Also a message map entry is needed like: 	
	//		ON_MESSAGE(WM_GETACCELERATOR, OnGetAccelerator)

	HACCEL * ret = ((HACCEL*)wParam);
	*ret = LoadAccelerators(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_SPRITESETVIEW));  
	return 1;
}

////////////////////////////////////////////////////////////////////////////////

