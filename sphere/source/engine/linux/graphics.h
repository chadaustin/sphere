#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__


#include "types.h"
#include "colortypes.h"


typedef struct
{
  int width;
  int height;
  void* data;
} IMAGE;


#ifdef __cplusplus
extern "C" {
#endif


extern bool FlipScreen(void);
extern bool SetClippingRectangle(int x, int y, int w, int h);
extern bool CreateImage(IMAGE* image, int width, int height, RGBA* data);
extern bool GrabImage(IMAGE* image, int x, int y, int width, int height);
extern bool DestroyImage(IMAGE* image);
extern bool BlitImage(const IMAGE* image, int x, int y);


#ifdef __cplusplus
}
#endif


#endif
