#ifndef IMAGE_WINDOW_HPP
#define IMAGE_WINDOW_HPP


#include <afxcmn.h>
#include "SaveableDocumentWindow.hpp"
#include "../common/Image32.hpp"
#include "ImageView.hpp"
#include "PaletteView.hpp"
#include "ColorView.hpp"
#include "AlphaView.hpp"


class CImageWindow
  : public CSaveableDocumentWindow
  , private IImageViewHandler
  , private IPaletteViewHandler
  , private IColorViewHandler
  , private IAlphaViewHandler
{
public:
  CImageWindow(const char* image = NULL);
  ~CImageWindow();

private:
  void UpdateImageView();

  afx_msg void OnSize(UINT type, int cx, int cy);

  afx_msg void OnImageResize();
  afx_msg void OnImageRescale();
  afx_msg void OnImageResample();
  afx_msg void OnImageRotate();
  afx_msg void OnImageViewOriginalSize();

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
  CImageView m_ImageView;

  CPaletteView m_PaletteView;
  CColorView   m_ColorView;
  CAlphaView   m_AlphaView;

  DECLARE_MESSAGE_MAP()
};


#endif
