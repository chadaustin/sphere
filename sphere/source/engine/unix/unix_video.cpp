#include "unix_video.h"
#include "unix_input.h"
#include "../../common/primitives.hpp"
#include <cstring>

SFONT* FPSFont;
static bool FPSDisplayed;
static SDL_Surface* screen;
static SDL_Surface* double_buffer; /* double trouble */
static RGBA global_mask; /* bad, jcore! bad! */
static SDL_PixelFormat* image_format;

/* a special clipping rect that the primitives routines can deal with */
typedef struct _CLIPPER {
  int left, right, top, bottom;
} CLIPPER;

static CLIPPER clipping_rectangle; /* convenience for primitives */

inline CLIPPER MakeClipper (SDL_Rect rect) {
  CLIPPER clipper;

  clipper.left = rect.x;
  clipper.right = rect.x + rect.w - 1;
  clipper.top = rect.y;
  clipper.bottom = rect.y + rect.h - 1;

  return clipper;
}

template <typename leftT, typename rightT>
inline leftT MIN (leftT left, rightT right) {
  left < right ? left : right;
}

template <typename leftT, typename rightT>
inline leftT MAX (leftT left, rightT right) {
  left > right ? left : right;
}

void straight_copy (Uint32& dest, Uint32 src) {
  dest = src;
}

void straight_copyRGBA (Uint32& dest, RGBA src) {
  dest = SDL_MapRGBA(screen->format, src.red, src.green, src.blue, src.alpha);
}

void blend_alpha (Uint32& dest, Uint32 src) {
  Uint8 sr, sg, sb, sa;
  Uint8 dr, dg, db, da;

  SDL_GetRGBA(src, image_format, &sr, &sg, &sb, &sa);
  SDL_GetRGBA(dest, screen->format, &dr, &dg, &db, &da);
  sa = (int)sa * global_mask.alpha / 256;
  sr = (int)sr * global_mask.red / 256;
  sg = (int)sg * global_mask.green / 256;
  sb = (int)sb * global_mask.blue / 256;
  dr = (dr * (256 - sa) + sr * sa) / 256;
  dg = (dg * (256 - sa) + sg * sa) / 256;
  db = (db * (256 - sa) + sb * sa) / 256;
  dest = SDL_MapRGBA(screen->format, dr, dg, db, da);
}

void blend_alphaRGBA (Uint32& dest, RGBA src) {
  Uint8 r, g, b, a;

  SDL_GetRGBA(dest, screen->format, &r, &g, &b, &a);
  a = src.alpha;
  a = a * global_mask.alpha / 256;
  src.red = src.red * global_mask.red / 256;
  src.green = src.green * global_mask.green / 256;
  src.blue = src.blue * global_mask.blue / 256;
  r = (r * (256 - a) + r * a) / 256;
  g = (g * (256 - a) + g * a) / 256;
  b = (b * (256 - a) + b * a) / 256;
  dest = SDL_MapRGBA(screen->format, r, g, b, a);
}

void blend_copy (Uint32& dest, Uint32 src) {
  Uint8 sr, sg, sb, sa;
  Uint8 dr, dg, db, da;

  SDL_GetRGBA(src, image_format, &sr, &sg, &sb, &sa);
  SDL_GetRGBA(dest, screen->format, &dr, &dg, &db, &da);
  dr = (sr * sa + dr * (256 - sa)) / 256;
  dg = (sg * sa + dg * (256 - sa)) / 256;
  db = (db * sa + db * (256 - sa)) / 256;
  dest = SDL_MapRGBA(screen->format, dr, dg, db, da);
}

void blend_copyRGBA (Uint32& dest, RGBA src) {
  Uint8 r, g, b, a;

  SDL_GetRGBA(dest, screen->format, &r, &g, &b, &a);
  r = (src.red * src.alpha + r * (256 - src.alpha)) / 256;
  g = (src.green * src.alpha + g * (256 - src.alpha)) / 256;
  b = (src.blue * src.alpha + b * (256 - src.alpha)) / 256;
  dest = SDL_MapRGBA(screen->format, r, g, b, a);
}

class constant_color {
 public:
  constant_color (Uint32 color)
    : m_color(color) { }

  Uint32 operator() (int i, int range) {
    return m_color;
  }

 private:
  Uint32 m_color;
};

class constant_colorRGBA {
 public:
  constant_colorRGBA (RGBA color) {
    m_color = SDL_MapRGBA(screen->format, color.red, color.blue,
                          color.green, color.alpha);
  }
  Uint32 operator() (int i, int range) {
  }
 private:
  Uint32 m_color;
};

