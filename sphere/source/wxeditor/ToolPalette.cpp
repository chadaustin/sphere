#include "ToolPalette.hpp"
#include "Editor.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
#include "IDs.hpp"
#include "../common/rgb.hpp"
#include <wx/wx.h>


const int TOOL_BORDER = 2;
const int TOOL_WIDTH  = 20 + TOOL_BORDER * 2;
const int TOOL_HEIGHT = 20 + TOOL_BORDER * 2;
const int BUTTON_BASE = wID_TOOLPALETTE_base;
const int MAX_BUTTONS = 256;


/*
BEGIN_MESSAGE_MAP(CToolPalette, CPaletteWindow)

  ON_WM_SIZE()
  ON_WM_PAINT()

  ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnNeedText)

  ON_COMMAND_RANGE(BUTTON_BASE, BUTTON_BASE + MAX_BUTTONS - 1, OnToolSelected)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wToolPalette, wPaletteWindow)
  EVT_SIZE(wToolPalette::OnSize)
  EVT_PAINT(wToolPalette::OnPaint)
  //EVT_CUSTOM_RANGE(wxEVT_LEFT_DOWN, BUTTON_BASE, BUTTON_BASE + MAX_BUTTONS - 1, wToolPalette::OnLClick)
  EVT_LEFT_DOWN(wToolPalette::OnLClick)
  EVT_MOTION(wToolPalette::OnMouseMove)
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wToolPalette::wToolPalette(wDocumentWindow* owner, IToolPaletteHandler* handler, const char* name, wxRect rect, bool visible)
: wPaletteWindow(owner, name, rect, visible)
, m_Handler(handler)
, m_CurrentTool(0)
{
  //wxToolTip::Enable(true);
  wxSizeEvent evt(GetSize());
  OnSize(evt);
}

////////////////////////////////////////////////////////////////////////////////

bool
wToolPalette::Destroy()
{
//  // destroy buttons
//  for (int i = 0; i < m_Buttons.size(); i++) {
//    delete m_Buttons[i];
//  }
  for (unsigned i = 0; i < m_Bitmaps.size(); i++) {
    delete m_Bitmaps[i];
  }

/*
  DestroyWindow();
*/
  
  return wPaletteWindow::Destroy();
}

////////////////////////////////////////////////////////////////////////////////

void
wToolPalette::AddTool(const char **xpm_image, const char* label)
{
  wxBitmap* bitmap = new wxBitmap(xpm_image);
  m_Bitmaps.push_back(bitmap);
  m_Labels.push_back(label);
  //wxStaticBitmap* button = new wxStaticBitmap(this, BUTTON_BASE + m_Buttons.size(), *bitmap, wxDefaultPosition, wxSize(TOOL_WIDTH, TOOL_HEIGHT), 0, "wToolPaletteButton");
  //button->SetBitmap(bitmap);
  //button->SetToolTip(label);
  //m_Buttons.push_back(button);
  wxSizeEvent evt(GetSize());
  OnSize(evt);
  Refresh();
/*
  button->Create("",
    BS_ICON | BS_AUTORADIOBUTTON | BS_PUSHLIKE | WS_CHILD | WS_VISIBLE,
    CRect(0, 0, 0, 0), this, BUTTON_BASE + m_Buttons.size());
  button->SetIcon(AfxGetApp()->LoadIcon(MAKEINTRESOURCE(icon_id)));

  if (m_CurrentTool == m_Buttons.size()) {
    button->SetCheck(BST_CHECKED);
  }

  CRect rect(0, 0, 0, 0);
  m_Buttons.push_back(button);
  m_Labels.push_back(label);

  // make sure the tools are in the correct place now
  RECT cr;
  GetClientRect(&cr);
  OnSize(0, cr.right, cr.bottom);
*/
}

////////////////////////////////////////////////////////////////////////////////

