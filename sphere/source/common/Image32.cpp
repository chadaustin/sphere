#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include <algorithm>
#include <memory>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <corona.h>

#include "Image32.hpp"
#include "Filters.hpp"
#include "packed.hpp"
#include "primitives.hpp"
#include "strcmp_ci.hpp"
#include "minmax.hpp"
#include "resample.hpp"

unsigned char alpha_old[256][256]={
#include "alpha_old.table"
};
unsigned char alpha_new[256][256]={
#include "alpha_new.table"
};

/*
struct CoronaFileAdapter : public corona::DLLImplementation<corona::File> {
  CoronaFileAdapter(IFile* file) {
    m_File = file;
  }

  int COR_CALL read(void* buffer, int size) {
    return m_File->Read(buffer, size);
  }

  int COR_CALL write(const void* buffer, int size) {
    return m_File->Write(buffer, size);
  }

  bool COR_CALL seek(int pos, SeekMode mode) {
    int abs_pos;
    switch (mode) {
      case BEGIN:   abs_pos = pos; break;
      case CURRENT: abs_pos = m_File->Tell() + pos; break;
      case END:     abs_pos = m_File->Size() + pos; break;
      default:      return false;
    }
    m_File->Seek(abs_pos);
    return (abs_pos == m_File->Tell());
  }

  int COR_CALL tell() {
    return m_File->Tell();
  }

private:
  IFile* m_File;
};
*/


// INLINE HELPER FUNCTIONS

////////////////////////////////////////////////////////////////////////////////

inline double RotateX(double x, double y, double radians)
{
  return ((x * cos(radians)) - (y * sin(radians)));
}

////////////////////////////////////////////////////////////////////////////////

inline double RotateY(double x, double y, double radians)
{
  return ((x * sin(radians)) + (y * cos(radians)));
}

////////////////////////////////////////////////////////////////////////////////

inline int sgn(int i)
{
  return (i < 0 ? -1 : (i > 0 ? 1 : 0));
}
           
////////////////////////////////////////////////////////////////////////////////


// CImage32 Method Definitions


////////////////////////////////////////////////////////////////////////////////

