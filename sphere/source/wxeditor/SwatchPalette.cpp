#pragma warning(disable : 4786)

//#define WXWIN_COMPATIBILITY_EVENT_TYPES 1

#include <vector>
#include <wx/wx.h>
#include "SwatchPalette.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
#include "../common/configfile.hpp"
#include "../common/common_palettes.hpp"
//#include "resource.h"
#include "IDs.hpp"


#define SWATCH_TILE_SIZE 10

/*
BEGIN_MESSAGE_MAP(CSwatchPalette, CPaletteWindow)

  ON_WM_SIZE()
  ON_WM_PAINT()
  ON_WM_VSCROLL()
  ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONDOWN()

  ON_COMMAND(ID_SWATCHPALETTE_FILE_LOAD,     OnFileLoad)
  ON_COMMAND(ID_SWATCHPALETTE_FILE_SAVE,     OnFileSave)
  ON_COMMAND(ID_SWATCHPALETTE_DEFAULT_DOS,   OnDefaultDOS)
  ON_COMMAND(ID_SWATCHPALETTE_DEFAULT_VERGE, OnDefaultVERGE)

  ON_COMMAND(ID_SWATCHPALETTE_INSERTBEFORE, OnInsertColorBefore)
  ON_COMMAND(ID_SWATCHPALETTE_INSERTAFTER,  OnInsertColorAfter)
  ON_COMMAND(ID_SWATCHPALETTE_REPLACE,      OnReplaceColor)
  ON_COMMAND(ID_SWATCHPALETTE_DELETE,       OnDeleteColor)

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wSwatchPalette, wPaletteWindow)
  EVT_SIZE(wSwatchPalette::OnSize)

  EVT_PAINT(OnPaint)

  //EVT_MENU(wID_TILEOBSTRUCTION_PRESET_UNBLOCKED,       wTileObstructionDialog::OnPresetUnblocked)

  EVT_SCROLL(wSwatchPalette::OnVScroll)

  EVT_LEFT_DOWN(wSwatchPalette::OnLButtonDown)
  EVT_RIGHT_DOWN(wSwatchPalette::OnRButtonDown)

  EVT_MENU(wID_SWATCHPALETTE_FILE_LOAD,     wSwatchPalette::OnFileLoad)
  EVT_MENU(wID_SWATCHPALETTE_FILE_SAVE,     wSwatchPalette::OnFileSave)
  EVT_MENU(wID_SWATCHPALETTE_DEFAULT_DOS,   wSwatchPalette::OnDefaultDOS)
  EVT_MENU(wID_SWATCHPALETTE_DEFAULT_VERGE, wSwatchPalette::OnDefaultVERGE)

  EVT_MENU(wID_SWATCHPALETTE_INSERTBEFORE, wSwatchPalette::OnInsertColorBefore)
  EVT_MENU(wID_SWATCHPALETTE_INSERTAFTER,  wSwatchPalette::OnInsertColorAfter)
  EVT_MENU(wID_SWATCHPALETTE_REPLACE,      wSwatchPalette::OnReplaceColor)
  EVT_MENU(wID_SWATCHPALETTE_DELETE,       wSwatchPalette::OnDeleteColor)

END_EVENT_TABLE()


////////////////////////////////////////////////////////////////////////////////

wSwatchPalette::wSwatchPalette(wDocumentWindow* owner, ISwatchPaletteHandler* handler)
: wPaletteWindow(owner, "Swatch",
  Configuration::Get(KEY_SWATCH_RECT),
  Configuration::Get(KEY_SWATCH_VISIBLE))
, m_Handler(handler)

, m_Color(CreateRGBA(0, 0, 0, 255))

, m_TopRow(0)
, m_SelectedColor(0)
, m_RightClickColor(0)
{
/*todo:*/
//  wxRect rect(GetClientOrigin(), GetClientSize());
  
