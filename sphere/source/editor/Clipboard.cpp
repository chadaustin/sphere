#include "Clipboard.hpp"

#include <windows.h>

static unsigned int s_ClipboardFormat;

///////////////////////////////////////////////////////////////////////////////

CClipboard::CClipboard()
{
  s_ClipboardFormat = RegisterClipboardFormat("FlatImage32");
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
  HGLOBAL memory = (HGLOBAL)GetClipboardData(s_ClipboardFormat);
  if (memory == NULL) {
    return NULL;
  }

  // get the height and pixels from the clipboard
  dword* ptr = (dword*)GlobalLock(memory);
  if (ptr == NULL) {
    return NULL;
  }

  width = *ptr++;
  height = *ptr++;

  if (width <= 0 || height <= 0) {
    return NULL;
  }

  RGBA* clipboard = (RGBA*)ptr;
  RGBA* pixels = new RGBA[width * height];
  if (pixels == NULL) {
    return NULL;
  }

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      pixels[y * width + x] = clipboard[y * width + x];
    }
  }

  GlobalUnlock(memory);

  return pixels;
}

///////////////////////////////////////////////////////////////////////////////

bool
CClipboard::PutFlatImageOntoClipboard(int width, int height, const RGBA* flat_pixels)
{
  if (width <= 0 || height <= 0) {
    return false;
  }

  // copy the image as a flat 32-bit color image
  HGLOBAL memory = GlobalAlloc(GHND, 8 + width * height * 4);
  if (memory == NULL) {
    return false;
  }

  dword* ptr = (dword*)GlobalLock(memory);
  if (ptr == NULL) {
    return false;
  }

  *ptr++ = width;
  *ptr++ = height;

  memcpy(ptr, flat_pixels, width * height * sizeof(RGBA));

  // put the image on the clipboard
  GlobalUnlock(memory);
  SetClipboardData(s_ClipboardFormat, memory);

  return true;
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
  HBITMAP bitmap = (HBITMAP)GetClipboardData(CF_BITMAP);
  if (bitmap == NULL) {
    return NULL;
  }

  BITMAP b;
  GetObject(bitmap, sizeof(b), &b);

  HDC dc = CreateCompatibleDC(NULL);
  HBITMAP oldbitmap = (HBITMAP)SelectObject(dc, bitmap);

  // work out the possible width
  for (width = 0; width < 4096; width++)
    if (GetPixel(dc, width, 0) == CLR_INVALID)
      break;

  // work out the possible height
  for (height = 0; height < 4096; height++)
    if (GetPixel(dc, 0, height) == CLR_INVALID)
      break;

  if (width <= 0 || height <= 0) {
    return NULL;
  }

  RGBA* pixels = new RGBA[width * height];
  if (pixels == NULL) {
    return NULL;
  }

  for (int iy = 0; iy < height; iy++)
    for (int ix = 0; ix < width; ix++)
    {
      COLORREF pixel = GetPixel(dc, ix, iy);
      if (pixel == CLR_INVALID)
        pixel = RGB(0, 0, 0);

      pixels[iy * width + ix].red   = GetRValue(pixel);
      pixels[iy * width + ix].green = GetGValue(pixel);
      pixels[iy * width + ix].blue  = GetBValue(pixel);
      pixels[iy * width + ix].alpha = 255;  // there is no alpha so we use a default
    }

  SelectObject(dc, oldbitmap);
  DeleteDC(dc);

  return pixels;
}

///////////////////////////////////////////////////////////////////////////////

bool
CClipboard::PutBitmapImageOntoClipboard(int width, int height, const RGBA* source)
{
  // ADD DIB
  // create a pixel array to initialize the bitmap
  BGRA* pixels = new BGRA[width * height];
  if (pixels == NULL) {
    return false;
  }

  for (int iy = 0; iy < height; iy++) {
    for (int ix = 0; ix < width; ix++)
    {
      pixels[(height - iy - 1) * (width) + ix].red   = source[iy * width + ix].red;
      pixels[(height - iy - 1) * (width) + ix].green = source[iy * width + ix].green;
      pixels[(height - iy - 1) * (width) + ix].blue  = source[iy * width + ix].blue;
      pixels[(height - iy - 1) * (width) + ix].alpha = source[iy * width + ix].alpha;
    }
  }

  // create the bitmap
  HBITMAP bitmap = CreateBitmap(width, height, 1, 32, pixels);
	BITMAPINFOHEADER header;
	header.biSize = sizeof(header);
	header.biWidth = width;
	header.biHeight = height;
	header.biPlanes = 1;
	header.biBitCount = 32;
	header.biCompression = BI_RGB;
	header.biSizeImage = 0;
	header.biXPelsPerMeter = 0;
	header.biYPelsPerMeter = 0;
	header.biClrUsed = 0;
	header.biClrImportant = 0;

	HGLOBAL hDIB = GlobalAlloc(GHND, sizeof(header) + width * height * 4);
  char* dibPtr = (char*)GlobalLock(hDIB);
  if (dibPtr == NULL) {
    CloseClipboard();
    delete[] pixels;
    return false;
  }
	memcpy(dibPtr, &header, sizeof(header));
	memcpy(dibPtr+sizeof(header), pixels, width * height * 4);
	GlobalUnlock(hDIB);

  // put the bitmap in the clipboard
  SetClipboardData(CF_DIB, hDIB);
	delete[] pixels;

  return true;
}

///////////////////////////////////////////////////////////////////////////////
