#ifndef WINDOW_STYLE_WINDOW_HPP
#define WINDOW_STYLE_WINDOW_HPP


#include "SaveableDocumentWindow.hpp"
#include "DIBSection.hpp"
#include "../common/WindowStyle.hpp"
#include "ImageView.hpp"
#include "PaletteView.hpp"
#include "ColorView.hpp"
#include "AlphaView.hpp"


class CProject;  // #include "Project.hpp"


class CWindowStyleWindow
  : public CSaveableDocumentWindow
  , private IImageViewHandler
  , private IPaletteViewHandler
  , private IColorViewHandler
  , private IAlphaViewHandler
{
public:
  CWindowStyleWindow(const char* window_style = NULL);
  ~CWindowStyleWindow();

private:
  void UpdateDIBSection(int bitmap);
  void UpdateDIBSections();
  void SetBitmap();
  void SetZoomFactor(int factor);
  void SelectBitmap(int bitmap);

  void DrawCorner(CDC& dc, int bitmap, int x, int y, int w, int h);
  void DrawEdgeH(CDC& dc, int bitmap, int x, int y, int x2, int h);
  void DrawEdgeV(CDC& dc, int bitmap, int x, int y, int y2, int w);

  afx_msg void OnLButtonDown(UINT flags, CPoint point);
  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnPaint();

  afx_msg void OnEditUpperLeft();
  afx_msg void OnEditTop();
  afx_msg void OnEditUpperRight();
  afx_msg void OnEditRight();
  afx_msg void OnEditLowerRight();
  afx_msg void OnEditBottom();
  afx_msg void OnEditLowerLeft();
  afx_msg void OnEditLeft();
  afx_msg void OnEditBackground();

  afx_msg void OnResizeSection();

  afx_msg void OnZoom1x();
  afx_msg void OnZoom2x();
  afx_msg void OnZoom4x();
  afx_msg void OnZoom8x();

  afx_msg void OnProperties();

  afx_msg void OnUpdateEditUpperLeft (CCmdUI* cmdui);
  afx_msg void OnUpdateEditTop       (CCmdUI* cmdui);
  afx_msg void OnUpdateEditUpperRight(CCmdUI* cmdui);
  afx_msg void OnUpdateEditRight     (CCmdUI* cmdui);
  afx_msg void OnUpdateEditLowerRight(CCmdUI* cmdui);
  afx_msg void OnUpdateEditBottom    (CCmdUI* cmdui);
  afx_msg void OnUpdateEditLowerLeft (CCmdUI* cmdui);
  afx_msg void OnUpdateEditLeft      (CCmdUI* cmdui);
  afx_msg void OnUpdateEditBackground(CCmdUI* cmdui);

  afx_msg void OnUpdateZoom1x(CCmdUI* cmdui);
  afx_msg void OnUpdateZoom2x(CCmdUI* cmdui);
  afx_msg void OnUpdateZoom4x(CCmdUI* cmdui);
  afx_msg void OnUpdateZoom8x(CCmdUI* cmdui);

  virtual bool GetSavePath(char* path);
  virtual bool SaveDocument(const char* path);

  // view handlers
  virtual void IV_ImageChanged();
  virtual void IV_ColorChanged(RGBA color);
  virtual void PV_ColorChanged(RGB color);
  virtual void CV_ColorChanged(RGB color);
  virtual void AV_AlphaChanged(byte alpha);

  int GetBorderWidth_Left() const;
  int GetBorderWidth_Top() const;
  int GetBorderWidth_Right() const;
  int GetBorderWidth_Bottom() const;
  void GetEditRect(RECT* rect);

private:
  sWindowStyle m_WindowStyle;

  int m_SelectedBitmap;
  int m_ZoomFactor;

  CDIBSection* m_DIBs[9];  // elements of window style

  bool         m_Created;
  CImageView   m_ImageView;
  CPaletteView m_PaletteView;
  CColorView   m_ColorView;
  CAlphaView   m_AlphaView;

  CPen* m_HighlightPen;

  DECLARE_MESSAGE_MAP()
};


#endif