#include "LayerPropertiesDialog.hpp"
#include "../common/Map.hpp"
//#include "resource.h"
#include "IDs.hpp"


const int MIN_RANGE = -160;
const int MAX_RANGE = 160;
const int PAGE_SIZE = 10;


/*
BEGIN_MESSAGE_MAP(CLayerPropertiesDialog, CDialog)

  ON_BN_CLICKED(IDC_HAS_PARALLAX, OnEnableParallax)
  ON_WM_HSCROLL()
  
END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wLayerPropertiesDialog, wxDialog)
  EVT_BUTTON(wxID_OK, wLayerPropertiesDialog::OnOK)
  EVT_CHECKBOX(wID_LAYERPROPERTIESDIALOG_HAS_PARALAX, wLayerPropertiesDialog::OnEnableParallax)
  EVT_SCROLL(wLayerPropertiesDialog::OnScroll)
END_EVENT_TABLE()

static inline int FloatToScrollValue(float32 f) {
  return int((f + 0.05) * 10); // sadly, there is roundoff error
}

static inline float32 ScrollValueToFloat(int value) {
  return (float32)value / 10;
}


////////////////////////////////////////////////////////////////////////////////

wLayerPropertiesDialog::wLayerPropertiesDialog(wxWindow *parent, sLayer& layer, bool parallax_allowed)
: wxDialog(parent, -1, "Layer Properies", wxDefaultPosition, wxSize(208, 72), wxDIALOG_MODAL | wxCAPTION | wxSYSTEM_MENU)
, m_Layer(layer)
, m_ParallaxAllowed(parallax_allowed)
{
  wxBoxSizer *mainsizer;
  wxBoxSizer *subsizer;
  wxStaticBoxSizer *boxsizer;
  wxFlexGridSizer *gridsizer;


  mainsizer = new wxBoxSizer(wxVERTICAL);
    gridsizer = new wxFlexGridSizer(2);
      gridsizer->Add(new wxStaticText(this, -1, "Name"));
      gridsizer->Add(m_LayerName = new wxTextCtrl(this, -1, m_Layer.GetName()));

      gridsizer->Add(new wxStaticText(this, -1, "Width"));
      gridsizer->Add(m_LayerWidth = new wxSpinCtrl(this, -1, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 4096, m_Layer.GetWidth()));

      gridsizer->Add(new wxStaticText(this, -1, "Height"));
      gridsizer->Add(m_LayerHeight = new wxSpinCtrl(this, -1, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 4096, m_Layer.GetHeight()));
    mainsizer->Add(gridsizer);

    mainsizer->Add(m_Reflective = new wxCheckBox(this, -1, "Reflective"));
    mainsizer->Add(m_HasParallax = new wxCheckBox(this, wID_LAYERPROPERTIESDIALOG_HAS_PARALAX, "Parallax and autoscrolling (one layer can't have this)"));

    boxsizer = new wxStaticBoxSizer(new wxStaticBox(this, -1, "Parallax"), wxVERTICAL);
      gridsizer = new wxFlexGridSizer(3);

        gridsizer->Add(new wxStaticText(this, -1, "Horizontal"));
        gridsizer->Add(m_ParallaxX = new wxSlider(this, -1, FloatToScrollValue(m_Layer.GetXParallax()), MIN_RANGE, MAX_RANGE));
        gridsizer->Add(m_ParallaxXText = new wxStaticText(this, -1, "Static"));

        gridsizer->Add(new wxStaticText(this, -1, "Vertical"));
        gridsizer->Add(m_ParallaxY = new wxSlider(this, -1, FloatToScrollValue(m_Layer.GetYParallax()), MIN_RANGE, MAX_RANGE));
        gridsizer->Add(m_ParallaxYText = new wxStaticText(this, -1, "Static"));

      boxsizer->Add(gridsizer);
    mainsizer->Add(boxsizer);

    boxsizer = new wxStaticBoxSizer(new wxStaticBox(this, -1, "Automatic scrolling"), wxVERTICAL);
      gridsizer = new wxFlexGridSizer(3);

        gridsizer->Add(new wxStaticText(this, -1, "Horizontal"));
        gridsizer->Add(m_ScrollX = new wxSlider(this, -1, FloatToScrollValue(m_Layer.GetXScrolling()), MIN_RANGE, MAX_RANGE));
        gridsizer->Add(m_ScrollXText = new wxStaticText(this, -1, "Static"));

        gridsizer->Add(new wxStaticText(this, -1, "Vertical"));
        gridsizer->Add(m_ScrollY = new wxSlider(this, -1, FloatToScrollValue(m_Layer.GetYScrolling()), MIN_RANGE, MAX_RANGE));
        gridsizer->Add(m_ScrollYText = new wxStaticText(this, -1, "Static"));

      boxsizer->Add(gridsizer);
    mainsizer->Add(boxsizer);

    subsizer = new wxBoxSizer(wxHORIZONTAL);
      subsizer->Add(new wxButton(this, wxID_OK, "OK"));
      subsizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"));
    mainsizer->Add(subsizer);

  SetSizer(mainsizer);
  mainsizer->SetSizeHints(this);

  m_Reflective->SetValue(m_Layer.IsReflective());
  UpdateParallaxEnabled();
  UpdateLabels();
  
  m_LayerWidth->SetValue(m_Layer.GetWidth());
  m_LayerHeight->SetValue(m_Layer.GetHeight());
}

/*
////////////////////////////////////////////////////////////////////////////////

BOOL
CLayerPropertiesDialog::OnInitDialog()
{
  // set the default values in the dialog box
  SetDlgItemText(IDC_NAME,  m_Layer.GetName());
  SetDlgItemInt(IDC_WIDTH,  m_Layer.GetWidth());
  SetDlgItemInt(IDC_HEIGHT, m_Layer.GetHeight());

  CEdit* name = (CEdit*)GetDlgItem(IDC_NAME);
  name->SetFocus();
  name->SetSel(0, -1); // select all of the text

  // check "reflective" button
  CheckDlgButton(IDC_REFLECTIVE, (m_Layer.IsReflective() ? BST_CHECKED : BST_UNCHECKED));

  // check "has parallax" button
  CheckDlgButton(IDC_HAS_PARALLAX, (m_Layer.HasParallax() ? BST_CHECKED : BST_UNCHECKED));

  // if parallax is not allowed, disable "enable parallax" button
  if (m_ParallaxAllowed == false) {
    GetDlgItem(IDC_HAS_PARALLAX)->EnableWindow(FALSE);
    CheckDlgButton(IDC_HAS_PARALLAX, BST_UNCHECKED);
  }

  // set scroller ranges
  SendDlgItemMessage(IDC_HORIZONTAL_PARALLAX,  TBM_SETRANGE, TRUE, MAKELONG(MIN_RANGE, MAX_RANGE));
  SendDlgItemMessage(IDC_VERTICAL_PARALLAX,    TBM_SETRANGE, TRUE, MAKELONG(MIN_RANGE, MAX_RANGE));
  SendDlgItemMessage(IDC_HORIZONTAL_SCROLLING, TBM_SETRANGE, TRUE, MAKELONG(MIN_RANGE, MAX_RANGE));
  SendDlgItemMessage(IDC_VERTICAL_SCROLLING,   TBM_SETRANGE, TRUE, MAKELONG(MIN_RANGE, MAX_RANGE));

  // set scroller positions
  SendDlgItemMessage(IDC_HORIZONTAL_PARALLAX,  TBM_SETPOS, TRUE, FloatToScrollValue(m_Layer.GetXParallax()));
  SendDlgItemMessage(IDC_VERTICAL_PARALLAX,    TBM_SETPOS, TRUE, FloatToScrollValue(m_Layer.GetYParallax()));
  SendDlgItemMessage(IDC_HORIZONTAL_SCROLLING, TBM_SETPOS, TRUE, FloatToScrollValue(m_Layer.GetXScrolling()));
  SendDlgItemMessage(IDC_VERTICAL_SCROLLING,   TBM_SETPOS, TRUE, FloatToScrollValue(m_Layer.GetYScrolling()));

  UpdateParallaxEnabled();
  UpdateLabels();

  return FALSE;  // we've set the focus
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wLayerPropertiesDialog::OnOK(wxCommandEvent &event)
{
  m_Layer.SetName(m_LayerName->GetValue().c_str());

  int width = m_LayerWidth->GetValue();
  int height = m_LayerHeight->GetValue();

  if (width < 1 || width > 4096 ||
      height < 1 || height > 4096
  ) {
    ::wxMessageBox("Width and height must be between 1 and 4096");
    return;
  }

  m_Layer.Resize(width, height);

  m_Layer.SetReflective(m_Reflective->GetValue());

  // close the dialog
  wxDialog::OnOK(event);
}

////////////////////////////////////////////////////////////////////////////////

void
wLayerPropertiesDialog::OnEnableParallax(wxCommandEvent &event)
{
  m_Layer.EnableParallax(!m_Layer.HasParallax());

  UpdateParallaxEnabled();
}

////////////////////////////////////////////////////////////////////////////////

void
wLayerPropertiesDialog::OnScroll(wxScrollEvent &event)
{
/*
  float32 f = ScrollValueToFloat(sb->SendMessage(TBM_GETPOS));

  // put the modified value back
  if (sb == GetDlgItem(IDC_HORIZONTAL_PARALLAX)) {
    m_Layer.SetXParallax(f);
  } else if (sb == GetDlgItem(IDC_VERTICAL_PARALLAX)) {
    m_Layer.SetYParallax(f);
  } else if (sb == GetDlgItem(IDC_HORIZONTAL_SCROLLING)) {
    m_Layer.SetXScrolling(f);
  } else if (sb == GetDlgItem(IDC_VERTICAL_SCROLLING)) {
    m_Layer.SetYScrolling(f);
  }
*/
  m_Layer.SetXParallax(ScrollValueToFloat(m_ParallaxX->GetValue()));
  m_Layer.SetYParallax(ScrollValueToFloat(m_ParallaxY->GetValue()));
  m_Layer.SetXScrolling(ScrollValueToFloat(m_ScrollX->GetValue()));
  m_Layer.SetYScrolling(ScrollValueToFloat(m_ScrollY->GetValue()));
  UpdateLabels();
}

////////////////////////////////////////////////////////////////////////////////

void
wLayerPropertiesDialog::UpdateParallaxEnabled()
{
  bool enabled = (m_ParallaxAllowed && m_Layer.HasParallax());//m_HasParallax->GetValue());
  
  m_HasParallax->SetValue(enabled);

  m_ParallaxX->Enable(enabled);
  m_ParallaxY->Enable(enabled);
  m_ScrollX->Enable(enabled);
  m_ScrollY->Enable(enabled);
}

////////////////////////////////////////////////////////////////////////////////

void
wLayerPropertiesDialog::UpdateLabels()
{
  char label[80];

  sprintf(label, "%.1f", m_Layer.GetXParallax());  m_ParallaxXText->SetLabel(label);
  sprintf(label, "%.1f", m_Layer.GetYParallax());  m_ParallaxYText->SetLabel(label);
  sprintf(label, "%.1f", m_Layer.GetXScrolling()); m_ScrollXText->SetLabel(label);
  sprintf(label, "%.1f", m_Layer.GetYScrolling()); m_ScrollYText->SetLabel(label);
}

////////////////////////////////////////////////////////////////////////////////
