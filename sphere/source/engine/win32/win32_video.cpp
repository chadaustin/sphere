#include <windows.h>
#include <stdio.h>
#include "win32_video.hpp"
#include "win32_time.hpp"
#include "win32_internal.hpp"
#include "../sfont.hpp"


static bool InitVDriver(int x, int y);
static void CloseVDriver();


// this function should not be exposed
static void (__stdcall * _FlipScreen)();


void (__stdcall * SetClippingRectangle)(int x, int y, int w, int h);
void (__stdcall * GetClippingRectangle)(int* x, int* y, int* w, int* h);

IMAGE (__stdcall * CreateImage)(int width, int height, const RGBA* data);
IMAGE (__stdcall * GrabImage)(int x, int y, int width, int height);
void  (__stdcall * DestroyImage)(IMAGE image);
void  (__stdcall * BlitImage)(IMAGE image, int x, int y);
void  (__stdcall * BlitImageMask)(IMAGE image, int x, int y, RGBA mask);
void  (__stdcall * TransformBlitImage)(IMAGE image, int x[4], int y[4]);
void  (__stdcall * TransformBlitImageMask)(IMAGE image, int x[4], int y[4], RGBA mask);
int   (__stdcall * GetImageWidth)(IMAGE image);
int   (__stdcall * GetImageHeight)(IMAGE image);
RGBA* (__stdcall * LockImage)(IMAGE image);
void  (__stdcall * UnlockImage)(IMAGE image);
void  (__stdcall * DirectBlit)(int x, int y, int w, int h, RGBA* pixels);
void  (__stdcall * DirectTransformBlit)(int x[4], int y[4], int w, int h, RGBA* pixels);
void  (__stdcall * DirectGrab)(int x, int y, int w, int h, RGBA* pixels);

void  (__stdcall * DrawPoint)(int x, int y, RGBA color);
void  (__stdcall * DrawLine)(int x[2], int y[2], RGBA color);
void  (__stdcall * DrawGradientLine)(int x[2], int y[2], RGBA color[2]);
void  (__stdcall * DrawTriangle)(int x[3], int y[3], RGBA color);
void  (__stdcall * DrawGradientTriangle)(int x[3], int y[3], RGBA color[3]);
void  (__stdcall * DrawRectangle)(int x, int y, int w, int h, RGBA color);
void  (__stdcall * DrawGradientRectangle)(int x, int y, int w, int h, RGBA color[4]);


static HWND      SphereWindow;
static HINSTANCE GraphicsDriver;

static int ScreenWidth;
static int ScreenHeight;

static SFONT* FPSFont;
static bool   FPSDisplayed;


////////////////////////////////////////////////////////////////////////////////

// helps eliminate warnings
template<typename T, typename U>
void assign(T& dest, U src)
{
  dest = (T&)src;
}

