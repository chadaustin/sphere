#ifndef PALETTE_VIEW_HPP
#define PALETTE_VIEW_HPP



#include <afxwin.h>
#include "../common/rgb.hpp"


class CDIBSection;  // #include "DIBSection.hpp"



class IPaletteViewHandler
{
public:
  virtual void PV_ColorChanged(RGB color) = 0;
};


class CPaletteView : public CWnd
{
public:
  CPaletteView();
  ~CPaletteView();

  BOOL Create(IPaletteViewHandler* pHandler, CWnd* pParentWindow);

  RGB GetColor() const;

private:
  void UpdatePalette(int w, int h, int pitch, BGR* pixels);

  afx_msg void OnPaint();
  afx_msg void OnLButtonDown(UINT flags, CPoint point);
  afx_msg void OnLButtonUp(UINT flags, CPoint point);
  afx_msg void OnMouseMove(UINT flags, CPoint point);
  afx_msg void OnSize(UINT type, int cx, int cy);

private:
  IPaletteViewHandler* m_pHandler;

  CDIBSection* m_pPaletteDIB;
  RGB          m_Color;
  bool         m_bMouseDown;

  DECLARE_MESSAGE_MAP()
};



#endif