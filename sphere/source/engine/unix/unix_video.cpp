#include "../../common/primitives.hpp"
#include "../sfont.hpp"
#include "unix_video.h"
#include "unix_time.h"
#include "unix_input.h"

/*! \file unix_video.cpp

  This is a modified version of standard32.cpp */

static void FillImagePixels(IMAGE image, const RGBA* data);
static void OptimizeBlitRoutine(IMAGE image);

static void NullBlit(IMAGE image, int x, int y);
static void TileBlit(IMAGE image, int x, int y);
static void SpriteBlit(IMAGE image, int x, int y);
static void NormalBlit(IMAGE image, int x, int y);

SDL_Surface* screen;
SFONT* FPSFont;
static bool FPSDisplayed;

typedef struct _clipper {
  int left, right, bottom, top;
} clipper;

static clipper ClippingRectangle;

static int ScreenWidth;
static int ScreenHeight;

#define calculate_clipping_metrics(width, height)   /* EVIL! */      \
  int image_offset_x = 0;                                            \
  int image_offset_y = 0;                                            \
  int image_blit_width = width;                                      \
  int image_blit_height = height;                                    \
                                                                     \
  if (x < ClippingRectangle.left) {                                  \
    image_offset_x = (ClippingRectangle.left - x);                   \
    image_blit_width -= image_offset_x;                              \
  }                                                                  \
  if (y < ClippingRectangle.top) {                                   \
    image_offset_y = (ClippingRectangle.top - y);                    \
    image_blit_height -= image_offset_y;                             \
  }                                                                  \
  if (x + (int)width - 1 > ClippingRectangle.right)                  \
    image_blit_width -= (x + width - ClippingRectangle.right - 1);   \
  if (y + (int)height - 1 > ClippingRectangle.bottom)                \
    image_blit_height -= (y + height - ClippingRectangle.bottom - 1)

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
bool SwitchResolution (int x, int y, bool fullscreen) {
  static bool initialized = false;

  if (!initialized) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD) == -1)
      return false;
	 SDL_WM_SetCaption("sphere 0.93a", NULL);
  } else {
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD) == -1)
      return false;
	 /* FIXME: the title of the game or something should be set here instead */
	 SDL_WM_SetCaption("sphere 0.93a", NULL);
  }
  if (fullscreen)
	 screen = SDL_SetVideoMode(x, y, 32, SDL_DOUBLEBUF | SDL_FULLSCREEN);
  else
    screen = SDL_SetVideoMode(x, y, 32, SDL_DOUBLEBUF);
  if (screen == NULL)
    return false;
  /* screen = SDL_CreateRGBSurface(0, real_screen->w, real_screen->h, 32, 0, 0, 0, 0);
  SDL_ConvertSurface(screen, real_screen->format, 0); */
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  FPSDisplayed = false;
  initialized = true;
  ScreenWidth = screen->w;
  ScreenHeight = screen->h;
  SetClippingRectangle(0, 0, screen->w, screen->h);
  return true;
}

int GetScreenWidth() {
  return ScreenWidth;
}

int GetScreenHeight() {
  return ScreenHeight;
}

void ToggleFullscreen () {
  static bool fullscreen = false;

  SwitchResolution(GetScreenWidth(), GetScreenHeight(), fullscreen ? false : true);
  fullscreen = !fullscreen;
}

void FillImagePixels(IMAGE image, const RGBA* pixels) {
  // fill the image pixels
  image->bgra = new BGRA[image->width * image->height];
  for (int i = 0; i < image->width * image->height; i++) {
    image->bgra[i].red   = (pixels[i].red   * pixels[i].alpha) / 256;
    image->bgra[i].green = (pixels[i].green * pixels[i].alpha) / 256;
    image->bgra[i].blue  = (pixels[i].blue  * pixels[i].alpha) / 256;
  }
  // fill the alpha array
  image->alpha = new byte[image->width * image->height];
  for (int i = 0; i < image->width * image->height; i++)
    image->alpha[i] = pixels[i].alpha;
}

