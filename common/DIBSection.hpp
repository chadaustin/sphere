#ifndef __DIBSECTION_HPP
#define __DIBSECTION_HPP


#include <windows.h>


class CDIBSection
{
public:
  CDIBSection(int width, int height, int bpp);
  ~CDIBSection();

  HDC   GetDC();

  void*       GetPixels();
  const void* GetPixels() const;

  int GetWidth() const;
  int GetHeight() const;

private:
  HDC     m_hDC;
  HBITMAP m_hBitmap;
  void*   m_pPixels;

  int m_iWidth;
  int m_iHeight;
};


#endif
