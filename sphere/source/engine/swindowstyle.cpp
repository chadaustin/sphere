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

int
SWINDOWSTYLE::GetBackgroundMode() {
  return m_WindowStyle.GetBackgroundMode();
}

////////////////////////////////////////////////////////////////////////////////

bool
SWINDOWSTYLE::DrawBackground(int x, int y, int w, int h, int background_mode)
{
  IMAGE image = m_Images[8];

  if (background_mode == sWindowStyle::TILED) {
    int ox, oy, ow, oh;
    int width = GetImageWidth(image);
    int height = GetImageHeight(image);
    GetClippingRectangle(&ox, &oy, &ow, &oh);
    SetClippingRectangle(x, y, w, h);

    for (int ix = 0; ix < w / width + 1; ix++) {
      for (int iy = 0; iy < h / height + 1; iy++) {
        BlitImage(image, x + ix * width, y + iy * height);
      }
    }

    SetClippingRectangle(ox, oy, ow, oh);
  } else if (background_mode == sWindowStyle::STRETCHED) {
    int tx[4] = { x, x + w, x + w, x };
    int ty[4] = { y, y, y + h, y + h };
    TransformBlitImage(image, tx, ty);
  } else {
    RGBA colors[4];
    colors[0] = m_WindowStyle.GetBackgroundColor(sWindowStyle::BACKGROUND_UPPER_LEFT);
    colors[1] = m_WindowStyle.GetBackgroundColor(sWindowStyle::BACKGROUND_UPPER_RIGHT);
    colors[2] = m_WindowStyle.GetBackgroundColor(sWindowStyle::BACKGROUND_LOWER_RIGHT);
    colors[3] = m_WindowStyle.GetBackgroundColor(sWindowStyle::BACKGROUND_LOWER_LEFT);
    DrawGradientRectangle(x, y, w, h, colors);
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
SWINDOWSTYLE::DrawCorner(int index, int x, int y)
{
  if (index < 0 || index >= 9)
    return false;
  BlitImage(m_Images[index], x, y);
  return true;
}


///////////////////////////////////////////////////////////////////////////////

bool
SWINDOWSTYLE::DrawUpperLeftCorner(int x, int y)
{
  return DrawCorner(0, x, y);
}

bool
SWINDOWSTYLE::DrawUpperRightCorner(int x, int y)
{
  return DrawCorner(2, x, y);
}

bool
SWINDOWSTYLE::DrawLowerRightCorner(int x, int y)
{
  return DrawCorner(4, x, y);
}

bool
SWINDOWSTYLE::DrawLowerLeftCorner(int x, int y)
{
  return DrawCorner(6, x, y);
}

////////////////////////////////////////////////////////////////////////////////

bool
SWINDOWSTYLE::DrawHorizontalEdge(int index, int x, int y, int w, int h)
{
  if (index < 0 || index >= 9)
    return false;

  IMAGE image = m_Images[index];
  int width = GetImageWidth(image);
  int height = GetImageHeight(image);

  int ox, oy, ow, oh;
  GetClippingRectangle(&ox, &oy, &ow, &oh);

  SetClippingRectangle(x, y, width, h);

  for (int i = 0; i < h / height + 1; i++)
    BlitImage(image, x, y + i * height);

  SetClippingRectangle(ox, oy, ow, oh);
  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
SWINDOWSTYLE::DrawVerticalEdge(int index, int x, int y, int w, int h)
{
  if (index < 0 || index >= 9)
    return false;

  IMAGE image = m_Images[index]; 
  int width = GetImageWidth(image);
  int height = GetImageHeight(image);

  int ox, oy, ow, oh;
  GetClippingRectangle(&ox, &oy, &ow, &oh);

  SetClippingRectangle(x, y, w, height);

  for (int i = 0; i < w / width + 1; i++)
    BlitImage(image, x + i * width, y);

  SetClippingRectangle(ox, oy, ow, oh);
  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
SWINDOWSTYLE::DrawTopEdge(int x, int y, int w, int h)
{
  return DrawVerticalEdge(1, x, y, w, h);
}

///////////////////////////////////////////////////////////////////////////////

bool
SWINDOWSTYLE::DrawBottomEdge(int x, int y, int w, int h)
{
  return DrawVerticalEdge(5, x, y, w, h);
}

////////////////////////////////////////////////////////////////////////////////

bool
SWINDOWSTYLE::DrawLeftEdge(int x, int y, int w, int h)
{
  return DrawHorizontalEdge(7, x, y, w, h);
}

////////////////////////////////////////////////////////////////////////////////

bool
SWINDOWSTYLE::DrawRightEdge(int x, int y, int w, int h)
{
  return DrawHorizontalEdge(3, x, y, w, h);
}

////////////////////////////////////////////////////////////////////////////////

bool
SWINDOWSTYLE::DrawWindow(int x, int y, int w, int h)
{
  IMAGE image = m_Images[8];
  int width = GetImageWidth(image);
  int height = GetImageHeight(image);

  //---- Draw middle ----//
  DrawBackground(x, y, w, h, m_WindowStyle.GetBackgroundMode());
  
  //---- Draw edges ----//
  DrawTopEdge(x, (y - GetImageHeight(m_Images[1])), w, h);
  DrawBottomEdge(x, (y + h), w, h);
  DrawLeftEdge((x - GetImageWidth(m_Images[7])), y, w, h);
  DrawRightEdge((x + w), y, w, h);

  //---- Draw corners ----//
  DrawUpperLeftCorner (x - GetImageWidth(m_Images[0]), y - GetImageHeight(m_Images[0]));
  DrawUpperRightCorner(x + w, y - GetImageHeight(m_Images[2]));
  DrawLowerRightCorner(x + w, y + h);
  DrawLowerLeftCorner (x - GetImageWidth(m_Images[6]), y + h);

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