void OptimizeBlitRoutine(IMAGE image) {
  // null blit
  bool is_empty = true;
  for (int i = 0; i < image->width * image->height; i++)
    if (image->alpha[i] != 0) {
      is_empty = false;
      break;
    }
  if (is_empty) {
    image->blit_routine = NullBlit;
    return;
  }
  // tile blit
  bool is_tile = true;
  for (int i = 0; i < image->width * image->height; i++)
    if (image->alpha[i] != 255) {
      is_tile = false;
      break;
    }
  if (is_tile) {
    image->blit_routine = TileBlit;
    return;
  }
  // sprite blit
  bool is_sprite = true;
  for (int i = 0; i < image->width * image->height; i++)
    if (image->alpha[i] != 0 && image->alpha[i] != 255) {
      is_sprite = false;
      break;
    }
  if (is_sprite) {
    image->blit_routine = SpriteBlit;
    return;
  }
  // normal blit
  image->blit_routine = NormalBlit;
}

void SetClippingRectangle(int x, int y, int w, int h) {
  int x1 = x;
  int y1 = y;
  int x2 = x + w - 1;
  int y2 = y + h - 1;

  // validate x values
  if (x1 < 0)
	 x1 = 0;
  else if (x1 > ScreenWidth - 1)
	 x1 = ScreenWidth - 1;

  if (x2 < 0)
	 x2 = 0;
  else if (x2 > ScreenWidth - 1)
	 x2 = ScreenWidth - 1;

  // validate y values
  if (y1 < 0)
	 y1 = 0;
  else if (y1 > ScreenHeight - 1)
	 y1 = ScreenHeight - 1;

  if (y2 < 0)
	 y2 = 0;
  else if (y2 > ScreenHeight - 1)
	 y2 = ScreenHeight - 1;

  ClippingRectangle.left = x1;
  ClippingRectangle.right = x2;
  ClippingRectangle.top = y1;
  ClippingRectangle.bottom = y2;
}

void GetClippingRectangle(int* x, int* y, int* w, int* h) {
  *x = ClippingRectangle.left;
  *y = ClippingRectangle.top;
  *w = ClippingRectangle.right - ClippingRectangle.left + 1;
  *h = ClippingRectangle.bottom - ClippingRectangle.top + 1;
}

void FlipScreen () {
  static bool initialized = false;
  static int LastUpdate;
  static int FPS;
  static int CurrentFrames;

  if (!initialized) {
	 LastUpdate = GetTime();
	 FPS = 0;
	 CurrentFrames = 0;
	 initialized = true;
  }
  if (FPSFont && FPSDisplayed) {
    if (GetTime() > LastUpdate + 1000) {
      FPS = CurrentFrames;
      CurrentFrames = 0;
      LastUpdate = GetTime();
    }
    char fps[80];
    sprintf(fps, "FPS: %d", FPS);
    FPSFont->DrawString(0, 0, fps, CreateRGBA(255, 255, 255, 255));
  }
  CurrentFrames++;
  static int NumFlips;
  if (NumFlips++ % 8 == 0);
    RefreshInput();
  SDL_Flip(screen);
  /* SDL_Rect dest = {0, 0, 0, 0};
  SDL_BlitSurface(screen, NULL, real_screen, &dest);
  SDL_UpdateRect(real_screen, 0, 0, ScreenWidth, ScreenHeight); */
}

IMAGE CreateImage(int width, int height, const RGBA* pixels) {
  // allocate the image
  IMAGE image = new _IMAGE;
  image->width  = width;
  image->height = height;

  FillImagePixels(image, pixels);
  OptimizeBlitRoutine(image);
  return image;
}

IMAGE GrabImage(int x, int y, int width, int height) {
  if (x < 0 || y < 0 || x + width > ScreenWidth || y + height > ScreenHeight)
    return NULL;

  IMAGE image = new _IMAGE;
  image->width        = width;
  image->height       = height;
  image->blit_routine = TileBlit;

  if (SDL_LockSurface(screen) != 0)
	 return NULL;
  BGRA* Screen = (BGRA*)screen->pixels;
  image->bgra = new BGRA[width * height];
  for (int iy = 0; iy < height; iy++)
    memcpy(image->bgra + iy * width, Screen + (y + iy) * ScreenWidth + x,
           width * 4);
  image->alpha = new byte[width * height];
  memset(image->alpha, 255, width * height);
  SDL_UnlockSurface(screen);
  return image;
}

void DestroyImage(IMAGE image) {
  delete[] image->bgra;
  delete[] image->alpha;
  delete image;
}

