#ifndef PALETTE_WINDOW_HPP
#define PALETTE_WINDOW_HPP


//#include <afxwin.h>
#include <wx/wx.h>
#include <wx/minifram.h>
class wDocumentWindow;  // #include "DocumentWindow.hpp"


//const int WM_UPDATE_PALETTE_MENU = (WM_APP + 800);

#ifdef OPT_MOTIF
class wPaletteWindow : public wxMiniFrame
#else
class wPaletteWindow : public wxFrame
#endif
{
protected:
  // protected so only derived classes can construct
  wPaletteWindow(wDocumentWindow* owner, const char* name, wxRect rect = wxRect(wxPoint(64, 64), wxSize(256, 256)), bool visible = true);
  ~wPaletteWindow();

private:
  //void OnDestroy();
  void OnClose(wxCloseEvent& event);
  //void OnShowWindow(BOOL show, UINT status);

private:
  wDocumentWindow* m_Owner;

private:
//  DECLARE_CLASS(wPaletteWindow);
  DECLARE_EVENT_TABLE();
};



#endif
