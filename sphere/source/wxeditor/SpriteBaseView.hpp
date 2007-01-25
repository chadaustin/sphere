#ifndef SPRITE_BASE_VIEW_HPP
#define SPRITE_BASE_VIEW_HPP


//#include <afxwin.h>
#include <wx/wx.h>
#include "../common/Spriteset.hpp"


class ISpriteBaseViewHandler
{
public:
  virtual void SBV_SpritesetModified() = 0;
};


class wSpriteBaseView : public wxWindow
{
public:
  wSpriteBaseView(wxWindow* parent, ISpriteBaseViewHandler* handler, sSpriteset* spriteset);
  ~wSpriteBaseView();

  //BOOL Create(CWnd* parent, ISpriteBaseViewHandler* handler, sSpriteset* spriteset);
  void SetSprite(const CImage32* sprite);

private:
  void BeginDrag(int x, int y);
  void Drag(int x, int y);
  void WindowToImage(int* x, int* y);

  void OnSize(wxSizeEvent &event);
  void OnPaint(wxPaintEvent &event);

  void OnLButtonDown(wxMouseEvent &event);
  void OnLButtonUp(wxMouseEvent &event);
  void OnMouseMove(wxMouseEvent &event);

private:
  ISpriteBaseViewHandler* m_Handler;
  
  sSpriteset*     m_Spriteset;
  const CImage32* m_Sprite;

  bool m_MouseDown;

private:
//  DECLARE_CLASS(wSpriteBaseView);
  DECLARE_EVENT_TABLE();
};


#endif
