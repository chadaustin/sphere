#include "ResizeDialog.hpp"
//#include "resource.h"

BEGIN_EVENT_TABLE(wResizeDialog, wxDialog)
  EVT_BUTTON(wxID_OK, OnOK)
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wResizeDialog::wResizeDialog(wxWindow *parent, const char* caption, int default_width, int default_height)
: wxDialog(parent, -1, caption, wxDefaultPosition, wxSize(208, 72), wxDIALOG_MODAL | wxCAPTION | wxSYSTEM_MENU)
, m_Caption(caption)
, m_Width(default_width)
, m_Height(default_height)
, m_MinWidth(0)
, m_MaxWidth(65535)
, m_MinHeight(0)
, m_MaxHeight(65535)
{
  wxBoxSizer *subsizer = NULL;
  wxBoxSizer *mainsizer = new wxBoxSizer(wxVERTICAL);
  wxFlexGridSizer *gridsizer;

    gridsizer = new wxFlexGridSizer(2);
      gridsizer->Add(new wxStaticText(this, -1, "Width"), 0, wxALL, 10);
      gridsizer->Add(m_WidthCtrl = new wxSpinCtrl(this, -1, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, m_MinWidth, m_MaxWidth, m_Width), 0, wxALL, 10);

      gridsizer->Add(new wxStaticText(this, -1, "Height"), 0, wxALL, 10);
      gridsizer->Add(m_HeightCtrl = new wxSpinCtrl(this, -1, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, m_MinHeight, m_MaxHeight, m_Height), 0, wxALL, 10);
    mainsizer->Add(gridsizer);

    subsizer = new wxBoxSizer(wxHORIZONTAL);
      subsizer->Add(new wxButton(this, wxID_OK, "OK"), 0, wxALL, 10);
      subsizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 10);
    mainsizer->Add(subsizer);

  SetSizer(mainsizer);
  mainsizer->SetSizeHints(this);

  m_WidthCtrl->SetValue(m_Width);
  m_HeightCtrl->SetValue(m_Height);
}

////////////////////////////////////////////////////////////////////////////////

void
wResizeDialog::SetRange(int min_width, int max_width, int min_height, int max_height)
{
  m_MinWidth  = min_width;
  m_MaxWidth  = max_width;
  m_MinHeight = min_height;
  m_MaxHeight = max_height;

  m_WidthCtrl->SetRange(m_MinWidth, m_MaxWidth);
  m_HeightCtrl->SetRange(m_MinHeight, m_MaxHeight);
}

////////////////////////////////////////////////////////////////////////////////

int
wResizeDialog::GetWidth() const
{
  return m_Width;
}

////////////////////////////////////////////////////////////////////////////////

int
wResizeDialog::GetHeight() const
{
  return m_Height;
}

/*
////////////////////////////////////////////////////////////////////////////////

BOOL
CResizeDialog::OnInitDialog()
{
  SetWindowText(m_Caption.c_str());
  SetDlgItemInt(IDC_WIDTH,  m_Width);
  SetDlgItemInt(IDC_HEIGHT, m_Height);
  GetDlgItem(IDC_WIDTH)->SetFocus();
  ((CEdit*)GetDlgItem(IDC_WIDTH))->SetSel(0, -1);
 
  return FALSE;
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wResizeDialog::OnOK(wxCommandEvent &event)
{
  int w = m_WidthCtrl->GetValue();
  int h = m_HeightCtrl->GetValue();

  if (w < m_MinWidth || w > m_MaxWidth ||
      h < m_MinHeight || h > m_MaxHeight)
  {
    char message[520];
    sprintf(message,
      "Width must be between %d and %d.\n"
      "Height must be between %d and %d.",
      m_MinWidth, m_MaxWidth,
      m_MinHeight, m_MaxHeight);
    ::wxMessageBox(message, "Resize");
    return;
  }

  m_Width  = w;
  m_Height = h;
  wxDialog::OnOK(event);
}

////////////////////////////////////////////////////////////////////////////////
