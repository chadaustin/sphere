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
#ifdef USE_SIZECBAR
	DECLARE_DYNAMIC(CBrowseWindow)
#endif
public:
  CBrowseWindow(const char* folder = NULL, const char* filter = NULL);
  ~CBrowseWindow();

private:
  void Create();
  void Destroy();

  bool LoadImages(const char* folder, const char* filter = NULL);

  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnKeyDown(UINT vk, UINT repeat, UINT flags);
  afx_msg void OnPaint();
  afx_msg void OnLButtonDown(UINT flags, CPoint point);
  afx_msg void OnMouseMove(UINT flags, CPoint point);

private:

  afx_msg void OnBrowseListRefresh();

private:
  //sMap m_Map;
  
  bool m_Created;  // whether or not the window has been created

  CDIBSection* m_BlitTile;
  std::string m_Folder;
  std::string m_Filter;

  int m_TopRow;
  int m_SelectedImage;
  double m_ZoomFactor;

  //int m_ImageWidth;
  //int m_ImageHeight;

  std::vector<CBrowseInfo*> m_BrowseList;

  DECLARE_MESSAGE_MAP()
};


#endif
