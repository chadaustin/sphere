#include <d3d8.h>
#include <d3dx8tex.h>
#include <d3dx8math.h>
#include <stdio.h>
#include <math.h>
#include "../../common/rgb.hpp"
#include "../common/win32x.hpp"
#include "resource.h"


// macros
#define EXPORT __stdcall


// types
struct CONFIGURATION
{
  bool fullscreen;
};

struct DRIVERINFO
{
  const char* name;
  const char* author;
  const char* date;
  const char* version;
  const char* description;
};


typedef struct IMAGE_STRUCT
{
  int width;
  int height;
  RGBA* pixels;

  IDirect3DTexture8* texture;
  float tex_width;
  float tex_height;
}* IMAGE;


#define COLOR_VERTEX_FORMAT (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
struct COLOR_VERTEX
{
  FLOAT x, y, z, rhw;
  DWORD color;
};

#define TEXTURE_VERTEX_FORMAT (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
struct TEXTURE_VERTEX
{
  FLOAT x, y, z, rhw;
  DWORD color;
  FLOAT tu, tv;
};


// function prototypes
static void GetConfigFile(char filename[MAX_PATH]);
static void LoadConfig();
static void SaveConfig();
static BOOL CALLBACK ConfigureDriverDialogProc(HWND dialog, UINT message, WPARAM wparam, LPARAM lparam);
static bool CreateTexture(IMAGE image);

void EXPORT DirectGrab(int x, int y, int w, int h, RGBA* pixels);
void EXPORT BlitImageMask(IMAGE image, int x, int y, RGBA mask);
void EXPORT TransformBlitImageMask(IMAGE image, int x[4], int y[4], RGBA mask);


// globals
static HINSTANCE         s_DriverInstance;
static CONFIGURATION     s_Configuration;
static IDirect3D8*       s_Direct3D;
static IDirect3DDevice8* s_Direct3DDevice;
static int               s_ScreenWidth;
static int               s_ScreenHeight;
static bool              s_Fullscreen;

////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
  s_DriverInstance = instance;
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT GetDriverInfo(DRIVERINFO* driverinfo)
{
  driverinfo->name        = "DirectX 8";
  driverinfo->author      = "Chad Austin";
  driverinfo->date        = __DATE__;
  driverinfo->version     = "0.50";
  driverinfo->description = "DirectX 8 hardware accelerated driver";
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT ConfigureDriver(HWND parent)
{
  LoadConfig();

  DialogBox(
    s_DriverInstance,
    MAKEINTRESOURCE(IDD_CONFIGURE),
    parent,
    ConfigureDriverDialogProc
  );
}

////////////////////////////////////////////////////////////////////////////////

void GetConfigFile(char filename[MAX_PATH])
{
    GetModuleFileName(s_DriverInstance, filename, MAX_PATH);
    if (strrchr(filename, '\\')) {
        *strrchr(filename, '\\') = 0;
        strcat(filename, "\\");
    } else {
        filename[0] = 0;
    }
    strcat(filename, "sphere_dx8.cfg");
}

////////////////////////////////////////////////////////////////////////////////

void LoadConfig()
{
  char filename[MAX_PATH];
  GetConfigFile(filename);

  s_Configuration.fullscreen = (0 != GetPrivateProfileInt("sphere_dx8", "fullscreen", 0, filename));
}

////////////////////////////////////////////////////////////////////////////////

BOOL WritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nInt, LPCTSTR lpFileName)
{
  char str[80];
  sprintf(str, "%d", nInt);
  return WritePrivateProfileString(lpAppName, lpKeyName, str, lpFileName);
}

////////////////////////////////////////////////////////////////////////////////

void SaveConfig()
{
  char filename[MAX_PATH];
  GetConfigFile(filename);

  WritePrivateProfileInt("sphere_dx8", "fullscreen", s_Configuration.fullscreen, filename);
}

