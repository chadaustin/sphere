#ifndef TILE_PREVIEW_PALETTE_HPP
#define TILE_PREVIEW_PALETTE_HPP


#include "PaletteWindow.hpp"
#include "DIBSection.hpp"
#include "../common/Image32.hpp"


class CDocumentWindow;


class CTilePreviewPalette : public CPaletteWindow
{
public:
  CTilePreviewPalette(CDocumentWindow* owner, CImage32 image);
  
  virtual void Destroy();

	void OnImageChanged(CImage32 image);

private:
	void OnZoom(double zoom);

  afx_msg void OnPaint();
  afx_msg void OnRButtonUp(UINT flags, CPoint point);
	afx_msg void OnTimer(UINT event);
  BOOL OnEraseBkgnd(CDC* pDC);

private:
  CImage32 m_Image;
	double m_ZoomFactor;

  CDIBSection* m_BlitImage;
  DECLARE_MESSAGE_MAP()
};


#endif
