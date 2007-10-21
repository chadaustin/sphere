#ifndef UNIX_VIDEO_H
#define UNIX_VIDEO_H

#include "../../common/rgb.hpp"
#include "../../common/ParticleStructs.hpp"
#include "unix_sphere_config.h"
#include "unix_filesystem.h"
#include <SDL.h>

#include <iostream>

class SFONT; /* for drawing FPS */

extern int scaling;

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

bool SwitchResolution(int x, int y);
bool SwitchResolution(int x, int y, bool fullscreen, bool update_cliprect, int scalex = 0);
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
void UnlockImage(IMAGE image, bool pixels_changed);
void DirectBlit(int x, int y, int w, int h, RGBA* pixels);
void DirectTransformBlit(int x[4], int y[4], int w, int h, RGBA* pixels);
void DirectGrab(int x, int y, int w, int h, RGBA* pixels);

void DrawPoint(int x, int y, RGBA color);
void DrawPointSeries(VECTOR_INT** points, int length, RGBA color);
void DrawLine(int x[2], int y[2], RGBA color);
void DrawGradientLine(int x[2], int y[2], RGBA color[2]);
void DrawLineSeries(VECTOR_INT** points, int length, RGBA color, int type);
void DrawBezierCurve(int x[4], int y[4], double step, RGBA color, int cubic);
void DrawTriangle(int x[3], int y[3], RGBA color);
void DrawGradientTriangle(int x[3], int y[3], RGBA color[3]);
void DrawPolygon(VECTOR_INT** points, int length, int invert, RGBA color);
void DrawOutlinedRectangle(int x, int y, int w, int h, int size, RGBA color);
void DrawRectangle(int x, int y, int w, int h, RGBA color);
void DrawGradientRectangle(int x, int y, int w, int h, RGBA color[4]);
void DrawOutlinedComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, RGBA color, int antialias);
void DrawFilledComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, float angle, float frac_size, int fill_empty, RGBA colors[2]);
void DrawGradientComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, float angle, float frac_size, int fill_empty, RGBA colors[3]);
void DrawOutlinedEllipse(int x, int y, int rx, int ry, RGBA color);
void DrawFilledEllipse(int x, int y, int rx, int ry, RGBA color);
void DrawOutlinedCircle(int x, int y, int r, RGBA color, int antialias);
void DrawFilledCircle(int x, int y, int r, RGBA color, int antialias);
void DrawGradientCircle(int x, int y, int r, RGBA color[2], int antialias);

bool SetWindowTitle(const char* text);

#endif /* UNIX_VIDEO_H */