void BlitImage(IMAGE image, int x, int y) {
  // don't draw it if it's off the screen
  if (x + (int)image->width < ClippingRectangle.left ||
      y + (int)image->height < ClippingRectangle.top ||
      x > ClippingRectangle.right ||
      y > ClippingRectangle.bottom)
    return;
  image->blit_routine(image, x, y);
}

template<typename pixelT>
class render_pixel_mask {
 public:
  render_pixel_mask(RGBA mask) : m_mask(mask) { }
  void operator()(pixelT& dst, pixelT src, byte alpha) {
    // do the masking on the source pixel
    alpha     = (int)alpha     * m_mask.alpha / 256;
    src.red   = (int)src.red   * m_mask.red   / 256;
    src.green = (int)src.green * m_mask.green / 256;
    src.blue  = (int)src.blue  * m_mask.blue  / 256;
    // blit to the dest pixel
    dst.red   = (dst.red   * (256 - alpha) + src.red   * alpha) / 256;
    dst.green = (dst.green * (256 - alpha) + src.green * alpha) / 256;
    dst.blue  = (dst.blue  * (256 - alpha) + src.blue  * alpha) / 256;
  }
 private:
  RGBA m_mask;
};

void BlitImageMask(IMAGE image, int x, int y, RGBA mask) {
  if (SDL_LockSurface(screen) == 0) {
    primitives::Blit((BGRA*)screen->pixels, ScreenWidth, x, y,
                     image->bgra, image->alpha, image->width,
                     image->height, ClippingRectangle,
                     render_pixel_mask<BGRA>(mask));
    SDL_UnlockSurface(screen);
  }
}

void aBlendBGRA(struct BGRA& d, struct BGRA s, int a) {
  // blit to the dest pixel
  d.red   = (d.red   * (256 - a)) / 256 + s.red;
  d.green = (d.green * (256 - a)) / 256 + s.green;
  d.blue  = (d.blue  * (256 - a)) / 256 + s.blue;
}

void TransformBlitImage(IMAGE image, int x[4], int y[4]) {
  if (SDL_LockSurface(screen) == 0) {
    primitives::TexturedQuad((BGRA*)screen->pixels, ScreenWidth, x, y,
                             image->bgra, image->alpha, image->width,
                             image->height, ClippingRectangle,
                             aBlendBGRA);
    SDL_UnlockSurface(screen);
  }
}

void TransformBlitImageMask(IMAGE image, int x[4], int y[4], RGBA mask) {
  if (SDL_LockSurface(screen) == 0) {
    primitives::TexturedQuad((BGRA*)screen->pixels, ScreenWidth, x, y,
                             image->bgra, image->alpha, image->width,
                             image->height, ClippingRectangle,
                             render_pixel_mask<BGRA>(mask) );
    SDL_UnlockSurface(screen);
  }
}

void NullBlit(IMAGE image, int x, int y) {
}

void TileBlit(IMAGE image, int x, int y) {
  calculate_clipping_metrics(image->width, image->height);

  if (SDL_LockSurface(screen) == 0) {
    BGRA* dest = (BGRA*)screen->pixels + (y + image_offset_y) * ScreenWidth + image_offset_x + x;
    BGRA* src = (BGRA*)image->bgra + image_offset_y * image->width + image_offset_x;

    int iy = image_blit_height;
    while (iy-- > 0) {
      memcpy(dest, src, image_blit_width * sizeof(BGRA));
      dest += ScreenWidth;
      src += image->width;
    }
    SDL_UnlockSurface(screen);
  }
}

void SpriteBlit(IMAGE image, int x, int y) {
  calculate_clipping_metrics(image->width, image->height);

  if (SDL_LockSurface(screen) == 0) {
    BGRA* dest = (BGRA*)screen->pixels + (y + image_offset_y) * ScreenWidth + image_offset_x + x;
    BGRA* src = (BGRA*)image->bgra + image_offset_y * image->width + image_offset_x;
    byte* alpha = image->alpha + image_offset_y * image->width + image_offset_x;

    int dest_inc = ScreenWidth - image_blit_width;
    int src_inc = image->width - image_blit_width;

    int iy = image_blit_height;
    while (iy-- > 0) {
      int ix = image_blit_width;
      while (ix-- > 0) {
        if (*alpha) {
          *dest = *src;
        }
        dest++;
        src++;
        alpha++;
      }
      dest += dest_inc;
      src += src_inc;
      alpha += src_inc;
    }
	 SDL_UnlockSurface(screen);
  }
}