class gradient_color {
 public:
  gradient_color (Uint32 color1, Uint32 color2) {
    SDL_GetRGBA(color1, image_format, &m_r1, &m_g1, &m_b1, &m_a1);
    SDL_GetRGBA(color2, image_format, &m_r2, &m_g2, &m_b2, &m_a2);
  }

  Uint32 operator() (int i, int range) {
    if (range == 0) {
      return SDL_MapRGBA(screen->format, m_r1, m_g1, m_b1, m_a1);
    }
    Uint8 r, g, b, a;

    r = (i * m_r2 + (range - i) * m_r1) / range;
    g = (i * m_g2 + (range - i) * m_g1) / range;
    b = (i * m_b2 + (range - i) * m_b1) / range;
    a = (i * m_a2 + (range - i) * m_a1) / range;
    return SDL_MapRGBA(screen->format, r, g, b, a);
  }

 private:
  Uint8 m_r1, m_g1, m_b1, m_a1;
  Uint8 m_r2, m_g2, m_b2, m_a2;
};

class gradient_colorRGBA {
 public:
  gradient_colorRGBA (RGBA color1, RGBA color2)
    : m_color1(color1), m_color2(color2) { }
  Uint32 operator() (int i, int range) {
    if (range == 0) {
      return SDL_MapRGBA(screen->format, m_color1.red, m_color1.green,
                         m_color1.blue, m_color1.alpha);
    }
    RGBA c;

    c.red = (i * m_color2.red + (range - i) * m_color1.red) / range;
    c.green = (i * m_color2.green + (range - i) * m_color1.green) / range;
    c.blue = (i * m_color2.blue + (range - i) * m_color1.blue) / range;
    c.alpha = (i * m_color2.alpha + (range - i) * m_color1.alpha) / range;
    return SDL_MapRGBA(screen->format, c.red, c.blue, c.green, c.alpha);
  }
 private:
  RGBA m_color1, m_color2;
};

/* \brief set the fps font

  This font will be used to the display the frames per second counter on the screen */
void SetFPSFont (SFONT* font) {
  FPSFont = font;
}

/* \brief toggle display of frames per second */
void ToggleFPS () {
  FPSDisplayed = !FPSDisplayed;
}

/*! \brief switch the screen resolution

  This is where all the fun begins.  If this is the first time that SwitchResolution
  is called, SDL is initialized. */
bool SwitchResolution (int x, int y) {
  static bool initialized = false;

  if (!initialized) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD) == -1)
      return false;
	 screen = SDL_SetVideoMode(x, y, 32, SDL_HWSURFACE | SDL_DOUBLEBUF); /* | SDL_FULLSCREEN); */
    if (screen == NULL)
      return false;
    FPSDisplayed = false;
    initialized = true;
    SetClippingRectangle(0, 0, screen->w, screen->h);
    return true;
  } else {
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
      return false;
	 screen = SDL_SetVideoMode(x, y, 32, SDL_HWSURFACE | SDL_DOUBLEBUF); /* | SDL_FULLSCREEN); */
    if (screen == NULL)
      return false;
    FPSDisplayed = false;
    initialized = true;
	 SetClippingRectangle(0, 0, screen->w, screen->h);
    return true;
  }
}

int GetScreenWidth () {
  return screen->w;
}

int GetScreenHeight () {
  return screen->h;
}

void FlipScreen () {
  SDL_Flip(screen);
	RefreshInput();
}

void SetClippingRectangle (int x, int y, int w, int h) {
  SDL_Rect rect;

  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;
  SDL_SetClipRect(screen, &rect);
  clipping_rectangle = MakeClipper(rect);
}

void GetClippingRectangle (int* x, int* y, int* w, int* h) {
  SDL_Rect rect;

  SDL_GetClipRect(screen, &rect);
  *x = rect.x;
  *y = rect.y;
  *w = rect.w;
  *h = rect.h;
}

IMAGE CreateImage (int width, int height, const RGBA* pixels) {
  SDL_Surface* surface;
  Uint32 rmask, gmask, bmask, amask;

  #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xFF000000;
    gmask = 0x00FF0000;
    bmask = 0x0000FF00;
    amask = 0x000000FF;
  #else
    rmask = 0x000000FF;
    gmask = 0x0000FF00;
    bmask = 0x00FF0000;
    amask = 0xFF000000;
  #endif

  surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32,
                                 rmask, gmask, bmask, amask);
  if (surface == NULL)
    return NULL;
  if (SDL_LockSurface(surface) == -1) {
    SDL_FreeSurface(surface);
    return NULL;
  }
  if (image_format == NULL) {
	  image_format = new SDL_PixelFormat;
	  memcpy(image_format, surface->format, sizeof(SDL_PixelFormat));
  }
  memcpy(surface->pixels, pixels, 4 * width * height);
  SDL_UnlockSurface(surface);
  return surface;
}