//  OnSize(0, rect.right, rect.bottom);
  m_Menu = new wxMenu();
  wxMenu *submenu;

  submenu = new wxMenu();
  submenu->Append(wID_SWATCHPALETTE_FILE_LOAD, "Load...", "Load a palette from file");
  submenu->Append(wID_SWATCHPALETTE_FILE_SAVE, "Save...", "Save a palette to file");
  m_Menu->Append(wID_SWATCHPALETTE_FILE_, "File", submenu);

  submenu = new wxMenu();
  submenu->Append(wID_SWATCHPALETTE_DEFAULT_DOS, "DOS", "Use default palette for 256 color VGA under DOS");
  submenu->Append(wID_SWATCHPALETTE_DEFAULT_VERGE, "VERGE", "Use default palette for VERGE");
  m_Menu->Append(wID_SWATCHPALETTE_DEFAULT_, "Default", submenu);

  m_Menu->AppendSeparator();
  m_Menu->Append(wID_SWATCHPALETTE_INSERTBEFORE, "Insert Color Before", "Insert current color before the right clicked color");
  m_Menu->Append(wID_SWATCHPALETTE_INSERTAFTER, "Insert Color After", "Insert current color after the right clicked color");
  m_Menu->Append(wID_SWATCHPALETTE_REPLACE, "Replace Color", "Replace right clicked color with current color");
  m_Menu->Append(wID_SWATCHPALETTE_DELETE, "Delete", "Delete right clicked color");
}

////////////////////////////////////////////////////////////////////////////////

bool
wSwatchPalette::Destroy()
{
  delete m_Menu;

  // store state
  wxRect rect = GetRect();
  Configuration::Set(KEY_SWATCH_RECT, rect);

  Configuration::Set(KEY_SWATCH_VISIBLE, true); /*todo:IsWindowVisible() != FALSE);*/

  //DestroyWindow();

  return wPaletteWindow::Destroy();
}

////////////////////////////////////////////////////////////////////////////////

void
wSwatchPalette::SetColor(RGBA color)
{
  m_Color = color;
}

////////////////////////////////////////////////////////////////////////////////

void
wSwatchPalette::OnSize(wxSizeEvent &event)
{
  if (event.GetSize().GetWidth() > 0)
  {
    // if the current top row is greater than the total number of rows minus the page size
    if (m_TopRow > GetNumRows() - GetPageSize())
    {
      // move the top row up
      m_TopRow = GetNumRows() - GetPageSize();
      if (m_TopRow < 0)
        m_TopRow = 0;
      Refresh();
    }
  }

  // reflect the changes
  UpdateScrollBar();
  Refresh();

  wPaletteWindow::OnSize(event);
}

////////////////////////////////////////////////////////////////////////////////

void
wSwatchPalette::OnPaint(wxPaintEvent &event)
{
  CSwatchServer* swatch = CSwatchServer::Instance();

  wxPaintDC dc(this);
  wxSize area(GetClientSize());
  wxRegion region = GetUpdateRegion();
  
  for (int iy = 0; iy < area.GetHeight() / SWATCH_TILE_SIZE + 1; iy++)
    for (int ix = 0; ix < area.GetWidth() / SWATCH_TILE_SIZE + 1; ix++)
    {
      int num_colors_x = area.GetWidth() / SWATCH_TILE_SIZE;
      int ic = (iy + m_TopRow) * num_colors_x + ix;

      wxRect Color_Location(
        ix * SWATCH_TILE_SIZE,
        iy * SWATCH_TILE_SIZE,
        SWATCH_TILE_SIZE,
        SWATCH_TILE_SIZE
      );

      if (region.Contains(Color_Location) == wxOutRegion)
        continue;
      if (ix < num_colors_x && ic < swatch->GetNumColors())
      {
        RGBA rgba = swatch->GetColor(ic);

        if (ic == m_SelectedColor)
        {
          dc.SetBrush(wxBrush(wxColour(rgba.red, rgba.green, rgba.blue), wxSOLID));
          dc.SetPen(wxPen(wxColour(255, 255, 255), 1, wxSOLID));
          dc.DrawRectangle(Color_Location);
          dc.SetBrush(wxBrush(wxColour(0, 0, 0), wxTRANSPARENT));
          dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxDOT));
          dc.DrawRectangle(Color_Location);

          dc.SetBrush(wxNullBrush);
          dc.SetPen(wxNullPen);
        } else {
          dc.SetBrush(wxBrush(wxColour(rgba.red, rgba.green, rgba.blue), wxSOLID));
          dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxSOLID));
          dc.DrawRectangle(Color_Location);

          dc.SetBrush(wxNullBrush);
          dc.SetPen(wxNullPen);
        }
      }
      else
      {
        // draw black rectangle
        dc.SetBrush(wxBrush(wxColour(0, 0, 0), wxSOLID));
        dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxSOLID));
        dc.DrawRectangle(Color_Location);

        dc.SetBrush(wxNullBrush);
        dc.SetPen(wxNullPen);
      }
    }
  