void NormalBlit(IMAGE image, int x, int y) {
  calculate_clipping_metrics(image->width, image->height);

  if (SDL_LockSurface(screen) == 0) {
    BGRA* dest = (BGRA*)screen->pixels + (y + image_offset_y) * ScreenWidth + image_offset_x + x;
    BGRA* src = (BGRA*)image->bgra + image_offset_y * image->width + image_offset_x;
    byte* alpha = image->alpha + image_offset_y * image->width + image_offset_x;

    int dest_inc = ScreenWidth - image_blit_width;
    int src_inc = image->width - image_blit_width;

    int iy = image_blit_height;
    while (iy-- > 0) {
      int ix = image_blit_width;
      while (ix-- > 0) {
        word a = *alpha;
        word b = 256 - a;

        dest->red = (dest->red * b) / 256 + src->red;
        dest->green = (dest->green * b) / 256 + src->green;
        dest->blue = (dest->blue * b) / 256 + src->blue;

        dest++;
        src++;
        alpha++;
      }
      dest += dest_inc;
      src += src_inc;
      alpha += src_inc;
    }
	 SDL_UnlockSurface(screen);
  }
}

int GetImageWidth(IMAGE image) {
  return image->width;
}

int GetImageHeight(IMAGE image) {
  return image->height;
}

RGBA* LockImage(IMAGE image) {
  image->locked_pixels = new RGBA[image->width * image->height];
  // rgb
  for (int i = 0; i < image->width * image->height; i++) {
    image->locked_pixels[i].red   = (image->bgra[i].red   * 256) / image->alpha[i];
    image->locked_pixels[i].green = (image->bgra[i].green * 256) / image->alpha[i];
    image->locked_pixels[i].blue  = (image->bgra[i].blue  * 256) / image->alpha[i];
  }
  // alpha
  for (int i = 0; i < image->width * image->height; i++)
    image->locked_pixels[i].alpha = image->alpha[i];
  return image->locked_pixels;
}

void UnlockImage(IMAGE image) {
  delete[] image->bgra;
  delete[] image->alpha;
  
  FillImagePixels(image, image->locked_pixels);
  OptimizeBlitRoutine(image);
  delete[] image->locked_pixels;
}

void DirectBlit(int x, int y, int w, int h, RGBA* pixels) {
  calculate_clipping_metrics(w, h);

  if (SDL_LockSurface(screen) == 0) {
    for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
      for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++) {
        BGRA* dest = (BGRA*)screen->pixels + (y + iy) * ScreenWidth + x + ix;
        RGBA src = pixels[iy * w + ix];

        if (src.alpha == 255) {
          dest->red = src.red;
          dest->green = src.green;
          dest->blue = src.blue;
        }
        else if (src.alpha > 0) {
          dest->red = (dest->red * (256 - src.alpha) + src.red * src.alpha) / 256;
          dest->green = (dest->green * (256 - src.alpha) + src.green * src.alpha) / 256;
          dest->blue = (dest->blue * (256 - src.alpha) + src.blue * src.alpha) / 256;
        }
      }
	 SDL_UnlockSurface(screen);
  }
}

inline void BlendRGBAtoBGRA(BGRA& d, RGBA src, RGBA alpha) {
  Blend3(d, src, alpha.alpha);
}

void DirectTransformBlit(int x[4], int y[4], int w, int h, RGBA* pixels) {
  if (SDL_LockSurface(screen) == 0) {
    primitives::TexturedQuad((BGRA*)screen->pixels, ScreenWidth, x, y,
                             pixels, pixels, w, h, ClippingRectangle,
                             BlendRGBAtoBGRA);
    SDL_UnlockSurface(screen);
  }
}

void DirectGrab(int x, int y, int w, int h, RGBA* pixels) {
  if (x < 0 || y < 0 || x + w > ScreenWidth || y + h > ScreenHeight)
    return;
  if (SDL_LockSurface(screen) == 0) {
    BGRA* Screen = (BGRA*)screen->pixels;
    for (int iy = 0; iy < h; iy++)
      for (int ix = 0; ix < w; ix++) {
        pixels[iy * w + ix].red   = Screen[(y + iy) * ScreenWidth + x + ix].red;
        pixels[iy * w + ix].green = Screen[(y + iy) * ScreenWidth + x + ix].green;
        pixels[iy * w + ix].blue  = Screen[(y + iy) * ScreenWidth + x + ix].blue;
        pixels[iy * w + ix].alpha = 255;
    }
	 SDL_UnlockSurface(screen);
  }
}

