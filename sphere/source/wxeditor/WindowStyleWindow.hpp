#ifndef WINDOW_STYLE_WINDOW_HPP
#define WINDOW_STYLE_WINDOW_HPP


#include "SaveableDocumentWindow.hpp"
#include "DIBSection.hpp"
#include "../common/WindowStyle.hpp"
#include "ImageView.hpp"
#include "PaletteView.hpp"
#include "ColorView.hpp"
#include "AlphaView.hpp"


class wProject;  // #include "Project.hpp"


class wWindowStyleWindow
  : public wSaveableDocumentWindow
  , private IImageViewHandler
  , private IPaletteViewHandler
  , private IColorViewHandler
  , private IAlphaViewHandler
{
public:
  wWindowStyleWindow(const char* window_style = NULL);
  ~wWindowStyleWindow();

private:
  void UpdateDIBSection(int bitmap);
  void UpdateDIBSections();
  void SetBitmap();
  void SetZoomFactor(int factor);
  void SelectBitmap(int bitmap);

  void DrawBit(wxDC& dc, int bitmap, int x, int y, int w, int h);
  //void DrawCorner(wxDC& dc, int bitmap, int x, int y, int w, int h);
  //void DrawEdgeH(wxDC& dc, int bitmap, int x, int y, int x2, int h);
  //void DrawEdgeV(wxDC& dc, int bitmap, int x, int y, int y2, int w);

  void OnLButtonDown(wxMouseEvent &event);
  void OnSize(wxSizeEvent &event);
  void OnPaint(wxPaintEvent &event);

  void OnEditUpperLeft(wxCommandEvent &event);
  void OnEditTop(wxCommandEvent &event);
  void OnEditUpperRight(wxCommandEvent &event);
  void OnEditRight(wxCommandEvent &event);
  void OnEditLowerRight(wxCommandEvent &event);
  void OnEditBottom(wxCommandEvent &event);
  void OnEditLowerLeft(wxCommandEvent &event);
  void OnEditLeft(wxCommandEvent &event);
  void OnEditBackground(wxCommandEvent &event);

  void OnResizeSection(wxCommandEvent &event);

  void OnZoom1x(wxCommandEvent &event);
  void OnZoom2x(wxCommandEvent &event);
  void OnZoom4x(wxCommandEvent &event);
  void OnZoom8x(wxCommandEvent &event);

  void OnProperties(wxCommandEvent &event);

/*todo:
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
*/
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
  int GetBackgroundWidth() const;
  int GetBackgroundHeight() const;
  void GetEditRect(wxRect* rect);
  void GetInnerRect(wxRect* rect);

private:
  sWindowStyle m_WindowStyle;

  int m_SelectedBitmap;
  int m_ZoomFactor;

  wDIBSection* m_DIBs[9];  // elements of window style

  bool         m_Created;
  wImageView   *m_ImageView;
  wPaletteView *m_PaletteView;
  wColorView   *m_ColorView;
  wAlphaView   *m_AlphaView;

  wxPen m_HighlightPen;

private:
//  DECLARE_CLASS(wWindowStyleWindow)
  DECLARE_EVENT_TABLE()
};


#endif
