#pragma warning(disable : 4786)


#include "WindowStyleWindow.hpp"
#include "ResizeDialog.hpp"
#include "FileDialogs.hpp"
#include "WindowStylePropertiesDialog.hpp"
//#include "resource.h"
#include "IDs.hpp"


//const int ID_ALPHASLIDER = 7001;
//const int ID_ALPHASTATIC = 7002;


/*
BEGIN_MESSAGE_MAP(CWindowStyleWindow, CSaveableDocumentWindow)

  ON_WM_LBUTTONDOWN()
  ON_WM_SIZE()
  ON_WM_VSCROLL()
  ON_WM_PAINT()

  ON_COMMAND(ID_WINDOWSTYLE_EDIT_UPPERLEFT,  OnEditUpperLeft)
  ON_COMMAND(ID_WINDOWSTYLE_EDIT_TOP,        OnEditTop)
  ON_COMMAND(ID_WINDOWSTYLE_EDIT_UPPERRIGHT, OnEditUpperRight)
  ON_COMMAND(ID_WINDOWSTYLE_EDIT_RIGHT,      OnEditRight)
  ON_COMMAND(ID_WINDOWSTYLE_EDIT_LOWERRIGHT, OnEditLowerRight)
  ON_COMMAND(ID_WINDOWSTYLE_EDIT_BOTTOM,     OnEditBottom)
  ON_COMMAND(ID_WINDOWSTYLE_EDIT_LOWERLEFT,  OnEditLowerLeft)
  ON_COMMAND(ID_WINDOWSTYLE_EDIT_LEFT,       OnEditLeft)
  ON_COMMAND(ID_WINDOWSTYLE_EDIT_BACKGROUND, OnEditBackground)

  ON_COMMAND(ID_WINDOWSTYLE_RESIZESECTION,    OnResizeSection)

  ON_COMMAND(ID_WINDOWSTYLE_ZOOM_1X, OnZoom1x)
  ON_COMMAND(ID_WINDOWSTYLE_ZOOM_2X, OnZoom2x)
  ON_COMMAND(ID_WINDOWSTYLE_ZOOM_4X, OnZoom4x)
  ON_COMMAND(ID_WINDOWSTYLE_ZOOM_8X, OnZoom8x)

  ON_COMMAND(ID_WINDOWSTYLE_PROPERTIES, OnProperties)

  ON_UPDATE_COMMAND_UI(ID_WINDOWSTYLE_EDIT_UPPERLEFT,  OnUpdateEditUpperLeft)
  ON_UPDATE_COMMAND_UI(ID_WINDOWSTYLE_EDIT_TOP,        OnUpdateEditTop)
  ON_UPDATE_COMMAND_UI(ID_WINDOWSTYLE_EDIT_UPPERRIGHT, OnUpdateEditUpperRight)
  ON_UPDATE_COMMAND_UI(ID_WINDOWSTYLE_EDIT_RIGHT,      OnUpdateEditRight)
  ON_UPDATE_COMMAND_UI(ID_WINDOWSTYLE_EDIT_LOWERRIGHT, OnUpdateEditLowerRight)
  ON_UPDATE_COMMAND_UI(ID_WINDOWSTYLE_EDIT_BOTTOM,     OnUpdateEditBottom)
  ON_UPDATE_COMMAND_UI(ID_WINDOWSTYLE_EDIT_LOWERLEFT,  OnUpdateEditLowerLeft)
  ON_UPDATE_COMMAND_UI(ID_WINDOWSTYLE_EDIT_LEFT,       OnUpdateEditLeft)
  ON_UPDATE_COMMAND_UI(ID_WINDOWSTYLE_EDIT_BACKGROUND, OnUpdateEditBackground)

  ON_UPDATE_COMMAND_UI(ID_WINDOWSTYLE_ZOOM_1X, OnUpdateZoom1x)
  ON_UPDATE_COMMAND_UI(ID_WINDOWSTYLE_ZOOM_2X, OnUpdateZoom2x)
  ON_UPDATE_COMMAND_UI(ID_WINDOWSTYLE_ZOOM_4X, OnUpdateZoom4x)
  ON_UPDATE_COMMAND_UI(ID_WINDOWSTYLE_ZOOM_8X, OnUpdateZoom8x)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wWindowStyleWindow, wSaveableDocumentWindow)
  EVT_LEFT_DOWN(wWindowStyleWindow::OnLButtonDown)
  EVT_SIZE(wWindowStyleWindow::OnSize)
  //EVT_SCROLL()
  EVT_PAINT(wWindowStyleWindow::OnPaint)

  EVT_MENU(wID_WINDOWSTYLE_EDIT_UPPERLEFT,  wWindowStyleWindow::OnEditUpperLeft)
  EVT_MENU(wID_WINDOWSTYLE_EDIT_TOP,        wWindowStyleWindow::OnEditTop)
  EVT_MENU(wID_WINDOWSTYLE_EDIT_UPPERRIGHT, wWindowStyleWindow::OnEditUpperRight)
  EVT_MENU(wID_WINDOWSTYLE_EDIT_RIGHT,      wWindowStyleWindow::OnEditRight)
  EVT_MENU(wID_WINDOWSTYLE_EDIT_LOWERRIGHT, wWindowStyleWindow::OnEditLowerRight)
  EVT_MENU(wID_WINDOWSTYLE_EDIT_BOTTOM,     wWindowStyleWindow::OnEditBottom)
  EVT_MENU(wID_WINDOWSTYLE_EDIT_LOWERLEFT,  wWindowStyleWindow::OnEditLowerLeft)
  EVT_MENU(wID_WINDOWSTYLE_EDIT_LEFT,       wWindowStyleWindow::OnEditLeft)
  EVT_MENU(wID_WINDOWSTYLE_EDIT_BACKGROUND, wWindowStyleWindow::OnEditBackground)

  EVT_MENU(wID_WINDOWSTYLE_RESIZESECTION,    wWindowStyleWindow::OnResizeSection)

  EVT_MENU(wID_WINDOWSTYLE_ZOOM_1X, wWindowStyleWindow::OnZoom1x)
  EVT_MENU(wID_WINDOWSTYLE_ZOOM_2X, wWindowStyleWindow::OnZoom2x)
  EVT_MENU(wID_WINDOWSTYLE_ZOOM_4X, wWindowStyleWindow::OnZoom4x)
  EVT_MENU(wID_WINDOWSTYLE_ZOOM_8X, wWindowStyleWindow::OnZoom8x)

  EVT_MENU(wID_WINDOWSTYLE_PROPERTIES, wWindowStyleWindow::OnProperties)
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wWindowStyleWindow::wWindowStyleWindow(const char* window_style)
: wSaveableDocumentWindow(window_style, wID_WINDOWSTYLE_base /*todo:IDR_WINDOWSTYLE*/, wxSize(200, 150))
, m_Created(false)
, m_SelectedBitmap(sWindowStyle::UPPER_LEFT)
, m_ZoomFactor(4)
, m_HighlightPen(wxColour(0xff, 0x00, 0xff), 1, wxSOLID)
{
  if (window_style) {

    // load the window style
    if (m_WindowStyle.Load(window_style) == false)
    {
      ::wxMessageBox("Could not load window style.  Creating new.");
      m_WindowStyle.Create(16, 16);
    }
  
    // allocate DIB sections and empty them
    for (int i = 0; i < 9; i++)
      m_DIBs[i] = NULL;
    UpdateDIBSections();

    SetSaved(true);
    SetModified(false);

  } else {

    m_WindowStyle.Create(16, 16);

    // allocate DIB sections and empty them
    for (int i = 0; i < 9; i++)
      m_DIBs[i] = NULL;
    UpdateDIBSections();

    SetSaved(false);
    SetModified(false);

  }

  // create the window and child widgets
  //Create(AfxRegisterWndClass(0, ::LoadCursor(NULL, IDC_ARROW), NULL, AfxGetApp()->LoadIcon(IDI_WINDOWSTYLE)));

  m_ImageView = new wImageView(this, this, this);
  m_PaletteView = new wPaletteView(this, this);
  m_ColorView = new wColorView(this, this);
  m_AlphaView = new wAlphaView(this, this);
  SetBitmap();

  // we're done creating the windows, so make sure everything is in the right place
  m_Created = true;
/*todo:
  RECT ClientRect;
  GetClientRect(&ClientRect);
  OnSize(0, ClientRect.right, ClientRect.bottom);
*/
  // make sure the various views start with matching values
  m_ColorView->SetColor(CreateRGB(0, 0, 0));
  m_ImageView->SetColor(CreateRGBA(0, 0, 0, 255));
  m_AlphaView->SetAlpha(255);
}

