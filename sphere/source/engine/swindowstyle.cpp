#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "swindowstyle.hpp"


////////////////////////////////////////////////////////////////////////////////

SWINDOWSTYLE::SWINDOWSTYLE()
{
  for (int i = 0; i < 9; i++)
    m_Images[i] = NULL;
}

////////////////////////////////////////////////////////////////////////////////

SWINDOWSTYLE::~SWINDOWSTYLE()
{
  Destroy();
}

////////////////////////////////////////////////////////////////////////////////

bool
SWINDOWSTYLE::Load(const char* filename, IFileSystem& fs)
{
  Destroy();

  // load the file
  if (m_WindowStyle.Load(filename, fs) == false)
    return false;

  Initialize();

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
SWINDOWSTYLE::CreateFromWindowStyle(const sWindowStyle& ws)
{
  Destroy();
  m_WindowStyle = ws;
  Initialize();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
SWINDOWSTYLE::Destroy()
{
  for (int i = 0; i < 9; i++) {
    if (m_Images[i]) {
      DestroyImage(m_Images[i]);
      m_Images[i] = NULL;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

bool
SWINDOWSTYLE::DrawWindow(int x, int y, int w, int h)
{
  IMAGE image;
  int width, height;

  int ox, oy, ow, oh;
  GetClippingRectangle(&ox, &oy, &ow, &oh);

  //---- Draw middle ----//

  image = m_Images[8];
  width = GetImageWidth(image);
  height = GetImageHeight(image);

  SetClippingRectangle(x, y, w, h);

  if (m_WindowStyle.GetBackgroundMode() == sWindowStyle::TILED) {
    for (int ix = 0; ix < w / width + 1; ix++) {
      for (int iy = 0; iy < h / height + 1; iy++) {
        BlitImage(image, x + ix * width, y + iy * height);
      }
    }
  } else {
    RGBA colors[4];
    colors[0] = m_WindowStyle.GetBackgroundColor(sWindowStyle::BACKGROUND_UPPER_LEFT);
    colors[1] = m_WindowStyle.GetBackgroundColor(sWindowStyle::BACKGROUND_UPPER_RIGHT);
    colors[2] = m_WindowStyle.GetBackgroundColor(sWindowStyle::BACKGROUND_LOWER_RIGHT);
    colors[3] = m_WindowStyle.GetBackgroundColor(sWindowStyle::BACKGROUND_LOWER_LEFT);
    DrawGradientRectangle(x, y, w, h, colors);
  }
  
  //---- Draw edges ----//

  // top

  image = m_Images[1];
  width = GetImageWidth(image);
  height = GetImageHeight(image);

  SetClippingRectangle(x, y - height, w, height);

  for (int i = 0; i < w / width + 1; i++)
    BlitImage(image, x + i * width, y - height);

  // bottom

  image = m_Images[5];
  width = GetImageWidth(image);
  height = GetImageHeight(image);

  SetClippingRectangle(x, y + h, w, height);

  for (int i = 0; i < w / width + 1; i++)
    BlitImage(image, x + i * width, y + h);

  // left

  image = m_Images[7];
  width = GetImageWidth(image);
  height = GetImageHeight(image);

  SetClippingRectangle(x - width, y, width, h);

  for (int i = 0; i < h / height + 1; i++)
    BlitImage(image, x - width, y + i * height);

  // right

  image = m_Images[3];
  width = GetImageWidth(image);
  height = GetImageHeight(image);

  SetClippingRectangle(x + w, y, x + w + width, h);

  for (int i = 0; i < h / height + 1; i++)
    BlitImage(image, x + w, y + i*height);


  //---- Draw corners ----//

  SetClippingRectangle(ox, oy, ow, oh);

  // upper-left
  image = m_Images[0];
  width = GetImageWidth(image);
  height = GetImageHeight(image);
  BlitImage(image,
    x - width,
    y - height);

  // upper-right
  image = m_Images[2];
  width = GetImageWidth(image);
  height = GetImageHeight(image);
  BlitImage(image,
    x + w,
    y - height);

  // lower-right
  image = m_Images[4];
  width = GetImageWidth(image);
  height = GetImageHeight(image);
  BlitImage(image,
    x + w,
    y + h);

  // lower-left
  image = m_Images[6];
  width = GetImageWidth(image);
  height = GetImageHeight(image);
  BlitImage(image,
    x - width,
    y + h);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
SWINDOWSTYLE::Initialize()
{
  // convert the file data into usable images
  for (int i = 0; i < 9; i++) {
    CImage32& image = m_WindowStyle.GetBitmap(i);
    m_Images[i] = CreateImage(image.GetWidth(), image.GetHeight(), image.GetPixels());
  }
}

////////////////////////////////////////////////////////////////////////////////
