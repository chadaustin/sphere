#ifndef FONT_PREVIEW_PALETTE_HPP
#define FONT_PREVIEW_PALETTE_HPP


#include "PaletteWindow.hpp"
#include "DIBSection.hpp"
#include "../common/Font.hpp"

class CDocumentWindow;


class CFontPreviewPalette : public CPaletteWindow
{
public:
  CFontPreviewPalette(CDocumentWindow* owner, sFont* font);
  virtual void Destroy();

	void OnCharacterChanged(int character);
  afx_msg void OnSetText();

private:
	void OnZoom(double zoom);

  afx_msg void OnPaint();
  afx_msg void OnRButtonUp(UINT flags, CPoint point);
	afx_msg void OnTimer(UINT event);
  BOOL OnEraseBkgnd(CDC* pDC);

private:
  sFont* m_Font;
	double m_ZoomFactor;
  int m_RedrawCharacter;
  std::string m_Text;

  CDIBSection* m_BlitImage;
  DECLARE_MESSAGE_MAP()
};


#endif