#include "unix_video.h"
#include "SDL.h"
#include <cstring>

SFONT* FPSFont;
bool FPSDisplayed;
SDL_Surface* screen;
SDL_Surface* double_buffer; /* double trouble */


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
    if (SDL_Init(SDL_INIT_VIDEO) == -1)
      return false;
    screen = SDL_SetVideoMode(x, y, 32, SDL_FULLSCREEN | SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (screen == NULL)
      return false;
    FPSDisplayed = false;
    return true;
  } else {
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
      return false;
    screen = SDL_SetVideoMode(x, y, 32, SDL_FULLSCREEN | SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (screen == NULL)
      return false;
    FPSDisplayed = false;
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
}

void SetClippingRectangle (int x, int y, int w, int h) {
  SDL_Rect rect;

  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;
  SDL_SetClipRect(screen, &rect);
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
  SDL_FreeSurface(surface);
}

void UtilityBlit (IMAGE image, int x, int y, RGBA mask) {
}

void BlitImage (IMAGE image, int x, int y) {
  SDL_Rect dest;

  dest.x = x;
  dest.y = y;
  SDL_BlitSurface(image, NULL, screen, &dest);
}

void BlitImageMask (IMAGE image, int x, int y, RGBA mask) {
  int lcv;
  int firstscan;
  int lastscan;
  SDL_Rect clip;
  Uint32* pixel;

  if (SDL_LockSurface(screen) == 0) {
    clip = SDL_GetClipRect(screen, &clip);
    firstscan = x > clip.x ? x : clip.x;
    lastscan = (image->h + y) < (clip.y + clip.h) ? (image->h + y) : (clip.y + clip.h);
    for (lcv = firstscan; lcv <= lastscan; lcv++) { 
      
    }
  }
}

void TransformBlitImage (IMAGE image, int x[4], int y[4]) {
}

void TransformBlitImageMask (IMAGE image, int x[4], int y[4], RGBA mask) {
}

int GetImageWidth (IMAGE image) {
}

int GetImageHeight (IMAGE image) {
}

RGBA* LockImage (IMAGE image) {
}

void UnlockImage (IMAGE image) {
}

void DirectBlit (int x, int y, int w, int h, RGBA* pixels) {
}

void DirectTransformBlit (int x[4], int y[4], int w, int h, RGBA* pixels) {
}

void DirectGrab (int x, int y, int w, int h, RGBA* pixels) {
}

void DrawPoint (int x, int y, RGBA color) {
}

void DrawLine (int x[2], int y[2], RGBA color) {
}

void DrawGradientLine (int x[2], int y[2], RGBA color[2]) {
}

void DrawTriangle (int x[3], int y[3], RGBA color) {
}

void DrawGradientTriangle (int x[3], int y[3], RGBA color[3]) {
}

void DrawRectangle (int x, int y, int w, int h, RGBA color) {
}

void DrawGradientRectangle (int x, int y, int w, int h, RGBA color[4]) {
}
