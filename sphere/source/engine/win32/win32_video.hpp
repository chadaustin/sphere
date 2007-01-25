#ifndef WIN32_VIDEO_H
#define WIN32_VIDEO_H
#include "../../common/rgb.hpp"
class SFONT; // for drawing FPS
typedef struct IMAGEimp* IMAGE;
extern void SetFPSFont(SFONT* font);
extern bool SwitchResolution(int x, int y);
extern int  GetScreenWidth();
extern int  GetScreenHeight();
extern bool ToggleFullScreen();
extern void FlipScreen();
extern void (__stdcall * SetClippingRectangle)(int x, int y, int w, int h);
extern void (__stdcall * GetClippingRectangle)(int* x, int* y, int* w, int* h);
extern IMAGE (__stdcall * CreateImage)(int width, int height, const RGBA* pixels);
extern IMAGE (__stdcall * GrabImage)(int x, int y, int width, int height);
extern void  (__stdcall * DestroyImage)(IMAGE image);
extern void  (__stdcall * BlitImage)(IMAGE image, int x, int y);
extern void  (__stdcall * BlitImageMask)(IMAGE image, int x, int y, RGBA mask);
extern void  (__stdcall * TransformBlitImage)(IMAGE image, int x[4], int y[4]);
extern void  (__stdcall * TransformBlitImageMask)(IMAGE image, int x[4], int y[4], RGBA mask);
extern int   (__stdcall * GetImageWidth)(IMAGE image);
extern int   (__stdcall * GetImageHeight)(IMAGE image);
extern RGBA* (__stdcall * LockImage)(IMAGE image);
extern void  (__stdcall * UnlockImage)(IMAGE image, bool pixels_changed);
extern void  (__stdcall * DirectBlit)(int x, int y, int w, int h, RGBA* pixels);
extern void  (__stdcall * DirectTransformBlit)(int x[4], int y[4], int w, int h, RGBA* pixels);
extern void  (__stdcall * DirectGrab)(int x, int y, int w, int h, RGBA* pixels);
extern void  (__stdcall * DrawPoint)(int x, int y, RGBA color);
extern void  (__stdcall * DrawLine)(int x[2], int y[2], RGBA color);
extern void  (__stdcall * DrawGradientLine)(int x[2], int y[2], RGBA color[2]);
extern void  (__stdcall * DrawTriangle)(int x[3], int y[3], RGBA color);
extern void  (__stdcall * DrawGradientTriangle)(int x[3], int y[3], RGBA color[3]);
extern void  (__stdcall * DrawRectangle)(int x, int y, int w, int h, RGBA color);
extern void  (__stdcall * DrawGradientRectangle)(int x, int y, int w, int h, RGBA color[4]);
extern bool SetWindowTitle(const char* text);
#endif
