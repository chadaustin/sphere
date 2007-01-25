#ifndef IMAGE_WINDOW_HPP
#define IMAGE_WINDOW_HPP


//#include <afxcmn.h>
#include <wx/wx.h>
#include "SaveableDocumentWindow.hpp"
#include "../common/Image32.hpp"
#include "ImageView.hpp"
#include "PaletteView.hpp"
#include "ColorView.hpp"
#include "AlphaView.hpp"


class wImageWindow
  : public wSaveableDocumentWindow
  , private IImageViewHandler
  , private IPaletteViewHandler
  , private IColorViewHandler
  , private IAlphaViewHandler
{
public:
  wImageWindow(const char* image = NULL);
  ~wImageWindow();

private:
  void UpdateImageView();

  void OnSize(wxSizeEvent &event);

  void OnImageResize(wxCommandEvent &event);
  void OnImageRescale(wxCommandEvent &event);
  void OnImageViewOriginalSize(wxCommandEvent &event);

  virtual bool GetSavePath(char* path);
  virtual bool SaveDocument(const char* path);

  // view handlers
  virtual void IV_ImageChanged();
  virtual void IV_ColorChanged(RGBA color);
  virtual void PV_ColorChanged(RGB color);
  virtual void CV_ColorChanged(RGB color);
  virtual void AV_AlphaChanged(byte alpha);

private:
  bool       m_Created;
  CImage32   m_Image;
  wImageView *m_ImageView;

  wPaletteView *m_PaletteView;
  wColorView   *m_ColorView;
  wAlphaView   *m_AlphaView;

private:
//  DECLARE_CLASS(wImageWindow)
  DECLARE_EVENT_TABLE()
};


#endif