CImage32::CImage32(int width, int height, const RGBA* pixels)
: m_Width(width)
, m_Height(height)
, m_Pixels(new RGBA[width * height])
, m_BlendMode(BLEND)
{
  if (m_Pixels) {
    if (pixels == NULL) {
      memset(m_Pixels, 0, width * height * sizeof(RGBA));
    } else {
      memcpy(m_Pixels, pixels, width * height * sizeof(RGBA));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

CImage32::CImage32(int width, int height, RGBA pixel)
: m_Width(width)
, m_Height(height)
, m_Pixels(new RGBA[width * height])
, m_BlendMode(BLEND)
{
  if (m_Pixels) {
    for (int i = 0; i < m_Width * m_Height; ++i) {
      m_Pixels[i] = pixel;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

CImage32::CImage32(const CImage32& image)
{
  m_Width = 0;
  m_Height = 0;
  m_Pixels = new RGBA[image.m_Width * image.m_Height];
  if (m_Pixels) {
    m_Width  = image.m_Width;
    m_Height = image.m_Height;
    memcpy(m_Pixels, image.m_Pixels, m_Width * m_Height * sizeof(RGBA));
  }
  m_BlendMode = image.m_BlendMode;
}

////////////////////////////////////////////////////////////////////////////////

CImage32&
CImage32::operator=(const CImage32& image)
{
  if (this != &image) {
    // don't reallocate pixels if number of bytes is the same
    if (m_Width * m_Height != image.m_Width * image.m_Height) {
      delete[] m_Pixels;
      m_Pixels = new RGBA[image.m_Width * image.m_Height];
    }

    m_Width = 0;
    m_Height = 0;

    // copy members over
    if (m_Pixels) {
      m_Width  = image.m_Width;
      m_Height = image.m_Height;
      memcpy(m_Pixels, image.m_Pixels, m_Width * m_Height * sizeof(RGBA));
      m_BlendMode = image.m_BlendMode;
    }
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////

bool
CImage32::operator==(const CImage32& rhs) const
{
  return (
    m_Width == rhs.m_Width &&
    m_Height == rhs.m_Height &&
    memcmp(m_Pixels, rhs.m_Pixels, m_Width * m_Height * sizeof(RGBA)) == 0
  );
}

////////////////////////////////////////////////////////////////////////////////

bool
CImage32::Create(int width, int height)
{
  delete[] m_Pixels;

  m_Width  = width;
  m_Height = height;
  m_Pixels = new RGBA[width * height];
  if (m_Pixels == NULL) {
    m_Width = 0;
    m_Height = 0;
    return false;
  }

  for (int i = 0; i < width * height; i++)
    m_Pixels[i] = CreateRGBA(0, 0, 0, 255);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::Clear()
{
  for (int i = 0; i < m_Width * m_Height; i++)
    m_Pixels[i] = CreateRGBA(0, 0, 0, 255);
}

////////////////////////////////////////////////////////////////////////////////

bool
CImage32::Load(const char* filename, IFileSystem& fs)
{
  using namespace corona;

  /*
  std::auto_ptr<IFile> file(fs.Open(filename, IFileSystem::read));
  if (!file.get()) {
    return false;
  }
  */

  //CoronaFileAdapter cfa(file.get());
  std::auto_ptr<Image> img(
    OpenImage(/*&cfa*/ filename, FF_AUTODETECT, PF_R8G8B8A8));
  if (!img.get()) {
    return false;
  }

  if (m_Width * m_Height != img->getWidth() * img->getHeight()) {
    delete[] m_Pixels;
    m_Pixels = new RGBA[img->getWidth() * img->getHeight()];
  }

  m_Width = 0;
  m_Height = 0;

  if (m_Pixels) {
    m_Width  = img->getWidth();
    m_Height = img->getHeight();
    memcpy(m_Pixels, img->getPixels(), m_Width * m_Height * 4);
  }

  return (m_Pixels && m_Width > 0 && m_Height > 0);
}

////////////////////////////////////////////////////////////////////////////////

bool
CImage32::Save(const char* filename, IFileSystem& fs) const
{
  using namespace corona;

  //std::auto_ptr<IFile> file(fs.Open(filename, IFileSystem::write));
  //if (!file.get()) {
  //  return false;
  //}

  std::auto_ptr<Image> img(CreateImage(m_Width, m_Height, PF_R8G8B8A8));
  if (!img.get()) {
    return false;
  }
  memcpy(img->getPixels(), m_Pixels, m_Width * m_Height * 4);

  //CoronaFileAdapter cfa(file.get());
  //return SaveImage(&cfa, FF_AUTODETECT, img.get());
  return SaveImage(filename, FF_AUTODETECT, img.get());
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::SetBlendMode(BlendMode mode)
{
  m_BlendMode = mode;
}

////////////////////////////////////////////////////////////////////////////////

CImage32::BlendMode
CImage32::GetBlendMode() const
{
  return m_BlendMode;
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::ApplyColorFX(int x1, int y1, int w, int h, const CColorMatrix &c)
{
  int i;
  int x, y;
  int x2, y2;
  RGBA pixel;
  RGBA pixeld;
  if(x1 < 0) {
    w += x1;
    x1 = 0;
  }
  if(y1 < 0) {
    h += y1;
    y = 0;
  }
  if(x1 + w >= m_Width) {
    w = m_Width - x1;
  }
  if(y1 + h >= m_Height) {
    h = m_Height - y1;
  }
  if(w <= 0 || h <= 0) {
    return;
  }
  x2 = x1 + w;
  y2 = y1 + h;

  for(y = y1; y < y2; y++) {
    for(x = x1, i = y * m_Width + x1; x < x2; x++, i++) {
      pixel = m_Pixels[i];
      pixeld.red   = std::max(0, std::min(255, c.rn + ((pixel.red * c.rr + pixel.green * c.rg + pixel.blue * c.rb) / 255)));
      pixeld.green = std::max(0, std::min(255, c.gn + ((pixel.red * c.gr + pixel.green * c.gg + pixel.blue * c.gb) / 255)));
      pixeld.blue  = std::max(0, std::min(255, c.bn + ((pixel.red * c.br + pixel.green * c.bg + pixel.blue * c.bb) / 255)));
      pixeld.alpha = pixel.alpha;
      m_Pixels[i] = pixeld;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void 
CImage32::ApplyColorFX4(int x1, int y1, int w, int h, const CColorMatrix &c1, const CColorMatrix &c2, const CColorMatrix &c3, const CColorMatrix &c4)
{
  int i;
  int x, y;
  int x0 = x1, y0 = y1;
  int w0 = w, h0 = h;
  int x2, y2;
  RGBA pixel;
  RGBA pixeld;
  CColorMatrix ca, cb;
  int i1, i2, i3;

  if(x1 < 0) {
    w += x1;
    x1 = 0;
  }
  if(y1 < 0) {
    h += y1;
    y1 = 0;
  }
  if(x1 + w >= m_Width) {
    w = m_Width - x1;
  }
  if(y1 + h >= m_Height) {
    h = m_Height - y1;
  }
  if(w <= 0 || h <= 0) {
    return;
  }
  x2 = x1 + w;
  y2 = y1 + h;
#define INTER(src1, src2, dst, member) dst.member = (src1.member * i1 + src2.member * i2) / i3;
  for(y = y1; y < y2; y++) {
    i1 = (y0 + h0 - y);
    i2 = (y - y0);
    i3 = h0;
    INTER(c1, c3, ca, rn) INTER(c2, c4, cb, rn)
    INTER(c1, c3, ca, rr) INTER(c2, c4, cb, rr)
    INTER(c1, c3, ca, rg) INTER(c2, c4, cb, rg)
    INTER(c1, c3, ca, rb) INTER(c2, c4, cb, rb)

    INTER(c1, c3, ca, gn) INTER(c2, c4, cb, gn)
    INTER(c1, c3, ca, gr) INTER(c2, c4, cb, gr)
    INTER(c1, c3, ca, gg) INTER(c2, c4, cb, gg)
    INTER(c1, c3, ca, gb) INTER(c2, c4, cb, gb)

    INTER(c1, c3, ca, bn) INTER(c2, c4, cb, bn)
    INTER(c1, c3, ca, br) INTER(c2, c4, cb, br)
    INTER(c1, c3, ca, bg) INTER(c2, c4, cb, bg)
    INTER(c1, c3, ca, bb) INTER(c2, c4, cb, bb)

#undef INTER
#define INTER(src1, src2, member) (src1.member * i1 + src2.member * i2)
    for(x = x1, i = y * m_Width + x1; x < x2; x++, i++) {
      //i1 = (x0 + w0 - x) * 256 / w0;
      i2 = (x - x0) * 256 / w0;
      i1 = 256 - i2;

      pixel = m_Pixels[i];
      pixeld.red   = std::max(0, std::min(255, (INTER(ca, cb, rn) >> 8) + ((pixel.red * INTER(ca, cb, rr) + pixel.green * INTER(ca, cb, rg) + pixel.blue * INTER(ca, cb, rb)) >> 16)));
      pixeld.green = std::max(0, std::min(255, (INTER(ca, cb, gn) >> 8) + ((pixel.red * INTER(ca, cb, gr) + pixel.green * INTER(ca, cb, gg) + pixel.blue * INTER(ca, cb, gb)) >> 16)));
      pixeld.blue  = std::max(0, std::min(255, (INTER(ca, cb, bn) >> 8) + ((pixel.red * INTER(ca, cb, br) + pixel.green * INTER(ca, cb, bg) + pixel.blue * INTER(ca, cb, bb)) >> 16)));
      pixeld.alpha = pixel.alpha;
      m_Pixels[i] = pixeld;
    }
  }
#undef INTER
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::Resize(int width, int height)
{
  if (width * height <= 0)
    return;

  if (m_Width == width && m_Height == height)
    return;

  RGBA* new_pixels = new RGBA[width * height];
  if (new_pixels == NULL)
    return;

  for (int ix = 0; ix < width; ix++)
    for (int iy = 0; iy < height; iy++)
    {
      if (ix < m_Width && iy < m_Height)
        new_pixels[iy * width + ix] = m_Pixels[iy * m_Width + ix];
      else
        new_pixels[iy * width + ix] = CreateRGBA(0, 0, 0, 255);
    }

  m_Width  = width;
  m_Height = height;
  delete[] m_Pixels;
  m_Pixels = new_pixels;
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::Rescale(int width, int height)
{
  if (width * height <= 0)
    return;

  RGBA* NewPixels = new RGBA[width * height];
  if (NewPixels == NULL)
    return;

  double HorzAspectRatio, VertAspectRatio;
  int x,y;
  double ix, iy;
  
  HorzAspectRatio = (double)width / (double)m_Width;   // (dstWidth / srcWidth) * 100
  VertAspectRatio = (double)height / (double)m_Height; // (dstHeight / srcHeight) * 100

  // floating point, should be faster than my crappy fixed-point...
  for (y=0; y<height; y++) {
    for (x=0; x<width; x++)
    {
      ix = x / HorzAspectRatio;
      iy = y / VertAspectRatio;

      if ((ix >= 0) && (ix < m_Width))
        if ((iy >= 0) && (iy < m_Height))
          NewPixels[(y * width) + x] = m_Pixels[((int)iy * m_Width) + (int)ix];
    }
  }

  m_Width  = width;
  m_Height = height;
  delete[] m_Pixels;
  m_Pixels = NewPixels;
}

///////////////////////////////////////////////////////////////////////////////

void
CImage32::Resample(int width, int height, bool weighted)
{
  RGBA* NewPixels = resample(m_Pixels, m_Width, m_Height, width, height);
  if (NewPixels) {
    m_Width  = width;
    m_Height = height;
    delete[] m_Pixels;
    m_Pixels = NewPixels;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::FlipHorizontal()
{
  FlipHorizontally(m_Width, m_Height, m_Pixels);
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::FlipVertical()
{
  FlipVertically(m_Width, m_Height, m_Pixels);
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::Translate(int dx, int dy)
{
  ::Translate(m_Width, m_Height, m_Pixels, dx, dy);
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::Rotate(double radians, bool autoSize)
{
  double ix, iy;
  double tx, ty;
  int width, height;
  double xOff, yOff;

  if (!autoSize)
  {
    width = m_Width;
    height = m_Height;
    xOff = 0;
    yOff = 0;
  }
  else
  {
    double xA, xB, xC, xD, yA, yB, yC, yD;
    double xNOff, yNOff;

    // probably the slowest bit in the entire routine...
    // finds the largest and smallest point and use that. 
    // TopLeft, BotRight, TopRight, BotLeft
    xOff = xNOff = 0;
    yOff = yNOff = 0;
    xA = RotateX(0, 0, radians);
    yA = RotateY(0, 0, radians);
    xB = RotateX(m_Width, m_Height, radians);
    yB = RotateY(m_Width, m_Height, radians);
    xC = RotateX(m_Width, 0, radians);
    yC = RotateY(m_Width, 0, radians);
    xD = RotateX(0, m_Height, radians);
    yD = RotateY(0, m_Height, radians);
    xOff = std::max(xOff, xA); xNOff = std::min(xNOff, xA);
    xOff = std::max(xOff, xB); xNOff = std::min(xNOff, xB);
    xOff = std::max(xOff, xC); xNOff = std::min(xNOff, xC);
    xOff = std::max(xOff, xD); xNOff = std::min(xNOff, xD);
    yOff = std::max(yOff, yA); yNOff = std::min(yNOff, yA);
    yOff = std::max(yOff, yB); yNOff = std::min(yNOff, yB);
    yOff = std::max(yOff, yC); yNOff = std::min(yNOff, yC);
    yOff = std::max(yOff, yD); yNOff = std::min(yNOff, yD);
    
    xOff = (xOff - xNOff) - m_Width;
    yOff = (yOff - yNOff) - m_Height;

    width  = m_Width + (int)xOff;
    height = m_Height + (int)yOff;
    xOff /= 2.0;
    yOff /= 2.0;
  }

  RGBA* NewPixels = new RGBA[width * height];
  if (NewPixels == NULL) return;
  memset(NewPixels, 0, width * height * sizeof(RGBA));

  double sine = sin(radians);
  double cosi = cos(radians);

  for (int y=0; y<height; y++)
    for (int x=0; x<width; x++)
    {
      // realigns the rotating axis to 0,0 (of a graphical point of view)
      tx = x - (m_Width/2) - xOff;
      ty = y - (m_Height/2) - yOff;
      
      ix = (cosi * tx) - (sine * ty);
      iy = (sine * tx) + (cosi * ty);

      ix += m_Width/2; iy += m_Height/2;

      // autoclip. Note: the iy & ix info is supposed to be used on m_Pixels!
      if ((ix>=0) && (ix<m_Width))
        if ((iy>=0) && (iy<m_Height))
          NewPixels[(y * width) + x] = m_Pixels[((int)iy * m_Width) + (int)ix];
    }


  // finish up
  if (autoSize)
  {
    m_Width = width;
    m_Height = height;
  }

  delete[] m_Pixels;
  m_Pixels = NewPixels;
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::RotateCW()
{
  if (::RotateCW(m_Width, m_Height, m_Pixels)) {
    int temp = m_Width;
    m_Width = m_Height;
    m_Height = temp;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::RotateCCW()
{
  if (::RotateCCW(m_Width, m_Height, m_Pixels)) {
    int temp = m_Width;
    m_Width = m_Height;
    m_Height = temp;
  }
}

////////////////////////////////////////////////////////////////////////////////

inline void copyRGBA(RGBA& dest, RGBA src)
{
  dest = src;
}

inline void blendRGBA(RGBA& dest, RGBA src)
{
  Blend3(dest, src, src.alpha);
  dest.alpha = 255;
}

inline void copyRGB(RGBA& dest, RGBA src)
{
  dest.red   = src.red;
  dest.green = src.green;
  dest.blue  = src.blue;
}

inline void copyAlpha(RGBA& dest, RGBA src)
{
  dest.alpha = src.alpha;
}

////////////////////////////////////////////////////////////////////////////////

class constant_color
{
public:
  constant_color(RGBA color) : m_color(color) { }
  RGBA operator()(int i, int range) { return m_color; }
private:
  RGBA m_color;
};

void
CImage32::SetPixel(int x, int y, RGBA color, clipper clip)
{
  switch (m_BlendMode) {
    case REPLACE:    primitives::Point(m_Pixels, m_Width, x, y, color, clip, copyRGBA);  break;
    case BLEND:      primitives::Point(m_Pixels, m_Width, x, y, color, clip, blendRGBA); break;
    case RGB_ONLY:   primitives::Point(m_Pixels, m_Width, x, y, color, clip, copyRGB);   break;
    case ALPHA_ONLY: primitives::Point(m_Pixels, m_Width, x, y, color, clip, copyAlpha); break;
  }
}

void
CImage32::SetPixel(int x, int y, RGBA color)
{
  clipper clip = { 0, 0, m_Width - 1, m_Height - 1 };
  SetPixel(x, y, color, clip);
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::SetAlpha(int alpha)
{
  for (int i = 0; i < m_Width * m_Height; i++) {
    m_Pixels[i].alpha = alpha;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::SetColorAlpha(int x, int y, int w, int h, RGB color, int alpha)
{
  for (int iy = y; iy < (y + h); iy++)
    for (int ix = x; ix < (x + w); ix++) {
      RGBA& p = m_Pixels[iy * m_Width + ix];
      if (p.red == color.red &&
          p.green == color.green &&
          p.blue == color.blue) {
        p.alpha = alpha;
      }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::SetColorAlpha(RGB color, int alpha)
{
  for (int i = 0; i < m_Width * m_Height; i++) {
    RGBA& p = m_Pixels[i];
    if (p.red == color.red &&
        p.green == color.green &&
        p.blue == color.blue) {
      p.alpha = alpha;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////

void
CImage32::ReplaceColor(RGBA oldColor, RGBA newColor) {
  for (int i = 0; i < m_Width * m_Height; ++i) {
    RGBA& p = m_Pixels[i];
    if (p == oldColor) {
      p = newColor;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::ApplyLookup(int x, int y, int w, int h, unsigned char rlut[256], unsigned char glut[256], unsigned char blut[256], unsigned char alut[256])
{
  for (int iy = y; iy < (y + h); ++iy) {
    for (int ix = x; ix < (x + w); ++ix) {
      unsigned int i = iy * m_Width + ix;
      m_Pixels[i].red   = rlut[m_Pixels[i].red];
      m_Pixels[i].green = glut[m_Pixels[i].green];
      m_Pixels[i].blue  = blut[m_Pixels[i].blue];
      m_Pixels[i].alpha = alut[m_Pixels[i].alpha];
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::Line(int x1, int y1, int x2, int y2, RGBA color)
{
  clipper clip = { 0, 0, m_Width - 1, m_Height - 1 };
  Line(x1, y1, x2, y2, color, clip);
}

void
CImage32::Line(int x1, int y1, int x2, int y2, RGBA color, clipper clip)
{
  switch (m_BlendMode) {
    case REPLACE:    primitives::Line(m_Pixels, m_Width, x1, y1, x2, y2, constant_color(color), clip, copyRGBA);  break;
    case BLEND:      primitives::Line(m_Pixels, m_Width, x1, y1, x2, y2, constant_color(color), clip, blendRGBA); break;
    case RGB_ONLY:   primitives::Line(m_Pixels, m_Width, x1, y1, x2, y2, constant_color(color), clip, copyRGB);   break;
    case ALPHA_ONLY: primitives::Line(m_Pixels, m_Width, x1, y1, x2, y2, constant_color(color), clip, copyAlpha); break;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::Circle(int x, int y, int r, RGBA color)
{
  // use C segments to draw the circle
  const int C = 20;

  const double pi_2 = acos(-1) * 2;
  for (int i = 0; i < C; i++) {
    int j = (i + 1) % C;
    int x1 = int(x + r * sin(pi_2 * i / C));
    int y1 = int(y + r * cos(pi_2 * i / C));
    int x2 = int(x + r * sin(pi_2 * j / C));
    int y2 = int(y + r * cos(pi_2 * j / C));
    Line(x1, y1, x2, y2, color);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::Ellipse(int cx, int cy, int radx, int rady, RGBA color, int fill)
{
  clipper clip = {0, 0, m_Width - 1, m_Height - 1};
  Ellipse(cx, cy, radx, rady, color, fill, clip);
}

void
CImage32::Ellipse(int cx, int cy, int radx, int rady, RGBA color, int fill, clipper clip)
{
  int mx1, my1, mx2, my2;
  int aq, bq;
  int dx, dy;
  int r, rx, ry;
  int x;

  if (fill) {
    Line(cx - radx, cy, cx + radx, cy, color, clip);
  }
  else
  {
    SetPixel(cx + radx, cy, color, clip);
    SetPixel(cx - radx, cy, color, clip);
  }

  mx1 = cx - radx;  my1 = cy;
  mx2 = cx + radx;  my2 = cy;

  aq = radx * radx;
  bq = rady * rady;
  dx = aq<<1;
  dy = bq<<1;
  r = radx * bq;
  rx = r<<1;
  ry = 0;
  x = radx;

  while (x > 0)
  {
    if (r > 0)
    {
      my1++;
      my2--;
      ry += dx;
      r -= ry;
    }
    if (r <= 0)
    {
      x--;
      mx1++;
      mx2--;
      rx-=dy;
      r+=rx;
    }

    if (fill)
    {
      Line(mx1, my1, mx2, my1, color, clip);
      Line(mx1, my2, mx2, my2, color, clip);
    }
    else
    {
      SetPixel(mx1, my1, color, clip);
      SetPixel(mx2, my1, color, clip);
      SetPixel(mx1, my2, color, clip);
      SetPixel(mx2, my2, color, clip);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::Rectangle(int x1, int y1, int x2, int y2, RGBA color)
{
  clipper clip = { 0, 0, m_Width -1, m_Height - 1 };
  Rectangle(x1, y1, x2, y2, color, clip);
}

void
CImage32::Rectangle(int x1, int y1, int x2, int y2, RGBA color, clipper clip)
{
  // make sure x1 < x2 and y1 < y2 so we can get good w and h values
  if (x1 > x2) {
    std::swap(x1, x2);
  }
  if (y1 > y2) {
    std::swap(y1, y2);
  }
  int w = x2 - x1 + 1;
  int h = y2 - y1 + 1;

  switch (m_BlendMode) {
    case REPLACE:    primitives::Rectangle(m_Pixels, m_Width, x1, y1, w, h, color, clip, copyRGBA);  break;
    case BLEND:      primitives::Rectangle(m_Pixels, m_Width, x1, y1, w, h, color, clip, blendRGBA); break;
    case RGB_ONLY:   primitives::Rectangle(m_Pixels, m_Width, x1, y1, w, h, color, clip, copyRGB);   break;
    case ALPHA_ONLY: primitives::Rectangle(m_Pixels, m_Width, x1, y1, w, h, color, clip, copyAlpha); break;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::Triangle(int x1, int y1, int x2, int y2, int x3, int y3, RGBA color)
{
  int x[] = {x1, x2, x3};
  int y[] = {y1, y2, y3};

  clipper clip = { 0, 0, m_Width - 1, m_Height - 1 };
  switch (m_BlendMode) {
    case REPLACE:    primitives::Triangle(m_Pixels, m_Width, x, y, color, clip, copyRGBA); break;
    case BLEND:      primitives::Triangle(m_Pixels, m_Width, x, y, color, clip, blendRGBA); break;
    case RGB_ONLY:   primitives::Triangle(m_Pixels, m_Width, x, y, color, clip, copyRGB); break;
    case ALPHA_ONLY: primitives::Triangle(m_Pixels, m_Width, x, y, color, clip, copyAlpha); break;
  }
}

////////////////////////////////////////////////////////////////////////////////

class gradient_color
{
public:
  gradient_color(RGBA color1, RGBA color2)
  : m_color1(color1)
  , m_color2(color2) {
  }

  RGBA operator()(int i, int range) {
    if (range == 0) {
      return m_color1;
    }
    RGBA color;
    color.red   = (i * m_color1.red   + (range - i) * m_color2.red)   / range;
    color.green = (i * m_color1.green + (range - i) * m_color2.green) / range;
    color.blue  = (i * m_color1.blue  + (range - i) * m_color2.blue)  / range;
    color.alpha = (i * m_color1.alpha + (range - i) * m_color2.alpha) / range;
    return color;
  }

private:
  RGBA m_color1;
  RGBA m_color2;
};

inline RGBA interpolateRGBA(RGBA a, RGBA b, int i, int range)
{
  if (range == 0) {
    return a;
  }

  RGBA result = {
    (a.red   * (range - i) + b.red   * i) / range,
    (a.green * (range - i) + b.green * i) / range,
    (a.blue  * (range - i) + b.blue  * i) / range,
    (a.alpha * (range - i) + b.alpha * i) / range,
  };

  return result;
}

void
CImage32::GradientLine(int x1, int y1, int x2, int y2, RGBA c[2])
{
  clipper clip = { 0, 0, m_Width - 1, m_Height - 1 };
  switch (m_BlendMode) {
    case REPLACE:    primitives::Line((RGBA*)m_Pixels, m_Width, x1, y1, x2, y2, gradient_color(c[0], c[1]), clip, copyRGBA);  break;
    case BLEND:      primitives::Line((RGBA*)m_Pixels, m_Width, x1, y1, x2, y2, gradient_color(c[0], c[1]), clip, blendRGBA); break;
    case RGB_ONLY:   primitives::Line((RGBA*)m_Pixels, m_Width, x1, y1, x2, y2, gradient_color(c[0], c[1]), clip, copyRGB);   break;
    case ALPHA_ONLY: primitives::Line((RGBA*)m_Pixels, m_Width, x1, y1, x2, y2, gradient_color(c[0], c[1]), clip, copyAlpha); break;
  }
}

void
CImage32::GradientRectangle(int x, int y, int w, int h, RGBA c[4])
{
  clipper clip = { 0, 0, m_Width - 1, m_Height - 1 };
  switch (m_BlendMode) {
    case REPLACE:    primitives::GradientRectangle((RGBA*)m_Pixels, m_Width, x, y, w, h, c, clip, copyRGBA, interpolateRGBA);  break;
    case BLEND:      primitives::GradientRectangle((RGBA*)m_Pixels, m_Width, x, y, w, h, c, clip, blendRGBA, interpolateRGBA); break;
    case RGB_ONLY:   primitives::GradientRectangle((RGBA*)m_Pixels, m_Width, x, y, w, h, c, clip, copyRGB, interpolateRGBA);   break;
    case ALPHA_ONLY: primitives::GradientRectangle((RGBA*)m_Pixels, m_Width, x, y, w, h, c, clip, copyAlpha, interpolateRGBA); break;
  }
}

void
CImage32::GradientTriangle(int x1, int y1, int x2, int y2, int x3, int y3, RGBA c[3])
{
  clipper clip = { 0, 0, m_Width - 1, m_Height - 1 };

  int x[3] = {x1, x2, x3};
  int y[3] = {y1, y2, y3};

  switch (m_BlendMode) {
    case REPLACE:    primitives::GradientTriangle((RGBA*)m_Pixels, m_Width, x, y, c, clip, copyRGBA, interpolateRGBA);  break;
    case BLEND:      primitives::GradientTriangle((RGBA*)m_Pixels, m_Width, x, y, c, clip, blendRGBA, interpolateRGBA); break;
    case RGB_ONLY:   primitives::GradientTriangle((RGBA*)m_Pixels, m_Width, x, y, c, clip, copyRGB, interpolateRGBA);   break;
    case ALPHA_ONLY: primitives::GradientTriangle((RGBA*)m_Pixels, m_Width, x, y, c, clip, copyAlpha, interpolateRGBA); break;
  }
}

////////////////////////////////////////////////////////////////////////////////

inline void replaceRenderer(RGBA& dest, RGBA src, RGBA alpha) {
  dest = src;
}

inline void blendRenderer(RGBA& dest, RGBA src, RGBA alpha) {
  Blend4(dest, src, alpha.alpha);
}

inline void rgbRenderer(RGBA& dest, RGBA src, RGBA alpha) {
  dest.red   = src.red;
  dest.green = src.green;
  dest.blue  = src.blue;
}

inline void alphaRenderer(RGBA& dest, RGBA src, RGBA alpha) {
  dest.alpha = src.alpha;
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::BlitImage(CImage32& image, int x, int y)
{
  clipper clip = {
    0, 0, m_Width - 1, m_Height - 1
  };

  switch (m_BlendMode) {
    case REPLACE:
      primitives::Blit(
        m_Pixels, m_Width,
        x, y,
        image.GetPixels(),
        image.GetPixels(),
        image.GetWidth(),
        image.GetHeight(),
        clip,
        replaceRenderer);
      break;
    case BLEND:
      primitives::Blit(
        m_Pixels, m_Width,
        x, y,
        image.GetPixels(),
        image.GetPixels(),
        image.GetWidth(),
        image.GetHeight(),
        clip,
        blendRenderer);
      break;
    case RGB_ONLY:
      primitives::Blit(
        m_Pixels, m_Width,
        x, y,
        image.GetPixels(),
        image.GetPixels(),
        image.GetWidth(),
        image.GetHeight(),
        clip,
        rgbRenderer);
      break;
    case ALPHA_ONLY:
      primitives::Blit(
        m_Pixels, m_Width,
        x, y,
        image.GetPixels(),
        image.GetPixels(),
        image.GetWidth(),
        image.GetHeight(),
        clip,
        alphaRenderer);
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::TransformBlitImage(CImage32& image, int x[4], int y[4])
{
  clipper clip = {
    0, 0, m_Width - 1, m_Height - 1
  };

  switch (m_BlendMode) {
    case REPLACE:
      primitives::TexturedQuad(
        m_Pixels,
        m_Width,
        x,
        y,
        image.GetPixels(),
        image.GetPixels(),
        image.GetWidth(),
        image.GetHeight(),
        clip,
        replaceRenderer
      );
    break;

    case BLEND:
      primitives::TexturedQuad(
        m_Pixels,
        m_Width,
        x,
        y,
        image.GetPixels(),
        image.GetPixels(),
        image.GetWidth(),
        image.GetHeight(),
        clip,
        blendRenderer
      );
    break;

    case RGB_ONLY:
      primitives::TexturedQuad(
        m_Pixels,
        m_Width,
        x,
        y,
        image.GetPixels(),
        image.GetPixels(),
        image.GetWidth(),
        image.GetHeight(),
        clip,
        rgbRenderer);
      break;

    case ALPHA_ONLY:
      primitives::TexturedQuad(
        m_Pixels,
        m_Width,
        x,
        y,
        image.GetPixels(),
        image.GetPixels(),
        image.GetWidth(),
        image.GetHeight(),
        clip,
        alphaRenderer);
    break;

  }
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::BlitImageMask(CImage32& image, int x, int y, RGBA mask)
{
  BlitImage(image, x, y);
}

////////////////////////////////////////////////////////////////////////////////

void
CImage32::TransformBlitImageMask(CImage32& image, int x[4], int y[4], RGBA mask)
{
  TransformBlitImage(image, x, y);
}

////////////////////////////////////////////////////////////////////////////////
