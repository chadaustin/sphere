#ifndef UNIX_VIDEO_H
#define UNIX_VIDEO_H

#include "../../common/rgb.hpp"
#include <SDL.h>

class SFONT; /* for drawing FPS */
typedef struct _IMAGE {
  int width;
  int height;
  BGRA* bgra;
  byte* alpha;
  void (*blit_routine)(_IMAGE* image, int x, int y);
  RGBA* locked_pixels;
}* IMAGE;

void SetFPSFont(SFONT* font);
void ToggleFPS();

bool SwitchResolution(int x, int y, bool fullscreen = false);
void ToggleFullscreen();
int  GetScreenWidth();
int  GetScreenHeight();

void FlipScreen();
void SetClippingRectangle(int x, int y, int w, int h);
void GetClippingRectangle(int* x, int* y, int* w, int* h);

IMAGE CreateImage(int width, int height, const RGBA* pixels);
IMAGE GrabImage(int x, int y, int width, int height);
void DestroyImage(IMAGE image);
void BlitImage(IMAGE image, int x, int y);
void BlitImageMask(IMAGE image, int x, int y, RGBA mask);
void TransformBlitImage(IMAGE image, int x[4], int y[4]);
void TransformBlitImageMask(IMAGE image, int x[4], int y[4], RGBA mask);
int GetImageWidth(IMAGE image);
int GetImageHeight(IMAGE image);
RGBA* LockImage(IMAGE image);
void UnlockImage(IMAGE image);
void DirectBlit(int x, int y, int w, int h, RGBA* pixels);
void DirectTransformBlit(int x[4], int y[4], int w, int h, RGBA* pixels);
void DirectGrab(int x, int y, int w, int h, RGBA* pixels);

void DrawPoint(int x, int y, RGBA color);
void DrawLine(int x[2], int y[2], RGBA color);
void DrawGradientLine(int x[2], int y[2], RGBA color[2]);
void DrawTriangle(int x[3], int y[3], RGBA color);
void DrawGradientTriangle(int x[3], int y[3], RGBA color[3]);
void DrawRectangle(int x, int y, int w, int h, RGBA color);
void DrawGradientRectangle(int x, int y, int w, int h, RGBA color[4]);

bool SetWindowTitle(const char* text);

#endif /* UNIX_VIDEO_H */
