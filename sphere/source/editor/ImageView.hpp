#ifndef IMAGE_VIEW_HPP
#define IMAGE_VIEW_HPP


#include <afxwin.h>
#include "../common/Image32.hpp"
#include "SwatchPalette.hpp"
#include "ImageToolPalette.hpp"


class CDocumentWindow;  // #include "DocumentWindow.hpp"


class IImageViewHandler
{
public:
  virtual void IV_ImageChanged() = 0;
  virtual void IV_ColorChanged(RGBA color) = 0;
};


class CImageView :
  public CWnd,
  public ISwatchPaletteHandler,
  public IToolPaletteHandler
{
public:
  CImageView();
  ~CImageView();

  BOOL Create(CDocumentWindow* owner, IImageViewHandler* handler, CWnd* parent_window);

  bool        SetImage(int width, int height, const RGBA* pixels);
  int         GetWidth() const;
  int         GetHeight() const;
  RGBA*       GetPixels();
  const RGBA* GetPixels() const;

  void SetColor(RGBA color);
  RGBA GetColor() const;

  void FillRGB();
  void FillAlpha();

  bool Copy();
  bool Paste();

  bool CanUndo() const;
  void Undo();

private:
  enum Tool { Tool_Pencil, Tool_Line, Tool_Rectangle, Tool_Circle, Tool_Fill };

private:
  virtual void SP_ColorSelected(RGBA color);
  virtual void TP_ToolSelected(int tool);

  POINT ConvertToPixel(POINT point);

  void Click(bool force_draw);
  void Fill();
  void FillMe(int x, int y, RGBA colorToReplace);
  void Line();
  void Rectangle();
  void Circle();
  void GetColor(RGBA* color, int x, int y);

  void AddUndoState();
  void ResetUndoStates();

  afx_msg void OnPaint();
  void PaintLine(CImage32& pImage);
  void PaintRectangle(CImage32& pImage);
  void PaintCircle(CImage32& pImage);
  afx_msg void OnSize(UINT type, int cx, int cy);

  afx_msg void OnLButtonDown(UINT flags, CPoint point);
  afx_msg void OnLButtonUp(UINT flags, CPoint point);
  afx_msg void OnRButtonUp(UINT flags, CPoint point);
  afx_msg void OnMouseMove(UINT flags, CPoint point);

  afx_msg void OnColorPicker();
  afx_msg void OnUndo();
  afx_msg void OnCopy();
  afx_msg void OnPaste();
  afx_msg void OnBlendModeBlend();
  afx_msg void OnBlendModeReplace();
  afx_msg void OnBlendModeRGBOnly();
  afx_msg void OnBlendModeAlphaOnly();
  afx_msg void OnRotateCW();
  afx_msg void OnRotateCCW();
  afx_msg void OnSlideUp();
  afx_msg void OnSlideRight();
  afx_msg void OnSlideDown();
  afx_msg void OnSlideLeft();
  afx_msg void OnFlipHorizontally();
  afx_msg void OnFlipVertically();
  afx_msg void OnFillRGB();
  afx_msg void OnFillAlpha();
  afx_msg void OnFillBoth();
  afx_msg void OnFilterBlur();
  afx_msg void OnFilterNoise();
  afx_msg void OnSetColorAlpha();
  afx_msg void OnScaleAlpha();

private:
  struct Image
  {
    int width, height;
    RGBA* pixels;
  };

private:
  IImageViewHandler* m_Handler;

  CImage32 m_Image;

  CSwatchPalette*    m_SwatchPalette;
  CImageToolPalette* m_ToolPalette;

  // drawing
  RGBA  m_Color;
  POINT m_StartPoint;
  POINT m_CurPoint;
  POINT m_LastPoint;
  bool  m_MouseDown;
  int   m_CurrentTool;

  // undo
  int    m_NumUndoImages;
  Image* m_UndoImages;

  DECLARE_MESSAGE_MAP()
};


#endif