class constant_color {
 public:
  constant_color(RGBA color)
    : m_color(color) {
  }
  RGBA operator()(int i, int range) {
    return m_color;
  }
 private:
  RGBA m_color;
};

class gradient_color {
 public:
  gradient_color(RGBA color1, RGBA color2)
    : m_color1(color1), m_color2(color2) {
  }
  RGBA operator()(int i, int range) {
    if (range == 0) {
      return m_color1;
    }
    RGBA color;
    color.red   = (i * m_color2.red   + (range - i) * m_color1.red)   / range;
    color.green = (i * m_color2.green + (range - i) * m_color1.green) / range;
    color.blue  = (i * m_color2.blue  + (range - i) * m_color1.blue)  / range;
    color.alpha = (i * m_color2.alpha + (range - i) * m_color1.alpha) / range;
    return color;
  }
 private:
  RGBA m_color1;
  RGBA m_color2;
};

inline void copyBGRA(BGRA& dest, BGRA source) {
  dest = source;
}

inline void blendBGRA(BGRA& dest, RGBA source) {
  Blend3(dest, source, source.alpha);
}

void DrawPoint(int x, int y, RGBA color) {
  if (SDL_LockSurface(screen) == 0) {
    primitives::Point((BGRA*)screen->pixels, ScreenWidth, x, y, color,
                      ClippingRectangle, blendBGRA);
	 SDL_UnlockSurface(screen);
  }
}

void DrawLine(int x[2], int y[2], RGBA color) {
  if (SDL_LockSurface(screen) == 0) {
    primitives::Line((BGRA*)screen->pixels, ScreenWidth, x[0], y[0],
                     x[1], y[1], constant_color(color),
							ClippingRectangle, blendBGRA);
	 SDL_UnlockSurface(screen);
  }
}

void DrawGradientLine(int x[2], int y[2], RGBA colors[2]) {
  if (SDL_LockSurface(screen) == 0) {
    primitives::Line((BGRA*)screen->pixels, ScreenWidth, x[0], y[0],
                     x[1], y[1], gradient_color(colors[0], colors[1]),
							ClippingRectangle, blendBGRA);
	 SDL_UnlockSurface(screen);
  }
}

void DrawTriangle(int x[3], int y[3], RGBA color) {
  if (SDL_LockSurface(screen) == 0) {
    primitives::Triangle((BGRA*)screen->pixels, ScreenWidth, x, y,
                         color, ClippingRectangle, blendBGRA);
	 SDL_UnlockSurface(screen);
  }
}

inline RGBA interpolateRGBA(RGBA a, RGBA b, int i, int range) {
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

void DrawGradientTriangle(int x[3], int y[3], RGBA colors[3]) {
  if (SDL_LockSurface(screen) == 0) {
    primitives::GradientTriangle((BGRA*)screen->pixels, ScreenWidth,
                                 x, y, colors, ClippingRectangle,
										   blendBGRA, interpolateRGBA);
	 SDL_UnlockSurface(screen);
  }
}

void DrawRectangle(int x, int y, int w, int h, RGBA color) {
  if (SDL_LockSurface(screen) == 0) {
    if (color.alpha == 0) { // no mask
      return;
    } else if (color.alpha == 255) { // full mask
      BGRA bgra = { color.blue, color.green, color.red };
      primitives::Rectangle((BGRA*)screen->pixels, ScreenWidth, x, y,
		                      w, h, bgra, ClippingRectangle, copyBGRA);
    } else {
      primitives::Rectangle((BGRA*)screen->pixels, ScreenWidth, x, y,
		                      w, h, color, ClippingRectangle, blendBGRA);
    }
	 SDL_UnlockSurface(screen);
  }
}

void DrawGradientRectangle(int x, int y, int w, int h, RGBA colors[4]) {
  if (SDL_LockSurface(screen) == 0) {
    primitives::GradientRectangle((BGRA*)screen->pixels, ScreenWidth,
                                  x, y, w, h, colors, ClippingRectangle,
											 blendBGRA, interpolateRGBA);
	 SDL_UnlockSurface(screen);
  }
}