IMAGE GrabImage (int x, int y, int width, int height) {
  SDL_Surface* surface;
  SDL_Rect source;

  if ((width > GetScreenWidth()) || (height > GetScreenHeight()))
    return NULL;
  surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32,
                                 0, 0, 0, 0);
  if (surface == NULL)
    return NULL;
  source.x = x;
  source.y = y;
  source.w = width;
  source.h = height;
  if (SDL_BlitSurface(screen, &source, surface, NULL) == -1) {
    SDL_FreeSurface(surface);
    return NULL;
  }
  return surface;
}

void DestroyImage (IMAGE image) {
  SDL_FreeSurface(image);
}

void BlitImage (IMAGE image, int x, int y) {
  SDL_Rect dest;

  dest.x = x;
  dest.y = y;
  SDL_BlitSurface(image, NULL, screen, &dest);
}

template <typename routineT>
void StraightBlit (IMAGE image, int x, int y, routineT routine) {
  int lcv_v, lcv_h;
  int scanlines;
  int width;
  SDL_Rect clip;
  Uint32* dpixel;
  Uint32* spixel;

  if (SDL_LockSurface(screen) == 0) {
    SDL_GetClipRect(screen, &clip);
    scanlines = MIN(image->h + y, clip.h) - MAX(y, clip.y);
    width = MIN(image->w + x, clip.w) - MAX(x, clip.x);
    dpixel = static_cast<Uint32*>(screen->pixels) + (MAX(y, clip.y) * screen->w + MAX(x, clip.x));
    spixel = static_cast<Uint32*>(image->pixels);
    for (lcv_v = 0; lcv_v < scanlines; lcv_v++) {
      for (lcv_h = 0; lcv_h < width; lcv_h++) {
        routine(*(dpixel + lcv_h), *(spixel + lcv_h));
      }
      spixel += image->w;
      dpixel += screen->w;
    }
	 SDL_UnlockSurface(screen);
  }
}

template<typename pixelT, typename clipT, typename renderT>
void unixBlit(
  pixelT* surface,
  int pitch,
  const int x,
  const int y,
  pixelT* texture,
  int tex_width,
  int tex_height,
  clipT clipper,
  renderT renderer)
{
  int image_offset_x = 0;
  int image_offset_y = 0;
  int image_blit_width = tex_width;
  int image_blit_height = tex_height;

  if (x < clipper.left) {
    image_offset_x = (clipper.left - x);
    image_blit_width -= image_offset_x;
  }

  if (y < clipper.top) {
    image_offset_y = (clipper.top - y);
    image_blit_height -= image_offset_y;
  }
  
  if (x + (int)tex_width - 1 > clipper.right) {
    image_blit_width -= (x + tex_width - clipper.right - 1);
  }

  if (y + (int)tex_height - 1 > clipper.bottom) {
    image_blit_height -= (y + tex_height - clipper.bottom - 1);
  }

  // heh, funny abbreviations
  pixelT* dst = surface + (y + image_offset_y) * pitch + image_offset_x + x;
  pixelT* src = texture +       image_offset_y * tex_width + image_offset_x;

  int dst_inc = pitch - image_blit_width;
  int src_inc = tex_width - image_blit_width;

  int iy = image_blit_height;
  while (iy-- > 0) {
    int ix = image_blit_width;
    while (ix-- > 0) {
		renderer(*dst, *src);

      dst++;
      src++;
    }

    dst += dst_inc;
    src += src_inc;
  }
}

void BlitImageMask (IMAGE image, int x, int y, RGBA mask) {
  global_mask = mask;
  /* StraightBlit(image, x, y, blend_alpha); */
  unixBlit((Uint32*)(screen->pixels), screen->w, x, y,
           (Uint32*)(image->pixels), image->w, image->h,
			  clipping_rectangle, blend_alpha);
}

void aBlendRGBA (Uint32& dest, Uint32 src) {
  Uint8 sr, sg, sb, sa;
  Uint8 dr, dg, db, da;

  SDL_GetRGBA(src, screen->format, &sr, &sg, &sb, &sa);
  SDL_GetRGBA(dest, screen->format, &dr, &dg, &db, &da);
  dest = SDL_MapRGBA(screen->format,
    (dr * (256 - sa)) / 256 + sr,
    (dg * (256 - sa)) / 256 + sg,
    (db * (256 - sa)) / 256 + sb,
    da
  );
}

