#include "../../common/primitives.hpp"
#include "../sfont.hpp"
#include "unix_video.h"
#include "unix_time.h"
#include "unix_input.h"
#include "unix_internal.h"
#include "unix_sphere_config.h"
#include <string>

/*! \file unix_video.cpp
  This is a modified version of standard32.cpp
 */

static void FillImagePixels(IMAGE image, const RGBA* data);
static void OptimizeBlitRoutine(IMAGE image);

static void NullBlit(IMAGE image, int x, int y);
static void TileBlit(IMAGE image, int x, int y);
static void SpriteBlit(IMAGE image, int x, int y);
static void NormalBlit(IMAGE image, int x, int y);

SDL_Surface* screen;
SDL_Surface* scalen;
SFONT* FPSFont;
static bool FPSDisplayed;
static bool fullscreen;

int scaling;

const int NORMAL = 0;
const int SCALE2 = 1;
const int SCALE3 = 2;
const int SCALE1 = 3;

#if 0 // already defined in Image32
typedef struct _clipper {
  int left;
  int top;
  int right;
  int bottom;
} clipper;
#endif

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
  This font will be used to the display the frames per second counter on the screen
 */
void SetFPSFont (SFONT* font) {
  FPSFont = font;
}

/*
 \brief toggle display of frames per second
 */
void ToggleFPS () {
  FPSDisplayed = !FPSDisplayed;
}




static std::string s_window_title;
std::string GetWindowTitle() {
  return s_window_title;
}

/*
 \brief change the title of the game to text
 */
bool SetWindowTitle(const char* text) {
  // SDL forgets the title when going to/from fullscreen, so remember it
  s_window_title = text;
  SDL_WM_SetCaption(text, NULL);
  return true;
}


/*!
  \brief switch the screen resolution

  This is where all the fun begins.
  If this is the first time that SwitchResolution is called, SDL is initialized.
 */
bool SwitchResolution (int x, int y, bool fullscr, bool update_cliprect, int scalex) {
  static bool initialized = false;
  int driverflags;
  int tx, ty; // The true width and height for scaling
  SPHERECONFIG config;

  LoadSphereConfig(&config, (GetSphereDirectory() + "/engine.ini").c_str());

  // Isn't this passed as an argument yet then try reading it fuzzy from config file
  if (config.scaling == "")
     scalex = NORMAL;
  if (config.scaling == "scale2x")
     scalex = SCALE2;
  else if (config.scaling == "scale3x")
     scalex = SCALE3;
  else
     scalex = NORMAL;

  bool filtering = config.filter;
  bool showcurs  = config.showcursor;

  // Supported "drivers": sdl16 (16-bit) and sdl32 (32-bit)
  int drvdepth;
  if (config.videodriver == "sdl16")
    drvdepth = 16;
  else if (config.videodriver == "sdl32")
    drvdepth = 32;
  else
    drvdepth = 32;

  // Do some pre-initialisation
  switch (scalex)
  {
    case SCALE2:
      tx = x * 2;
      ty = y * 2;
      break;
    case SCALE3:
      tx = x * 3;
      ty = y * 3;
      break;
    default:
      break;
  }

  // Set the global variable
  scaling = scalex;

  // We 'need' a custom buffer so we can filter.
  //if(scaling == 0 && filtering)
  if (scaling == 0)
  {
    scaling = SCALE1;
    // Enable creation of custom software plane double buffering for filtering (evil)
    scalex = 1;
    tx = x;
    ty = y;
  }

  // Center the window on the display
  putenv("SDL_VIDEO_CENTERED=1");

  if (!initialized) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD) == -1)
      return false;

    // Load fullscreen preference from file.
    fullscr = fullscreen = config.fullscreen;

    // Clean up on exit, exit on window close and interrupt
    atexit(SDL_Quit);

    InitializeInput();
    initialized = true;

    SDL_InitSubSystem(SDL_INIT_JOYSTICK);

    if (SDL_WasInit(SDL_INIT_JOYSTICK)) {
      printf("Joysticks are initialized.\n");
      printf("There are %d joysticks attached.\n", SDL_NumJoysticks());
      for (int i = 0; i < SDL_NumJoysticks(); i++) {
        printf("Joystick[%d].name = %s\n", i, SDL_JoystickName(i));
      }
    }
    else {
      printf("Joysticks are not initialized.\n");
    }
  } else {
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD) == -1)
      return false;

    // keep the window title as what it was
    SetWindowTitle(GetWindowTitle().c_str());
  }

  if (fullscr)
    scalen = SDL_SetVideoMode(tx, ty, drvdepth, SDL_SWSURFACE | SDL_FULLSCREEN);
  else
    scalen = SDL_SetVideoMode(tx, ty, drvdepth, SDL_SWSURFACE);
  if (filtering)
    screen = SDL_CreateRGBSurface(SDL_HWSURFACE, x, y, 32, config.r, config.g, config.b, config.a);
  else
    screen = SDL_CreateRGBSurface(SDL_HWSURFACE, x, y, 32, 0, 0, 0, 0);
  if (scalen == NULL || screen == NULL)
    return false;

  SDL_ShowCursor(showcurs);

  ScreenWidth = x;
  ScreenHeight = y;

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  if (update_cliprect) {
    SetClippingRectangle(0, 0, screen->w, screen->h);
  }

  return true;
}

