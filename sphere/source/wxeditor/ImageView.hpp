#ifndef IMAGE_VIEW_HPP
#define IMAGE_VIEW_HPP


//#include <afxwin.h>
#include <wx/wx.h>
#include "../common/Image32.hpp"
#include "SwatchPalette.hpp"
#include "ImageToolPalette.hpp"


class wDocumentWindow;  // #include "DocumentWindow.hpp"


class IImageViewHandler
{
public:
  virtual void IV_ImageChanged() = 0;
  virtual void IV_ColorChanged(RGBA color) = 0;
};


class wImageView :
  public wxWindow,
  public ISwatchPaletteHandler,
  public IToolPaletteHandler
{
public:
  wImageView(wxWindow* parent_window, wDocumentWindow* owner, IImageViewHandler* handler);
  ~wImageView();

  //BOOL Create(CDocumentWindow* owner, IImageViewHandler* handler, CWnd* parent_window);

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

  wxPoint ConvertToPixel(wxPoint point);
  bool InImage(wxPoint p);

  void Click(bool force_draw);
  void Fill();
  void FillMe(int x, int y, RGBA colorToReplace);
  void Line();
  void Rectangle();
  void Circle();
  void GetColor(RGBA* color, int x, int y);

  void AddUndoState();
  void ResetUndoStates();

  void OnPaint(wxPaintEvent &event);
  void PaintLine(CImage32& pImage);
  void PaintRectangle(CImage32& pImage);
  void PaintCircle(CImage32& pImage);
  void OnSize(wxSizeEvent &event);

  void OnLButtonDown(wxMouseEvent &event);
  void OnLButtonUp(wxMouseEvent &event);
  void OnRButtonUp(wxMouseEvent &event);
  void OnMouseMove(wxMouseEvent &event);

  void OnColorPicker(wxCommandEvent &event);
  void OnUndo(wxCommandEvent &event);
  void OnCopy(wxCommandEvent &event);
  void OnPaste(wxCommandEvent &event);
  void OnBlendModeBlend(wxCommandEvent &event);
  void OnBlendModeReplace(wxCommandEvent &event);
  void OnBlendModeRGBOnly(wxCommandEvent &event);
  void OnBlendModeAlphaOnly(wxCommandEvent &event);
  void OnRotateCW(wxCommandEvent &event);
  void OnRotateCCW(wxCommandEvent &event);
  void OnSlideUp(wxCommandEvent &event);
  void OnSlideRight(wxCommandEvent &event);
  void OnSlideDown(wxCommandEvent &event);
  void OnSlideLeft(wxCommandEvent &event);
  void OnFlipHorizontally(wxCommandEvent &event);
  void OnFlipVertically(wxCommandEvent &event);
  void OnFillRGB(wxCommandEvent &event);
  void OnFillAlpha(wxCommandEvent &event);
  void OnFillBoth(wxCommandEvent &event);
  void OnReplaceRGBA(wxCommandEvent &event);
  void OnReplaceRGB(wxCommandEvent &event);
  void OnReplaceAlpha(wxCommandEvent &event);
  void OnFilterBlur(wxCommandEvent &event);
  void OnFilterNoise(wxCommandEvent &event);
  void OnSetColorAlpha(wxCommandEvent &event);
  void OnScaleAlpha(wxCommandEvent &event);

private:
  struct Image
  {
    int width, height;
    RGBA* pixels;
  };

private:
  IImageViewHandler* m_Handler;

  CImage32 m_Image;

  wSwatchPalette*    m_SwatchPalette;
  wImageToolPalette* m_ToolPalette;

  // drawing
  RGBA  m_Color;
  wxPoint m_StartPoint;
  wxPoint m_CurPoint;
  wxPoint m_LastPoint;
  bool  m_MouseDown;
  int   m_CurrentTool;

  // undo
  int    m_NumUndoImages;
  Image* m_UndoImages;

  wxMenu *m_Menu;

private:
//  DECLARE_CLASS(wImageView);
  DECLARE_EVENT_TABLE();
};


#endif
