#ifndef RGB_HPP
#define RGB_HPP

#define USE_ALPHA_TABLE

#include "types.h"

#ifdef USE_ALPHA_TABLE
extern unsigned char alpha_old[256][256];
extern unsigned char alpha_new[256][256];
#endif

#define STRUCT_NAME RGB
#define STRUCT_BODY \
  byte red;         \
  byte green;       \
  byte blue;
#include "packed_struct.h"

#define STRUCT_NAME BGR
#define STRUCT_BODY \
  byte blue;        \
  byte green;       \
  byte red;
#include "packed_struct.h"

#define STRUCT_NAME RGBA
#define STRUCT_BODY \
  byte red;         \
  byte green;       \
  byte blue;        \
  byte alpha;
#include "packed_struct.h"

#define STRUCT_NAME BGRA
#define STRUCT_BODY \
  byte blue;        \
  byte green;       \
  byte red;         \
  byte alpha;
#include "packed_struct.h"


inline bool operator==(const RGB& c1, const RGB& c2) {
  return (c1.red   == c2.red &&
          c1.green == c2.green &&
          c1.blue  == c2.blue);
}

inline bool operator==(const BGR& c1, const BGR& c2) {
  return (c1.red   == c2.red &&
          c1.green == c2.green &&
          c1.blue  == c2.blue);
}

inline bool operator==(const RGBA& c1, const RGBA& c2) {
  return (c1.red   == c2.red &&
          c1.green == c2.green &&
          c1.blue  == c2.blue &&
          c1.alpha == c2.alpha);
}

inline bool operator==(const BGRA& c1, const BGRA& c2) {
  return (c1.red   == c2.red &&
          c1.green == c2.green &&
          c1.blue  == c2.blue &&
          c1.alpha == c2.alpha);
}


// premultiply alpha
template<typename destT>
void PremultiplyAlpha(destT& d, int alpha) {
#ifndef USE_ALPHA_TABLE
  d.red   = d.red   * alpha / 256;
  d.green = d.green * alpha / 256;
  d.blue  = d.blue  * alpha / 256;
#else
  d.red   = alpha_new[d.red][alpha];
  d.green = alpha_new[d.green][alpha];
  d.blue  = alpha_new[d.blue][alpha];
#endif
}


// blends two colors together
template<typename destT, typename srcT>
void Blend3(destT& d, srcT s, int alpha) {
#ifndef USE_ALPHA_TABLE
  d.red   = (s.red   * alpha + d.red   * (256 - alpha)) / 256;
  d.green = (s.green * alpha + d.green * (256 - alpha)) / 256;
  d.blue  = (s.blue  * alpha + d.blue  * (256 - alpha)) / 256;
#else
  d.red   = alpha_new[s.red][alpha]   + alpha_old[d.red][alpha];
  d.green = alpha_new[s.green][alpha] + alpha_old[d.green][alpha];
  d.blue  = alpha_new[s.blue][alpha]  + alpha_old[d.blue][alpha];
#endif
}


template<typename destT, typename srcT>
void Blend4(destT& d, srcT s, int alpha) {
#ifndef USE_ALPHA_TABLE
  d.red   = (s.red   * alpha + d.red   * (256 - alpha)) / 256;
  d.green = (s.green * alpha + d.green * (256 - alpha)) / 256;
  d.blue  = (s.blue  * alpha + d.blue  * (256 - alpha)) / 256;
  d.alpha = (s.alpha * alpha + d.alpha * (256 - alpha)) / 256;
#else
  d.red   =  alpha_new[s.red][alpha]   + alpha_old[d.red][alpha];
  d.green =  alpha_new[s.green][alpha] + alpha_old[d.green][alpha];
  d.blue  =  alpha_new[s.blue][alpha]  + alpha_old[d.blue][alpha];
  d.alpha  = alpha_new[s.alpha][alpha] + alpha_old[d.alpha][alpha];
#endif
}


// blends two colors if source has premultiplied alpha
template<typename destT, typename srcT>
void BlendPA(destT& d, srcT s, int alpha) {
#ifndef USE_ALPHA_TABLE
  d.red   = s.red   + d.red   * (256 - alpha) / 256;
  d.green = s.green + d.green * (256 - alpha) / 256;
  d.blue  = s.blue  + d.blue  * (256 - alpha) / 256;
#else
  d.red   = s.red   + alpha_old[d.red][alpha];
  d.green = s.green + alpha_old[d.green][alpha];
  d.blue  = s.blue  + alpha_old[d.blue][alpha];
#endif
}


// creates red-green-blue color
template<typename colorT>
colorT CreateColor3(byte red, byte green, byte blue) {
  colorT c;
  c.red   = red;
  c.green = green;
  c.blue  = blue;
  return c;
}


// creates red-green-blue-alpha color
template<typename colorT>
colorT CreateColor4(byte red, byte green, byte blue, byte alpha) {
  colorT c;
  c.red   = red;
  c.green = green;
  c.blue  = blue;
  c.alpha = alpha;
  return c;
}

inline RGBA CreateRGBA(byte red, byte green, byte blue, byte alpha) {
  return CreateColor4<RGBA>(red, green, blue, alpha);
}

inline BGRA CreateBGRA(byte red, byte green, byte blue, byte alpha) {
  return CreateColor4<BGRA>(red, green, blue, alpha);
}

inline RGB CreateRGB(byte red, byte green, byte blue) {
  return CreateColor3<RGB>(red, green, blue);
}


#endif