/*!
  This changes the resolution but keeps fullscreen mode in tact
 */
bool SwitchResolution (int x, int y) {
  return SwitchResolution(x, y, fullscreen, true);
}

int GetScreenWidth() {
  return ScreenWidth;
}

int GetScreenHeight() {
  return ScreenHeight;
}

void ToggleFullscreen ()
{
  if ( SwitchResolution(GetScreenWidth(), GetScreenHeight(), !fullscreen, false) ) {
    fullscreen = !fullscreen;
  }
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

  SDL_Rect rect;
  rect.x = x1; rect.y = y1; rect.w = w; rect.h = h;

  SDL_SetClipRect(screen, &rect);

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

/* \brief   Tung: Blits a surface to another surface, scaled.
 *          For FlipScreen().
 *
 * \param   src The source surface.
 * \param   dest    The destination surface.
 * \param   sc      Scale factor.
 */
void BlitScaleSurface (SDL_Surface *src, SDL_Surface *dest, int sc) {
    SDL_LockSurface(src);
    SDL_LockSurface(dest);

    // Tung: Most of this is for 32- to 16-bit conversion optimisation.
    int i, j, k, dpos, spos;
    int h, w, sw;
    Uint16 *d16;
    Uint32 *s32, *d32;
    SDL_PixelFormat *sf, *df;
    Uint32 srm, sgm, sbm, sam, drm, dgm, dbm, dam;
    Uint8 srs, sgs, sbs, sas, drs, dgs, dbs, das;
    Uint8 srl, sgl, sbl, sal, drl, dgl, dbl, dal;
    Uint32 spixel;
    Uint16 dpixel;

    s32 = (Uint32 *) src->pixels;
    sw = src->w;
    switch (dest->format->BitsPerPixel)
    {
        case 16:
            d16 = (Uint16 *) dest->pixels;
            sf = src->format; df = dest->format;
            srm = sf->Rmask; sgm = sf->Gmask; sbm = sf->Bmask; sam = sf->Amask;
            drm = df->Rmask; dgm = df->Gmask; dbm = df->Bmask; dam = df->Amask;
            srs = sf->Rshift; sgs = sf->Gshift; sbs = sf->Bshift; sas = sf->Ashift;
            drs = df->Rshift; dgs = df->Gshift; dbs = df->Bshift; das = df->Ashift;
            srl = sf->Rloss; sgl = sf->Gloss; sbl = sf->Bloss; sal = sf->Aloss;
            drl = df->Rloss; dgl = df->Gloss; dbl = df->Bloss; dal = df->Aloss;
            for (j = 0, h = dest->h, w = dest->w; j < h; ++j)
                for (i = 0; i < w; )
                {
                    // Tung: Ugh, translation from 32-bit to 16-bit
                    dpos = j * w + i;
                    dpixel = 0; // = d16[dpos];
                    spixel = s32[j / sc * sw + i / sc];

                    dpixel |= ((((spixel & srm) >> srs) << srl) >> drl) << drs;
                    dpixel |= ((((spixel & sgm) >> sgs) << sgl) >> dgl) << dgs;
                    dpixel |= ((((spixel & sbm) >> sbs) << sbl) >> dbl) << dbs;
                    dpixel |= ((((spixel & sam) >> sas) << sal) >> dal) << das;

                    for (k = 0; k < sc; ++k, ++i)
                        d16[dpos + k] = dpixel;
                }
            break;

        case 32:
            d32 = (Uint32 *) dest->pixels;
            for (j = 0, h = dest->h, w = dest->w; j < h; ++j)
            {
                for (i = 0; i < w; )
                {
                    dpos = j * w + i;
                    spos = j / sc * sw + i / sc;
                    for (k = 0; k < sc; ++k, ++i)
                        d32[dpos + k] = s32[spos];
                }
            }
            break;
    }

    SDL_UnlockSurface(dest);
    SDL_UnlockSurface(src);
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
  if (NumFlips++ % 8 == 0) {
    UpdateSystem();
  }

  if (ShouldTakeScreenshot) {
    ShouldTakeScreenshot = false;
    TakeScreenshot();
  }

  // Are we scaling?
  /*if (scaling)
  {*/
  switch (scaling)
  {
    case SCALE2:
      BlitScaleSurface(screen, scalen, 2);
      break;
    case SCALE3:
        BlitScaleSurface(screen, scalen, 3);
      break;
    case SCALE1:
    default:
      SDL_BlitSurface(screen, NULL, scalen, NULL);
       break;
  }
  SDL_Flip(scalen);
  /*}
  else
    SDL_Flip(screen);*/
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

void DestroyImage(IMAGE image) {
  delete[] image->bgra;
  delete[] image->alpha;
  delete image;
}

IMAGE GrabImage(int x, int y, int width, int height) {
  if (x < 0 || y < 0 || x + width > ScreenWidth || y + height > ScreenHeight)
    return NULL;

  IMAGE image = new _IMAGE;
  image->width        = width;
  image->height       = height;
  image->blit_routine = TileBlit;
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) != 0)
    return NULL;

   BGRA* Screen = (BGRA*)screen->pixels;
   image->bgra = new BGRA[width * height];
   for (int iy = 0; iy < height; iy++) {
     memcpy(image->bgra + iy * width, Screen + (y + iy) * ScreenWidth + x, width * 4);
   }

   image->alpha = new byte[width * height];
   memset(image->alpha, 255, width * height);

   SDL_UnlockSurface(screen);
  }
  else
  {
   BGRA* Screen = (BGRA*)screen->pixels;
   image->bgra = new BGRA[width * height];
   for (int iy = 0; iy < height; iy++) {
     memcpy(image->bgra + iy * width, Screen + (y + iy) * ScreenWidth + x, width * 4);
   }

   image->alpha = new byte[width * height];
   memset(image->alpha, 255, width * height);
  }
  return image;
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
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     primitives::Blit((BGRA*)screen->pixels, ScreenWidth, x, y,
                      image->bgra, image->alpha, image->width,
                      image->height, ClippingRectangle,
                      render_pixel_mask<BGRA>(mask));
     SDL_UnlockSurface(screen);
   }
  }
  else
  {
    primitives::Blit((BGRA*)screen->pixels, ScreenWidth, x, y,
                      image->bgra, image->alpha, image->width,
                      image->height, ClippingRectangle,
                      render_pixel_mask<BGRA>(mask));
  }
}

