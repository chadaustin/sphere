#ifndef FONT_WINDOW_HPP
#define FONT_WINDOW_HPP


#include "SaveableDocumentWindow.hpp"
#include "ImageView.hpp"
#include "PaletteView.hpp"
#include "ColorView.hpp"
#include "AlphaView.hpp"
#include "../common/Font.hpp"


class CFontWindow
: public CSaveableDocumentWindow
, private IImageViewHandler
, private IPaletteViewHandler
, private IColorViewHandler
, private IAlphaViewHandler
{
public:
  CFontWindow(const char* font = NULL);
  ~CFontWindow();

private:
  void Create();
  void UpdateWindowTitle();
  void SetImage();

  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnHScroll(UINT sbcode, UINT pos, CScrollBar* scroll_bar);

  afx_msg void OnFontResize();
  afx_msg void OnFontResizeAll();
  afx_msg void OnFontSimplify();
  afx_msg void OnFontGenerateGradient();

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
  CImageView   m_ImageView;
  CPaletteView m_PaletteView;
  CColorView   m_ColorView;
  CAlphaView   m_AlphaView;
  CScrollBar   m_ScrollBar;

  DECLARE_MESSAGE_MAP()
};


#endif
