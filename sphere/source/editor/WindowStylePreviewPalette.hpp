#ifndef WINDOWSTYLE_PREVIEW_PALETTE_HPP
#define WINDOWSTYLE_PREVIEW_PALETTE_HPP


#include "PaletteWindow.hpp"
#include "DIBSection.hpp"
#include "../common/WindowStyle.hpp"


class CDocumentWindow;


class CWindowStylePreviewPalette : public CPaletteWindow
{
public:
  CWindowStylePreviewPalette(CDocumentWindow* owner, sWindowStyle* windowstyle);
  
  virtual void Destroy();

	void OnWindowStyleChanged();

private:
	void OnZoom(double zoom);

  afx_msg void OnPaint();
  afx_msg void OnRButtonUp(UINT flags, CPoint point);
	afx_msg void OnTimer(UINT event);

private:
  sWindowStyle* m_WindowStyle;
	double m_ZoomFactor;

  CDIBSection* m_BlitImage;
  DECLARE_MESSAGE_MAP()
};


#endif