void aBlendBGRA(struct BGRA& d, struct BGRA s, int a) {
  // blit to the dest pixel
  d.red   = s.red   + (d.red   * (256 - a)) / 256;
  d.green = s.green + (d.green * (256 - a)) / 256;
  d.blue  = s.blue  + (d.blue  * (256 - a)) / 256;
}

void TransformBlitImage(IMAGE image, int x[4], int y[4]) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     primitives::TexturedQuad((BGRA*)screen->pixels, ScreenWidth, x, y,
                              image->bgra, image->alpha, image->width,
                              image->height, ClippingRectangle,
                              aBlendBGRA);
     SDL_UnlockSurface(screen);
   }
  }
  else
  {
     primitives::TexturedQuad((BGRA*)screen->pixels, ScreenWidth, x, y,
                              image->bgra, image->alpha, image->width,
                              image->height, ClippingRectangle,
                              aBlendBGRA);
  }
}

void TransformBlitImageMask(IMAGE image, int x[4], int y[4], RGBA mask) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     primitives::TexturedQuad((BGRA*)screen->pixels, ScreenWidth, x, y,
                              image->bgra, image->alpha, image->width,
                              image->height, ClippingRectangle,
                              render_pixel_mask<BGRA>(mask) );
     SDL_UnlockSurface(screen);
   }
  }
  else
  {
   primitives::TexturedQuad((BGRA*)screen->pixels, ScreenWidth, x, y,
                               image->bgra, image->alpha, image->width,
                               image->height, ClippingRectangle,
                               render_pixel_mask<BGRA>(mask) );
  }
}

