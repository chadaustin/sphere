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
CSpritesetView::SetZoomFactor(int zoom)
{
  m_ZoomFactor = zoom;
	m_Handler->SV_ZoomFactorChanged(zoom);
  UpdateMaxSizes();
  Invalidate();
  UpdateScrollBars();
}

////////////////////////////////////////////////////////////////////////////////

int
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

    if (animation->GetWidth() != frame_width
     || animation->GetHeight() != frame_height) {
      char message[1000];
      sprintf(message, "Invalid animation width or height: %d %d",
        animation->GetWidth(), animation->GetHeight());
      MessageBox(message);
      return;
    }

    RGBA* pixels = new RGBA[animation->GetWidth() * animation->GetHeight()];
    if ( !pixels )
      return;

    int current_direction = m_CurrentDirection;
    m_Spriteset->InsertDirection(current_direction);
    m_Spriteset->SetDirectionName(current_direction, dialog.GetFileName());

    int max_frames = animation->GetNumFrames() == 0 ? 255 : animation->GetNumFrames();

    for (int frame_number = 0; frame_number < max_frames; frame_number++) {
      int delay = animation->GetDelay();
      
      if (animation->ReadNextFrame((RGBA*) pixels) == false)
        break;

      if (animation->IsEndOfAnimation())
        break;

      int current_image = -1;

      if (1) {
        for (int i = 0; i < m_Spriteset->GetNumImages(); i++) {
          if (animation->GetWidth()  == m_Spriteset->GetImage(i).GetWidth()
           && animation->GetHeight() == m_Spriteset->GetImage(i).GetHeight()) {
            if (memcmp(pixels, m_Spriteset->GetImage(i).GetPixels(), sizeof(RGBA) * animation->GetWidth() * animation->GetHeight()) == 0) {
              current_image = i;
              break;
            }
          }
        }
      }

      if (current_image == -1) {
        current_image = m_Spriteset->GetNumImages();
        m_Spriteset->InsertImage(current_image);

        if (m_Spriteset->GetNumImages() != current_image + 1) {
          delete[] pixels;
          return;
        }

        CImage32& image = m_Spriteset->GetImage(current_image);
        for (int sy = 0; (sy < image.GetHeight() && sy < animation->GetHeight()); sy++) {
          for (int sx = 0; (sx < image.GetWidth() && sx < animation->GetWidth()); sx++) {
            image.SetPixel(sx, sy, pixels[sy * animation->GetWidth() + sx]);
          }
        }
      }

      int __num_frames__ = m_Spriteset->GetNumFrames(current_direction) + 1;
      m_Spriteset->InsertFrame(current_direction, frame_number);
      if (m_Spriteset->GetNumFrames(current_direction) == __num_frames__) {
        m_Spriteset->SetFrameIndex(current_direction, frame_number, current_image);
        m_Spriteset->SetFrameDelay(current_direction, frame_number, delay);
      }
    }

    if (m_Spriteset->GetNumFrames(current_direction) > 0)
      m_Spriteset->DeleteFrame(current_direction, m_Spriteset->GetNumFrames(current_direction) - 1);

    delete[] pixels;
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
  char filename[1024];
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

///////////////////////////////////////////////////////////

void image_add_filter_byte(const RGBA* pixels, const int width, const int height, unsigned char* filtered)
{
  int x;
	int y;
  unsigned char* ptr = filtered;

  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++)
    {
      if (x == 0) {
  			*ptr++ = 0;
      }

      *ptr++ = pixels[(y * width) + x].red;
      *ptr++ = pixels[(y * width) + x].green;
  	  *ptr++ = pixels[(y * width) + x].blue;
      *ptr++ = pixels[(y * width) + x].alpha;
		}
	}
}

///////////////////////////////////////////////////////////

static mng_retcode
SaveMNGAnimationFromImages(const char* filename, const std::vector<CImage32>& images)
{
  int max_frame_width = 0;
  int max_frame_height = 0;

  if (!(images.size() >= 1))
    return -1;

  for (int i = 0; i < images.size(); i++) {
    if (max_frame_width < images[i].GetWidth())
      max_frame_width = images[i].GetWidth();
    if (max_frame_height < images[i].GetHeight())
      max_frame_height = images[i].GetHeight();
  }

  if (max_frame_width <= 0 || max_frame_height <= 0) {
    return -1;
  }

	mng_handle hMNG = mng_initialize (MNG_NULL, mng_alloc, mng_free, MNG_NULL);
  if (!hMNG)
    return -1;

  userdatap pMydata = (userdatap)calloc (1, sizeof (userdata));
  if (!pMydata)
    return -1;

  strcpy(pMydata->filename, filename);
  mng_set_userdata(hMNG, pMydata);

  mng_retcode iRC = 0;

  iRC = mng_setcb_writedata(hMNG, mng_write_stream);
  if (iRC != 0) return iRC;
	iRC = mng_setcb_openstream(hMNG, mng_open_stream);
	if (iRC != 0) return iRC;
  iRC = mng_setcb_closestream(hMNG, mng_close_stream);
  if (iRC != 0) return iRC;

  iRC = mng_create (hMNG);
  if (iRC != 0) return iRC;
 
  iRC = mng_putchunk_mhdr (hMNG, max_frame_width, max_frame_width,
          2, 0, images.size(), 0, MNG_SIMPLICITY_TRANSPARENCY);

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
                          strlen("REPEAT_TYPE"), "REPEAT_TYPE",
                          strlen(text), text);
  if (iRC != 0) return iRC;
  sprintf (text, "%d", repeating ? 1 : repeat_count);
  iRC = mng_putchunk_text(hMNG,
                          strlen("REPEAT_COUNT"), "REPEAT_COUNT",
                          strlen(text), text);
  if (iRC != 0) return iRC;
  iRC = mng_putchunk_text(hMNG,
                          strlen("DATE"), "DATE",
                          strlen(__DATE__), __DATE__);
  if (iRC != 0) return iRC;
  iRC = mng_putchunk_text(hMNG,
                          strlen("TIME"), "TIME",
                          strlen(__TIME__), __TIME__);
  if (iRC != 0) return iRC;
