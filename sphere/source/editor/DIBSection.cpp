#include <windows.h>
#include <assert.h>
#include "DIBSection.hpp"



////////////////////////////////////////////////////////////////////////////////

CDIBSection::CDIBSection(int width, int height, int bpp)
{
  if (width == 0 || height == 0 || bpp == 0)
  {
    m_hDC = NULL;
    m_hBitmap = NULL;
    return;
  }

  // create the DC
  m_hDC = ::CreateCompatibleDC(NULL);
  assert(m_hDC != NULL);

  // define bitmap attributes
  BITMAPINFO bmi;
  memset(&bmi, 0, sizeof(bmi));
  BITMAPINFOHEADER& bmih = bmi.bmiHeader;
  bmih.biSize        = sizeof(bmih);
  bmih.biWidth       = width;
  bmih.biHeight      = -height;
  bmih.biPlanes      = 1;
  bmih.biBitCount    = bpp;
  bmih.biCompression = BI_RGB;

  // create the bitmap
  m_hBitmap = ::CreateDIBSection(::GetDC(NULL), &bmi, DIB_RGB_COLORS, &m_pPixels, NULL, 0);
  assert(m_hBitmap != NULL);

  // select the bitmap into the DC
  ::SelectObject(m_hDC, m_hBitmap);

  m_iWidth = width;
  m_iHeight = height;
}

////////////////////////////////////////////////////////////////////////////////

CDIBSection::~CDIBSection()
{
  if (m_hDC != NULL)
    ::DeleteDC(m_hDC);
  if (m_hBitmap != NULL)
    ::DeleteObject(m_hBitmap);
}

////////////////////////////////////////////////////////////////////////////////

HDC
CDIBSection::GetDC()
{
  return m_hDC;
}

////////////////////////////////////////////////////////////////////////////////

void*
CDIBSection::GetPixels()
{
  return m_pPixels;
}

////////////////////////////////////////////////////////////////////////////////

const void*
CDIBSection::GetPixels() const
{
  return m_pPixels;
}

////////////////////////////////////////////////////////////////////////////////

int
CDIBSection::GetWidth() const
{
  return m_iWidth;
}

////////////////////////////////////////////////////////////////////////////////

int
CDIBSection::GetHeight() const
{
  return m_iHeight;
}

////////////////////////////////////////////////////////////////////////////////