void NullBlit(IMAGE image, int x, int y) {
}

void TileBlit(IMAGE image, int x, int y) {
  calculate_clipping_metrics(image->width, image->height);

  if(SDL_MUSTLOCK(screen))
  {
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
  else
  {
     BGRA* dest = (BGRA*)screen->pixels + (y + image_offset_y) * ScreenWidth + image_offset_x + x;
     BGRA* src = (BGRA*)image->bgra + image_offset_y * image->width + image_offset_x;

     int iy = image_blit_height;
     while (iy-- > 0) {
       memcpy(dest, src, image_blit_width * sizeof(BGRA));
       dest += ScreenWidth;
       src += image->width;
     }
  }

}

void SpriteBlit(IMAGE image, int x, int y) {
  calculate_clipping_metrics(image->width, image->height);
  if(SDL_MUSTLOCK(screen))
  {
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
  else
  {
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
  }
}

void NormalBlit(IMAGE image, int x, int y) {
  calculate_clipping_metrics(image->width, image->height);

  if(SDL_MUSTLOCK(screen))
  {
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
         byte inverse_alpha = 255 - *alpha;

         dest->red   = src->red   + (dest->red   * inverse_alpha) / 255;
         dest->green = src->green + (dest->green * inverse_alpha) / 255;
         dest->blue  = src->blue  + (dest->blue  * inverse_alpha) / 255;

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
  else
  {
     BGRA* dest = (BGRA*)screen->pixels + (y + image_offset_y) * ScreenWidth + image_offset_x + x;
     BGRA* src = (BGRA*)image->bgra + image_offset_y * image->width + image_offset_x;
     byte* alpha = image->alpha + image_offset_y * image->width + image_offset_x;

     int dest_inc = ScreenWidth - image_blit_width;
     int src_inc = image->width - image_blit_width;

     int iy = image_blit_height;
     while (iy-- > 0) {
       int ix = image_blit_width;
       while (ix-- > 0) {
         byte inverse_alpha = 255 - *alpha;

         dest->red   = src->red   + (dest->red   * inverse_alpha) / 255;
         dest->green = src->green + (dest->green * inverse_alpha) / 255;
         dest->blue  = src->blue  + (dest->blue  * inverse_alpha) / 255;

         dest++;
         src++;
         alpha++;
       }
       dest += dest_inc;
       src += src_inc;
       alpha += src_inc;
     }
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
  if (image->locked_pixels == NULL)
    return NULL;

  // rgb
  for (int i = 0; i < image->width * image->height; i++) {
    if (image->alpha[i]) {
      image->locked_pixels[i].red   = (image->bgra[i].red   * 256) / image->alpha[i];
      image->locked_pixels[i].green = (image->bgra[i].green * 256) / image->alpha[i];
      image->locked_pixels[i].blue  = (image->bgra[i].blue  * 256) / image->alpha[i];
    }
  }

  // alpha
  for (int i = 0; i < image->width * image->height; i++)
    image->locked_pixels[i].alpha = image->alpha[i];

  return image->locked_pixels;
}

void UnlockImage(IMAGE image, bool pixels_changed) {

  if (pixels_changed) {
    delete[] image->bgra;
    delete[] image->alpha;

    FillImagePixels(image, image->locked_pixels);
    OptimizeBlitRoutine(image);
  }

  delete[] image->locked_pixels;
}

void DirectBlit(int x, int y, int w, int h, RGBA* pixels) {
  calculate_clipping_metrics(w, h);
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++) {
       for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++) {
         BGRA* dest = (BGRA*)screen->pixels + (y + iy) * ScreenWidth + x + ix;
         RGBA src = pixels[iy * w + ix];

         if (src.alpha == 255) {
           dest->red = src.red;
           dest->green = src.green;
           dest->blue = src.blue;
         }
         else if (src.alpha > 0) {
           dest->red =   (dest->red   * (256 - src.alpha) + src.red   * src.alpha) / 256;
           dest->green = (dest->green * (256 - src.alpha) + src.green * src.alpha) / 256;
           dest->blue =  (dest->blue  * (256 - src.alpha) + src.blue  * src.alpha) / 256;
         }
       }
     }
     SDL_UnlockSurface(screen);
   }
  }
  else
  {
     for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++) {
       for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++) {
         BGRA* dest = (BGRA*)screen->pixels + (y + iy) * ScreenWidth + x + ix;
         RGBA src = pixels[iy * w + ix];

         if (src.alpha == 255) {
           dest->red = src.red;
           dest->green = src.green;
           dest->blue = src.blue;
         }
         else if (src.alpha > 0) {
           dest->red =   (dest->red   * (256 - src.alpha) + src.red   * src.alpha) / 256;
           dest->green = (dest->green * (256 - src.alpha) + src.green * src.alpha) / 256;
           dest->blue =  (dest->blue  * (256 - src.alpha) + src.blue  * src.alpha) / 256;
         }
       }
     }
  }
}

