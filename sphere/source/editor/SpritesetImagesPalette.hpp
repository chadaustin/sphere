#ifndef SPRITESET_IMAGES_PALETTE_HPP
#define SPRITESET_IMAGES_PALETTE_HPP


#include "PaletteWindow.hpp"
#include "DIBSection.hpp"
#include "../common/Spriteset.hpp"


class CDocumentWindow;


struct ISpritesetImagesPaletteHandler
{
  virtual void SIP_IndexChanged(int index) = 0;
  virtual void SIP_SpritesetModified() = 0;  
};


class CSpritesetImagesPalette : public CPaletteWindow
{
public:
  CSpritesetImagesPalette(CDocumentWindow* owner, ISpritesetImagesPaletteHandler* handler, sSpriteset* spriteset);
  
  virtual void Destroy();

  void SetCurrentImage(int image);

private:
  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnPaint();
  afx_msg void OnLButtonDown(UINT flags, CPoint point);
  afx_msg void OnRButtonUp(UINT flags, CPoint point);

  afx_msg void OnMoveBack();
  afx_msg void OnMoveForward();
  afx_msg void OnInsertImage();
  afx_msg void OnRemoveImage();

  void OnSwap(int new_index);

  void OnZoom(double zoom);

private:
  ISpritesetImagesPaletteHandler* m_Handler;
  sSpriteset* m_Spriteset;

  int m_TopRow;
  double m_ZoomFactor;

  int m_SelectedImage;
  CDIBSection* m_BlitImage;

  DECLARE_MESSAGE_MAP()
};


#endif
