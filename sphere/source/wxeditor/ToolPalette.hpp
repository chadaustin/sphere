#ifndef TOOL_PALETTE_HPP
#define TOOL_PALETTE_HPP


#pragma warning(disable : 4786)


//#include <afxcmn.h>
#include <vector>
#include "PaletteWindow.hpp"


class wMainWindow;


class IToolPaletteHandler
{
public:
  virtual void TP_ToolSelected(int tool) = 0;
};


class wToolPalette : public wPaletteWindow
{
public:
  wToolPalette(wDocumentWindow* owner, IToolPaletteHandler* handler, const char* name, wxRect rect = wxRect(wxPoint(64, 64), wxSize(256, 256)), bool visible = true);

  virtual bool Destroy();

  void AddTool(const char **xpm_image, const char* label);

private:
  void OnSize(wxSizeEvent &event);
  void OnPaint(wxPaintEvent &event);
  void OnLClick(wxMouseEvent &event);
  void OnMouseMove(wxMouseEvent &event);

/*
  bool OnNeedText(wxEvent &event);
  void OnToolSelected(wxEvent &event);
*/

private:
  IToolPaletteHandler* m_Handler;
  std::vector<wxBitmap*> m_Bitmaps;
  //std::vector<wxStaticBitmap*> m_Buttons;
  std::vector<wxString> m_Labels;
  int m_CurrentTool;
  
  friend wMainWindow;

private:
//  DECLARE_CLASS(wToolPalette);
  DECLARE_EVENT_TABLE();
};


#endif
