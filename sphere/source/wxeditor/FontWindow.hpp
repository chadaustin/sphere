#ifndef FONT_WINDOW_HPP
#define FONT_WINDOW_HPP


#include "SaveableDocumentWindow.hpp"
#include "ImageView.hpp"
#include "PaletteView.hpp"
#include "ColorView.hpp"
#include "AlphaView.hpp"
#include "../common/Font.hpp"


class wFontWindow
: public wSaveableDocumentWindow
, private IImageViewHandler
, private IPaletteViewHandler
, private IColorViewHandler
, private IAlphaViewHandler
{
public:
  wFontWindow(const char* font = NULL);
  ~wFontWindow();

private:
  void Create();
  void UpdateWindowTitle();
  void SetImage();

  void OnSize(wxSizeEvent &event);
  void OnHScroll(wxScrollEvent &event);

  void OnFontResize(wxCommandEvent &event);
  void OnFontResizeAll(wxCommandEvent &event);
  void OnFontSimplify(wxCommandEvent &event);
  void OnFontMakeColorTransparent(wxCommandEvent &event);
  void OnFontGenerateGradient(wxCommandEvent &event);

  virtual bool GetSavePath(char* path);
  virtual bool SaveDocument(const char* path);

  // view handlers
  virtual void IV_ImageChanged();
  virtual void IV_ColorChanged(RGBA color);
  virtual void PV_ColorChanged(RGB color);
  virtual void CV_ColorChanged(RGB color);
  virtual void AV_AlphaChanged(byte alpha);

private:
  sFont m_Font;
  int   m_CurrentCharacter;
  RGBA  m_CurrentColor;

  bool         m_Created;
  wImageView   *m_ImageView;
  wPaletteView *m_PaletteView;
  wColorView   *m_ColorView;
  wAlphaView   *m_AlphaView;
  wxScrollBar  *m_ScrollBar;

private:
//  DECLARE_CLASS(wFontWindow)
  DECLARE_EVENT_TABLE()
};


#endif