using primitives::bracket;
/* this is a slightly modified copy of a function in primitives.hpp
   it was modified so that it could work on surfaces without a separate
   alpha channel */
template<typename destT, typename srcT, typename clipT,
         typename renderT>
void unixTexturedQuad (destT* surface, int pitch, const int x[4], const int y[4],
                   srcT* texture, int tex_width, int tex_height,
                   clipT clipper, renderT renderer) {
  // find top and bottom points
  int top = 0;
  int bottom = 0;
  for (int i = 1; i < 4; i++) {
    if (y[i] < y[top]) {
      top = i;
    }
    if (y[i] > y[bottom]) {
      bottom = i;
    }
  }

  // perform clipping in the y axis
  int oldMinY = y[top];
  int oldMaxY = y[bottom];
  int minY = bracket<int>(y[top], clipper.top, clipper.bottom);
  int maxY = bracket<int>(y[bottom], clipper.top, clipper.bottom);

  // precalculate line segment information
  struct segment {
    // y1 < y2, always
    int x1, x2;
    int y1, y2;
    int u1, u2;
    int v1, v2;
  } segments[4];

  // segment 0 = top
  // segment 1 = right
  // segment 2 = bottom
  // segment 3 = left

  for (int i = 0; i < 4; i++) {
    segment* s = segments + i;

    int p1 = i;
    int p2 = (i + 1) & 3;  // x & 3 == x % 4

    s->x1 = x[p1];
    s->y1 = y[p1];
    s->u1 = (i == 1 || i == 2 ? tex_width  - 1 : 0);
    s->v1 = (i == 2 || i == 3 ? tex_height - 1 : 0);

    s->x2 = x[p2];
    s->y2 = y[p2];
    s->u2 = (i == 0 || i == 1 ? tex_width  - 1 : 0);
    s->v2 = (i == 1 || i == 2 ? tex_height - 1 : 0);

    if (y[p1] > y[p2]) {
      std::swap(s->x1, s->x2);
      std::swap(s->y1, s->y2);
      std::swap(s->u1, s->u2);
      std::swap(s->v1, s->v2);
    }
  }

  // draw scanlines
  for (int iy = minY; iy <= maxY; iy++) {

    // find minimum and maximum x values

    // initial boundary values
    int minX = clipper.right + 1;
    int maxX = clipper.left - 1;

    // default values in case no 
    int minU = 0;
    int minV = 0;
    int maxU = 0;
    int maxV = 0;

    // intersect iy in each line
    for (int i = 0; i < 4; i++) {
      segment* s = segments + i;

      // if iy is even in the segment and segment's length is not zero
      if (s->y1 <= iy && iy <= s->y2) {
        int x = (s->y1 == s->y2 ?
          s->x1 :
          s->x1 + (iy - s->y1) * (s->x2 - s->x1) / (s->y2 - s->y1));
        int u = (s->y1 == s->y2 ?
          s->u1 :
          s->u1 + (iy - s->y1) * (s->u2 - s->u1) / (s->y2 - s->y1));
        int v = (s->y1 == s->y2 ?
          s->v1 :
          s->v1 + (iy - s->y1) * (s->v2 - s->v1) / (s->y2 - s->y1));

        // update minimum and maximum x values
        if (x < minX) {
          minX = x;
          minU = u;
          minV = v;
        }
        if (x > maxX) {
          maxX = x;
          maxU = u;
          maxV = v;
        }
      }
    }

    // now clip the x extents
    int oldMinX = minX;
    int oldMaxX = maxX;
    minX = bracket<int>(minX, clipper.left, clipper.right);
    maxX = bracket<int>(maxX, clipper.left, clipper.right);
   
    // render the scanline pixels
    if (minX == maxX) {
      renderer(surface[iy * pitch + minX], texture[minV * tex_width + minU]);
    } else {
      for (int ix = minX; ix <= maxX; ix++) {
        int iu = minU + (ix - oldMinX) * (maxU - minU) / (oldMaxX - oldMinX);
        int iv = minV + (ix - oldMinX) * (maxV - minV) / (oldMaxX - oldMinX);
        renderer(surface[iy * pitch + ix], texture[iv * tex_width + iu]);
      }
    }
  } // end for scanlines
}

void TransformBlitImage (IMAGE image, int x[4], int y[4]) {
  if (SDL_LockSurface(screen) == 0) {
    unixTexturedQuad((Uint32*)(screen->pixels), screen->w, x, y,
                     (Uint32*)(image->pixels), image->w, image->h,
                     clipping_rectangle, aBlendRGBA);
    SDL_UnlockSurface(screen);
  }
}