////////////////////////////////////////////////////////////////////////////////

wWindowStyleWindow::~wWindowStyleWindow()
{
  for (int i = 0; i < 9; i++)
    delete m_DIBs[i];

}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::UpdateDIBSection(int bitmap)
{
  // delete the old DIB section and allocate a new one
  delete m_DIBs[bitmap];
  int dib_width  = m_WindowStyle.GetBitmap(bitmap).GetWidth()  * m_ZoomFactor;
  int dib_height = m_WindowStyle.GetBitmap(bitmap).GetHeight() * m_ZoomFactor;
  m_DIBs[bitmap] = new wDIBSection(dib_width, dib_height);
  
  // fill the DIB with data
  RGB* dest  = (RGB*)m_DIBs[bitmap]->GetPixels();
  CImage32& b = m_WindowStyle.GetBitmap(bitmap);
  RGBA* src   = b.GetPixels();
  for (int by = 0; by < b.GetHeight(); by++)
    for (int bx = 0; bx < b.GetWidth(); bx++)
    {
      // get the pixel to draw
      RGBA rgba = src[by * b.GetWidth() + bx];
      RGB rgb =
      {
        rgba.blue  = rgba.alpha * rgba.blue  / 256,
        rgba.green = rgba.alpha * rgba.green / 256,
        rgba.red   = rgba.alpha * rgba.red   / 256,
      };

      // fill the square with it
      for (int dy = 0; dy < m_ZoomFactor; dy++)
        for (int dx = 0; dx < m_ZoomFactor; dx++)
        {
          dest[(by * m_ZoomFactor + dy) * dib_width + (bx * m_ZoomFactor + dx)] = rgb;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::UpdateDIBSections()
{
  for (int i = 0; i < 9; i++)
    UpdateDIBSection(i);
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::SetBitmap()
{
  CImage32& b = m_WindowStyle.GetBitmap(m_SelectedBitmap);
  m_ImageView->SetImage(b.GetWidth(), b.GetHeight(), b.GetPixels());
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::SetZoomFactor(int factor)
{
  m_ZoomFactor = factor;

  // resize the DIBs
  UpdateDIBSections();
  Refresh();

  // move everything to its correct place
  OnSize(wxSizeEvent(GetClientSize()));
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::SelectBitmap(int bitmap)
{
  m_SelectedBitmap = bitmap;
  SetBitmap();
  Refresh(FALSE);
  m_ImageView->Refresh(FALSE);
}

////////////////////////////////////////////////////////////////////////////////

static void BlackRect(wxDC& dc, int x, int y, int w, int h)
{
  dc.SetBrush(*wxBLACK_BRUSH);
  dc.SetPen(*wxBLACK_PEN);
  dc.DrawRectangle(x, y, w, h);
  dc.SetBrush(wxNullBrush);
  dc.SetPen(wxNullPen);
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::DrawBit(wxDC& dc, int bitmap, int x, int y, int w, int h)
{
  wDIBSection* dib = m_DIBs[bitmap];

  // calculate correct corner offsets so the the images are flush with the image editor
  int offsetx = 0;
  int offsety = 0;
  int width = dib->GetWidth();
  int height = dib->GetHeight();
  bool repeatx = 0;
  bool repeaty = 0;
  switch (bitmap)
  {
    case sWindowStyle::UPPER_LEFT:
      offsetx = w - dib->GetWidth();
      offsety = h - dib->GetHeight();
      break;

    case sWindowStyle::UPPER_RIGHT:
      offsetx = 0;
      offsety = h - dib->GetHeight();
      break;

    case sWindowStyle::LOWER_LEFT:
      offsetx = w - dib->GetWidth();
      offsety = 0;
      break;

    case sWindowStyle::LOWER_RIGHT:
      offsetx = 0;
      offsety = 0;
      break;


    case sWindowStyle::TOP:
      offsetx = 0;
      offsety = 0;
      repeatx = true;
      width = w;
      break;

    case sWindowStyle::BOTTOM:
      offsetx = 0;
      offsety = h - dib->GetHeight();
      repeatx = true;
      width = w;
      break;

    
    case sWindowStyle::LEFT:
      offsetx = 0;
      offsety = 0;
      repeaty = true;
      height = h;
      break;
    
    case sWindowStyle::RIGHT:
      offsetx = w - dib->GetWidth();
      offsety = 0;
      repeaty = true;
      height = h;
      break;


    case sWindowStyle::BACKGROUND:
      offsetx = 0;
      offsety = 0;
      repeatx = true;
      repeaty = true;
      width = w;
      height = h;
      break;

  }

  // create a clipping region for the DIB and select it in
  wxRegion region(
    offsetx + x,
    offsety + y,
    w,
    h
  );
  if(bitmap == sWindowStyle::BACKGROUND) {
    wxRect EditRect;
    GetEditRect(&EditRect);
    region.Subtract(EditRect);
  }
  dc.SetClippingRegion(region);

  wxBitmap dib_bitmap = dib->GetImage()->ConvertToBitmap();
  int xx;
  int yy;
  // draw the DIB
  if(!repeatx && !repeaty) {
    dc.DrawBitmap(dib_bitmap, x + offsetx, y + offsety);
  } else if (repeatx && !repeaty) {
    for(xx = x; xx < x + w; xx += dib->GetWidth()) {
      dc.DrawBitmap(dib_bitmap, xx + offsetx, y + offsety);
    }
  } else if (!repeatx && repeaty) {
    for(yy = y; yy < y + h; yy += dib->GetHeight()) {
      dc.DrawBitmap(dib_bitmap, x + offsetx, yy + offsety);
    }
  } else if (repeatx && repeaty) {
    for(yy = y; yy < y + h; yy += dib->GetHeight()) {
      for(xx = x; xx < x + w; xx += dib->GetWidth()) {
        dc.DrawBitmap(dib_bitmap, xx + offsetx, yy + offsety);
      }
    }
  }
/*
    x + offsetx, y + offsety, dib->GetWidth(), dib->GetHeight(),
    CDC::FromHandle(dib->GetDC()),
    0, 0, SRCCOPY);
*/

  if(!(repeatx && repeaty)) {
    // select the region opposite of the previous one
    wxRegion region2(x, y, w, h);
    region2.Subtract(region);
    dc.SetClippingRegion(region2);

    // fill the rest of the area with black
    BlackRect(dc, x, y, w, h);
  }

  // remove the clipping region
  dc.DestroyClippingRegion();

  // if it's the selected bitmap, put a pink rectangle around it
  if (bitmap == m_SelectedBitmap)
  {
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(m_HighlightPen);
    dc.DrawRectangle(offsetx + x, offsety + y, width, height);
    dc.SetBrush(wxNullBrush);
    dc.SetPen(wxNullPen);
  }
}

/*
////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::DrawCorner(wxDC& dc, int bitmap, int x, int y, int w, int h)
{
  wDIBSection* dib = m_DIBs[bitmap];

  // calculate correct corner offsets so the the images are flush with the image editor
  int offsetx = 0;
  int offsety = 0;
  switch (bitmap)
  {
    case sWindowStyle::UPPER_LEFT:
      offsetx = w - dib->GetWidth();
      offsety = h - dib->GetHeight();
      break;

    case sWindowStyle::UPPER_RIGHT:
      offsetx = 0;
      offsety = h - dib->GetHeight();
      break;

    case sWindowStyle::LOWER_LEFT:
      offsetx = w - dib->GetWidth();
      offsety = 0;
      break;

    case sWindowStyle::LOWER_RIGHT:
      offsetx = 0;
      offsety = 0;
      break;
  }

  // create a clipping region for the DIB and select it in
  wxRegion region(
    offsetx + x,
    offsety + y,
    dib->GetWidth(),
    dib->GetHeight()
  );
  dc.SetClippingRegion(region);

  // draw the DIB
  dc.DrawBitmap(dib->GetImage()->ConvertToBitmap(), x + offsetx, y + offsety);
/*
    x + offsetx, y + offsety, dib->GetWidth(), dib->GetHeight(),
    CDC::FromHandle(dib->GetDC()),
    0, 0, SRCCOPY);
* /

  // select the region opposite of the previous one
  wxRegion region2(x, y, w, h);
  region2.Subtract(region);
  dc.SetClippingRegion(region2);

  // fill the rest of the area with black
  BlackRect(dc, x, y, w, h);

  // remove the clipping region
  dc.DestroyClippingRegion();

  // if it's the selected bitmap, put a pink rectangle around it
  if (bitmap == m_SelectedBitmap)
  {
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(wxPen(m_HighlightPen));
    dc.DrawRectangle(offset + x, offset + y, dib->GetWidth(), dib->GetHeight());
    dc.SelectBrush(wxNullBrush);
    dc.SelectPen(wxNullPen);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::DrawEdgeH(CDC& dc, int bitmap, int x, int y, int x2, int h)
{
  wDIBSection* dib = m_DIBs[bitmap];

  // calculate correct edge offsets so the the images are flush with the image editor
  int offsety = 0;
  switch (bitmap)
  {
    case sWindowStyle::TOP:
      offsety = h - dib->GetHeight();
      break;

    case sWindowStyle::BOTTOM:
      offsety = 0;
      break;
  }

  // create a clipping region for the DIBs
  wxRegion region(
    x,
    offsety + y,
    x2,
    offsety + y + dib->GetHeight()
  );
  dc.SetClippingRegion(region);

  // draw the edge
  int x1 = x;
  while (x1 < x2)
  {
    dc.BitBlt(
      x1, offsety + y, dib->GetWidth(), dib->GetHeight(),
      CDC::FromHandle(dib->GetDC()),
      0, 0, SRCCOPY);
    x1 += dib->GetWidth();
  }

  // select the clipping region opposite of the previous one
  dc.SelectClipRgn(NULL);
  dc.SelectClipRgn(&region, RGN_XOR);

  // fill rest of area with black
  BlackRect(dc, x, y, x2 - x, h);

  // reset the clipping rectangle
  dc.SelectClipRgn(NULL);
  region.DeleteObject();

  // if bitmap is selected, draw pink selection square
  if (bitmap == m_SelectedBitmap)
  {
    dc.SaveDC();
    dc.SelectObject(m_HighlightPen);
    dc.SelectStockObject(NULL_BRUSH);
    dc.Rectangle(x, offsety + y, x2, offsety + y + dib->GetHeight());
    dc.RestoreDC(-1);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CWindowStyleWindow::DrawEdgeV(CDC& dc, int bitmap, int x, int y, int y2, int w)
{
  CDIBSection* dib = m_DIBs[bitmap];

  // calculate correct edge offsets so the the images are flush with the image editor
  int offsetx = 0;
  switch (bitmap)
  {
    case sWindowStyle::LEFT:
      offsetx = w - dib->GetWidth();
      break;

    case sWindowStyle::RIGHT:
      offsetx = 0;
      break;
  }

  // create a clipping region for the DIBs
  CRgn region;
  region.CreateRectRgn(offsetx + x, y, offsetx + x + dib->GetWidth(), y2);
  dc.SelectClipRgn(&region);

  // draw the edge
  int y1 = y;
  while (y1 < y2)
  {
    dc.BitBlt(
      offsetx + x, y1, dib->GetWidth(), dib->GetHeight(),
      CDC::FromHandle(dib->GetDC()),
      0, 0, SRCCOPY);
    y1 += dib->GetHeight();
  }

  // select the clipping region opposite of the previous one
  dc.SelectClipRgn(NULL);
  dc.SelectClipRgn(&region, RGN_XOR);

  // fill the area with black
  BlackRect(dc, x, y, w, y2 - y);

  // reset the clipping rectangle
  dc.SelectClipRgn(NULL);
  region.DeleteObject();

  // if bitmap is selected, draw pink selection square
  if (bitmap == m_SelectedBitmap)
  {
    dc.SaveDC();
    dc.SelectObject(m_HighlightPen);
    dc.SelectStockObject(NULL_BRUSH);
    dc.Rectangle(offsetx + x, y, offsetx + x + dib->GetWidth(), y2);
    dc.RestoreDC(-1);
  }
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::OnLButtonDown(wxMouseEvent &event)
{
  wxRect EditRect;
  GetInnerRect(&EditRect);
  wxPoint point = event.GetPosition();

  if (point.x < EditRect.x &&
      point.y < EditRect.y) {
    SelectBitmap(sWindowStyle::UPPER_LEFT);
  } else if (point.x >= EditRect.x + EditRect.width &&
             point.y < EditRect.y) {
    SelectBitmap(sWindowStyle::UPPER_RIGHT);
  } else if (point.x < EditRect.x &&
             point.y >= EditRect.y + EditRect.height) {
    SelectBitmap(sWindowStyle::LOWER_LEFT);
  } else if (point.x >= EditRect.x + EditRect.width &&
             point.y >= EditRect.y + EditRect.height) {
    SelectBitmap(sWindowStyle::LOWER_RIGHT);
  } else if (point.x < EditRect.x) {
    SelectBitmap(sWindowStyle::LEFT);
  } else if (point.y < EditRect.y) {
    SelectBitmap(sWindowStyle::TOP);
  } else if (point.x >= EditRect.x + EditRect.width) {
    SelectBitmap(sWindowStyle::RIGHT);
  } else if (point.y >= EditRect.y + EditRect.height) {
    SelectBitmap(sWindowStyle::BOTTOM);
  } else if (!m_ImageView->GetRect().Inside(point)) {
    SelectBitmap(sWindowStyle::BACKGROUND);
  } else {
    event.Skip();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::OnSize(wxSizeEvent &event)
{
  if (m_Created)
  {
    int cx = GetClientSize().GetWidth();
    int cy = GetClientSize().GetHeight();
    wxRect EditRect;
    GetEditRect(&EditRect);
    m_ImageView->SetSize(EditRect);

    m_PaletteView->SetSize(cx - 60 - 32, 0, 60, cy - 60);
    m_ColorView->SetSize(cx - 60 - 32, cy - 60, 60, 60);
    m_AlphaView->SetSize(cx - 32, 0, 32, cy);

    Refresh();
  }

  wSaveableDocumentWindow::OnSize(event);
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::OnPaint(wxPaintEvent &event)
{
  wxPaintDC dc(this);

  wxRect EditRect;
  GetEditRect(&EditRect);

  wxRect InnerRect;
  GetInnerRect(&InnerRect);

  // corners
  DrawBit(dc, sWindowStyle::UPPER_LEFT,  0, 0,
    GetBorderWidth_Left(),
    GetBorderWidth_Top());
  DrawBit(dc, sWindowStyle::UPPER_RIGHT, InnerRect.x + InnerRect.width, 0,
    GetBorderWidth_Right(),
    GetBorderWidth_Top());
  DrawBit(dc, sWindowStyle::LOWER_LEFT,  0, InnerRect.y + InnerRect.height,
    GetBorderWidth_Left(),
    GetBorderWidth_Bottom());
  DrawBit(dc, sWindowStyle::LOWER_RIGHT, InnerRect.x + InnerRect.width, InnerRect.y + InnerRect.height, 
    GetBorderWidth_Right(),
    GetBorderWidth_Bottom());

  // edges
  DrawBit(dc, sWindowStyle::TOP,    InnerRect.x, 0, InnerRect.width, GetBorderWidth_Top());
  DrawBit(dc, sWindowStyle::BOTTOM, InnerRect.x, InnerRect.y + InnerRect.height, InnerRect.width, GetBorderWidth_Bottom());
  DrawBit(dc, sWindowStyle::LEFT,   0, InnerRect.y, GetBorderWidth_Left(), InnerRect.height);
  DrawBit(dc, sWindowStyle::RIGHT,  InnerRect.x + InnerRect.width, InnerRect.y, GetBorderWidth_Right(), InnerRect.height);

  DrawBit(dc, sWindowStyle::BACKGROUND, InnerRect.x, InnerRect.y, InnerRect.width, InnerRect.height);
/*
  // corners
  DrawCorner(dc, sWindowStyle::UPPER_LEFT,  0, 0,
    GetBorderWidth_Left(),
    GetBorderWidth_Top());
  DrawCorner(dc, sWindowStyle::UPPER_RIGHT, EditRect.right, 0,
    GetBorderWidth_Right(),
    GetBorderWidth_Top());
  DrawCorner(dc, sWindowStyle::LOWER_LEFT,  0, EditRect.bottom,
    GetBorderWidth_Left(),
    GetBorderWidth_Bottom());
  DrawCorner(dc, sWindowStyle::LOWER_RIGHT, EditRect.right, EditRect.bottom, 
    GetBorderWidth_Right(),
    GetBorderWidth_Bottom());

  // edges
  DrawEdgeH(dc, sWindowStyle::TOP,    EditRect.left, 0, EditRect.right, GetBorderWidth_Top());
  DrawEdgeH(dc, sWindowStyle::BOTTOM, EditRect.left, EditRect.bottom, EditRect.right, GetBorderWidth_Bottom());
  DrawEdgeV(dc, sWindowStyle::LEFT,   0, EditRect.top, EditRect.bottom, GetBorderWidth_Left());
  DrawEdgeV(dc, sWindowStyle::RIGHT,  EditRect.right, EditRect.top, EditRect.bottom, GetBorderWidth_Right());
*/

}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::OnEditUpperLeft(wxCommandEvent &event)
{
  SelectBitmap(sWindowStyle::UPPER_LEFT);
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::OnEditTop(wxCommandEvent &event)
{
  SelectBitmap(sWindowStyle::TOP);
}

////////////////////////////////////////////////////////////////////////////////

void 
wWindowStyleWindow::OnEditUpperRight(wxCommandEvent &event)
{
  SelectBitmap(sWindowStyle::UPPER_RIGHT);
}

////////////////////////////////////////////////////////////////////////////////

void 
wWindowStyleWindow::OnEditRight(wxCommandEvent &event)
{
  SelectBitmap(sWindowStyle::RIGHT);
}

////////////////////////////////////////////////////////////////////////////////

void 
wWindowStyleWindow::OnEditLowerRight(wxCommandEvent &event)
{
  SelectBitmap(sWindowStyle::LOWER_RIGHT);
}

////////////////////////////////////////////////////////////////////////////////

void 
wWindowStyleWindow::OnEditBottom(wxCommandEvent &event)
{
  SelectBitmap(sWindowStyle::BOTTOM);
}

////////////////////////////////////////////////////////////////////////////////

void 
wWindowStyleWindow::OnEditLowerLeft(wxCommandEvent &event)
{
  SelectBitmap(sWindowStyle::LOWER_LEFT);
}

////////////////////////////////////////////////////////////////////////////////

void 
wWindowStyleWindow::OnEditLeft(wxCommandEvent &event)
{
  SelectBitmap(sWindowStyle::LEFT);
}

////////////////////////////////////////////////////////////////////////////////

void 
wWindowStyleWindow::OnEditBackground(wxCommandEvent &event)
{
  SelectBitmap(sWindowStyle::BACKGROUND);
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::OnResizeSection(wxCommandEvent &event)
{
  CImage32& b = m_WindowStyle.GetBitmap(m_SelectedBitmap);
  wResizeDialog Dialog(this, "Resize Window Style Section", b.GetWidth(), b.GetHeight());
  Dialog.SetRange(1, 1024, 1, 1024);
  if (Dialog.ShowModal() == wxID_OK)
  {
    m_WindowStyle.GetBitmap(m_SelectedBitmap).Resize(Dialog.GetWidth(), Dialog.GetHeight());

    SetModified(true);

    // update the window
    SetBitmap();
    UpdateDIBSections();
    Refresh();

    // resize the window
    OnSize(wxSizeEvent(GetClientSize()));
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::OnZoom1x(wxCommandEvent &event)
{
  SetZoomFactor(1);
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::OnZoom2x(wxCommandEvent &event)
{
  SetZoomFactor(2);
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::OnZoom4x(wxCommandEvent &event)
{
  SetZoomFactor(4);
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::OnZoom8x(wxCommandEvent &event)
{
  SetZoomFactor(8);
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::OnProperties(wxCommandEvent &event)
{
  wWindowStylePropertiesDialog dialog(this, &m_WindowStyle);
  if (dialog.ShowModal() == wxID_OK) {
    SetModified(true);
  }
}

/*
////////////////////////////////////////////////////////////////////////////////

afx_msg void
CWindowStyleWindow::OnUpdateEditUpperLeft(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_SelectedBitmap == sWindowStyle::UPPER_LEFT);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CWindowStyleWindow::OnUpdateEditTop(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_SelectedBitmap == sWindowStyle::TOP);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CWindowStyleWindow::OnUpdateEditUpperRight(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_SelectedBitmap == sWindowStyle::UPPER_RIGHT);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CWindowStyleWindow::OnUpdateEditRight(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_SelectedBitmap == sWindowStyle::RIGHT);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CWindowStyleWindow::OnUpdateEditLowerRight(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_SelectedBitmap == sWindowStyle::LOWER_RIGHT);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CWindowStyleWindow::OnUpdateEditBottom(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_SelectedBitmap == sWindowStyle::BOTTOM);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CWindowStyleWindow::OnUpdateEditLowerLeft(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_SelectedBitmap == sWindowStyle::LOWER_LEFT);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CWindowStyleWindow::OnUpdateEditLeft(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_SelectedBitmap == sWindowStyle::LEFT);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CWindowStyleWindow::OnUpdateEditBackground(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_SelectedBitmap == sWindowStyle::BACKGROUND);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CWindowStyleWindow::OnUpdateZoom1x(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_ZoomFactor == 1);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CWindowStyleWindow::OnUpdateZoom2x(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_ZoomFactor == 2);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CWindowStyleWindow::OnUpdateZoom4x(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_ZoomFactor == 4);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CWindowStyleWindow::OnUpdateZoom8x(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_ZoomFactor == 8);
}
*/

////////////////////////////////////////////////////////////////////////////////

bool
wWindowStyleWindow::GetSavePath(char* path)
{
  wWindowStyleFileDialog Dialog(this, FDM_SAVE);
  if (Dialog.ShowModal() != wxID_OK)
    return false;

  strcpy(path, Dialog.GetPath());
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
wWindowStyleWindow::SaveDocument(const char* path)
{
  return m_WindowStyle.Save(path);
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::IV_ImageChanged()
{
  CImage32& bitmap = m_WindowStyle.GetBitmap(m_SelectedBitmap);
  memcpy(
    bitmap.GetPixels(),
    m_ImageView->GetPixels(),
    bitmap.GetWidth() * bitmap.GetHeight() * sizeof(RGBA));

  UpdateDIBSection(m_SelectedBitmap);
  Refresh();

  SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::IV_ColorChanged(RGBA color)
{
  RGB rgb = { color.red, color.green, color.blue };
  m_ColorView->SetColor(rgb);
  m_AlphaView->SetAlpha(color.alpha);
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::PV_ColorChanged(RGB color)
{
  RGBA rgba = { color.red, color.green, color.blue, m_AlphaView->GetAlpha() };
  m_ImageView->SetColor(rgba);
  m_ColorView->SetColor(color);
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::CV_ColorChanged(RGB color)
{
  RGBA rgba = { color.red, color.green, color.blue, m_AlphaView->GetAlpha() };
  m_ImageView->SetColor(rgba);
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::AV_AlphaChanged(byte alpha)
{
  RGBA rgba = m_ImageView->GetColor();
  rgba.alpha = alpha;
  m_ImageView->SetColor(rgba);
}

////////////////////////////////////////////////////////////////////////////////

int
wWindowStyleWindow::GetBorderWidth_Left() const
{
  int w1 = m_WindowStyle.GetBitmap(sWindowStyle::LEFT).GetWidth();
  int w2 = m_WindowStyle.GetBitmap(sWindowStyle::UPPER_LEFT).GetWidth();
  int w3 = m_WindowStyle.GetBitmap(sWindowStyle::LOWER_LEFT).GetWidth();
  return std::_cpp_max(w1, std::_cpp_max(w2, w3)) * m_ZoomFactor;
}

////////////////////////////////////////////////////////////////////////////////

int
wWindowStyleWindow::GetBorderWidth_Top() const
{
  int h1 = m_WindowStyle.GetBitmap(sWindowStyle::TOP).GetHeight();
  int h2 = m_WindowStyle.GetBitmap(sWindowStyle::UPPER_LEFT).GetHeight();
  int h3 = m_WindowStyle.GetBitmap(sWindowStyle::UPPER_RIGHT).GetHeight();
  return std::_cpp_max(h1, std::_cpp_max(h2, h3)) * m_ZoomFactor;
}

////////////////////////////////////////////////////////////////////////////////

int
wWindowStyleWindow::GetBorderWidth_Right() const
{
  int w1 = m_WindowStyle.GetBitmap(sWindowStyle::RIGHT).GetWidth();
  int w2 = m_WindowStyle.GetBitmap(sWindowStyle::UPPER_RIGHT).GetWidth();
  int w3 = m_WindowStyle.GetBitmap(sWindowStyle::LOWER_RIGHT).GetWidth();
  return std::_cpp_max(w1, std::_cpp_max(w2, w3)) * m_ZoomFactor;
}

////////////////////////////////////////////////////////////////////////////////

int
wWindowStyleWindow::GetBorderWidth_Bottom() const
{
  int h1 = m_WindowStyle.GetBitmap(sWindowStyle::BOTTOM).GetHeight();
  int h2 = m_WindowStyle.GetBitmap(sWindowStyle::LOWER_LEFT).GetHeight();
  int h3 = m_WindowStyle.GetBitmap(sWindowStyle::LOWER_RIGHT).GetHeight();
  return std::_cpp_max(h1, std::_cpp_max(h2, h3)) * m_ZoomFactor;
}

////////////////////////////////////////////////////////////////////////////////

int
wWindowStyleWindow::GetBackgroundWidth() const
{
  return m_WindowStyle.GetBitmap(sWindowStyle::BACKGROUND).GetWidth() * m_ZoomFactor;
}

////////////////////////////////////////////////////////////////////////////////

int
wWindowStyleWindow::GetBackgroundHeight() const
{
  return m_WindowStyle.GetBitmap(sWindowStyle::BACKGROUND).GetHeight() * m_ZoomFactor;
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::GetEditRect(wxRect* rect)
{
  wxSize ClientSize = GetClientSize();
  rect->x   = GetBorderWidth_Left() + GetBackgroundWidth();
  rect->y    = GetBorderWidth_Top() + GetBackgroundHeight();
  rect->width  = ClientSize.GetWidth()  - GetBorderWidth_Right() * 2 - GetBackgroundWidth() * 2 - 60 - 32;
  rect->height = ClientSize.GetHeight() - GetBorderWidth_Bottom() * 2 - GetBackgroundHeight() * 2;
}

////////////////////////////////////////////////////////////////////////////////

void
wWindowStyleWindow::GetInnerRect(wxRect* rect)
{
  wxSize ClientSize = GetClientSize();
  rect->x   = GetBorderWidth_Left();
  rect->y    = GetBorderWidth_Top();
  rect->width  = ClientSize.GetWidth()  - GetBorderWidth_Right() * 2 - 60 - 32;
  rect->height = ClientSize.GetHeight() - GetBorderWidth_Bottom() * 2;
}

////////////////////////////////////////////////////////////////////////////////