/*
  CPaintDC dc(this);
  CBrush brush;
  RECT rect;

  GetClientRect(&rect);
  
  for (int iy = 0; iy < rect.bottom / SWATCH_TILE_SIZE + 1; iy++)
    for (int ix = 0; ix < rect.right / SWATCH_TILE_SIZE + 1; ix++)
    {
      int num_colors_x = rect.right / SWATCH_TILE_SIZE;
      int ic = (iy + m_TopRow) * num_colors_x + ix;

      RECT Color_Location = {
        ix * SWATCH_TILE_SIZE,
        iy * SWATCH_TILE_SIZE,
        (ix + 1) * SWATCH_TILE_SIZE,
        (iy + 1) * SWATCH_TILE_SIZE
      };

      if (!dc.RectVisible(&Color_Location))
        continue;
      if (ix < num_colors_x && ic < swatch->GetNumColors())
      {
        RGBA rgba = swatch->GetColor(ic);
        brush.CreateSolidBrush(RGB(rgba.red, rgba.green, rgba.blue));
        dc.FillRect(&Color_Location, &brush);
        brush.DeleteObject();

        if (ic == m_SelectedColor)
        {
          brush.CreateSolidBrush(RGB(0, 0, 0));
          dc.FrameRect(&Color_Location, &brush);
          brush.DeleteObject();

          Color_Location.left++;
          Color_Location.top++;
          Color_Location.right--;
          Color_Location.bottom--;
          brush.CreateSolidBrush(RGB(255, 255, 255));
          dc.FrameRect(&Color_Location, &brush);
          brush.DeleteObject();

          Color_Location.left++;
          Color_Location.top++;
          Color_Location.right--;
          Color_Location.bottom--;
          brush.CreateSolidBrush(RGB(0, 0, 0));
          dc.FrameRect(&Color_Location, &brush);
          brush.DeleteObject();
        }
      }
      else
      {
        // draw black rectangle
        dc.FillRect(&Color_Location, CBrush::FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH)));
      }
    }
  
  brush.DeleteObject();
*/
}

////////////////////////////////////////////////////////////////////////////////

void
wSwatchPalette::OnVScroll(wxScrollEvent &event)
{
  int old_y = m_TopRow;

  if(event.GetEventType() == wxEVT_SCROLL_LINEDOWN)   m_TopRow++;
  if(event.GetEventType() == wxEVT_SCROLL_LINEUP)     m_TopRow--;
  if(event.GetEventType() == wxEVT_SCROLL_PAGEDOWN)   m_TopRow += GetPageSize();
  if(event.GetEventType() == wxEVT_SCROLL_PAGEUP)     m_TopRow -= GetPageSize();
  if(event.GetEventType() == wxEVT_SCROLL_THUMBTRACK) m_TopRow = GetScrollPos(wxVERTICAL);

  // validate the values
  if (m_TopRow > GetNumRows() - GetPageSize())
    m_TopRow = GetNumRows() - GetPageSize();
  if (m_TopRow < 0)
    m_TopRow = 0;

  UpdateScrollBar();
  Refresh();
/*todo:
  //Invalidate();
  CDC* dc = GetDC();
  CRgn updateRgn;
  dc->ScrollDC(0, (old_y - m_TopRow) * SWATCH_TILE_SIZE, NULL, NULL, &updateRgn, NULL);
  InvalidateRgn(&updateRgn);
*/
}

////////////////////////////////////////////////////////////////////////////////

