#ifndef BROWSE_WINDOW_HPP
#define BROWSE_WINDOW_HPP

#include <afxcmn.h>
#include "DocumentWindow.hpp"
#include "DIBSection.hpp"

#include <vector>
#include <string>

#include "../common/Image32.hpp"

///////////////////////////////////////////////////////////////////////////////

class CBrowseInfo {
public:
  std::string filename;
  CImage32 image;

public:
  const RGBA* GetPixels() const { return image.GetPixels(); }
  int GetWidth() const { return image.GetWidth(); }
  int GetHeight() const { return image.GetHeight(); }

};

///////////////////////////////////////////////////////////////////////////////

class CBrowseWindow
  : public CDocumentWindow
{
public:
  CBrowseWindow(const char* folder = NULL, const char* filter = NULL);
  ~CBrowseWindow();

private:
  void Create();
  void Destroy();

	afx_msg void OnTimer(UINT event);

  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnKeyDown(UINT vk, UINT repeat, UINT flags);
  afx_msg void OnPaint();
  afx_msg void OnLButtonDown(UINT flags, CPoint point);
  afx_msg void OnRButtonDown(UINT flags, CPoint point);
  afx_msg void OnMouseMove(UINT flags, CPoint point);
  afx_msg void OnVScroll(UINT code, UINT pos, CScrollBar* scroll_bar);

private:
  void ClearBrowseList();
  bool LoadFile(const char* filename);
  void OpenFile(int index);

private:
  int GetPageSize();
  int GetNumRows();
  void UpdateScrollBar();

  void InvalidateTile(int tile);

private:
  afx_msg void OnBrowseListRefresh();
	afx_msg void OnBrowseSetBrowseSize();

private:
  bool m_Created;  // whether or not the window has been created

  CDIBSection* m_BlitTile;
  std::string m_Folder;
  std::string m_Filter;

  int m_TopRow;
  int m_SelectedImage;
  double m_ZoomFactor;


  int m_ImageWidth;
  int m_ImageHeight;

  std::vector<CBrowseInfo*> m_BrowseList;
  std::vector<std::string> m_FileList;

private:
  UINT m_Timer;

  DECLARE_MESSAGE_MAP()
};


#endif
