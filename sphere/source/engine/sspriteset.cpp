#include <stdlib.h>
#include <string.h>
#include "sspriteset.hpp"
#include "../common/Spriteset.hpp"


////////////////////////////////////////////////////////////////////////////////

SSPRITESET::SSPRITESET()
: m_Images(NULL)
, m_FlipImages(NULL)
, m_RefCount(1)
, m_Filename("")
{
}

////////////////////////////////////////////////////////////////////////////////

SSPRITESET::SSPRITESET(sSpriteset& s)
: m_Images(NULL)
, m_FlipImages(NULL)
, m_RefCount(1)
, m_Filename("")
{
  m_Spriteset = s;

  m_Images = new IMAGE[m_Spriteset.GetNumImages()];
  m_FlipImages = new IMAGE[m_Spriteset.GetNumImages()];
  for (int i = 0; i < m_Spriteset.GetNumImages(); i++) {
    m_Images[i]     = CreateFrameImage(i);
    m_FlipImages[i] = CreateFlipFrameImage(i);
  }
}

////////////////////////////////////////////////////////////////////////////////

SSPRITESET::~SSPRITESET()
{
  for (int i = 0; i < m_Spriteset.GetNumImages(); i++) {
    DestroyImage(m_Images[i]);
    DestroyImage(m_FlipImages[i]);
  }
  delete[] m_Images;
  delete[] m_FlipImages;
}

////////////////////////////////////////////////////////////////////////////////

void
SSPRITESET::AddRef()
{
  m_RefCount++;
}

////////////////////////////////////////////////////////////////////////////////

void
SSPRITESET::Release()
{
  // this is dangerous
  // don't use refcounting on things on the stack
  if (--m_RefCount == 0) {
    delete this;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool
SSPRITESET::Load(const char* filename, IFileSystem& fs, std::string pfilename)
{
  if (m_Spriteset.Load(filename, fs) == false) {
    return false;
  }

  m_Filename = pfilename;

  m_Images = new IMAGE[m_Spriteset.GetNumImages()];
  m_FlipImages = new IMAGE[m_Spriteset.GetNumImages()];
  for (int i = 0; i < m_Spriteset.GetNumImages(); i++) {
    m_Images[i]     = CreateFrameImage(i);
    m_FlipImages[i] = CreateFlipFrameImage(i);
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

const sSpriteset&
SSPRITESET::GetSpriteset() const
{
  return m_Spriteset;
}
 
////////////////////////////////////////////////////////////////////////////////

std::string
SSPRITESET::GetFilename() const {
  return m_Filename;
}


////////////////////////////////////////////////////////////////////////////////

int
SSPRITESET::GetNumImages() const
{
  return m_Spriteset.GetNumImages();
}

////////////////////////////////////////////////////////////////////////////////

IMAGE
SSPRITESET::GetImage(int image) const
{
  return m_Images[image];
}

////////////////////////////////////////////////////////////////////////////////

IMAGE
SSPRITESET::GetFlipImage(int image) const
{
  return m_FlipImages[image];
}

////////////////////////////////////////////////////////////////////////////////

IMAGE
SSPRITESET::CreateFrameImage(int index)
{
  CImage32& sprite = m_Spriteset.GetImage(index);
  return CreateImage(sprite.GetWidth(), sprite.GetHeight(), sprite.GetPixels());
}

////////////////////////////////////////////////////////////////////////////////

IMAGE
SSPRITESET::CreateFlipFrameImage(int index)
{
  CImage32 sprite = m_Spriteset.GetImage(index);
  sprite.FlipVertical();
  return CreateImage(sprite.GetWidth(), sprite.GetHeight(), sprite.GetPixels());
}

////////////////////////////////////////////////////////////////////////////////
