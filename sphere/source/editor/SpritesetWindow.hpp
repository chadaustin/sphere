#ifndef SPRITESET_WINDOW_HPP
#define SPRITESET_WINDOW_HPP


#include "SaveableDocumentWindow.hpp"
#include "SpritesetView.hpp"
#include "ImageView.hpp"
#include "PaletteView.hpp"
#include "ColorView.hpp"
#include "AlphaView.hpp"
#include "SpriteBaseView.hpp"
#include "SpritesetImagesPalette.hpp"
#include "SpritesetAnimationPalette.hpp"
#include "../common/Spriteset.hpp"


class CSpritesetWindow
  : public CSaveableDocumentWindow
  , private ISpritesetViewHandler
  , private IImageViewHandler
  , private IPaletteViewHandler
  , private IColorViewHandler
  , private IAlphaViewHandler
  , private ISpriteBaseViewHandler
  , private ISpritesetImagesPaletteHandler
{
	DECLARE_DYNAMIC(CSpritesetWindow)
public:
  CSpritesetWindow(const char* filename = NULL);

private:
  void Create();
  void TabChanged(int tab);
  void ShowFramesTab(int show);
  void ShowEditTab(int show);
  void ShowBaseTab(int show);
  void UpdateImageView();

  afx_msg void OnDestroy();
  afx_msg void OnSize(UINT type, int cx, int cy);

  afx_msg void OnZoom1x();
  afx_msg void OnZoom2x();
  afx_msg void OnZoom4x();
  afx_msg void OnZoom8x();
  afx_msg void OnResize();
  afx_msg void OnRescale();
  afx_msg void OnFillDelay();
  afx_msg void OnFrameProperties();
  afx_msg void OnExportAsImage();

  afx_msg void OnUpdateZoom1x(CCmdUI* cmdui);
  afx_msg void OnUpdateZoom2x(CCmdUI* cmdui);
  afx_msg void OnUpdateZoom4x(CCmdUI* cmdui);
  afx_msg void OnUpdateZoom8x(CCmdUI* cmdui);

  afx_msg void OnTabChanged(NMHDR* ns, LRESULT* result);

  virtual bool GetSavePath(char* path);
  virtual bool SaveDocument(const char* path);

  virtual void SV_CurrentFrameChanged(int direction, int frame);
  virtual void SV_EditFrame();
  virtual void SV_SpritesetModified();
  virtual void SV_CopyCurrentFrame();
  virtual void SV_PasteCurrentFrame();
	virtual void SV_ZoomFactorChanged(int zoom);
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

  CTabCtrl    m_TabControl;

  // views
  CSpritesetView  m_SpritesetView;
  CImageView      m_ImageView;
  CPaletteView    m_PaletteView;
  CColorView      m_ColorView;
  CAlphaView      m_AlphaView;
  CSpriteBaseView m_SpriteBaseView;

  // palette
  CSpritesetImagesPalette* m_ImagesPalette;
	CSpritesetAnimationPalette* m_AnimationPalette;

  DECLARE_MESSAGE_MAP()
};


#endif