void TransformBlitImageMask (IMAGE image, int x[4], int y[4], RGBA mask) {
  if (SDL_LockSurface(screen) == 0) {
    global_mask = mask;
    unixTexturedQuad((Uint32*)(screen->pixels), screen->w, x, y,
                     (Uint32*)(image->pixels), image->w, image->h,
                     clipping_rectangle, blend_alpha);
    SDL_UnlockSurface(screen);
  }
}

int GetImageWidth (IMAGE image) {
  return image->w;
}

int GetImageHeight (IMAGE image) {
  return image->h;
}

RGBA* LockImage (IMAGE image) {
  SDL_LockSurface(image);
  return reinterpret_cast<RGBA*>(image->pixels);
}

void UnlockImage (IMAGE image) {
  SDL_UnlockSurface(image);
}

void DirectBlit (int x, int y, int w, int h, RGBA* pixels) {
  SDL_Surface* temp;

  temp = CreateImage(w, h, pixels);
  BlitImage(temp, x, y);
  DestroyImage(temp);
}

void DirectTransformBlit (int x[4], int y[4], int w, int h, RGBA* pixels) {
  SDL_Surface* temp;

  temp = CreateImage(w, h, pixels);
  TransformBlitImage(temp, x, y);
  DestroyImage(temp);
}

void DirectGrab (int x, int y, int w, int h, RGBA* pixels) {
  SDL_Surface* temp;

  temp = GrabImage(x, y, w, h);
  SDL_LockSurface(temp);
  memcpy(pixels, temp->pixels, w * h * 4);
  SDL_UnlockSurface(temp);
  DestroyImage(temp);
}

void DrawPoint (int x, int y, RGBA color) {
  if (SDL_LockSurface(screen) == 0) {
    primitives::Point((Uint32*)(screen->pixels), screen->w, x, y, color,
                      clipping_rectangle, blend_copyRGBA);
    SDL_UnlockSurface(screen);
  }
}

void DrawLine (int x[2], int y[2], RGBA color) {
  if (SDL_LockSurface(screen) == 0) {
    primitives::Line((Uint32*)(screen->pixels), screen->w, x[0], y[0],
                     x[1], y[1], constant_colorRGBA(color),
                     clipping_rectangle, blend_copy);
    SDL_UnlockSurface(screen);
  }
}

void DrawGradientLine (int x[2], int y[2], RGBA color[2]) {
  if (SDL_LockSurface(screen) == 0) {
    primitives::Line((Uint32*)(screen->pixels), screen->w, x[0], y[0],
                     x[1], y[1], gradient_colorRGBA(color[0], color[1]),
                     clipping_rectangle, blend_copy);
    SDL_UnlockSurface(screen);
  }
}

void DrawTriangle (int x[3], int y[3], RGBA color) {
  if (SDL_LockSurface(screen) == 0) {
    primitives::Triangle((Uint32*)(screen->pixels), screen->w, x, y,
                          color, clipping_rectangle, blend_copyRGBA);
    SDL_UnlockSurface(screen);
  }
}

inline RGBA interpolateRGBA(RGBA a, RGBA b, int i, int range) {
  if (range == 0)
    return a;
  RGBA result = {
    (a.red   * (range - i) + b.red   * i) / range,
    (a.green * (range - i) + b.green * i) / range,
    (a.blue  * (range - i) + b.blue  * i) / range,
    (a.alpha * (range - i) + b.alpha * i) / range
  };
  return result;
}

void DrawGradientTriangle (int x[3], int y[3], RGBA color[3]) {
  if (SDL_LockSurface(screen) == 0) {
    primitives::GradientTriangle((Uint32*)(screen->pixels), screen->w,
      x, y, color, clipping_rectangle, blend_copyRGBA, interpolateRGBA);
    SDL_UnlockSurface(screen);
  }
}

void DrawRectangle (int x, int y, int w, int h, RGBA color) {
  if (SDL_LockSurface(screen) == 0) {
    primitives::Rectangle((Uint32*)(screen->pixels), screen->w, x, y, w, h,
                          color, clipping_rectangle, blend_copyRGBA);
    SDL_UnlockSurface(screen);
  }
}

void DrawGradientRectangle (int x, int y, int w, int h, RGBA color[4]) {
  if (SDL_LockSurface(screen) == 0) {
    primitives::GradientRectangle((Uint32*)(screen->pixels), screen->w,
      x, y, w, h, color, clipping_rectangle, blend_copyRGBA, interpolateRGBA);
    SDL_UnlockSurface(screen);
  }
}
