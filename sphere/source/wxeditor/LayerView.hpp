#ifndef LAYER_VIEW_HPP
#define LAYER_VIEW_HPP


#include <vector>
#include <wx/wx.h>
#include "VScrollWindow.hpp"
//#include <afxext.h>


class sMap;  // #include "../sphere/common/Map.hpp"


class ILayerViewHandler
{
public:
  virtual void LV_MapChanged() = 0;
  virtual void LV_SelectedLayerChanged(int layer) = 0;
};


class wLayerView : public wVScrollWindow
{
public:
  wLayerView(wxWindow *parent, ILayerViewHandler* handler, sMap* map);
  ~wLayerView();

  //BOOL Create(ILayerViewHandler* handler, CWnd* parent, sMap* map);

  int GetSelectedLayer() const;
  void SetSelectedTile(int tile);

private:
  //afx_msg int OnCreate(CREATESTRUCT* createstruct);
  void OnSize(wxSizeEvent &event);
  void OnPaint(wxPaintEvent &event);
  void OnLButtonDown(wxMouseEvent &event);
  void OnLButtonUp(wxMouseEvent &event);
  void OnRButtonUp(wxMouseEvent &event);
  void OnMouseMove(wxMouseEvent &event);

  virtual void OnVScrollChanged(int y);

  void OnInsertLayer(wxCommandEvent &event);
  void OnInsertLayerFromImage(wxCommandEvent &event);
  void OnDeleteLayer(wxCommandEvent &event);
  void OnDuplicateLayer(wxCommandEvent &event);
  void OnLayerProperties(wxCommandEvent &event);
  void OnExportLayer(wxCommandEvent &event);
  
  void Click(int x, int y, bool left);
  void UpdateScrollBar();

private:
  ILayerViewHandler* m_Handler;
  sMap* m_Map;

  int m_TopLayer; // for scrolling (btw, this is upside down)
  int m_SelectedLayer;
  int m_SelectedTile;

  bool m_IsDragging;

  wxMenu *m_Menu;

private:
//  DECLARE_CLASS(wLayerView);
  DECLARE_EVENT_TABLE();
};


#endif