inline void BlendRGBAtoBGRA(BGRA& d, RGBA src, RGBA alpha) {
  Blend3(d, src, alpha.alpha);
}

void DirectTransformBlit(int x[4], int y[4], int w, int h, RGBA* pixels) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     primitives::TexturedQuad((BGRA*)screen->pixels, ScreenWidth, x, y,
                              pixels, pixels, w, h, ClippingRectangle,
                              BlendRGBAtoBGRA);
     SDL_UnlockSurface(screen);
   }
  }
  else
  {
     primitives::TexturedQuad((BGRA*)screen->pixels, ScreenWidth, x, y,
                              pixels, pixels, w, h, ClippingRectangle,
                              BlendRGBAtoBGRA);
  }
}

void DirectGrab(int x, int y, int w, int h, RGBA* pixels) {
  if (x < 0 || y < 0 || x + w > ScreenWidth || y + h > ScreenHeight)
    return;
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     BGRA* Screen = (BGRA*)screen->pixels;
     for (int iy = 0; iy < h; iy++) {
       for (int ix = 0; ix < w; ix++) {
         pixels[iy * w + ix].red   = Screen[(y + iy) * ScreenWidth + x + ix].red;
         pixels[iy * w + ix].green = Screen[(y + iy) * ScreenWidth + x + ix].green;
         pixels[iy * w + ix].blue  = Screen[(y + iy) * ScreenWidth + x + ix].blue;
         pixels[iy * w + ix].alpha = 255;
       }
     }
     SDL_UnlockSurface(screen);
   }
  }
  else
  {
     BGRA* Screen = (BGRA*)screen->pixels;
     for (int iy = 0; iy < h; iy++) {
       for (int ix = 0; ix < w; ix++) {
         pixels[iy * w + ix].red   = Screen[(y + iy) * ScreenWidth + x + ix].red;
         pixels[iy * w + ix].green = Screen[(y + iy) * ScreenWidth + x + ix].green;
         pixels[iy * w + ix].blue  = Screen[(y + iy) * ScreenWidth + x + ix].blue;
         pixels[iy * w + ix].alpha = 255;
       }
     }
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
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     primitives::Point((BGRA*)screen->pixels, ScreenWidth, x, y, color,
                       ClippingRectangle, blendBGRA);
    SDL_UnlockSurface(screen);
   }
  }
  else
  {
     primitives::Point((BGRA*)screen->pixels, ScreenWidth, x, y, color,
                       ClippingRectangle, blendBGRA);
  }
}

