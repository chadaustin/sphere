#ifndef SPRITESET_WINDOW_HPP
#define SPRITESET_WINDOW_HPP

#include <wx/wx.h>

#ifdef USE_WXTABCTRL
#include <wx/tabctrl.h>
#else
#include <wx/notebook.h>
#endif

#include "SaveableDocumentWindow.hpp"
#include "SpritesetView.hpp"
#include "ImageView.hpp"
#include "PaletteView.hpp"
#include "ColorView.hpp"
#include "AlphaView.hpp"
#include "SpriteBaseView.hpp"
#include "SpritesetImagesPalette.hpp"
#include "../common/Spriteset.hpp"


class wSpritesetWindow
  : public wSaveableDocumentWindow
  , private ISpritesetViewHandler
  , private IImageViewHandler
  , private IPaletteViewHandler
  , private IColorViewHandler
  , private IAlphaViewHandler
  , private ISpriteBaseViewHandler
  , private ISpritesetImagesPaletteHandler
{
public:
  wSpritesetWindow(const char* filename = NULL);

private:
  void Create();
  void TabChanged(int tab);
  void ShowFramesTab(bool show);
  void ShowEditTab(bool show);
  void ShowBaseTab(bool show);
  void UpdateImageView();

  //afx_msg void OnDestroy();
  void OnSize(wxSizeEvent &event);

  void OnZoom1x(wxCommandEvent &event);
  void OnZoom2x(wxCommandEvent &event);
  void OnZoom4x(wxCommandEvent &event);
  void OnZoom8x(wxCommandEvent &event);
  void OnResize(wxCommandEvent &event);
  void OnFillDelay(wxCommandEvent &event);
  void OnFrameProperties(wxCommandEvent &event);

  void OnFramesTab(wxCommandEvent &event);
  void OnEditTab(wxCommandEvent &event);
  void OnBaseTab(wxCommandEvent &event);


/*todo:
  void OnUpdateZoom1x(CCmdUI* cmdui);
  void OnUpdateZoom2x(CCmdUI* cmdui);
  void OnUpdateZoom4x(CCmdUI* cmdui);
  void OnUpdateZoom8x(CCmdUI* cmdui);
*/

#ifdef USE_WXTABCTRL
  void OnTabChanged(wxTabEvent &event);
#else
  void OnNotebookChanged(wxNotebookEvent &event);
#endif

  virtual bool GetSavePath(char* path);
  virtual bool SaveDocument(const char* path);

  virtual void SV_CurrentFrameChanged(int direction, int frame);
  virtual void SV_EditFrame();
  virtual void SV_SpritesetModified();
  virtual void SV_CopyCurrentFrame();
  virtual void SV_PasteCurrentFrame();
  virtual void IV_ImageChanged();
  virtual void IV_ColorChanged(RGBA color);
  virtual void PV_ColorChanged(RGB color);
  virtual void CV_ColorChanged(RGB color);
  virtual void AV_AlphaChanged(byte alpha);
  virtual void SBV_SpritesetModified();
  virtual void SP_ColorSelected(RGBA color);
  virtual void SIP_IndexChanged(int index);
  virtual void SIP_SpritesetModified();

private:
  sSpriteset m_Spriteset;
  sTileset m_Tile;
  int m_CurrentDirection;
  int m_CurrentFrame;

  bool m_Created;

#ifdef USE_WXTABCTRL
 wxTabCtrl    *m_TabControl;
#else
 wxNotebook *m_NotebookControl;
#endif

  // views
  wSpritesetView  *m_SpritesetView;
  wImageView      *m_ImageView;
  wPaletteView    *m_PaletteView;
  wColorView      *m_ColorView;
  wAlphaView      *m_AlphaView;
  wSpriteBaseView *m_SpriteBaseView;

  // palette
  wSpritesetImagesPalette* m_ImagesPalette;

private:
//  DECLARE_CLASS(wSpritesetWindow)
  DECLARE_EVENT_TABLE()
};


#endif
