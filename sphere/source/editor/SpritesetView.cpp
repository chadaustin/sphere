#include "SpritesetView.hpp"
#include "ResizeDialog.hpp"
#include "SpritePropertiesDialog.hpp"
#include "StringDialog.hpp"
#include "FileDialogs.hpp"
#include "resource.h"


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
  ON_COMMAND(ID_SPRITESETVIEWDIRECTIONS_EXPORT_AS_IMAGE, OnExportDirectionAsImage)

  ON_COMMAND(ID_SPRITESETVIEWFRAMES_INSERT,     OnInsertFrame)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_DELETE,     OnDeleteFrame)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_APPEND,     OnAppendFrame)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_COPY,       OnCopyFrame)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_PASTE,      OnPasteFrame)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_PROPERTIES, OnFrameProperties)

  ON_COMMAND(ID_SPRITESETVIEWFRAMES_REPLACE_COLOR_WITH_COLOR, OnReplaceColorWithColor)

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
  Invalidate();
  UpdateScrollBars();
  m_Handler->SV_SpritesetModified();
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
CSpritesetView::OnFrameProperties()
{
  CSpritePropertiesDialog dialog(m_Spriteset, m_CurrentDirection, m_CurrentFrame);
  if (dialog.DoModal() == IDOK)
    m_Handler->SV_SpritesetModified();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CSpritesetView::OnReplaceColorWithColor()
{
  int start_frame = 0;
  int num_frames_to_change = 0;

  CNumberDialog startFrameDialog("Start frame index", "Value", 0, 0, m_Spriteset->GetNumFrames(m_CurrentDirection) - 1);
  if (startFrameDialog.DoModal() == IDOK) {

    start_frame = startFrameDialog.GetValue();
    CNumberDialog endFrameDialog("End frame index", "Value", start_frame, start_frame, m_Spriteset->GetNumFrames(m_CurrentDirection) - 1);

    if (endFrameDialog.DoModal() == IDOK) {
      num_frames_to_change = endFrameDialog.GetValue() - start_frame;

      CFontGradientDialog colorChoiceDialog;
      if (colorChoiceDialog.DoModal() == IDOK) {

        RGBA old_color = colorChoiceDialog.GetTopColor();
        RGBA replacement_color = colorChoiceDialog.GetBottomColor();

        for (int i = start_frame; i < start_frame + num_frames_to_change; i++) {
          // probably should find out if frame_index has been done already to increase speed
          int frame_index = m_Spriteset->GetFrameIndex(m_CurrentDirection, i);
          m_Spriteset->ReplaceIndexColorWithColor(frame_index, old_color, replacement_color);
        }

        m_Handler->SV_SpritesetModified();
        Invalidate();
      }
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