void DrawPointSeries(VECTOR_INT** points, int length, RGBA color) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     primitives::PointSeries((BGRA*)screen->pixels, ScreenWidth, points, length, color,
                       ClippingRectangle, blendBGRA);
    SDL_UnlockSurface(screen);
   }
  }
  else
  {
     primitives::PointSeries((BGRA*)screen->pixels, ScreenWidth, points, length, color,
                       ClippingRectangle, blendBGRA);
  }
}

void DrawLine(int x[2], int y[2], RGBA color) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     primitives::Line((BGRA*)screen->pixels, ScreenWidth, x[0], y[0],
                      x[1], y[1], constant_color(color),
               ClippingRectangle, blendBGRA);
    SDL_UnlockSurface(screen);
   }
  }
  else
  {
     primitives::Line((BGRA*)screen->pixels, ScreenWidth, x[0], y[0],
                      x[1], y[1], constant_color(color),
               ClippingRectangle, blendBGRA);
  }
}

void DrawGradientLine(int x[2], int y[2], RGBA colors[2]) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     primitives::Line((BGRA*)screen->pixels, ScreenWidth, x[0], y[0],
                      x[1], y[1], gradient_color(colors[0], colors[1]),
               ClippingRectangle, blendBGRA);
    SDL_UnlockSurface(screen);
   }
  }
  else
  {
     primitives::Line((BGRA*)screen->pixels, ScreenWidth, x[0], y[0],
                      x[1], y[1], gradient_color(colors[0], colors[1]),
               ClippingRectangle, blendBGRA);
  }
}

void DrawLineSeries(VECTOR_INT** points, int length, RGBA color, int type) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     if (color.alpha == 0) { // no mask
       return;
     } else {
       primitives::LineSeries((BGRA*)screen->pixels, ScreenWidth, points,
                           length, color, type, ClippingRectangle, blendBGRA);
     }
    SDL_UnlockSurface(screen);
   }
  }
  else
  {
     if (color.alpha == 0) { // no mask
       return;
     } else {
       primitives::LineSeries((BGRA*)screen->pixels, ScreenWidth, points,
                           length, color, type, ClippingRectangle, blendBGRA);
     }
  }
}

void DrawBezierCurve(int x[4], int y[4], double step, RGBA color, int cubic) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     primitives::BezierCurve((BGRA*)screen->pixels, ScreenWidth, x, y, step,
                          color, cubic, ClippingRectangle, blendBGRA);
    SDL_UnlockSurface(screen);
   }
  }
  else
  {
     primitives::BezierCurve((BGRA*)screen->pixels, ScreenWidth, x, y, step,
                          color, cubic, ClippingRectangle, blendBGRA);
  }
}

void DrawTriangle(int x[3], int y[3], RGBA color) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     primitives::Triangle((BGRA*)screen->pixels, ScreenWidth, x, y,
                          color, ClippingRectangle, blendBGRA);
    SDL_UnlockSurface(screen);
   }
  }
  else
  {
     primitives::Triangle((BGRA*)screen->pixels, ScreenWidth, x, y,
                          color, ClippingRectangle, blendBGRA);
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
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     primitives::GradientTriangle((BGRA*)screen->pixels, ScreenWidth,
                                  x, y, colors, ClippingRectangle,
                        blendBGRA, interpolateRGBA);
    SDL_UnlockSurface(screen);
   }
  }
  else
  {
     primitives::GradientTriangle((BGRA*)screen->pixels, ScreenWidth,
                                  x, y, colors, ClippingRectangle,
                        blendBGRA, interpolateRGBA);
  }
}

void DrawPolygon(VECTOR_INT** points, int length, int invert, RGBA color) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     if (color.alpha == 0) { // no mask
       return;
     } else {
       primitives::Polygon((BGRA*)screen->pixels, ScreenWidth, points,
                           length, invert, color, ClippingRectangle, blendBGRA);
     }
    SDL_UnlockSurface(screen);
   }
  }
  else
  {
     if (color.alpha == 0) { // no mask
       return;
     } else {
       primitives::Polygon((BGRA*)screen->pixels, ScreenWidth, points,
                           length, invert, color, ClippingRectangle, blendBGRA);
     }
  }
}