////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK ConfigureDriverDialogProc(HWND dialog, UINT message, WPARAM wparam, LPARAM lparam)
{
  switch (message) {
    case WM_INITDIALOG: {
      // set fullscreen flag
      if (s_Configuration.fullscreen) {
        CheckDlgButton(dialog, IDC_FULLSCREEN, BST_CHECKED);
      }
      return TRUE;
    }

    case WM_COMMAND: {
      switch (LOWORD(wparam)) {
        case IDOK: {

          // store fullscreen setting
          s_Configuration.fullscreen = (IsDlgButtonChecked(dialog, IDC_FULLSCREEN) == BST_CHECKED);

          SaveConfig();
          EndDialog(dialog, 0);
          return TRUE;
        }

        case IDCANCEL: {
          EndDialog(dialog, 0);
          return TRUE;
        }
      }
    }

    default: {
      return FALSE;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

bool EXPORT InitVideoDriver(HWND window, int screen_width, int screen_height)
{
  static bool firstcall = true;

  LoadConfig();

  // create the Direct3D object
  s_Direct3D = Direct3DCreate8(D3D_SDK_VERSION);
  if (s_Direct3D == NULL) {
    MessageBox(window, "Direct3D object creation failed", "sphere_dx8", MB_OK | MB_ICONERROR);
    return false;
  }

  if (firstcall) {
    s_Fullscreen = s_Configuration.fullscreen;
    firstcall = false;
  }
  
  // set the display mode
  D3DDISPLAYMODE display_mode;
  if (s_Fullscreen) {   // FULLSCREEN

    display_mode.RefreshRate = 0;  // use default
    display_mode.Format      = D3DFMT_A8R8G8B8;
    
  } else {                            // WINDOWED

    // ask Windows for the current settings
    if (FAILED(s_Direct3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &display_mode))) {
      MessageBox(window, "Direct3D GetAdapterDisplayMode failed", "sphere_dx8", MB_OK | MB_ICONERROR);
      s_Direct3D->Release();
      s_Direct3D = NULL;
      return false;
    }

    CenterWindow(window, screen_width, screen_height);
  }

  display_mode.Width  = screen_width;
  display_mode.Height = screen_height;

  // define how we want our application displayed
  D3DPRESENT_PARAMETERS d3dpp;
  memset(&d3dpp, 0, sizeof(d3dpp));
  d3dpp.Windowed         = (s_Fullscreen ? FALSE : TRUE);
  d3dpp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
  d3dpp.BackBufferFormat = display_mode.Format;
  d3dpp.BackBufferWidth  = display_mode.Width;
  d3dpp.BackBufferHeight = display_mode.Height;

  // create the device object
  HRESULT result = s_Direct3D->CreateDevice(
    D3DADAPTER_DEFAULT,
    D3DDEVTYPE_HAL,
    window,
    D3DCREATE_SOFTWARE_VERTEXPROCESSING,
    &d3dpp,
    &s_Direct3DDevice
  );

  if (FAILED(result)) {
    s_Direct3D->Release();
    s_Direct3D = NULL;
    MessageBox(window, "Direct3D device creation failed", "sphere_dx8", MB_OK | MB_ICONERROR);
    return false;
  }

  // device setup
  BOOL succeeded = TRUE;

  // disable lighting
  succeeded &= !FAILED(s_Direct3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE));
  succeeded &= !FAILED(s_Direct3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
  succeeded &= !FAILED(s_Direct3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
  succeeded &= !FAILED(s_Direct3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

  if (!succeeded) {
    MessageBox(window, "Initial device setup failed", "sphere_dx8", MB_OK | MB_ICONERROR);
    return false;
  }

  // start the initial scene
  s_Direct3DDevice->BeginScene();

  s_ScreenWidth  = screen_width;
  s_ScreenHeight = screen_height;
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT CloseVideoDriver()
{
  // end the current scene
  s_Direct3DDevice->EndScene();

  // release the device
  if (s_Direct3DDevice) {
    s_Direct3DDevice->Release();
    s_Direct3DDevice = NULL;
  }

  // release the Direct3D object
  if (s_Direct3D) {
    s_Direct3D->Release();
    s_Direct3D = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool EXPORT ToggleFullScreen()
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT FlipScreen()
{
  // end the scene
  s_Direct3DDevice->EndScene();

  // flip
  s_Direct3DDevice->Present(NULL, NULL, NULL, NULL);

  s_Direct3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

  // start a new scene
  s_Direct3DDevice->BeginScene();
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT SetClippingRectangle(int x, int y, int w, int h)
{
  D3DVIEWPORT8 viewport;
  viewport.X      = x;
  viewport.Y      = y;
  viewport.Width  = w;
  viewport.Height = h;
  viewport.MinZ   = 0.0f;
  viewport.MaxZ   = 1.0f;

  s_Direct3DDevice->SetViewport(&viewport);
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT GetClippingRectangle(int* x, int* y, int* w, int* h)
{
  D3DVIEWPORT8 viewport;
  s_Direct3DDevice->GetViewport(&viewport);

  *x = viewport.X;
  *y = viewport.Y;
  *w = viewport.Width;
  *h = viewport.Height;
}

////////////////////////////////////////////////////////////////////////////////

IMAGE EXPORT CreateImage(int width, int height, RGBA* pixels)
{
  IMAGE i = new IMAGE_STRUCT;
  i->width  = width;
  i->height = height;
  i->pixels = new RGBA[width * height];
  memcpy(i->pixels, pixels, sizeof(RGBA) * width * height);

  if (!CreateTexture(i)) {
    delete[] i->pixels;
    delete i;
    return NULL;
  }

  return i;
}

////////////////////////////////////////////////////////////////////////////////

bool CreateTexture(IMAGE image)
{
  HRESULT result = D3DXCreateTexture(
    s_Direct3DDevice,
    image->width,
    image->height,
    1,
    0,
    D3DFMT_A8R8G8B8,
    D3DPOOL_MANAGED,
    &image->texture
  );

  if (FAILED(result)) {
    return false;
  }

  // now find out the real texture size
  D3DSURFACE_DESC ddsd;
  image->texture->GetLevelDesc(0, &ddsd);
  
  image->tex_width  = (float)image->width  / ddsd.Width;
  image->tex_height = (float)image->height / ddsd.Height;


  // lock
  D3DLOCKED_RECT locked_rect;
  result = image->texture->LockRect(0, &locked_rect, NULL, 0);
  if (FAILED(result)) {
    image->texture->Release();
    return false;
  }

  byte* pixels = (byte*)locked_rect.pBits;
  RGBA* src = image->pixels;

  int iy = image->height;
  while (iy--) {
    
    BGRA* dst = (BGRA*)pixels;

    int ix = image->width;
    while (ix--) {
      dst->red   = src->red;
      dst->green = src->green;
      dst->blue  = src->blue;
      dst->alpha = src->alpha;
      dst++;
      src++;
    }

    pixels += locked_rect.Pitch;
  }

  // unlock
  image->texture->UnlockRect(0);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

IMAGE EXPORT GrabImage(int x, int y, int width, int height)
{
  RGBA* pixels = new RGBA[width * height];
  DirectGrab(x, y, width, height, pixels);
  IMAGE result = CreateImage(width, height, pixels);
  delete[] pixels;
  return result;
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DestroyImage(IMAGE image)
{
  image->texture->Release();
  delete[] image->pixels;
  delete image;
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT BlitImage(IMAGE image, int x, int y)
{
  BlitImageMask(image, x, y, CreateRGBA(255, 255, 255, 255));
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT BlitImageMask(IMAGE image, int x, int y, RGBA mask)
{
  D3DCOLOR c = D3DCOLOR_RGBA(mask.red, mask.green, mask.blue, mask.alpha);
  int w = image->width;
  int h = image->height;
  float u = image->tex_width;
  float v = image->tex_height;

  TEXTURE_VERTEX vertex[] = {
    { (float)x,     (float)y,      0.0f, 1.0f, c, 0.0f, 0.0f, },
    { (float)x + w, (float)y,      0.0f, 1.0f, c, u,    0.0f, },
    { (float)x + w, (float)y + h,  0.0f, 1.0f, c, u,    v, },
    { (float)x,     (float)y + h,  0.0f, 1.0f, c, 0.0f, v, },
  };

  s_Direct3DDevice->SetVertexShader(TEXTURE_VERTEX_FORMAT);
  s_Direct3DDevice->SetTexture(0, image->texture);
  s_Direct3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertex, sizeof(*vertex));
  s_Direct3DDevice->SetTexture(0, NULL);
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT TransformBlitImage(IMAGE image, int x[4], int y[4])
{
  TransformBlitImageMask(image, x, y, CreateRGBA(255, 255, 255, 255));
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT TransformBlitImageMask(IMAGE image, int x[4], int y[4], RGBA mask)
{
  D3DCOLOR c = D3DCOLOR_RGBA(mask.red, mask.green, mask.blue, mask.alpha);
  int w = image->width;
  int h = image->height;
  float u = image->tex_width;
  float v = image->tex_height;

  TEXTURE_VERTEX vertex[] = {
    { (float)x[0],     (float)y[0],      0.0f, 1.0f, c, 0.0f, 0.0f, },
    { (float)x[1],     (float)y[1],      0.0f, 1.0f, c, u,    0.0f, },
    { (float)x[2],     (float)y[2],  0.0f, 1.0f, c, u,    v, },
    { (float)x[3],     (float)y[3],  0.0f, 1.0f, c, 0.0f, v, },
  };

  s_Direct3DDevice->SetVertexShader(TEXTURE_VERTEX_FORMAT);
  s_Direct3DDevice->SetTexture(0, image->texture);
  s_Direct3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertex, sizeof(*vertex));
  s_Direct3DDevice->SetTexture(0, NULL);
}

////////////////////////////////////////////////////////////////////////////////

int EXPORT GetImageWidth(IMAGE image)
{
  return image->width;
}

////////////////////////////////////////////////////////////////////////////////

int EXPORT GetImageHeight(IMAGE image)
{
  return image->height;
}

////////////////////////////////////////////////////////////////////////////////

RGBA* EXPORT LockImage(IMAGE image)
{
  return image->pixels;
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT UnlockImage(IMAGE image, bool pixels_changed)
{
  if (pixels_changed) {
    image->texture->Release();
    if (!CreateTexture(image)) {
      // uh oh...
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DirectBlit(int x, int y, int w, int h, RGBA* pixels)
{
  IMAGE i = CreateImage(w, h, pixels);
  if (i) {
    BlitImage(i, x, y);
    DestroyImage(i);
  }
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DirectTransformBlit(int x[4], int y[4], int w, int h, RGBA* pixels)
{
  IMAGE i = CreateImage(w, h, pixels);
  if (i) {
    TransformBlitImage(i, x, y);
    DestroyImage(i);
  }
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DirectGrab(int x, int y, int w, int h, RGBA* pixels)
{
  if (x < 0 || y < 0 || x + w > s_ScreenWidth || y + h > s_ScreenHeight) {
    return;
  }

  // get the backbuffer
  IDirect3DSurface8* backbuffer;
  HRESULT result = s_Direct3DDevice->GetBackBuffer(
    0,
    D3DBACKBUFFER_TYPE_MONO,
    &backbuffer
  );

  if (FAILED(result)) {
    return;
  }

  // now lock the backbuffer
  D3DLOCKED_RECT locked_rect;
  result = backbuffer->LockRect(&locked_rect, NULL, 0);
  if (FAILED(result)) {
    backbuffer->Release();
    return;
  }

  // copy the pixels out
  RGBA* dst = pixels;
  RGBA* src = (RGBA*)locked_rect.pBits + w;

  int iy = h;
  while (iy--) {

    int ix = w;
    while (ix--) {
      dst->red   = src->red;
      dst->green = src->green;
      dst->blue  = src->blue;
      dst->alpha = 255;
      pixels++;
      src++;
    }

    src += locked_rect.Pitch / sizeof(RGBA) - w;
  }

  backbuffer->UnlockRect();
  backbuffer->Release();
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DrawPoint(int x, int y, RGBA color)
{
  COLOR_VERTEX vertex = {
    (float)x,
    (float)y,
    0.0f,
    1.0f,
    D3DCOLOR_RGBA(color.red, color.green, color.blue, color.alpha),
  };

  s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
  s_Direct3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, 1, &vertex, sizeof(vertex));
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DrawLine(int x[2], int y[2], RGBA color)
{
  D3DCOLOR c = D3DCOLOR_RGBA(color.red, color.green, color.blue, color.alpha);

  COLOR_VERTEX vertex[] = {
    { (float)x[0], (float)y[0], 0.0f, 1.0f, c },
    { (float)x[1], (float)y[1], 0.0f, 1.0f, c },
  };

  s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
  s_Direct3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, vertex, sizeof(*vertex));
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DrawGradientLine(int x[2], int y[2], RGBA colors[2])
{
  D3DCOLOR color1 = D3DCOLOR_RGBA(colors[0].red, colors[0].green, colors[0].blue, colors[0].alpha);
  D3DCOLOR color2 = D3DCOLOR_RGBA(colors[1].red, colors[1].green, colors[1].blue, colors[1].alpha);

  COLOR_VERTEX vertex[] = {
    { (float)x[0], (float)y[0], 0.0f, 1.0f, color1 },
    { (float)x[1], (float)y[1], 0.0f, 1.0f, color2 },
  };

  s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
  s_Direct3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, vertex, sizeof(*vertex));
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DrawTriangle(int x[3], int y[3], RGBA color)
{
  D3DCOLOR c = D3DCOLOR_RGBA(color.red, color.green, color.blue, color.alpha);

  COLOR_VERTEX vertex[] = {
    { (float)x[0], (float)y[0], 0.0f, 1.0f, c },
    { (float)x[1], (float)y[1], 0.0f, 1.0f, c },
    { (float)x[2], (float)y[2], 0.0f, 1.0f, c },
  };

  s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
  s_Direct3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, vertex, sizeof(*vertex));
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DrawGradientTriangle(int x[3], int y[3], RGBA colors[3])
{
  D3DCOLOR color1 = D3DCOLOR_RGBA(colors[0].red, colors[0].green, colors[0].blue, colors[0].alpha);
  D3DCOLOR color2 = D3DCOLOR_RGBA(colors[1].red, colors[1].green, colors[1].blue, colors[1].alpha);
  D3DCOLOR color3 = D3DCOLOR_RGBA(colors[2].red, colors[2].green, colors[2].blue, colors[2].alpha);

  COLOR_VERTEX vertex[] = {
    { (float)x[0], (float)y[0], 0.0f, 1.0f, color1 },
    { (float)x[1], (float)y[1], 0.0f, 1.0f, color2 },
    { (float)x[2], (float)y[2], 0.0f, 1.0f, color3 },
  };

  s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
  s_Direct3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, vertex, sizeof(*vertex));
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DrawRectangle(int x, int y, int w, int h, RGBA color)
{
  D3DCOLOR c = D3DCOLOR_RGBA(color.red, color.green, color.blue, color.alpha);

  COLOR_VERTEX vertex[] = {
    { (float)x,     (float)y,      0.0f, 1.0f, c, },
    { (float)x + w, (float)y,      0.0f, 1.0f, c, },
    { (float)x + w, (float)y + h,  0.0f, 1.0f, c, },
    { (float)x,     (float)y + h,  0.0f, 1.0f, c, }
  };

  s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
  s_Direct3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertex, sizeof(*vertex));
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DrawGradientRectangle(int x, int y, int w, int h, RGBA colors[4])
{
  D3DCOLOR color1 = D3DCOLOR_RGBA(colors[0].red, colors[0].green, colors[0].blue, colors[0].alpha);
  D3DCOLOR color2 = D3DCOLOR_RGBA(colors[1].red, colors[1].green, colors[1].blue, colors[1].alpha);
  D3DCOLOR color3 = D3DCOLOR_RGBA(colors[2].red, colors[2].green, colors[2].blue, colors[2].alpha);
  D3DCOLOR color4 = D3DCOLOR_RGBA(colors[3].red, colors[3].green, colors[3].blue, colors[3].alpha);

  COLOR_VERTEX vertex[] = {
    { (float)x,     (float)y,      0.0f, 1.0f, color1, },
    { (float)x + w, (float)y,      0.0f, 1.0f, color2, },
    { (float)x + w, (float)y + h,  0.0f, 1.0f, color3, },
    { (float)x,     (float)y + h,  0.0f, 1.0f, color4, }
  };

  s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
  s_Direct3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertex, sizeof(*vertex));
}

////////////////////////////////////////////////////////////////////////////////
