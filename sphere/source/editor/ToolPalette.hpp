#ifndef TOOL_PALETTE_HPP
#define TOOL_PALETTE_HPP


#include <vector>
#include "PaletteWindow.hpp"


class CMainWindow;


class IToolPaletteHandler
{
public:
  virtual void TP_ToolSelected(int tool) = 0;
};


class CToolPalette : public CPaletteWindow
{
public:
  CToolPalette(CDocumentWindow* owner, IToolPaletteHandler* handler, const char* name, RECT rect, bool visible);

  virtual void Destroy();

  void AddTool(int icon_id);

private:
  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnPaint();

  afx_msg void OnToolSelected(UINT id);

private:
  IToolPaletteHandler* m_Handler;

  std::vector<CButton*> m_Buttons;

  int m_CurrentTool;

  friend CMainWindow;

  DECLARE_MESSAGE_MAP()
};


#endif