void DrawOutlinedRectangle(int x, int y, int w, int h, int size, RGBA color) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     if (color.alpha == 0) { // no mask
       return;
     } else {
       primitives::OutlinedRectangle((BGRA*)screen->pixels, ScreenWidth, x, y,
                           w, h, size, color, ClippingRectangle, blendBGRA);
     }
    SDL_UnlockSurface(screen);
   }
  }
  else
  {
     if (color.alpha == 0) { // no mask
       return;
     } else {
       primitives::OutlinedRectangle((BGRA*)screen->pixels, ScreenWidth, x, y,
                           w, h, size, color, ClippingRectangle, blendBGRA);
     }
  }
}

void DrawRectangle(int x, int y, int w, int h, RGBA color) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     if (color.alpha == 0) { // no mask
       return;
     } else if (color.alpha == 255) { // full mask

//      BGRA bgra = { color.blue, color.green, color.red };
//      primitives::Rectangle((BGRA*)screen->pixels, ScreenWidth, x, y,
//                        w, h, bgra, ClippingRectangle, copyBGRA);
       SDL_Rect rect;
       rect.x = x; rect.y = y; rect.w = w; rect.h = h;
       Uint32 sdl_color =
         SDL_MapRGB(screen->format, color.red, color.green, color.blue);
       SDL_FillRect(screen, &rect, sdl_color);

     } else {
       primitives::Rectangle((BGRA*)screen->pixels, ScreenWidth, x, y,
                           w, h, color, ClippingRectangle, blendBGRA);
     }
    SDL_UnlockSurface(screen);
   }
  }
  else
  {
     if (color.alpha == 0) { // no mask
       return;
     } else if (color.alpha == 255) { // full mask

//      BGRA bgra = { color.blue, color.green, color.red };
//      primitives::Rectangle((BGRA*)screen->pixels, ScreenWidth, x, y,
//                        w, h, bgra, ClippingRectangle, copyBGRA);
       SDL_Rect rect;
       rect.x = x; rect.y = y; rect.w = w; rect.h = h;
       Uint32 sdl_color =
         SDL_MapRGB(screen->format, color.red, color.green, color.blue);
       SDL_FillRect(screen, &rect, sdl_color);

     } else {
       primitives::Rectangle((BGRA*)screen->pixels, ScreenWidth, x, y,
                           w, h, color, ClippingRectangle, blendBGRA);
     }
  }
}

void DrawGradientRectangle(int x, int y, int w, int h, RGBA colors[4]) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     primitives::GradientRectangle((BGRA*)screen->pixels, ScreenWidth,
                                   x, y, w, h, colors, ClippingRectangle,
                        blendBGRA, interpolateRGBA);
    SDL_UnlockSurface(screen);
   }
  }
  else
  {
     primitives::GradientRectangle((BGRA*)screen->pixels, ScreenWidth,
                                   x, y, w, h, colors, ClippingRectangle,
                        blendBGRA, interpolateRGBA);
  }
}

void DrawOutlinedComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, RGBA color, int antialias) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     primitives::OutlinedComplex((BGRA*)screen->pixels, ScreenWidth,
                                   r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, color, antialias, ClippingRectangle,
                        blendBGRA);
    SDL_UnlockSurface(screen);
   }
  }
  else
  {
     primitives::OutlinedComplex((BGRA*)screen->pixels, ScreenWidth,
                                   r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, color, antialias, ClippingRectangle,
                        blendBGRA);
  }
}

void DrawFilledComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, float angle, float frac_size, int fill_empty, RGBA colors[2]) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     primitives::FilledComplex((BGRA*)screen->pixels, ScreenWidth,
                                   r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, angle, frac_size, fill_empty, colors, ClippingRectangle,
                        blendBGRA);
    SDL_UnlockSurface(screen);
   }
  }
  else
  {
     primitives::FilledComplex((BGRA*)screen->pixels, ScreenWidth,
                                   r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, angle, frac_size, fill_empty, colors, ClippingRectangle,
                        blendBGRA);
  }
}

void DrawGradientComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, float angle, float frac_size, int fill_empty, RGBA colors[3]) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     primitives::GradientComplex((BGRA*)screen->pixels, ScreenWidth,
                                   r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, angle, frac_size, fill_empty, colors, ClippingRectangle,
                        blendBGRA);
    SDL_UnlockSurface(screen);
   }
  }
  else
  {
     primitives::GradientComplex((BGRA*)screen->pixels, ScreenWidth,
                                   r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, angle, frac_size, fill_empty, colors, ClippingRectangle,
                        blendBGRA);
  }
}

void DrawOutlinedEllipse(int x, int y, int rx, int ry, RGBA color) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     if (color.alpha == 0) { // no mask
       return;
     } else {
       primitives::OutlinedEllipse((BGRA*)screen->pixels, ScreenWidth, x, y,
                           rx, ry, color, ClippingRectangle, blendBGRA);
     }
    SDL_UnlockSurface(screen);
   }
  }
  else
  {
     if (color.alpha == 0) { // no mask
       return;
     } else {
       primitives::OutlinedEllipse((BGRA*)screen->pixels, ScreenWidth, x, y,
                           rx, ry, color, ClippingRectangle, blendBGRA);
     }
  }
}

void DrawFilledEllipse(int x, int y, int rx, int ry, RGBA color) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     if (color.alpha == 0) { // no mask
       return;
     } else {
       primitives::FilledEllipse((BGRA*)screen->pixels, ScreenWidth, x, y,
                           rx, ry, color, ClippingRectangle, blendBGRA);
     }
    SDL_UnlockSurface(screen);
   }
  }
  else
  {
     if (color.alpha == 0) { // no mask
       return;
     } else {
       primitives::FilledEllipse((BGRA*)screen->pixels, ScreenWidth, x, y,
                           rx, ry, color, ClippingRectangle, blendBGRA);
     }
  }
}

void DrawOutlinedCircle(int x, int y, int r, RGBA color, int antialias) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     if (color.alpha == 0) { // no mask
       return;
     } else {
       primitives::OutlinedCircle((BGRA*)screen->pixels, ScreenWidth, x, y,
                           r, color, antialias, ClippingRectangle, blendBGRA);
     }
    SDL_UnlockSurface(screen);
   }
  }
  else
  {
     if (color.alpha == 0) { // no mask
       return;
     } else {
       primitives::OutlinedCircle((BGRA*)screen->pixels, ScreenWidth, x, y,
                           r, color, antialias, ClippingRectangle, blendBGRA);
     }
  }
}

void DrawFilledCircle(int x, int y, int r, RGBA color, int antialias) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     if (color.alpha == 0) { // no mask
       return;
     } else {
       primitives::FilledCircle((BGRA*)screen->pixels, ScreenWidth, x, y,
                           r, color, antialias, ClippingRectangle, blendBGRA);
     }
    SDL_UnlockSurface(screen);
   }
  }
  else
  {
     if (color.alpha == 0) { // no mask
       return;
     } else {
       primitives::FilledCircle((BGRA*)screen->pixels, ScreenWidth, x, y,
                           r, color, antialias, ClippingRectangle, blendBGRA);
     }
  }
}

void DrawGradientCircle(int x, int y, int r, RGBA colors[2], int antialias) {
  if(SDL_MUSTLOCK(screen))
  {
   if (SDL_LockSurface(screen) == 0) {
     if (colors[0].alpha == 0 && colors[1].alpha == 0) { // no mask
       return;
     } else {
       primitives::GradientCircle((BGRA*)screen->pixels, ScreenWidth, x, y,
                           r, colors, antialias, ClippingRectangle, blendBGRA);
     }
    SDL_UnlockSurface(screen);
   }
  }
  else
  {
     if (colors[0].alpha == 0 && colors[1].alpha == 0) { // no mask
       return;
     } else {
       primitives::GradientCircle((BGRA*)screen->pixels, ScreenWidth, x, y,
                           r, colors, antialias, ClippingRectangle, blendBGRA);
     }
  }
}
