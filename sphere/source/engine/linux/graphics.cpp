#include <stdio.h>
#include <SDL.h>
#include "graphics.h"
#include "internal.h"


static SDL_Surface* screen;
static SDL_Surface* backbuffer;


////////////////////////////////////////////////////////////////////////////////

bool InitGraphics(void)
{
  screen = SDL_SetVideoMode(320, 240, 32, SDL_ANYFORMAT);
  if (screen == NULL)
    return false;

  SDL_WM_SetCaption("Sphere", "");

  backbuffer = SDL_AllocSurface(0, 320, 240, 32,
    0x00FF0000,
    0x0000FF00,
    0x000000FF,
    0xFF000000);
  if (backbuffer == NULL)
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool CloseGraphics(void)
{
  SDL_FreeSurface(backbuffer);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool FlipScreen(void)
{
  SDL_BlitSurface(backbuffer, NULL, screen, NULL);
  SDL_UpdateRect(screen, 0, 0, 0, 0);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool SetClippingRectangle(int x, int y, int w, int h)
{
  SDL_SetClipping(backbuffer, x, y, x + w, w + h);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool ApplyColorMask(RGBA mask)
{
  if (SDL_MUSTLOCK(backbuffer) == 0)
    SDL_LockSurface(backbuffer);

  // do the color mask
  BGRA* pixels = (BGRA*)backbuffer->pixels;

  for (int iy = 0; iy < 240; iy++)
  {
    for (int ix = 0; ix < 320; ix++)
    {
      int alpha = mask.alpha;
      pixels[ix].red   = ((mask.red   * alpha) + (pixels[ix].red   * (255 - alpha))) / 256;
      pixels[ix].green = ((mask.green * alpha) + (pixels[ix].green * (255 - alpha))) / 256;
      pixels[ix].blue  = ((mask.blue  * alpha) + (pixels[ix].blue  * (255 - alpha))) / 256;
    }
  
    pixels += 320;
  }
  
  if (SDL_MUSTLOCK(backbuffer))
    SDL_UnlockSurface(backbuffer);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool CreateImage(IMAGE* image, int width, int height, RGBA* data)
{
  SDL_Surface* surface = SDL_AllocSurface(
    SDL_SRCALPHA,
    width,
    height,
    32,
    0x00FF0000,
    0x0000FF00,
    0x000000FF,
    0xFF000000);
  if (surface == NULL)
    return false;

  // lock the surface
  bool locked;
  if (SDL_MUSTLOCK(surface))
  {
    locked = true;
    SDL_LockSurface(surface);
  }
  else
    locked = false;

  BGRA* pixels = (BGRA*)surface->pixels;
  for (int i = 0; i < width * height; i++)
  {
    pixels[i].red   = data[i].red;
    pixels[i].green = data[i].green;
    pixels[i].blue  = data[i].blue;
    pixels[i].alpha = 255 - data[i].alpha;
  }

  // unlock it
  if (locked)
    SDL_UnlockSurface(surface);

  image->data = surface;
  image->width = width;
  image->height = height;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool GrabImage(IMAGE* image, int x, int y, int width, int height)
{
  // create the image
  SDL_Surface* surface = SDL_AllocSurface(
    0,
    width,
    height,
    32,
    0x00FF0000,
    0x0000FF00,
    0x000000FF,
    0xFF000000);
  
  // grab it
  SDL_Rect srcrect  = { x, y, width, height };
  SDL_Rect destrect = { 0, 0, image->width, image->height };
  SDL_BlitSurface(backbuffer, &srcrect, (SDL_Surface*)image->data, &destrect);

  image->data = surface;
  image->width = width;
  image->height = height;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DestroyImage(IMAGE* image)
{
  SDL_FreeSurface((SDL_Surface*)image->data);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool BlitImage(IMAGE* image, int x, int y)
{
  SDL_Rect srcrect = { 0, 0, image->width, image->height };
  SDL_Rect destrect = { x, y, image->width, image->height };
  return 0 == SDL_BlitSurface((SDL_Surface*)image->data, &srcrect, backbuffer, &destrect);
}

////////////////////////////////////////////////////////////////////////////////
