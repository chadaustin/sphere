#ifndef SPRITESET_IMAGES_PALETTE_HPP
#define SPRITESET_IMAGES_PALETTE_HPP


#include "PaletteWindow.hpp"
#include "DIBSection.hpp"
#include "../common/Spriteset.hpp"


class CDocumentWindow;


struct ISpritesetImagesPaletteHandler
{
  virtual void SIP_IndexChanged(int index) = 0;
  virtual void SIP_SpritesetModified() = 0;  
};


class CSpritesetImagesPalette : public CPaletteWindow
{
public:
  CSpritesetImagesPalette(CDocumentWindow* owner, ISpritesetImagesPaletteHandler* handler, sSpriteset* spriteset);
  
  virtual void Destroy();

  void SetCurrentImage(int image);

private:
  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnPaint();
  afx_msg void OnLButtonDown(UINT flags, CPoint point);
  afx_msg void OnRButtonUp(UINT flags, CPoint point);

  afx_msg void OnMoveBack();
  afx_msg void OnMoveForward();
  afx_msg void OnInsertImage();
  afx_msg void OnRemoveImage();

  void OnSwap(int new_index);

  void OnZoom(double zoom);

private:
  ISpritesetImagesPaletteHandler* m_Handler;
  sSpriteset* m_Spriteset;

  int m_TopRow;
  double m_ZoomFactor;

  int m_SelectedImage;
  CDIBSection* m_BlitImage;

  DECLARE_MESSAGE_MAP()
};


#endif

/*

#ifndef TILESET_VIEW_HPP
#define TILESET_VIEW_HPP


#include <afxwin.h>
#include <afxext.h>
#include "DIBSection.hpp"
#include "../common/Tileset.hpp"

#include "FontGradientDialog.hpp"


class ITilesetViewHandler
{
public:
  virtual void TV_SelectedTileChanged(int tile) = 0;
  virtual void TV_TilesetChanged() = 0;
  virtual void TV_InsertedTiles(int at, int numtiles) = 0;
  virtual void TV_DeletedTiles(int at, int numtiles) = 0;
};


class CTileView : public CWnd
{
public:
  CTilesetView();
  ~CTilesetView();

  BOOL Create(ITilesetViewHandler* handler, CWnd* parent, sTileset* tileset);

  void TileChanged(int tile);
  void TilesetChanged();

  void SetSelectedTile(int tile);
  int  GetSelectedTile() const;

private:
  int GetPageSize();
  int GetNumRows();
  void UpdateScrollBar();

  afx_msg void OnPaint();
  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnVScroll(UINT code, UINT pos, CScrollBar* scroll_bar);
  afx_msg void OnLButtonDown(UINT flags, CPoint point);
  afx_msg void OnMouseMove(UINT flags, CPoint point);
  afx_msg void OnRButtonUp(UINT flags, CPoint point);

  afx_msg void OnInsertTile();
  afx_msg void OnAppendTile();
  afx_msg void OnDeleteTile();
  afx_msg void OnTileProperties();
  afx_msg void OnInsertTiles();
  afx_msg void OnAppendTiles();
  afx_msg void OnDeleteTiles();
  afx_msg void OnInsertTileset();
  afx_msg void OnAppendTileset();

  void OnZoom(int zoom_factor);
  afx_msg void OnZoom1x();
  afx_msg void OnZoom2x();
  afx_msg void OnZoom4x();
  afx_msg void OnZoom8x();

  // edit range
  int GetEditRangeStartIndex();
  int GetEditRangeEndIndex(int start_frame);
  std::vector<int> GetEditRangeIndexes();

  afx_msg void OnEditRangeSlideOther();
  afx_msg void OnEditRangeFlipHorizontally();
  afx_msg void OnEditRangeFlipVertically();
  afx_msg void OnEditRangeReplaceRGBA();

private:
  ITilesetViewHandler* m_Handler;

  sTileset* m_Tileset;

  int m_TopRow;
  int m_SelectedTile;
  int m_ZoomFactor;

  CDIBSection* m_BlitTile;

  bool m_MenuShown;

  DECLARE_MESSAGE_MAP()
};


#endif
*/