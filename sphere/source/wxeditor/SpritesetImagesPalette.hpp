#ifndef SPRITESET_IMAGES_PALETTE_HPP
#define SPRITESET_IMAGES_PALETTE_HPP


#include "PaletteWindow.hpp"
#include "DIBSection.hpp"
#include "../common/Spriteset.hpp"


class wDocumentWindow;


struct ISpritesetImagesPaletteHandler
{
  virtual void SIP_IndexChanged(int index) = 0;
  virtual void SIP_SpritesetModified() = 0;  
};


class wSpritesetImagesPalette : public wPaletteWindow
{
public:
  wSpritesetImagesPalette(wDocumentWindow* owner, ISpritesetImagesPaletteHandler* handler, sSpriteset* spriteset);
  
  virtual bool Destroy();

private:
  void OnSize(wxSizeEvent &event);
  void OnPaint(wxPaintEvent &event);
  void OnLButtonDown(wxMouseEvent &event);
  void OnRButtonUp(wxMouseEvent &event);

  void OnMoveBack(wxEvent &event);
  void OnMoveForward(wxEvent &event);
  void OnInsertImage(wxEvent &event);
  void OnRemoveImage(wxEvent &event);

  void OnSwap(int new_index);

private:
  ISpritesetImagesPaletteHandler* m_Handler;
  sSpriteset* m_Spriteset;

  int m_TopRow;
  int m_ZoomFactor;

  int m_SelectedImage;
  wDIBSection* m_BlitImage;

  wxMenu *m_Menu;

private:
//  DECLARE_CLASS(wSpritesetImagesPalette);
  DECLARE_EVENT_TABLE();
};


#endif
