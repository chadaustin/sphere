#ifndef SPRITESET_VIEW_HPP
#define SPRITESET_VIEW_HPP


#include <afxwin.h>
#include "../common/Spriteset.hpp"
#include "DIBSection.hpp"


class ISpritesetViewHandler
{
public:
  virtual void SV_CurrentFrameChanged(int direction, int frame) = 0;
  virtual void SV_EditFrame() = 0;
  virtual void SV_SpritesetModified() = 0;
  virtual void SV_CopyCurrentFrame() = 0;
  virtual void SV_PasteCurrentFrame() = 0;
};


class CSpritesetView : public CWnd
{
public:
  CSpritesetView();
  ~CSpritesetView();

  BOOL Create(CWnd* parent, ISpritesetViewHandler* handler, sSpriteset* spriteset);

  void SetDirection(int direction);
  void SetFrame(int frame);
  
  void SetZoomFactor(int zoom);
  int GetZoomFactor() const;

  void SpritesetResized();

private:
  void UpdateMaxSizes();
  void DrawDirection(HDC dc, int direction, int y);
  void DrawFrame(HDC dc, int x, int y, int direction, int frame);
  void UpdateDrawBitmap(int direction, int frame);
  void UpdateScrollBars();
  void InvalidateFrame(int direction, int frame);

  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnLButtonDown(UINT flags, CPoint point);
  afx_msg void OnRButtonUp(UINT flags, CPoint point);
  afx_msg void OnLButtonDblClk(UINT flags, CPoint point);
  afx_msg void OnHScroll(UINT code, UINT pos, CScrollBar* scroll_bar);
  afx_msg void OnVScroll(UINT code, UINT pos, CScrollBar* scroll_bar);
  afx_msg void OnPaint();

  afx_msg void OnInsertDirection();
  afx_msg void OnDeleteDirection();
  afx_msg void OnAppendDirection();
  afx_msg void OnDirectionProperties();

  afx_msg void OnInsertFrame();
  afx_msg void OnDeleteFrame();
  afx_msg void OnAppendFrame();
  afx_msg void OnCopyFrame();
  afx_msg void OnPasteFrame();
  afx_msg void OnFrameProperties();

private:
  ISpritesetViewHandler* m_Handler;

  sSpriteset* m_Spriteset;
  int m_CurrentDirection;
  int m_CurrentFrame;
  
  CDIBSection* m_DrawBitmap;

  int m_ZoomFactor;
  int m_MaxFrameWidth;
  int m_MaxFrameHeight;

  // used for scrolling
  int m_TopDirection;
  int m_LeftFrame;

  bool m_MenuDisplayed;

  DECLARE_MESSAGE_MAP()
};


#endif