#ifndef SPRITESET_VIEW_HPP
#define SPRITESET_VIEW_HPP


//#include <afxwin.h>
#include <wx/wx.h>
#include "../common/Spriteset.hpp"
#include "DIBSection.hpp"
#include "ScrollWindow.hpp"


class ISpritesetViewHandler
{
public:
  virtual void SV_CurrentFrameChanged(int direction, int frame) = 0;
  virtual void SV_EditFrame() = 0;
  virtual void SV_SpritesetModified() = 0;
  virtual void SV_CopyCurrentFrame() = 0;
  virtual void SV_PasteCurrentFrame() = 0;
};


class wSpritesetView : public wScrollWindow
{
public:
  wSpritesetView(wxWindow* parent, ISpritesetViewHandler* handler, sSpriteset* spriteset);
  ~wSpritesetView();

  //BOOL Create(CWnd* parent, ISpritesetViewHandler* handler, sSpriteset* spriteset);

  void SetDirection(int direction);
  void SetFrame(int frame);
  
  void SetZoomFactor(int zoom);
  int GetZoomFactor() const;

  void SpritesetResized();

  void OnSize(wxSizeEvent &event);

private:
  void UpdateMaxSizes();
  void DrawDirection(wxDC &dc, int direction, int y);
  void DrawFrame(wxDC &dc, int x, int y, int direction, int frame);
  void UpdateDrawBitmap(int direction, int frame);
  void UpdateScrollBars();
  void InvalidateFrame(int direction, int frame);

  void OnLButtonDown(wxMouseEvent &event);
  void OnRButtonUp(wxMouseEvent &event);
  void OnLButtonDblClk(wxMouseEvent &event);
//  void OnScroll(wxScrollEvent &event);
//  void OnHScroll(wxScrollEvent &event);
//  void OnVScroll(wxScrollEvent &event);
  void OnPaint(wxPaintEvent &event);

  void OnInsertDirection(wxCommandEvent &event);
  void OnDeleteDirection(wxCommandEvent &event);
  void OnAppendDirection(wxCommandEvent &event);
  void OnDirectionProperties(wxCommandEvent &event);

  void OnInsertFrame(wxCommandEvent &event);
  void OnDeleteFrame(wxCommandEvent &event);
  void OnAppendFrame(wxCommandEvent &event);
  void OnCopyFrame(wxCommandEvent &event);
  void OnPasteFrame(wxCommandEvent &event);
  void OnFrameProperties(wxCommandEvent &event);

  void OnHScrollChanged(int x);
  void OnVScrollChanged(int y);

private:
  ISpritesetViewHandler* m_Handler;

  sSpriteset* m_Spriteset;
  int m_CurrentDirection;
  int m_CurrentFrame;
  
  wDIBSection* m_DrawBitmap;

  int m_ZoomFactor;
  int m_MaxFrameWidth;
  int m_MaxFrameHeight;

  // used for scrolling
  int m_TopDirection;
  int m_LeftFrame;

  bool m_MenuDisplayed;

  wxMenu *m_MenuDirection;
  wxMenu *m_MenuFrame;

private:
//  DECLARE_CLASS(wSpritesetView);
  DECLARE_EVENT_TABLE();
};


#endif