bool InitVideo(HWND window, SPHERECONFIG* config)
{
  // Loads driver
  std::string graphics_driver = "system/video/" + config->videodriver;
  GraphicsDriver = LoadLibrary(graphics_driver.c_str());
  if (GraphicsDriver == NULL)
    return false;

  // Gets addresses of all of the graphics functions
  assign(_FlipScreen,            GetProcAddress(GraphicsDriver, "FlipScreen"));
  assign(SetClippingRectangle,   GetProcAddress(GraphicsDriver, "SetClippingRectangle"));
  assign(GetClippingRectangle,   GetProcAddress(GraphicsDriver, "GetClippingRectangle"));

  assign(CreateImage,            GetProcAddress(GraphicsDriver, "CreateImage"));
  assign(GrabImage,              GetProcAddress(GraphicsDriver, "GrabImage"));
  assign(DestroyImage,           GetProcAddress(GraphicsDriver, "DestroyImage"));
  assign(BlitImage,              GetProcAddress(GraphicsDriver, "BlitImage"));
  assign(BlitImageMask,          GetProcAddress(GraphicsDriver, "BlitImageMask"));
  assign(TransformBlitImage,     GetProcAddress(GraphicsDriver, "TransformBlitImage"));
  assign(TransformBlitImageMask, GetProcAddress(GraphicsDriver, "TransformBlitImageMask"));
  assign(GetImageWidth,          GetProcAddress(GraphicsDriver, "GetImageWidth"));
  assign(GetImageHeight,         GetProcAddress(GraphicsDriver, "GetImageHeight"));
  assign(LockImage,              GetProcAddress(GraphicsDriver, "LockImage"));
  assign(UnlockImage,            GetProcAddress(GraphicsDriver, "UnlockImage"));
  assign(DirectBlit,             GetProcAddress(GraphicsDriver, "DirectBlit"));
  assign(DirectTransformBlit,    GetProcAddress(GraphicsDriver, "DirectTransformBlit"));
  assign(DirectGrab,             GetProcAddress(GraphicsDriver, "DirectGrab"));

  assign(DrawPoint,              GetProcAddress(GraphicsDriver, "DrawPoint"));
  assign(DrawLine,               GetProcAddress(GraphicsDriver, "DrawLine"));
  assign(DrawGradientLine,       GetProcAddress(GraphicsDriver, "DrawGradientLine"));
  assign(DrawTriangle,           GetProcAddress(GraphicsDriver, "DrawTriangle"));
  assign(DrawGradientTriangle,   GetProcAddress(GraphicsDriver, "DrawGradientTriangle"));
  assign(DrawRectangle,          GetProcAddress(GraphicsDriver, "DrawRectangle"));
  assign(DrawGradientRectangle,  GetProcAddress(GraphicsDriver, "DrawGradientRectangle"));

  if (!_FlipScreen ||
      !SetClippingRectangle ||
      !GetClippingRectangle ||
      !CreateImage ||
      !GrabImage ||
      !DestroyImage ||
      !BlitImage ||
      !BlitImageMask ||
      !TransformBlitImage ||
      !TransformBlitImageMask ||
      !GetImageWidth ||
      !GetImageHeight ||
      !LockImage ||
      !UnlockImage ||
      !DirectBlit ||
      !DirectTransformBlit ||
      !DirectGrab ||
      !DrawPoint ||
      !DrawLine ||
      !DrawGradientLine ||
      !DrawTriangle ||
      !DrawGradientTriangle ||
      !DrawRectangle ||
      !DrawGradientRectangle)
  {
    FreeLibrary(GraphicsDriver);
    return false;
  }

  SphereWindow = window;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void CloseVideo()
{
  if (GraphicsDriver)
  {
    CloseVDriver();
    FreeLibrary(GraphicsDriver);
  }
}

////////////////////////////////////////////////////////////////////////////////

void SetFPSFont(SFONT* font)
{
  FPSFont = font;
}

////////////////////////////////////////////////////////////////////////////////

void ToggleFPS()
{
  FPSDisplayed = !FPSDisplayed;
}

////////////////////////////////////////////////////////////////////////////////

bool InitVDriver(int x, int y)
{
  bool (__stdcall * InitVideoDriver)(HWND window, int screen_width, int screen_height);

  ScreenWidth = x;
  ScreenHeight = y;

  // Gets address of driver init func
  InitVideoDriver = (bool (__stdcall *)(HWND, int, int))GetProcAddress(GraphicsDriver, "InitVideoDriver");
  if (InitVideoDriver == NULL) {
    return false;
  }

  return InitVideoDriver(SphereWindow, ScreenWidth, ScreenHeight);
}

////////////////////////////////////////////////////////////////////////////////

void CloseVDriver()
{
  void (__stdcall * CloseVideoDriver)();

  // Gets address of
  CloseVideoDriver = (void (__stdcall *)())GetProcAddress(GraphicsDriver, "CloseVideoDriver");
  if (CloseVideoDriver == NULL) {
    FreeLibrary(GraphicsDriver);
    return;
  }

  CloseVideoDriver();
}

////////////////////////////////////////////////////////////////////////////////

bool SwitchResolution(int x, int y)
{
  if (x == ScreenWidth && y == ScreenHeight) {
    return true;
  }

  // if we haven't set a screen size, don't close the old driver
  if (ScreenWidth != 0 || ScreenHeight != 0) {
    CloseVDriver();
  }

  if (!InitVDriver(x, y)) {
    FreeLibrary(GraphicsDriver);
    QuitMessage("Could not switch resolutions");
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

int GetScreenWidth()
{
  return ScreenWidth;
}

////////////////////////////////////////////////////////////////////////////////

int GetScreenHeight()
{
  return ScreenHeight;
}

////////////////////////////////////////////////////////////////////////////////

void FlipScreen()
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

  _FlipScreen();
}

////////////////////////////////////////////////////////////////////////////////
