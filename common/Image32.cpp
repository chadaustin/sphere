#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Image32.hpp"
#include "huffman.h"
#include "lz.h"
#include "packed.h"
#include "x++.hpp"


////////////////////////////////////////////////////////////////////////////////

CImage32::CImage32()
: m_Width(0)
, m_Height(0)
, m_Pixels(NULL)
{
}

////////////////////////////////////////////////////////////////////////////////

CImage32::~CImage32()
{
  delete[] m_Pixels;
}

////////////////////////////////////////////////////////////////////////////////

bool
CImage32::Create(int width, int height)
{
  m_Width  = width;
  m_Height = height;
  m_Pixels = new RGBA[width * height];
  for (int i = 0; i < width * height; i++)
    m_Pixels[i] = rgbaBlack;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

PACKED_STRUCT(I32_HEADER)
  byte  signature[4]; // ".i32"
  byte  version;      // 1
  dword width;
  dword height;
  byte  compression;
  byte  reserved[242];
END_STRUCT(I32_HEADER)


ASSERT_STRUCT_SIZE(I32_HEADER, 256)

////////////////////////////////////////////////////////////////////////////////

static int bytesleft(FILE* file)
{
  int currentlocation = ftell(file);
  int filesize;
  fseek(file, 0, SEEK_END);
  filesize = ftell(file);
  fseek(file, currentlocation, SEEK_SET);
  return filesize - currentlocation;
}

////////////////////////////////////////////////////////////////////////////////

bool
CImage32::Load(const char* filename)
{
  // free the old image
  delete[] m_Pixels;

  FILE* file = fopen(filename, "rb");
  if (file == NULL)
    return false;

  // read and test image header
  I32_HEADER header;
  fread(&header, 1, sizeof(header), file);
  if (memcmp(header.signature, ".i32", 4) != 0 ||
      header.version != 1)
  {
    fclose(file);
    return false;
  }

  // allocate image
  m_Width  = header.width;
  m_Height = header.height;
  m_Pixels = new RGBA[m_Width * m_Height];

  // read pixel data
  if (header.compression == 0)  // no compression
  {
    fread(m_Pixels, sizeof(RGBA), m_Width * m_Height, file);
  }
  else                          // huffman
  {
    // read compressed data
    int left = bytesleft(file);
    BUFFER source = CreateSizedBuffer(left);
    fread(GetBufferData(source), 1, left, file);

    // uncompress it
    BUFFER result;
    if (header.compression == 1)
      result = HuffmanDecode(source);
    else
      result = LZDecode(source);

    memcpy(m_Pixels, GetBufferData(result), m_Width * m_Height * sizeof(RGBA));

    // free buffers
    DestroyBuffer(source);
    DestroyBuffer(result);
  }

  fclose(file);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CImage32::Save(const char* filename) const
{
  // open the file
  FILE* file = fopen(filename, "wb");
  if (file == NULL)
    return false;

  // write the header
  I32_HEADER header;
  memset(&header, 0, sizeof(header));
  memcpy(header.signature, ".i32", 4);
  header.version = 1;
  header.width = m_Width;
  header.height = m_Height;
  header.compression = 2;
  fwrite(&header, 1, sizeof(header), file);

  // write the data
  BUFFER source = CreateBuffer(sizeof(RGBA) * m_Width * m_Height, (byte*)m_Pixels);
  BUFFER result = LZEncode(source);

  fwrite(GetBufferData(result), 1, GetBufferSize(result), file);

  DestroyBuffer(source);
  DestroyBuffer(result);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CImage32::Import(const char* filename)
{
  m_Width  = 0;
  m_Height = 0;
  delete[] m_Pixels;
  m_Pixels = NULL;

  if (strcmp_ci(filename + strlen(filename) - 4, ".pcx") == 0)
    return Import_PCX(filename);

  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool
CImage32::Export(const char* filename)
{
  return false;
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::Resize(int width, int height)
{
  RGBA* NewPixels = new RGBA[width * height];
  for (int ix = 0; ix < width; ix++)
    for (int iy = 0; iy < height; iy++)
    {
      if (ix < m_Width && iy < m_Height)
        NewPixels[iy * width + ix] = m_Pixels[iy * m_Width + ix];
      else
        NewPixels[iy * width + ix] = rgbaBlack;
    }

  m_Width  = width;
  m_Height = height;
  delete[] m_Pixels;
  m_Pixels = NewPixels;
}

////////////////////////////////////////////////////////////////////////////////

int
CImage32::GetWidth() const
{
  return m_Width;
}

////////////////////////////////////////////////////////////////////////////////

int
CImage32::GetHeight() const
{
  return m_Height;
}

////////////////////////////////////////////////////////////////////////////////

RGBA*
CImage32::GetPixels()
{
  return m_Pixels;
}

////////////////////////////////////////////////////////////////////////////////

const RGBA*
CImage32::GetPixels() const
{
  return m_Pixels;
}

////////////////////////////////////////////////////////////////////////////////

PACKED_STRUCT(PCX_HEADER)
  byte manufacturer;
  byte version;
  byte encoding;
  byte bits_per_pixel;
  word xmin;
  word ymin;
  word xmax;
  word ymax;
  word hdpi;
  word vdpi;
  byte colormap[48];
  byte reserved;
  byte num_planes;
  word bytes_per_line;
  word palette_info;
  word h_screen_size;
  word v_screen_size;
  byte filler[54];
END_STRUCT(PCX_HEADER)

ASSERT_STRUCT_SIZE(PCX_HEADER, 128)

////////////////////////////////////////////////////////////////////////////////

bool
CImage32::Import_PCX(const char* filename)
{
  FILE* file = fopen(filename, "rb");
  if (file == NULL)
    return false;

  PCX_HEADER header;
  fread(&header, 1, sizeof(header), file);

  m_Width  = header.xmax - header.xmin + 1;
  m_Height = header.ymax - header.ymin + 1;
  m_Pixels = new RGBA[m_Width * m_Height];

  int   scansize = header.num_planes * header.bytes_per_line;
  byte* scanline = new byte[scansize];

  if (header.num_planes == 1)      // 256 colors
  {
    // read palette
    RGB palette[256];
    long position = ftell(file);
    fseek(file, -768, SEEK_END);
    fread(palette, 3, 256, file);
    fseek(file, position, SEEK_SET);

    for (int iy = 0; iy < m_Height; iy++)
    {
      PCX_ReadScanline(file, scansize, scanline);
      for (int ix = 0; ix < m_Width; ix++)
      {
        m_Pixels[iy * m_Width + ix].red   = palette[scanline[ix]].red;
        m_Pixels[iy * m_Width + ix].green = palette[scanline[ix]].green;
        m_Pixels[iy * m_Width + ix].blue  = palette[scanline[ix]].blue;
      }
    }
  }
  else if (header.num_planes == 3) // 24-bit color
  {
    for (int iy = 0; iy < m_Height; iy++)
    {
      PCX_ReadScanline(file, scansize, scanline);
      for (int ix = 0; ix < m_Width; ix++)
        m_Pixels[iy * m_Width + ix].red   = scanline[ix + 0 * header.bytes_per_line];
      for (int ix = 0; ix < m_Width; ix++)
        m_Pixels[iy * m_Width + ix].green = scanline[ix + 1 * header.bytes_per_line];
      for (int ix = 0; ix < m_Width; ix++)
      {
        m_Pixels[iy * m_Width + ix].blue  = scanline[ix + 2 * header.bytes_per_line];
        m_Pixels[iy * m_Width + ix].alpha = 255;
      }
    }
  }

  delete[] scanline;

  fclose(file);
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::PCX_ReadScanline(FILE* file, int scansize, byte* scanline)
{
  int bytesread = 0;
  while (bytesread < scansize)
  {
    byte data = fgetc(file);
    if (data == EOF)
      return;

    if (data > 192 && data < 256)
    {
      int numbytes = data - 192;
      data = fgetc(file);
      for (int i = 0; i < numbytes; i++)
      {
        scanline[bytesread] = data;
        bytesread++;
      }
    }
    else
    {
      scanline[bytesread] = data;
      bytesread++;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
