#ifndef PALETTE_WINDOW_HPP
#define PALETTE_WINDOW_HPP


#include <afxwin.h>


class CDocumentWindow;  // #include "DocumentWindow.hpp"


const int WM_UPDATE_PALETTE_MENU = (WM_APP + 800);


class CPaletteWindow : public CMiniFrameWnd
{
protected:
  // protected so only derived classes can construct
  CPaletteWindow(CDocumentWindow* owner, const char* name, RECT rect, bool visible);
  ~CPaletteWindow();

private:
  afx_msg void OnDestroy();
  afx_msg void OnClose();
  afx_msg void OnShowWindow(BOOL show, UINT status);

private:
  CDocumentWindow* m_Owner;

  DECLARE_MESSAGE_MAP()
};



#endif
