#include "Clipboard.hpp"

#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/image.h>

///////////////////////////////////////////////////////////////////////////////

CClipboard::CClipboard()
{

}

///////////////////////////////////////////////////////////////////////////////

bool
CClipboard::IsFlatImageOnClipbard()
{
  return true;
}

///////////////////////////////////////////////////////////////////////////////

RGBA*
CClipboard::GetFlatImageFromClipboard(int& width, int& height)
{
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////

bool
CClipboard::PutFlatImageOntoClipboard(int width, int height, const RGBA* flat_pixels)
{
  if (width <= 0 || height <= 0) {
    return false;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool
CClipboard::IsBitmapImageOnClipboard() {
  return true;
}

///////////////////////////////////////////////////////////////////////////////

RGBA*
CClipboard::GetBitmapImageFromClipboard(int& width, int& height)
{
  wxBitmapDataObject data_object;
  if (!wxTheClipboard->GetData(data_object))
    return NULL;

  wxBitmap bmp = data_object.GetBitmap();
  wxImage image = bmp.ConvertToImage();
  unsigned char* data = image.GetData();

  width = image.GetWidth();
  height = image.GetHeight();

  RGBA* pixels = new RGBA[width * height];
  if (pixels == NULL) {
    return NULL;
  }

  int index = 0;
  for (int y = 0; y < height; y++)
    for (int x = 0; x < width; x++) {
      pixels[y * width + x].red   = data[index++];
      pixels[y * width + x].green = data[index++];
      pixels[y * width + x].blue  = data[index++];
      pixels[y * width + x].alpha = 255;
    }

  return pixels;
}

///////////////////////////////////////////////////////////////////////////////

bool
CClipboard::PutBitmapImageOntoClipboard(int width, int height, const RGBA* source)
{  
  wxImage image;
  image.Create(width, height);
  if (image.GetWidth() != width || image.GetHeight() != height)
    return false;

  unsigned char* data = (unsigned char*) malloc(sizeof(unsigned char) * width * height * 3);
  if (data == NULL)
    return false;

  int index = 0;
  for (int y = 0; y < height; y++)
    for (int x = 0; x < width; x++) {
      data[index++] = source[y * width + x].red;
      data[index++] = source[y * width + x].green;
      data[index++] = source[y * width + x].blue;
    }

  image.SetData(data);

  wxBitmap bmp(image);

  wxBitmapDataObject* data_object = new wxBitmapDataObject(bmp);
  wxTheClipboard->AddData(data_object);

  return true;
}

///////////////////////////////////////////////////////////////////////////////
