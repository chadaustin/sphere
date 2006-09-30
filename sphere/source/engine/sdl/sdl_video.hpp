#ifndef SDL_VIDEO_HPP
#define SDL_VIDEO_HPP


#include "../../common/rgb.hpp"



class SFONT; // for drawing FPS


typedef struct IMAGEimp* IMAGE;


extern void SetFPSFont(SFONT* font);

extern bool SwitchResolution(int x, int y);
extern int  GetScreenWidth();
extern int  GetScreenHeight();
extern int  GetDefaultWidth();
extern int  GetDefaultHeight();

extern bool FlipScreen();
extern bool SetClippingRectangle(int x, int y, int w, int h);
extern bool GetClippingRectangle(int* x, int* y, int* w, int* h);

extern IMAGE CreateImage(int width, int height, const RGBA* pixels);
extern IMAGE GrabImage(int x, int y, int width, int height);
extern bool  DestroyImage(IMAGE image);
extern bool  BlitImage(IMAGE image, int x, int y);
extern void  BlitImageMask(IMAGE image, int x, int y, RGBA mask);
extern void  TransformBlitImage(IMAGE image, int x[4], int y[4]);
extern void  TransformBlitImageMask(IMAGE image, int x[4], int y[4], RGBA mask);
extern int   GetImageWidth(IMAGE image);
extern int   GetImageHeight(IMAGE image);
extern RGBA* LockImage(IMAGE image);
extern void  UnlockImage(IMAGE image, bool pixels_changed);
extern void  DirectBlit(int x, int y, int w, int h, RGBA* pixels);
extern void  DirectTransformBlit(int x[4], int y[4], int w, int h, RGBA* pixels);
extern void  DirectGrab(int x, int y, int w, int h, RGBA* pixels);

extern void  DrawPoint(int x, int y, RGBA color);
extern void  DrawLine(int x[2], int y[2], RGBA color);
extern void  DrawGradientLine(int x[2], int y[2], RGBA color[2]);
extern void  DrawTriangle(int x[3], int y[3], RGBA color);
extern void  DrawGradientTriangle(int x[3], int y[3], RGBA color[3]);
extern void  DrawRectangle(int x, int y, int w, int h, RGBA color);
extern void  DrawGradientRectangle(int x, int y, int w, int h, RGBA color[4]);

extern bool SetWindowTitle(const char* text);

#endif