void
wToolPalette::OnSize(wxSizeEvent &event)
{
/*
  wxSize area = GetClientSize();
  int nx = area.GetWidth() / TOOL_WIDTH;
  if (nx == 0) {
    nx = 1;
  }

  int ix = 0; // current x
  int iy = 0; // current y

  for (int i = 0; i < m_Buttons.size(); i++) {
    int x = ix * TOOL_WIDTH;
    int y = iy * TOOL_HEIGHT;
    m_Buttons[i]->SetSize(x + TOOL_BORDER, y + TOOL_BORDER, TOOL_WIDTH - TOOL_BORDER * 2, TOOL_HEIGHT - TOOL_BORDER * 2);
    m_Buttons[i]->Refresh();

    if (++ix >= nx) {
      ix = 0;
      iy++;
    }
  }
*/
}

////////////////////////////////////////////////////////////////////////////////

static wxPen CreateShadedPen(RGB color1, RGB color2, int alpha)
{
  wxColor color(
    (color1.red   * alpha + color2.red   * (256 - alpha)) / 256,
    (color1.green * alpha + color2.green * (256 - alpha)) / 256,
    (color1.blue  * alpha + color2.blue  * (256 - alpha)) / 256
  );
  return wxPen(color, 1, wxSOLID);
}

////////////////////////////////////////////////////////////////////////////////

void
wToolPalette::OnPaint(wxPaintEvent &event)
{
  wxPaintDC dc(this);
  wxSize area = GetClientSize();

  int nx = area.GetWidth() / TOOL_WIDTH;
  if (nx == 0) {
    nx = 1;
  }

  wxColour sys_color = wxSystemSettings::GetSystemColour(wxSYS_COLOUR_MENU);
  RGB menu_color = {
    sys_color.Red(),
    sys_color.Green(),
    sys_color.Blue(),
  };
  RGB white = { 255, 255, 255 };
  RGB black = { 0,   0,   0   };

  int ix = 0; // current x
  int iy = 0; // current y

  for (unsigned i = 0; i < m_Bitmaps.size(); i++) {
    int x = ix * TOOL_WIDTH;
    int y = iy * TOOL_HEIGHT;
    wxRect rect(x, y, TOOL_WIDTH, TOOL_HEIGHT);

    wxPen very_light = CreateShadedPen(menu_color, white, 64);
    wxPen light      = CreateShadedPen(menu_color, white, 192);
    wxPen dark       = CreateShadedPen(menu_color, black, 192);
    wxPen very_dark  = CreateShadedPen(menu_color, black, 64);

    if((int)i == m_CurrentTool) {
      std::swap(very_light, very_dark);
      std::swap(light, dark);
    }

    // draw the outer border
    dc.SetPen(very_light);
    dc.DrawLine(rect.x, rect.y, rect.x + rect.width - 1, rect.y);
    dc.DrawLine(rect.x, rect.y, rect.x, rect.y + rect.height - 1);

    dc.SetPen(very_dark);
    dc.DrawLine(rect.x + rect.width - 1, rect.y, rect.x + rect.width - 1, rect.y + rect.height - 1);
    dc.DrawLine(rect.x, rect.y + rect.height - 1, rect.x + rect.width - 1, rect.y + rect.height - 1);

    // draw the inner border
    dc.SetPen(light);
    dc.DrawLine(rect.x + 1, rect.y + 1, rect.x + rect.width - 2, rect.y + 1);
    dc.DrawLine(rect.x + 1, rect.y + 1, rect.x + 1, rect.y + rect.height - 2);

    dc.SetPen(dark);
    dc.DrawLine(rect.x + rect.width - 2, rect.y + 1, rect.x + rect.width - 2, rect.y + rect.height - 2);
    dc.DrawLine(rect.x + 1, rect.y + rect.height - 2, rect.x + rect.width - 2, rect.y + rect.height - 2);

/*todo: probably ugly and broken*/
/*
    dc.SetBrush(wxBrush(wxColour(0x00, 0x00, 0x00), wxTRANSPARENT));
    dc.SetPen(wxPen(wxColour(0xff, 0xff, 0x00), 3, wxSOLID));
    dc.DrawRectangle(x + 1, y + 1, TOOL_WIDTH - 2, TOOL_HEIGHT - 2);

    dc.SetBrush(wxNullBrush);
    dc.SetPen(wxNullPen);

    if(i == m_CurrentTool) {
      dc.SetBrush(wxBrush(wxColour(0x00, 0x00, 0x00), wxTRANSPARENT));
      dc.SetPen(wxPen(wxColour(0x00, 0x00, 0x00), 1, wxSOLID));
      dc.DrawRectangle(x, y, TOOL_WIDTH, TOOL_HEIGHT);

      dc.SetBrush(wxNullBrush);
      dc.SetPen(wxNullPen);
    }
*/
    dc.DrawBitmap(*m_Bitmaps[i], rect.x + (TOOL_WIDTH - m_Bitmaps[i]->GetWidth()) / 2, rect.y + (TOOL_HEIGHT - m_Bitmaps[i]->GetHeight()) / 2, TRUE);

    if (++ix >= nx) {
      ix = 0;
      iy++;
    }
  }

/*
  RECT cr;
  GetClientRect(&cr);

  CPaintDC dc(this);
  dc.FillRect(&cr, CBrush::FromHandle((HBRUSH)(COLOR_MENU + 1)));
*/
}