void
wSwatchPalette::OnLButtonDown(wxMouseEvent &event)
{
  CSwatchServer* swatch = CSwatchServer::Instance();

  wxSize area(GetClientSize());
  
  int num_colors_x = area.GetWidth() / SWATCH_TILE_SIZE;
  int ix = event.GetX() / SWATCH_TILE_SIZE;
  int iy = event.GetY() / SWATCH_TILE_SIZE;
  int ic = (iy + m_TopRow) * num_colors_x + ix;

  if (ix >= num_colors_x) {
    return;
  }

  if (ic < swatch->GetNumColors())
  {
    RGBA rgba = swatch->GetColor(ic);
    m_Color = rgba;
    m_Handler->SP_ColorSelected(rgba);

    // "clean" the old color
    if (m_SelectedColor >= m_TopRow * num_colors_x &&
        m_SelectedColor <  (m_TopRow + (area.GetHeight() / SWATCH_TILE_SIZE)) * num_colors_x)
    {
      int offset = m_SelectedColor - (m_TopRow * num_colors_x);
      int y = (offset / num_colors_x) * SWATCH_TILE_SIZE;
      int x = (offset % num_colors_x) * SWATCH_TILE_SIZE;
      Refresh(TRUE, &wxRect(x, y, SWATCH_TILE_SIZE, SWATCH_TILE_SIZE));
    }
    m_SelectedColor = ic;

    // refresh the new position
    int offset = ic - (m_TopRow * num_colors_x);
    iy = (offset / num_colors_x) * SWATCH_TILE_SIZE;
    ix = (offset % num_colors_x) * SWATCH_TILE_SIZE;
    Refresh(TRUE, &wxRect(ix, iy, SWATCH_TILE_SIZE, SWATCH_TILE_SIZE));
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wSwatchPalette::OnRButtonDown(wxMouseEvent &event)
{
  wxSize area = GetClientSize();
  
  int num_colors_x = area.GetWidth() / SWATCH_TILE_SIZE;
  int ix = event.GetX() / SWATCH_TILE_SIZE;
  int iy = event.GetY() / SWATCH_TILE_SIZE;
  int ic = (iy + m_TopRow) * num_colors_x + ix;

  if (ix >= num_colors_x) {
    return;
  }

  m_RightClickColor = ic;

  // popup menu
  PopupMenu(m_Menu, event.GetPosition());
}

////////////////////////////////////////////////////////////////////////////////

void
wSwatchPalette::OnFileLoad(wxEvent &event)
{
  CSwatchServer* swatch = CSwatchServer::Instance();

  wxFileDialog Dialog(
    this,
    wxString("Choose a file"),
    ::wxGetCwd(),
    wxString(""),
    "Sphere Swatch Files (*.sswatch)|*.sswatch|"
    "All Files (*.*)|*.*||",
    wxOPEN | wxFILE_MUST_EXIST | wxHIDE_READONLY
/*
    TRUE, "sswatch", NULL,
    OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
    "Sphere Swatch Files (*.sswatch)|*.sswatch|"
    "All Files (*.*)|*.*||"
*/
  );

  if (Dialog.ShowModal() != wxID_OK) {
    return;
  }

  // open file
  CConfigFile file(Dialog.GetPath().c_str());

  // check version  
  int ver = file.ReadInt("sphere_swatch", "version", 0);
  if (ver != 1) {
    return;
  }

  // read colors
  int NumColors = file.ReadInt("", "numcolors", 0);

  swatch->Clear();
  for (int i = 0; i < NumColors; i++) {
    char color_str[80];
    sprintf(color_str, "color%d", i);
    
    RGBA rgba;
    rgba.red   = file.ReadInt(color_str, "red",   0);
    rgba.green = file.ReadInt(color_str, "green", 0);
    rgba.blue  = file.ReadInt(color_str, "blue",  0);
    rgba.alpha = file.ReadInt(color_str, "alpha", 255);
    swatch->SetColor(i, rgba);
  }

  UpdateScrollBar();
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wSwatchPalette::OnFileSave(wxEvent &event)
{
  CSwatchServer* swatch = CSwatchServer::Instance();

  wxFileDialog Dialog(
    this,
    wxString("Choose a file"),
    ::wxGetCwd(),
    wxString(""),
    "Sphere Swatch Files (*.sswatch)|*.sswatch|"
    "All Files (*.*)|*.*||",
    wxSAVE | wxOVERWRITE_PROMPT | wxHIDE_READONLY
/*
    FALSE, "sswatch", NULL,
    OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
    "Sphere Swatch Files (*.sswatch)|*.sswatch|" \
    "All Files (*.*)|*.*||"
*/
  );

  if (Dialog.ShowModal() != wxID_OK) {
    return;
  }

  // create swatch file
  CConfigFile file;
  file.WriteInt("sphere_swatch", "version", 1);
  file.WriteInt("", "numcolors", swatch->GetNumColors());

  for (int i = 0; i < swatch->GetNumColors(); i++)
  {
    char color_str[80];
    sprintf(color_str, "color%d", i);

    RGBA rgba = swatch->GetColor(i);
    file.WriteInt(color_str, "red",   rgba.red);
    file.WriteInt(color_str, "green", rgba.green);
    file.WriteInt(color_str, "blue",  rgba.blue);
    file.WriteInt(color_str, "alpha", rgba.alpha);
  }

  file.Save(Dialog.GetPath());
}

////////////////////////////////////////////////////////////////////////////////

void
wSwatchPalette::OnDefaultDOS(wxEvent &event)
{
  CSwatchServer* swatch = CSwatchServer::Instance();

  swatch->Clear();
  for (int i = 0; i < 256; i++)
  {
    RGBA color = {
      dos_palette[i].red,
      dos_palette[i].green,
      dos_palette[i].blue,
      255
    };
    swatch->SetColor(i, color);
  }

  UpdateScrollBar();
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wSwatchPalette::OnDefaultVERGE(wxEvent &event)
{
  CSwatchServer* swatch = CSwatchServer::Instance();

  swatch->Clear();
  for (int i = 0; i < 256; i++)
  {
    RGBA color = {
      verge_palette[i].red,
      verge_palette[i].green,
      verge_palette[i].blue,
      255
    };
    swatch->SetColor(i, color);
  }

  UpdateScrollBar();
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wSwatchPalette::OnInsertColorBefore(wxEvent &event)
{
  CSwatchServer* swatch = CSwatchServer::Instance();

  swatch->InsertColor(m_RightClickColor, m_Color);

  UpdateScrollBar();
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wSwatchPalette::OnInsertColorAfter(wxEvent &event)
{
  CSwatchServer* swatch = CSwatchServer::Instance();
  swatch->InsertColor(m_RightClickColor + 1, m_Color);

  UpdateScrollBar();
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wSwatchPalette::OnReplaceColor(wxEvent &event)
{
  CSwatchServer* swatch = CSwatchServer::Instance();

  if (m_RightClickColor >= 0 && m_RightClickColor < swatch->GetNumColors()) {
    swatch->SetColor(m_RightClickColor, m_Color);

    UpdateScrollBar();
    Refresh();
  }
 }

////////////////////////////////////////////////////////////////////////////////

void
wSwatchPalette::OnDeleteColor(wxEvent &event)
{
  CSwatchServer* swatch = CSwatchServer::Instance();
  swatch->DeleteColor(m_RightClickColor);

  if (m_SelectedColor > swatch->GetNumColors() - 1) {
    m_SelectedColor = swatch->GetNumColors() - 1;
    if (m_SelectedColor == -1) {
      m_SelectedColor = 0;
    }
  }

  UpdateScrollBar();
  Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void
wSwatchPalette::UpdateScrollBar()
{
  SetScrollbar(wxVERTICAL, m_TopRow, GetPageSize(), GetNumRows());
}

////////////////////////////////////////////////////////////////////////////////

int
wSwatchPalette::GetPageSize()
{
  wxSize ClientSize = GetClientSize();
  return ClientSize.GetHeight() / SWATCH_TILE_SIZE;
}

////////////////////////////////////////////////////////////////////////////////

int
wSwatchPalette::GetNumRows()
{
  CSwatchServer* swatch = CSwatchServer::Instance();

  wxSize client_size = GetClientSize();
  int num_colors_x = client_size.GetWidth() / SWATCH_TILE_SIZE;

  if (num_colors_x == 0)
    return -1;
  else
    return (swatch->GetNumColors() + num_colors_x - 1) / num_colors_x;
}

////////////////////////////////////////////////////////////////////////////////
