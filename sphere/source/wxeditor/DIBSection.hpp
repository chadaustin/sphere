#ifndef __DIBSECTION_HPP
#define __DIBSECTION_HPP


//#include <windows.h>
#include <wx/dcmemory.h>
#include <wx/image.h>

class wDIBSection
{
public:
  wDIBSection(int width, int height);
  ~wDIBSection();

//  wxDC&  GetDC();

  void*       GetPixels();
  const void* GetPixels() const;

  wxImage*    GetImage();

  int GetWidth() const;
  int GetHeight() const;

private:
  wxImage m_Image;
};


#endif
