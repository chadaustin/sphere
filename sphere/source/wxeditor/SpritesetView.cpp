//#define WXWIN_COMPATIBILITY_EVENT_TYPES 1

#include "SpritesetView.hpp"
#include "ResizeDialog.hpp"
#include "SpritePropertiesDialog.hpp"
//#include "StringDialog.hpp"
//#include "resource.h"
#include "IDs.hpp"


#ifdef DrawText
#undef DrawText
#endif


#define LABEL_WIDTH 80


//static int s_SpritesetViewID = 100;

/*
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

  ON_COMMAND(ID_SPRITESETVIEWFRAMES_INSERT,     OnInsertFrame)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_DELETE,     OnDeleteFrame)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_APPEND,     OnAppendFrame)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_COPY,       OnCopyFrame)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_PASTE,      OnPasteFrame)
  ON_COMMAND(ID_SPRITESETVIEWFRAMES_PROPERTIES, OnFrameProperties)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wSpritesetView, wScrollWindow)
  EVT_SIZE(wSpritesetView::OnSize)
  EVT_LEFT_DOWN(wSpritesetView::OnLButtonDown)
  EVT_RIGHT_UP(wSpritesetView::OnRButtonUp)
  EVT_LEFT_DCLICK(wSpritesetView::OnLButtonDblClk)
  //EVT_SCROLL(wSpritesetView::OnScroll)
  EVT_PAINT(wSpritesetView::OnPaint)

  EVT_MENU(wID_SPRITESETVIEWDIRECTIONS_INSERT,     wSpritesetView::OnInsertDirection)
  EVT_MENU(wID_SPRITESETVIEWDIRECTIONS_DELETE,     wSpritesetView::OnDeleteDirection)
  EVT_MENU(wID_SPRITESETVIEWDIRECTIONS_APPEND,     wSpritesetView::OnAppendDirection)
  EVT_MENU(wID_SPRITESETVIEWDIRECTIONS_PROPERTIES, wSpritesetView::OnDirectionProperties)

  EVT_MENU(wID_SPRITESETVIEWFRAMES_INSERT,     wSpritesetView::OnInsertFrame)
  EVT_MENU(wID_SPRITESETVIEWFRAMES_DELETE,     wSpritesetView::OnDeleteFrame)
  EVT_MENU(wID_SPRITESETVIEWFRAMES_APPEND,     wSpritesetView::OnAppendFrame)
  EVT_MENU(wID_SPRITESETVIEWFRAMES_COPY,       wSpritesetView::OnCopyFrame)
  EVT_MENU(wID_SPRITESETVIEWFRAMES_PASTE,      wSpritesetView::OnPasteFrame)
  EVT_MENU(wID_SPRITESETVIEWFRAMES_PROPERTIES, wSpritesetView::OnFrameProperties)
END_EVENT_TABLE()


////////////////////////////////////////////////////////////////////////////////

wSpritesetView::wSpritesetView(wxWindow* parent, ISpritesetViewHandler* handler, sSpriteset* spriteset)
: wScrollWindow(parent)//wxWindow(parent, -1, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxHSCROLL)
, m_Handler(NULL)
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
  m_Handler = handler;
  m_Spriteset = spriteset;

  UpdateMaxSizes();

  m_MenuDirection = new wxMenu();
  m_MenuDirection->Append(wID_SPRITESETVIEWDIRECTIONS_INSERT,     "Insert");
  m_MenuDirection->Append(wID_SPRITESETVIEWDIRECTIONS_DELETE,     "Delete");
  m_MenuDirection->Append(wID_SPRITESETVIEWDIRECTIONS_APPEND,     "Append");
  m_MenuDirection->Append(wID_SPRITESETVIEWDIRECTIONS_PROPERTIES, "Properties");

  m_MenuFrame = new wxMenu();
  m_MenuFrame->Append(wID_SPRITESETVIEWFRAMES_INSERT,     "Insert");
  m_MenuFrame->Append(wID_SPRITESETVIEWFRAMES_DELETE,     "Delete");
  m_MenuFrame->Append(wID_SPRITESETVIEWFRAMES_APPEND,     "Append");
  m_MenuFrame->Append(wID_SPRITESETVIEWFRAMES_COPY,       "Copy");
  m_MenuFrame->Append(wID_SPRITESETVIEWFRAMES_PASTE,      "Paste");
  m_MenuFrame->Append(wID_SPRITESETVIEWFRAMES_PROPERTIES, "Properties");
}

////////////////////////////////////////////////////////////////////////////////

wSpritesetView::~wSpritesetView()
{
  delete m_DrawBitmap;
  delete m_MenuDirection;
  delete m_MenuFrame;
  //DestroyWindow();
}

/*
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
*/

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::SetDirection(int direction)
{
  m_CurrentDirection = direction;
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::SetFrame(int frame)
{
  m_CurrentFrame = frame;
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::SetZoomFactor(int zoom)
{
  m_ZoomFactor = zoom;
  UpdateMaxSizes();
  Refresh();
  UpdateScrollBars();
}

////////////////////////////////////////////////////////////////////////////////

int
wSpritesetView::GetZoomFactor() const
{
  return m_ZoomFactor;
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::SpritesetResized()
{
  UpdateMaxSizes();
  Refresh();
  UpdateScrollBars();
  m_Handler->SV_SpritesetModified();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::UpdateMaxSizes()
{
  // apply the zoom ratio
  m_MaxFrameWidth  = m_Spriteset->GetFrameWidth()  * m_ZoomFactor;
  m_MaxFrameHeight = m_Spriteset->GetFrameHeight() * m_ZoomFactor;

  // update the draw bitmap
  delete m_DrawBitmap;
  m_DrawBitmap = new wDIBSection(m_MaxFrameWidth, m_MaxFrameHeight);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::DrawDirection(wxDC &dc, int direction, int y)
{
  dc.SetBrush(*wxWHITE_BRUSH);
  dc.SetPen(*wxWHITE_PEN);
  dc.SetTextForeground(*wxBLACK);
  dc.DrawRectangle(0, y, LABEL_WIDTH, m_MaxFrameHeight);
  dc.DrawText(
    m_Spriteset->GetDirectionName(direction),
    0, y + m_MaxFrameHeight / 2 /*todo: make this placement more accurate*/
  );

  dc.SetBrush(wxNullBrush);
  dc.SetPen(wxNullPen);


  wxSize area(GetClientSize());
  for (int i = 0; i < area.GetWidth() / m_MaxFrameWidth + 1; i++)
  {
    wxRect c(
      LABEL_WIDTH + i * m_MaxFrameWidth,
      y,
      m_MaxFrameWidth,
      m_MaxFrameHeight
    );

    if (IsExposed(c))
    {
      if (i + m_LeftFrame < m_Spriteset->GetNumFrames(direction))
      {
        UpdateDrawBitmap(direction, i + m_LeftFrame);
        DrawFrame(dc, LABEL_WIDTH + i * m_MaxFrameWidth, y, direction, i + m_LeftFrame);
      } else {
        dc.SetBrush(*wxBLACK_BRUSH);
        dc.SetPen(*wxBLACK_PEN);
        dc.DrawRectangle(c);
        dc.SetBrush(wxNullBrush);
        dc.SetPen(wxNullPen);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::DrawFrame(wxDC &dc, int x, int y, int direction, int frame)
{
  int index = m_Spriteset->GetFrameIndex(direction, frame);
  CImage32& sprite = m_Spriteset->GetImage(index);
  dc.DrawBitmap(m_DrawBitmap->GetImage()->ConvertToBitmap(), x, y);

  if (direction == m_CurrentDirection && frame == m_CurrentFrame)
  {
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(wxPen(wxColour(0xff, 0x00, 0xff), 1, wxSOLID));

    dc.DrawRectangle(wxRect(x, y, sprite.GetWidth() * m_ZoomFactor, sprite.GetHeight() * m_ZoomFactor));

    dc.SetBrush(wxNullBrush);
    dc.SetPen(wxNullPen);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::UpdateDrawBitmap(int direction, int frame)
{
  int index = m_Spriteset->GetFrameIndex(direction, frame);
  CImage32& sprite = m_Spriteset->GetImage(index);
  int   src_width  = sprite.GetWidth();
  int   src_height = sprite.GetHeight();
  RGBA* src_pixels = sprite.GetPixels();

  int   dst_width  = m_DrawBitmap->GetWidth();
  int   dst_height = m_DrawBitmap->GetHeight();
  RGB*  dst_pixels = (RGB*)m_DrawBitmap->GetPixels();

  for (int iy = 0; iy < dst_height; iy++)
    for (int ix = 0; ix < dst_width; ix++)
    {
      if (ix < src_width * m_ZoomFactor && iy < src_height * m_ZoomFactor)
      {
        if(ix == 0 || iy == 0 || ix == src_width * m_ZoomFactor - 1 || iy == src_height * m_ZoomFactor - 1) {
          // draw a white border around the frame
          dst_pixels[iy * dst_width + ix] = CreateRGB(0xff, 0xff, 0xff);          
        } else {
          int sx = ix / m_ZoomFactor;
          int sy = iy / m_ZoomFactor;
          dst_pixels[iy * dst_width + ix].red   = src_pixels[sy * src_width + sx].red;
          dst_pixels[iy * dst_width + ix].green = src_pixels[sy * src_width + sx].green;
          dst_pixels[iy * dst_width + ix].blue  = src_pixels[sy * src_width + sx].blue;
        }
      }
      else
        dst_pixels[iy * dst_width + ix] = CreateRGB(0, 0, 0);
    }
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
wSpritesetView::UpdateScrollBars()
{
  if (m_MaxFrameWidth == 0 || m_MaxFrameHeight == 0)
    return;

  wxSize client_size = GetClientSize();
  int num_x_sprites = GetMaxNumFrames(m_Spriteset);
  int x_page_size   = (client_size.GetWidth() - LABEL_WIDTH) / m_MaxFrameWidth;
  int num_y_sprites = m_Spriteset->GetNumDirections();
  int y_page_size   = client_size.GetHeight() / m_MaxFrameHeight;

  // horizontal scrollbar (frames)
  //SetScrollbar(wxHORIZONTAL, m_LeftFrame, x_page_size, num_x_sprites - 1);
  SetHScrollRange(num_x_sprites, x_page_size);
  SetHScrollPosition(m_LeftFrame);

  // vertical scrollbar (directions)
  //SetScrollbar(wxVERTICAL, m_TopDirection, y_page_size, num_y_sprites - 1);
  SetVScrollRange(num_y_sprites, y_page_size);
  SetVScrollPosition(m_TopDirection);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::InvalidateFrame(int direction, int frame)
{
  direction -= m_TopDirection;
  frame     -= m_LeftFrame;

  wxRect r(
    LABEL_WIDTH + frame * m_MaxFrameWidth,
    direction * m_MaxFrameHeight,
    m_MaxFrameWidth,
    m_MaxFrameHeight
  );
  Refresh(TRUE, &r);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::OnSize(wxSizeEvent &event)
{
  wScrollWindow::OnSize(event);
  int cx = GetClientSize().GetWidth();
  int cy = GetClientSize().GetHeight();
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
  //CWnd::OnSize(type, cx, cy);
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::OnLButtonDown(wxMouseEvent &event)
{
  if (m_MenuDisplayed)
    return;

  if (event.GetX() < LABEL_WIDTH)
  {
    m_CurrentDirection = event.GetY() / m_MaxFrameHeight + m_TopDirection;
    if (m_CurrentDirection >= m_Spriteset->GetNumDirections())
      return;
    if (m_CurrentFrame > m_Spriteset->GetNumFrames(m_CurrentDirection) - 1)
      m_CurrentFrame = m_Spriteset->GetNumFrames(m_CurrentDirection) - 1;
    
    m_Handler->SV_CurrentFrameChanged(m_CurrentDirection, m_CurrentFrame);
    Refresh();
    return;
  }

  int direction = event.GetY() / m_MaxFrameHeight + m_TopDirection;
  int frame = (event.GetX() - LABEL_WIDTH) / m_MaxFrameWidth + m_LeftFrame;

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

void
wSpritesetView::OnRButtonUp(wxMouseEvent &event)
{
  if (m_MenuDisplayed)
    return;

  // select the frame
  OnLButtonDown(event);

  // if user right-clicked on a direction
  if (event.GetX() < LABEL_WIDTH)
  {
    // disable delete if only 1 direction left
    if (m_Spriteset->GetNumDirections() < 2) {
      m_MenuDirection->Enable(wID_SPRITESETVIEWDIRECTIONS_DELETE, FALSE);
    } else {
      m_MenuDirection->Enable(wID_SPRITESETVIEWDIRECTIONS_DELETE, TRUE);
    }

    //m_MenuDisplayed = true;

    // display it
    PopupMenu(m_MenuDirection, event.GetPosition());

    //m_MenuDisplayed = false;

  } else {
    int direction = event.GetY() / m_MaxFrameHeight + m_TopDirection;
    int frame = (event.GetX() - LABEL_WIDTH) / m_MaxFrameWidth;
    if (frame >= m_Spriteset->GetNumFrames(direction))
      return;

    // load the menu

    // enable/disable the menu items
    if (m_Spriteset->GetNumFrames(m_CurrentDirection) < 2) {
      m_MenuFrame->Enable(wID_SPRITESETVIEWFRAMES_DELETE, FALSE);
    } else {
      m_MenuFrame->Enable(wID_SPRITESETVIEWFRAMES_DELETE, TRUE);
    }

    //m_MenuDisplayed = true;

    // display it
    PopupMenu(m_MenuFrame, event.GetPosition());

    //m_MenuDisplayed = false;

  }
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::OnLButtonDblClk(wxMouseEvent &event)
{
  if (m_MenuDisplayed)
    return;

  if (event.GetX() - LABEL_WIDTH < 0)
    return;

  int direction = event.GetY() / m_MaxFrameHeight + m_TopDirection;
  int frame = (event.GetX() - LABEL_WIDTH) / m_MaxFrameWidth + m_LeftFrame;

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

/*
////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::OnScroll(wxScrollEvent &event)
{
  switch(event.GetOrientation()) {
  case wxHORIZONTAL:
    OnHScroll(event);
    break;
  case wxVERTICAL:
    OnVScroll(event);
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::OnHScroll(wxScrollEvent &event)
{
  wxSize client_size(GetClientSize());
  int page_width = (client_size.GetWidth() - LABEL_WIDTH) / m_MaxFrameWidth;
  int x_num_frames = GetMaxNumFrames(m_Spriteset);

  if(event.GetEventType() == wxEVT_SCROLL_TOP)          m_LeftFrame = 0;
  if(event.GetEventType() == wxEVT_SCROLL_BOTTOM)       m_LeftFrame = x_num_frames - page_width;
  if(event.GetEventType() == wxEVT_SCROLL_PAGEUP)       m_LeftFrame -= page_width;
  if(event.GetEventType() == wxEVT_SCROLL_PAGEDOWN)     m_LeftFrame += page_width;
  if(event.GetEventType() == wxEVT_SCROLL_LINEUP)       m_LeftFrame--;
  if(event.GetEventType() == wxEVT_SCROLL_LINEDOWN)     m_LeftFrame++;
  if(event.GetEventType() == wxEVT_SCROLL_THUMBRELEASE) m_LeftFrame = event.GetPosition();
  if(event.GetEventType() == wxEVT_SCROLL_THUMBTRACK)   m_LeftFrame = event.GetPosition();

  if (m_LeftFrame > x_num_frames - page_width)
    m_LeftFrame = x_num_frames - page_width;
  if (m_LeftFrame < 0)
    m_LeftFrame = 0;

  UpdateScrollBars();
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::OnVScroll(wxScrollEvent &event)
{
  wxSize client_size(GetClientSize());
  int page_width = client_size.GetHeight() / m_MaxFrameHeight;
  int y_num_frames = m_Spriteset->GetNumDirections();

  if(event.GetEventType() == wxEVT_SCROLL_TOP)          m_TopDirection = 0;
  if(event.GetEventType() == wxEVT_SCROLL_BOTTOM)       m_TopDirection = y_num_frames - page_width;
  if(event.GetEventType() == wxEVT_SCROLL_PAGEUP)       m_TopDirection -= page_width;
  if(event.GetEventType() == wxEVT_SCROLL_PAGEDOWN)     m_TopDirection += page_width;
  if(event.GetEventType() == wxEVT_SCROLL_LINEUP)       m_TopDirection--;
  if(event.GetEventType() == wxEVT_SCROLL_LINEDOWN)     m_TopDirection++;
  if(event.GetEventType() == wxEVT_SCROLL_THUMBRELEASE) m_TopDirection = event.GetPosition();
  if(event.GetEventType() == wxEVT_SCROLL_THUMBTRACK)   m_TopDirection = event.GetPosition();

  if (m_TopDirection > y_num_frames - page_width)
    m_TopDirection = y_num_frames - page_width;
  if (m_TopDirection < 0)
    m_TopDirection = 0;

  UpdateScrollBars();
  Refresh();
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::OnHScrollChanged(int x)
{
  int old_x = m_LeftFrame;
  wxSize client_size(GetClientSize());
  int page_width = (client_size.GetWidth() - LABEL_WIDTH) / m_MaxFrameWidth;
  int x_num_frames = GetMaxNumFrames(m_Spriteset);
  m_LeftFrame = x;
  if (m_LeftFrame > x_num_frames - page_width)
    m_LeftFrame = x_num_frames - page_width;
  if (m_LeftFrame < 0)
    m_LeftFrame = 0;

  int new_x = m_LeftFrame;
  if(new_x == old_x) {
    return;
  }
  ScrollArea((old_x - new_x) * m_MaxFrameWidth, 0, wxRect(wxPoint(LABEL_WIDTH, 0), wxSize(client_size.GetWidth() - LABEL_WIDTH, -1)));
  UpdateScrollBars();
  //Refresh(FALSE, &wxRect(wxPoint(0, 0), wxSize(LABEL_WIDTH, client_size.GetHeight())));
  //Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::OnVScrollChanged(int y)
{
  int old_y = m_TopDirection;
  wxSize client_size(GetClientSize());
  int page_height = client_size.GetHeight() / m_MaxFrameHeight;
  int y_num_frames = m_Spriteset->GetNumDirections();
  m_TopDirection = y;
  if (m_TopDirection > y_num_frames - page_height)
    m_TopDirection = y_num_frames - page_height;
  if (m_TopDirection < 0)
    m_TopDirection = 0;
  
  int new_y = m_TopDirection;
  if(new_y == old_y) {
    return;
  }
  ScrollArea(0, (old_y - new_y) * m_MaxFrameHeight);
  UpdateScrollBars();
  //Refresh();

}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::OnPaint(wxPaintEvent &event)
{
  wxPaintDC dc(this);

  wxSize area = GetClientSize();

  dc.SetClippingRegion(wxRect(wxPoint(0, 0), area));

  for (int i = 0; i < area.GetHeight() / m_MaxFrameHeight + 1; i++)
  {
    if (m_TopDirection + i < m_Spriteset->GetNumDirections())
    {
      DrawDirection(dc, m_TopDirection + i, i * m_MaxFrameHeight);
    }
    else
    {
      dc.SetBrush(*wxBLACK_BRUSH);
      dc.SetPen(*wxBLACK_PEN);
      dc.DrawRectangle(0, i * m_MaxFrameHeight, area.GetWidth(), m_MaxFrameHeight);
      dc.SetBrush(wxNullBrush);
      dc.SetPen(wxNullPen);
    }
  }
  dc.DestroyClippingRegion();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::OnInsertDirection(wxCommandEvent &event)
{
  m_Spriteset->InsertDirection(m_CurrentDirection);
  if (m_CurrentDirection > m_Spriteset->GetNumDirections() - 1)
    m_CurrentDirection = m_Spriteset->GetNumDirections() - 1;

  UpdateMaxSizes();
  UpdateScrollBars();
  Refresh();
  m_Handler->SV_CurrentFrameChanged(m_CurrentDirection, m_CurrentFrame);
  m_Handler->SV_SpritesetModified();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::OnDeleteDirection(wxCommandEvent &event)
{
  m_Spriteset->DeleteDirection(m_CurrentDirection);
  if (m_CurrentDirection > m_Spriteset->GetNumDirections() - 1)
    m_CurrentDirection = m_Spriteset->GetNumDirections() - 1;

  UpdateMaxSizes();
  UpdateScrollBars();
  Refresh();
  m_Handler->SV_CurrentFrameChanged(m_CurrentDirection, m_CurrentFrame);
  m_Handler->SV_SpritesetModified();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::OnAppendDirection(wxCommandEvent &event)
{
  m_Spriteset->InsertDirection(m_Spriteset->GetNumDirections());

  UpdateMaxSizes();
  UpdateScrollBars();
  Refresh();
  m_Handler->SV_CurrentFrameChanged(m_CurrentDirection, m_CurrentFrame);
  m_Handler->SV_SpritesetModified();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::OnDirectionProperties(wxCommandEvent &event)
{
  wxString s;
/*
  CStringDialog dialog(
    "Direction Name",
    m_Spriteset->GetDirectionName(m_CurrentDirection)
  );

  if (dialog.DoModal() == IDOK) {
    m_Spriteset->SetDirectionName(m_CurrentDirection, dialog.GetValue());
    Refresh();
    m_Handler->SV_SpritesetModified();
  }
*/
  s = ::wxGetTextFromUser("Enter new direction name", "Direction name", m_Spriteset->GetDirectionName(m_CurrentDirection), this, -1, -1, FALSE);
  if(s.length() != 0) {
    m_Spriteset->SetDirectionName(m_CurrentDirection, s);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::OnInsertFrame(wxCommandEvent &event)
{
  m_Spriteset->InsertFrame(m_CurrentDirection, m_CurrentFrame);
  UpdateMaxSizes(); 
  UpdateScrollBars();
  Refresh();
  m_Handler->SV_SpritesetModified();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::OnDeleteFrame(wxCommandEvent &event)
{
  m_Spriteset->DeleteFrame(m_CurrentDirection, m_CurrentFrame);
  if (m_CurrentFrame > m_Spriteset->GetNumFrames(m_CurrentDirection) - 1)
    m_CurrentFrame = m_Spriteset->GetNumFrames(m_CurrentDirection) - 1;

  UpdateMaxSizes();
  UpdateScrollBars();
  Refresh();
  m_Handler->SV_CurrentFrameChanged(m_CurrentDirection, m_CurrentFrame);
  m_Handler->SV_SpritesetModified();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::OnAppendFrame(wxCommandEvent &event)
{
  m_Spriteset->InsertFrame(m_CurrentDirection, m_Spriteset->GetNumFrames(m_CurrentDirection));
  UpdateMaxSizes(); 
  UpdateScrollBars();
  Refresh();
  m_Handler->SV_SpritesetModified();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::OnCopyFrame(wxCommandEvent &event)
{
  m_Handler->SV_CopyCurrentFrame();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::OnPasteFrame(wxCommandEvent &event)
{
  m_Handler->SV_PasteCurrentFrame();
}

////////////////////////////////////////////////////////////////////////////////

void
wSpritesetView::OnFrameProperties(wxCommandEvent &event)
{
  wSpritePropertiesDialog dialog(this, m_Spriteset, m_CurrentDirection, m_CurrentFrame);
  if (dialog.ShowModal() == wxID_OK)
    m_Handler->SV_SpritesetModified();
}

////////////////////////////////////////////////////////////////////////////////