////////////////////////////////////////////////////////////////////////////////

void
wToolPalette::OnLClick(wxMouseEvent &event) 
{
  wxSize size = GetClientSize();
  wxPoint point = event.GetPosition();
  int in = size.x / TOOL_WIDTH;
  if(in == 0) {
    in = 1;
  }
  int ix = point.x / TOOL_WIDTH;
  int iy = point.y / TOOL_HEIGHT;
  int ic = iy * in + ix;
  if(ix >= in || ic >= int(m_Bitmaps.size())) {
    return;
  }

  m_CurrentTool = ic;
  m_Handler->TP_ToolSelected(m_CurrentTool);

/*
  if(event.GetId() >= BUTTON_BASE && event.GetId() < BUTTON_BASE + MAX_BUTTONS) {
    m_CurrentTool = event.GetId() - BUTTON_BASE;
    m_Handler->TP_ToolSelected(m_CurrentTool);
    Refresh();
  }
*/
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wToolPalette::OnMouseMove(wxMouseEvent &event) 
{
  wxSize size = GetClientSize();
  wxPoint point = event.GetPosition();
  int in = size.x / TOOL_WIDTH;
  if(in == 0) {
    in = 1;
  }
  int ix = point.x / TOOL_WIDTH;
  int iy = point.y / TOOL_HEIGHT;
  int ic = iy * in + ix;
  if(ix >= in || ic >= int(m_Bitmaps.size())) {
    return;
  }

  SetStatus(m_Labels[ic]);
}

////////////////////////////////////////////////////////////////////////////////

#if 0
afx_msg BOOL
CToolPalette::OnNeedText(UINT /*id*/, NMHDR* hdr, LRESULT* result)
{
  TOOLTIPTEXT* ttt = (TOOLTIPTEXT*)hdr;
  UINT id = hdr->idFrom;
  if (ttt->uFlags & TTF_IDISHWND) {
    id = ::GetDlgCtrlID((HWND)id);
  }

  if (id >= BUTTON_BASE && id < BUTTON_BASE + m_Labels.size()) {
    ttt->lpszText = const_cast<char*>(m_Labels[id - BUTTON_BASE].c_str());
  } else {
    ttt->lpszText = "";
  }

  *result = 0;
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CToolPalette::OnToolSelected(UINT id)
{
  m_Handler->TP_ToolSelected(id - BUTTON_BASE);
}

////////////////////////////////////////////////////////////////////////////////
#endif
