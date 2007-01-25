#include <SDL.h>
#include "sdl_internal.hpp"
#include "sdl_time.hpp"
#include "sdl_video.hpp"
#include "../../common/primitives.hpp"
#include "../../common/rgb.hpp"
#include "../sfont.hpp"

// windows is a wierd creature. if you use the RECT it's not gonna work.
#ifdef WIN32
  #include <windows.h>
#else
  typedef struct tagRECT {
    int top;
    int bottom;
    int left;
    int right;
  } RECT;
#endif

SDL_Surface* s_FrameBuffer;
static SFONT* FPSFont;
static bool   FPSDisplayed;

////////////////////////////////////////////////////////////////////////////////

void ToggleFPS()
{
  FPSDisplayed = !FPSDisplayed;
}

////////////////////////////////////////////////////////////////////////////////

bool SetWindowTitle(const char* text) {
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool InitVideo(int x, int y)
{
  s_FrameBuffer = SDL_SetVideoMode(x, y, 32, 0);
  return (s_FrameBuffer != NULL);
}

////////////////////////////////////////////////////////////////////////////////

bool CloseVideo()
{
  SDL_FreeSurface(s_FrameBuffer);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void SetFPSFont(SFONT* font)
{
  FPSFont = font;
}

////////////////////////////////////////////////////////////////////////////////

bool SwitchResolution(int x, int y)
{
  CloseVideo();
  return InitVideo(x, y);
}

////////////////////////////////////////////////////////////////////////////////

int GetScreenWidth()
{
  return s_FrameBuffer->clip_rect.w;
}

////////////////////////////////////////////////////////////////////////////////

int GetScreenHeight()
{
  return s_FrameBuffer->clip_rect.h;
}

////////////////////////////////////////////////////////////////////////////////

int GetDefaultWidth()
{
  return 320;
}

////////////////////////////////////////////////////////////////////////////////

int GetDefaultHeight()
{
  return 240;
}

////////////////////////////////////////////////////////////////////////////////

bool FlipScreen()
{
  static bool FirstCall = true;
  static int LastUpdate;
  static int FPS;
  static int CurrentFrames;

  // initialize variables
  if (FirstCall) {
    LastUpdate = GetTime();
    FPS = 0;
    CurrentFrames = 0;
    
    FirstCall = false;
  }

  // display FPS
  if (FPSFont && FPSDisplayed) {

    // update FPS
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
  if (NumFlips++ % 8 == 0)
    UpdateSystem();

  if (ShouldTakeScreenshot) {
    ShouldTakeScreenshot = false;
    TakeScreenshot();
  }

  SDL_Flip(s_FrameBuffer);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void ClipRectangle(RECT* rect, RECT* clip)
{
  if (rect->left   < clip->left  ) rect->left   = clip->left;
  if (rect->right  > clip->right ) rect->right  = clip->right;
  if (rect->top    < clip->top   ) rect->top    = clip->top;
  if (rect->bottom > clip->bottom) rect->bottom = clip->bottom;

  if (rect->left > rect->right ) std::swap(rect->left, rect->right);
  if (rect->top  > rect->bottom) std::swap(rect->top, rect->bottom);
}

////////////////////////////////////////////////////////////////////////////////

bool SetClippingRectangle(int x, int y, int w, int h)
{
  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;
  SDL_SetClipRect(s_FrameBuffer, &rect);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool GetClippingRectangle(int* x, int* y, int* w, int* h)
{
  *x = s_FrameBuffer->clip_rect.x;
  *y = s_FrameBuffer->clip_rect.y;
  *w = s_FrameBuffer->clip_rect.w;
  *h = s_FrameBuffer->clip_rect.h;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

IMAGE CreateImage(int width, int height, const RGBA* pixels)
{
  // This ain't right!
  /*
  SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
    (RGBA*)pixels, width, height, 32, width * 4,
    0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
  */

  SDL_Surface* surface = SDL_CreateRGBSurface(
    SDL_SWSURFACE, width, height, 32,
    0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

  // lock surface, copy and then unlock.
  int result = SDL_LockSurface(surface);
  if (result == -1)
  {
    SDL_FreeSurface(surface);
    return NULL;
  }
  memcpy(surface->pixels, pixels, width * height * 4);
  SDL_UnlockSurface(surface);

  return (IMAGE)surface;
}

////////////////////////////////////////////////////////////////////////////////

IMAGE GrabImage(int x, int y, int width, int height)
{
  SDL_Surface* surface = SDL_CreateRGBSurface(
    SDL_SWSURFACE, width, height, 32,
    0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
  SDL_Rect src_rect = { x, y, width, height };
  SDL_Rect dst_rect = { 0, 0, width, height };
  SDL_BlitSurface(s_FrameBuffer, &src_rect, surface, &dst_rect);
  return (IMAGE)surface;
}

////////////////////////////////////////////////////////////////////////////////

bool DestroyImage(IMAGE image)
{
  SDL_Surface* surface = (SDL_Surface*)image;
  SDL_FreeSurface(surface);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool BlitImage(IMAGE image, int x, int y)
{
  SDL_Surface* surface = (SDL_Surface*)image;
  SDL_Rect rect = { x, y, surface->clip_rect.w, surface->clip_rect.h };
  int result = SDL_BlitSurface(surface, NULL, s_FrameBuffer, &rect);
  return (result == 0);
}

////////////////////////////////////////////////////////////////////////////////

void BlitImageMask(IMAGE image, int x, int y, RGBA mask)
{
  // truely some silly stuff
  SDL_Surface* s = (SDL_Surface*)image;
  int total_run = s->w * s->h;
  //int total_run = s->pitch * s->h; //warning... do not try this!
  int i = 0;

  RGBA* pixels = LockImage(image);
  while (i < total_run)
  {
    Blend3(*pixels++, mask, mask.alpha);
    i++;
  }

  UnlockImage(image, true);
  BlitImage(image, x, y);
}

////////////////////////////////////////////////////////////////////////////////

int GetImageWidth(IMAGE image)
{
  SDL_Surface* surface = (SDL_Surface*)image;
  return surface->w;
}

////////////////////////////////////////////////////////////////////////////////

int GetImageHeight(IMAGE image)
{
  SDL_Surface* surface = (SDL_Surface*)image;
  return surface->h;
}

////////////////////////////////////////////////////////////////////////////////

RGBA* LockImage(IMAGE image)
{
  SDL_Surface* surface = (SDL_Surface*)image;
  int result = SDL_LockSurface(surface);
  if (result == -1) {
    return NULL;
  }
  return (RGBA*)surface->pixels;
}

////////////////////////////////////////////////////////////////////////////////

void UnlockImage(IMAGE image, bool pixels_changed)
{
  SDL_Surface* surface = (SDL_Surface*)image;
  SDL_UnlockSurface(surface);
}

////////////////////////////////////////////////////////////////////////////////

// helper functions needed for the next functions
class constant_color
{
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

inline void copyRGBA(RGBA& dest, RGBA src)
{
  dest = src;
}

inline void blendBGRA(BGRA& dest, RGBA source) {
  Blend3(dest, source, source.alpha);
}

inline void ablendBGRA(BGRA& dest, RGBA source, RGBA alpha) {
  Blend3(dest, source, alpha.alpha);
}

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

////////////////////////////////////////////////////////////////////////////////

void  TransformBlitImage(IMAGE image, int x[4], int y[4])
{
  SDL_Surface* surface = (SDL_Surface*)image;
  RGBA* src = (RGBA*)surface->pixels;
  int result = SDL_LockSurface(s_FrameBuffer);
  if (result == -1)
    return;
  int c_x, c_y, c_w, c_h;
  GetClippingRectangle(&c_x, &c_y, &c_w, &c_h);
  RECT c_rect = { c_x, c_y, c_x + c_w - 1, c_y + c_h - 1 };

  primitives::TexturedQuad((BGRA*)s_FrameBuffer->pixels,
    s_FrameBuffer->w,
    x, y,
    src,
    src,
    surface->w,
    surface->h,
    c_rect,
    ablendBGRA);

  SDL_UnlockSurface(s_FrameBuffer);
}

////////////////////////////////////////////////////////////////////////////////

void TransformBlitImageMask(IMAGE image, int x[4], int y[4], RGBA mask) {

}

////////////////////////////////////////////////////////////////////////////////

void DirectBlit(int x, int y, int w, int h, RGBA* pixels)
{
  
  int result = SDL_LockSurface(s_FrameBuffer);
  if (result == -1)
    return;
  int c_x, c_y, c_w, c_h;
  GetClippingRectangle(&c_x, &c_y, &c_w, &c_h);
  RECT c_rect = { c_x, c_y, c_x + c_w - 1, c_y + c_h - 1 };
  RECT rect = { x, y, x + w - 1, y + h - 1 };
  ClipRectangle(&rect, &c_rect);

  int count = 0;
  BGRA* dest = (BGRA*)s_FrameBuffer->pixels;
  for (int j=0; j<h; j++)
    for (int i=0; i<w; i++)
    {
      int tx, ty;
      tx = i + rect.left;
      ty = j + rect.top;
      
      if (ty <= rect.bottom)
        if (tx <= rect.right)
          //dest[ty * s_FrameBuffer->w + tx] = pixels[j * h + i];
          Blend3(dest[ty * s_FrameBuffer->w + tx], pixels[j * h + i], pixels[j * h + i].alpha);
    }

  SDL_UnlockSurface(s_FrameBuffer);
  
}

////////////////////////////////////////////////////////////////////////////////

void DirectTransformBlit(int x[4], int y[4], int w, int h, RGBA* pixels)
{
  int result = SDL_LockSurface(s_FrameBuffer);
  if (result == -1)
    return;
  int c_x, c_y, c_w, c_h;
  GetClippingRectangle(&c_x, &c_y, &c_w, &c_h);
  RECT c_rect = { c_x, c_y, c_x + c_w - 1, c_y + c_h - 1 };

  primitives::TexturedQuad((BGRA*)s_FrameBuffer->pixels,
    s_FrameBuffer->w,
    x, y,
    pixels,
    pixels,
    w,
    h,
    c_rect,
    ablendBGRA);

  SDL_UnlockSurface(s_FrameBuffer);
}

////////////////////////////////////////////////////////////////////////////////

void DirectGrab(int x, int y, int w, int h, RGBA* pixels)
{
  // ?
  SDL_Surface* surface = SDL_CreateRGBSurface(
  SDL_SWSURFACE, w, h, 32,
  0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
  SDL_Rect src_rect = { x, y, w, h };
  SDL_Rect dst_rect = { 0, 0, w, h };
  SDL_BlitSurface(s_FrameBuffer, &src_rect, surface, &dst_rect);
  memcpy(pixels, surface->pixels, w * h * 4);
  SDL_FreeSurface(surface);
}

////////////////////////////////////////////////////////////////////////////////

void DrawPoint(int x, int y, RGBA color)
{
  int result = SDL_LockSurface(s_FrameBuffer);
  if (result == -1)
    return;
  int c_x, c_y, c_w, c_h;
  GetClippingRectangle(&c_x, &c_y, &c_w, &c_h);
  RECT rect = { c_x, c_y, c_x + c_w - 1, c_y + c_h - 1 };

  primitives::Point((BGRA*)s_FrameBuffer->pixels,
    s_FrameBuffer->w,
    x, y,
    color, 
    rect,
    blendBGRA);

  SDL_UnlockSurface(s_FrameBuffer);
}

////////////////////////////////////////////////////////////////////////////////

void DrawLine(int x[2], int y[2], RGBA color)
{
  int result = SDL_LockSurface(s_FrameBuffer);
  if (result == -1)
    return;

  int c_x, c_y, c_w, c_h;
  GetClippingRectangle(&c_x, &c_y, &c_w, &c_h);
  RECT rect = { c_x, c_y, c_x + c_w - 1, c_y + c_h - 1 };

  primitives::Line((BGRA*)s_FrameBuffer->pixels,
    s_FrameBuffer->w,
    x[0], y[0],
    x[1], y[1],
    constant_color(color),
    rect,
    blendBGRA);

  SDL_UnlockSurface(s_FrameBuffer);  
}

////////////////////////////////////////////////////////////////////////////////

void DrawGradientLine(int x[2], int y[2], RGBA color[2])
{
  int result = SDL_LockSurface(s_FrameBuffer);
  if (result == -1)
    return;
  int c_x, c_y, c_w, c_h;
  GetClippingRectangle(&c_x, &c_y, &c_w, &c_h);
  RECT rect = { c_x, c_y, c_x + c_w - 1, c_y + c_h - 1 };

  primitives::Line((BGRA*)s_FrameBuffer->pixels,
    s_FrameBuffer->w,
    x[0], y[0],
    x[1], y[1],
    gradient_color(color[0], color[1]), 
    rect,
    blendBGRA);

  SDL_UnlockSurface(s_FrameBuffer);
}

////////////////////////////////////////////////////////////////////////////////

void DrawTriangle(int x[3], int y[3], RGBA color)
{
  int result = SDL_LockSurface(s_FrameBuffer);
  if (result == -1)
    return;
  int c_x, c_y, c_w, c_h;
  GetClippingRectangle(&c_x, &c_y, &c_w, &c_h);
  RECT rect = { c_x, c_y, c_x + c_w - 1, c_y + c_h - 1 };

  primitives::Triangle((BGRA*)s_FrameBuffer->pixels,
    s_FrameBuffer->w,
    x, y,
    color, 
    rect,
    blendBGRA);

  SDL_UnlockSurface(s_FrameBuffer);
}

////////////////////////////////////////////////////////////////////////////////

void DrawGradientTriangle(int x[3], int y[3], RGBA color[3])
{
  int result = SDL_LockSurface(s_FrameBuffer);
  if (result == -1)
    return;
  int c_x, c_y, c_w, c_h;
  GetClippingRectangle(&c_x, &c_y, &c_w, &c_h);
  RECT rect = { c_x, c_y, c_x + c_w - 1, c_y + c_h - 1 };

  primitives::GradientTriangle((BGRA*)s_FrameBuffer->pixels,
    s_FrameBuffer->w,
    x, y,
    color, 
    rect,
    blendBGRA,
    interpolateRGBA);

  SDL_UnlockSurface(s_FrameBuffer);
}

////////////////////////////////////////////////////////////////////////////////

void DrawRectangle(int x, int y, int w, int h, RGBA color)
{
  int result = SDL_LockSurface(s_FrameBuffer);
  if (result == -1)
    return;
  int c_x, c_y, c_w, c_h;
  GetClippingRectangle(&c_x, &c_y, &c_w, &c_h);
  RECT rect = { c_x, c_y, c_x + c_w - 1, c_y + c_h - 1 };

  primitives::Rectangle((BGRA*)s_FrameBuffer->pixels,
    s_FrameBuffer->w,
    x, y, w, h,
    color, 
    rect,
    blendBGRA);

  SDL_UnlockSurface(s_FrameBuffer);
}

////////////////////////////////////////////////////////////////////////////////

void DrawGradientRectangle(int x, int y, int w, int h, RGBA color[4])
{
  int result = SDL_LockSurface(s_FrameBuffer);
  if (result == -1)
    return;
  int c_x, c_y, c_w, c_h;
  GetClippingRectangle(&c_x, &c_y, &c_w, &c_h);
  RECT rect = { c_x, c_y, c_x + c_w - 1, c_y + c_h - 1 };

  primitives::GradientRectangle((BGRA*)s_FrameBuffer->pixels,
    s_FrameBuffer->w,
    x, y, w, h,
    color, 
    rect,
    blendBGRA,
    interpolateRGBA);

  SDL_UnlockSurface(s_FrameBuffer);
}

////////////////////////////////////////////////////////////////////////////////