#endif

  iRC = mng_set_srgb(hMNG, true);
  if (iRC != 0) return iRC;

  //iRC = mng_putchunk_back (hMNG, 255, 255, 255, 0, 0, MNG_BACKGROUNDIMAGE_NOTILE);
  //if (iRC != 0) return iRC;

  iRC = mng_putchunk_fram  (hMNG,
                            MNG_FALSE,
                            MNG_FRAMINGMODE_3,
                            0,
                            NULL,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0);
  if (iRC != 0) return iRC;

  for (int i = 0; i < images.size(); i++) {
    const CImage32& image = images[i];

    /*
    iRC = mng_putchunk_defi(hMNG,
                            i,
                            MNG_DONOTSHOW_VISIBLE,
                            MNG_CONCRETE,
                            MNG_TRUE,
                            0,
                            0,
                            MNG_TRUE,
                            0,
                            image.GetWidth(),
                            0,
                            image.GetHeight());
    if (iRC != 0) return iRC;
    */

    iRC = mng_putchunk_ihdr (hMNG, image.GetWidth(), image.GetHeight(),
			MNG_BITDEPTH_8, MNG_COLORTYPE_RGBA, MNG_COMPRESSION_DEFLATE,
			MNG_FILTER_NONE, MNG_INTERLACE_NONE);
    if (iRC != 0) return iRC;

    mng_uint32 filter_len     = (sizeof(RGBA) * image.GetWidth() * image.GetHeight()) + image.GetHeight();
    mng_uint32 compressed_len = (sizeof(RGBA) * image.GetWidth() * image.GetHeight()) + image.GetHeight();
  	           compressed_len += compressed_len / 100 + 12 + 8;	// extra 8 for safety

    unsigned char* buffer = new unsigned char[filter_len];
    if (buffer == NULL)
      return MNG_OUTOFMEMORY;

     unsigned char* compressed = new unsigned char[compressed_len];
     if (compressed == NULL) {
       delete[] buffer;
       return MNG_OUTOFMEMORY;
     }

    image_add_filter_byte(image.GetPixels(), image.GetWidth(), image.GetHeight(), buffer);

    uLong dstLen = compressed_len;
    uLong srcLen = filter_len;
    if (compress2(compressed, &dstLen, buffer, srcLen, 9) != Z_OK) {
      delete[] buffer;
      delete[] compressed;
      return MNG_ZLIBERROR;
    }

    iRC = mng_putchunk_idat(hMNG, dstLen, compressed);

    delete[] buffer;
    delete[] compressed;

    if (iRC != 0) return iRC;

    iRC = mng_putchunk_iend (hMNG);
    if (iRC != 0) return iRC;  
  }

  iRC = mng_putchunk_mend (hMNG);
  if (iRC != 0) return iRC;

  iRC = mng_write(hMNG);
  if (iRC != 0) return iRC;

  iRC = mng_cleanup(&hMNG);

  return iRC;
}

///////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnExportDirectionAsAnimation()
{
  CAnimationFileDialog dialog(FDM_SAVE, "Export Direction As Animation");
  if (dialog.DoModal() == IDOK) {

    std::vector<CImage32> images;
    for (int i = 0; i < m_Spriteset->GetNumFrames(m_CurrentDirection); i++) {
      images.push_back(m_Spriteset->GetImage(m_Spriteset->GetFrameIndex(m_CurrentDirection, i)));
    }

    bool is_mng = strcmp_ci(dialog.GetFileExt(), "mng") == 0;
    bool is_fli = strcmp_ci(dialog.GetFileExt(), "flic") == 0
               || strcmp_ci(dialog.GetFileExt(), "flc")  == 0
               || strcmp_ci(dialog.GetFileExt(), "fli")  == 0;

    if (is_mng) {
      mng_retcode iRC = SaveMNGAnimationFromImages(dialog.GetPathName(), images);
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

