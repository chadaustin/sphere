#include "FontGradientDialog.hpp"
//#include "resource.h"

BEGIN_EVENT_TABLE(wFontGradientDialog, wxDialog)
  EVT_BUTTON(wxID_OK, OnOK)
END_EVENT_TABLE()


////////////////////////////////////////////////////////////////////////////////

wFontGradientDialog::wFontGradientDialog(wxWindow *parent)
: wxDialog(parent, -1, "Font Gradient", wxDefaultPosition, wxSize(160, 128), wxDIALOG_MODAL | wxCAPTION | wxSYSTEM_MENU)
, m_top_color(CreateRGBA(255, 255, 255, 255))
, m_bottom_color(CreateRGBA(255, 255, 255, 255))
{

  wxBoxSizer *subsizer = NULL;
  wxBoxSizer *mainsizer = new wxBoxSizer(wxVERTICAL);
  wxFlexGridSizer *gridsizer;

    gridsizer = new wxFlexGridSizer(2);
      gridsizer->Add(new wxStaticText(this, -1, "Top Color"), 0, wxALL, 10);
      gridsizer->Add(m_top_view = new wRGBAView(this, 64, 8, 48, 64, m_top_color), 0, wxALL, 10);
      gridsizer->Add(new wxStaticText(this, -1, "Bottom Color"), 0, wxALL, 10);
      gridsizer->Add(m_bottom_view = new wRGBAView(this, 64, 8, 48, 64, m_bottom_color), 0, wxALL, 10);
    mainsizer->Add(gridsizer);
    subsizer = new wxBoxSizer(wxHORIZONTAL);
      subsizer->Add(new wxButton(this, wxID_OK, "OK"), 0, wxALL, 10);
      subsizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 10);
    mainsizer->Add(subsizer);

  SetSizer(mainsizer);
  mainsizer->SetSizeHints(this);

}

////////////////////////////////////////////////////////////////////////////////

RGBA
wFontGradientDialog::GetTopColor()
{
  return m_top_color;
}

////////////////////////////////////////////////////////////////////////////////

RGBA
wFontGradientDialog::GetBottomColor()
{
  return m_bottom_color;
}

/*
////////////////////////////////////////////////////////////////////////////////

BOOL
CFontGradientDialog::OnInitDialog()
{
  InitializeView(m_top_view,    m_top_color,    IDC_TOP_COLOR);
  InitializeView(m_bottom_view, m_bottom_color, IDC_BOTTOM_COLOR);

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void
CFontGradientDialog::InitializeView(CRGBAView& view, RGBA color, int control_id)
{
  RECT rect;
  GetDlgItem(control_id)->GetWindowRect(&rect);
  ScreenToClient(&rect);
  view.Create(
    this,
    rect.left,
    rect.top,
    rect.right - rect.left,
    rect.bottom - rect.top,
    color
  );
}
*/
////////////////////////////////////////////////////////////////////////////////

void
wFontGradientDialog::OnOK(wxCommandEvent &event)
{
  m_top_color    = m_top_view->GetColor();
  m_bottom_color = m_bottom_view->GetColor();

  wxDialog::OnOK(event);
}

////////////////////////////////////////////////////////////////////////////////
