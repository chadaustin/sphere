#ifndef __IMAGE32_HPP
#define __IMAGE32_HPP


#include <stdio.h>
#include "colorspace.h"


class CImage32
{
public:
  CImage32();
  ~CImage32();

  bool Create(int width, int height);
  bool Load(const char* filename);
  bool Save(const char* filename) const;

  bool Import(const char* filename);
  bool Export(const char* filename);

  void Resize(int width, int height);

  int GetWidth() const;
  int GetHeight() const;
  
  RGBA* GetPixels();
  const RGBA* GetPixels() const;

private:
  bool Import_PCX(const char* filename);
  void PCX_ReadScanline(FILE* file, int scansize, byte* scanline);

private:
  int   m_Width;
  int   m_Height;
  RGBA* m_Pixels;
};


#endif
