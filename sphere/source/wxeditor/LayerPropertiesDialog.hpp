#ifndef LAYER_PROPERTIES_DIALOG_HPP
#define LAYER_PROPERTIES_DIALOG_HPP


//#include <afxwin.h>
#include <wx/wx.h>
#include <wx/spinctrl.h>

class sLayer;  // #include "../sphere/common/Map.hpp"


class wLayerPropertiesDialog : public wxDialog
{
public:
  wLayerPropertiesDialog(wxWindow *parent, sLayer& layer, bool parallax_allowed);

  //virtual bool OnInitDialog();
  virtual void OnOK(wxCommandEvent &event);

  void OnEnableParallax(wxCommandEvent &event);
  void OnScroll(wxScrollEvent &event);

  void UpdateParallaxEnabled();
  void UpdateLabels();

private:
  sLayer& m_Layer;
  bool m_ParallaxAllowed;

  wxTextCtrl *m_LayerName;

  wxSpinCtrl *m_LayerWidth;
  wxSpinCtrl *m_LayerHeight;

  wxCheckBox *m_Reflective;
  wxCheckBox *m_HasParallax;

  wxSlider *m_ParallaxX;
  wxSlider *m_ParallaxY;
  wxSlider *m_ScrollX;
  wxSlider *m_ScrollY;

  wxStaticText *m_ParallaxXText;
  wxStaticText *m_ParallaxYText;
  wxStaticText *m_ScrollXText;
  wxStaticText *m_ScrollYText;

private:
//  DECLARE_CLASS(wLayerPropertiesDialog)
  DECLARE_EVENT_TABLE()
};


#endif
