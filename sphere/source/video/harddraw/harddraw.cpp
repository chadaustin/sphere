#define DIRECTDRAW_VERSION 0x0300
#include <windows.h>
#include <ddraw.h>
#include <stdio.h>
#include "../../source/common/rgb.hpp"
#include "../../common/primitives.hpp"
#include "../common/video.hpp"
#include "../common/win32x.hpp"
#include "resource.h"


typedef struct IMAGEimp
{
  int width;
  int height;

  LPDIRECTDRAWSURFACE surface;

  void* pixels;
  byte* alpha;

  bool (*blit_routine)(IMAGEimp* image, int x, int y);

  RGBA* locked_pixels;
}* IMAGE;


enum BIT_DEPTH
{
  BD_32_24,
  BD_16_15,
};

struct CONFIGURATION
{
  BIT_DEPTH bit_depth;
};


// INLINE FUNCTIONS

inline word Pack565(RGBA pixel)
{
  return (word)(((pixel.red   >> 3) << 11) +
                ((pixel.green >> 2) <<  5) +
                ((pixel.blue  >> 3) <<  0));
}

inline RGBA Unpack565(word pixel)
{
  RGBA rgba = {
    ((pixel & 0xF800) >> 11) << 3, // 11111 000000 00000
    ((pixel & 0x07E0) >>  5) << 2, // 00000 111111 00000
    ((pixel & 0x001F) >>  0) << 3, // 00000 000000 11111
  };
  return rgba;
}

inline word Pack555(RGBA pixel)
{
  return (word)(
    ((pixel.red   >> 3) << 10) + 
    ((pixel.green >> 3) << 5) +
    ((pixel.blue  >> 3) << 0));
}

inline RGBA Unpack555(word pixel)
{
  RGBA rgba = {
    ((pixel & 0x7C00) >> 10) << 3, // 0 11111 00000 00000
    ((pixel & 0x03E0) >>  5) << 3, // 0 00000 11111 00000
    ((pixel & 0x001F) >>  0) << 3, // 0 00000 00000 11111
  };
  return rgba;
}

static bool GetLockedSurface(DDSURFACEDESC* ddsd);

static void LoadConfiguration();
static void SaveConfiguration();
static BOOL CALLBACK ConfigureDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

static bool SetDisplayMode();
static bool CreateSurfaces();

static void FillImagePixels(IMAGE image, RGBA* data);
static void OptimizeBlitRoutine(IMAGE image);

static bool NullBlit(IMAGE image, int x, int y);
static bool TileBlit(IMAGE image, int x, int y);
static bool SpriteBlit(IMAGE image, int x, int y);
static bool NormalBlit(IMAGE image, int x, int y);


static CONFIGURATION Configuration;
static int           BitsPerPixel;

static HWND  SphereWindow;

static LONG OldWindowStyle;
static LONG OldWindowStyleEx;

// fullscreen output
static LPDIRECTDRAW dd;
static LPDIRECTDRAWSURFACE ddPrimary;
static LPDIRECTDRAWSURFACE ddSecondary;
static LPDIRECTDRAWCLIPPER ddClipper;

FILE* logfile = fopen("harddraw.log", "w");
#define CLOG(str) fputs(str, logfile); fputc('\n', logfile); fflush(logfile);
#define CLOGPARAM(str, param) fprintf(logfile, str, param); fputc('\n', logfile); fflush(logfile);


////////////////////////////////////////////////////////////////////////////////

static bool GetLockedSurface(DDSURFACEDESC* ddsd) {
  memset(ddsd, 0, sizeof(DDSURFACEDESC));
  ddsd->dwSize = sizeof(DDSURFACEDESC);
  HRESULT result = ddSecondary->Lock(NULL, ddsd, DDLOCK_WAIT, NULL);
  return (FAILED(result) == false);
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) GetDriverInfo(DRIVERINFO* driverinfo)
{
  driverinfo->name        = "Hardware Accelerated Driver";
  driverinfo->author      = "Chad Austin";
  driverinfo->date        = __DATE__;
  driverinfo->version     = "1.00";
  driverinfo->description = "15/16/24/32-bit color output in hardware-accelerated fullscreen modes";
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) ConfigureDriver(HWND parent)
{
  LoadConfiguration();
  DialogBox(DriverInstance, MAKEINTRESOURCE(IDD_CONFIGURE), parent, ConfigureDialogProc);
  SaveConfiguration();
}

////////////////////////////////////////////////////////////////////////////////

void LoadConfiguration()
{
  char config_file_name[MAX_PATH];
  GetDriverConfigFile(config_file_name);

  // load the fields from the file
  int bit_depth = GetPrivateProfileInt("harddraw", "BitDepth", 0, config_file_name);
  Configuration.bit_depth = (bit_depth == 32 ? BD_32_24 : BD_16_15);
}

////////////////////////////////////////////////////////////////////////////////

void SaveConfiguration()
{
  char config_file_name[MAX_PATH];
  GetDriverConfigFile(config_file_name);

  // save the fields to the file
  int bit_depth = (Configuration.bit_depth == BD_32_24 ? 32 : 24);
  WritePrivateProfileInt("harddraw", "BitDepth", bit_depth, config_file_name);
}

////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK ConfigureDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  switch (message)
  {
    case WM_INITDIALOG:
      // set the bit depth radio buttons
      if (Configuration.bit_depth == BD_32_24)
        SendDlgItemMessage(window, IDC_BITDEPTH_32_24, BM_SETCHECK, BST_CHECKED, 0);
      else if (Configuration.bit_depth == BD_16_15)
        SendDlgItemMessage(window, IDC_BITDEPTH_16_15, BM_SETCHECK, BST_CHECKED, 0);

      return TRUE;

    ////////////////////////////////////////////////////////////////////////////

    case WM_COMMAND:
      switch (LOWORD(wparam))
      {
        case IDOK:
          if (IsDlgButtonChecked(window, IDC_BITDEPTH_32_24))
            Configuration.bit_depth = BD_32_24;
          else if (IsDlgButtonChecked(window, IDC_BITDEPTH_16_15))
            Configuration.bit_depth = BD_16_15;

          EndDialog(window, 1);
          return TRUE;

        case IDCANCEL:
          EndDialog(window, 0);
          return TRUE;
      }
      return FALSE;

    ////////////////////////////////////////////////////////////////////////////

    default:
      return FALSE;
  }
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(bool) InitVideoDriver(HWND window, int screen_width, int screen_height)
{
  CLOG("Got in!");

  SphereWindow = window;
  ScreenWidth  = screen_width;
  ScreenHeight = screen_height;

  LoadConfiguration();

  // store old window styles
  OldWindowStyle = GetWindowLong(SphereWindow, GWL_STYLE);
  OldWindowStyleEx = GetWindowLong(SphereWindow, GWL_EXSTYLE);

  SetWindowLong(SphereWindow, GWL_STYLE, WS_POPUP);
  SetWindowLong(SphereWindow, GWL_EXSTYLE, 0);

  // create DirectDraw object
  HRESULT ddrval = DirectDrawCreate(NULL, &dd, NULL);
  if (ddrval != DD_OK)
  {
    MessageBox(SphereWindow, "DirectDrawCreate() failed", "harddraw", MB_OK);
    return false;
  }

  // set application behavior
  ddrval = dd->SetCooperativeLevel(SphereWindow, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
  if (ddrval != DD_OK)
  {
    dd->Release();
    MessageBox(SphereWindow, "SetCooperativeLevel() failed", "harddraw", MB_OK);
    return false;
  }

  // set display mode
  if (!SetDisplayMode())
  {
    dd->Release();
    MessageBox(SphereWindow, "SetDisplayMode() failed", "harddraw", MB_OK);
    return false;
  }

  // create surfaces
  if (!CreateSurfaces())
  {
    dd->Release();
    MessageBox(SphereWindow, "CreateSurfaces() failed", "harddraw", MB_OK);
    return false;
  }

  ddrval = dd->CreateClipper(0, &ddClipper, NULL);
  if (FAILED(ddrval)) {
    dd->Release();
    MessageBox(SphereWindow, "CreateClipper() failed", "harddraw", MB_OK);
    return false;
  }

  ddrval = ddSecondary->SetClipper(ddClipper);
  if (FAILED(ddrval)) {
    dd->Release();
    MessageBox(SphereWindow, "SetClipper() failed", "harddraw", MB_OK);
    return false;
  }

  // set default clipping rectangle
  SetClippingRectangle(0, 0, screen_width, screen_height);

  ShowCursor(FALSE);
  
  CLOG("Got out!");
  return true;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(bool) ToggleFullScreen()
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool SetDisplayMode()
{
  HRESULT ddrval;

  BitsPerPixel = 0;

  switch (Configuration.bit_depth)
  {
    case BD_32_24:
      ddrval = dd->SetDisplayMode(ScreenWidth, ScreenHeight, 32);
      if (ddrval == DD_OK) {
        BitsPerPixel = 32;
      } else {
        ddrval = dd->SetDisplayMode(ScreenWidth, ScreenHeight, 24);
        if (ddrval == DD_OK) {
          BitsPerPixel = 32;
        }
      }
      return ddrval == DD_OK;

    case BD_16_15:
      ddrval = dd->SetDisplayMode(ScreenWidth, ScreenHeight, 16);
      if (ddrval == DD_OK) {
        // determine bits per pixel from pixel mask
        DDPIXELFORMAT ddpf;
        ddpf.dwSize = sizeof(ddpf);
        ddpf.dwFlags = DDPF_RGB;
        HRESULT result = ddPrimary->GetPixelFormat(&ddpf);
        if (result == DD_OK) {
          // 5:6:5 -- F800 07E0 001F
          // 5:5:5 -- 7C00 03E0 001F

          if (ddpf.dwRBitMask == 0xF800)
            BitsPerPixel = 16;
          else if (ddpf.dwRBitMask == 0x7C00)
            BitsPerPixel = 15;
        }
      }
      return ddrval == DD_OK;

    default:
      return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool CreateSurfaces()
{
  // define the surface
  DDSURFACEDESC ddsd;
  ddsd.dwSize            = sizeof(ddsd);
  ddsd.dwFlags           = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
  ddsd.ddsCaps.dwCaps    = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
  ddsd.dwBackBufferCount = 1;

  // create the primary surface
  HRESULT ddrval = dd->CreateSurface(&ddsd, &ddPrimary, NULL);
  if (ddrval != DD_OK)
    return false;

  ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
  ddrval = ddPrimary->GetAttachedSurface(&ddsd.ddsCaps, &ddSecondary);
  if (ddrval != DD_OK)
  {
    ddPrimary->Release();
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(bool) CloseVideoDriver()
{
  SetWindowLong(SphereWindow, GWL_STYLE,   OldWindowStyle);
  SetWindowLong(SphereWindow, GWL_EXSTYLE, OldWindowStyleEx);

  ShowCursor(TRUE);

  dd->Release();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void FillImagePixels(IMAGE image, RGBA* pixels)
{
  // fill the image pixels
  switch (BitsPerPixel) {
    case 32: {
      image->pixels = new BGRA[image->width * image->height];
      for (int i = 0; i < image->width * image->height; i++)
      {
        ((BGRA*)image->pixels)[i].red   = pixels[i].red;
        ((BGRA*)image->pixels)[i].green = pixels[i].green;
        ((BGRA*)image->pixels)[i].blue  = pixels[i].blue;
      }
      break;
    }

    case 24: {
      image->pixels = new BGR[image->width * image->height];
      for (int i = 0; i < image->width * image->height; i++)
      {
        ((BGR*)image->pixels)[i].red   = pixels[i].red;
        ((BGR*)image->pixels)[i].green = pixels[i].green;
        ((BGR*)image->pixels)[i].blue  = pixels[i].blue;
      }
      break;
    }

    case 16: {
      image->pixels = new word[image->width * image->height];
      for (int i = 0; i < image->width * image->height; i++) {
        ((word*)image->pixels)[i] = Pack565(pixels[i]);
      }
      break;
    }

    case 15: {
      image->pixels = new word[image->width * image->height];
      for (int i = 0; i < image->width * image->height; i++) {
        ((word*)image->pixels)[i] = Pack555(pixels[i]);
      }
      break;
    }
  } // end switch

  // fill the alpha array
  image->alpha = new byte[image->width * image->height];
  for (int i = 0; i < image->width * image->height; i++)
    image->alpha[i] = pixels[i].alpha;
}

////////////////////////////////////////////////////////////////////////////////

void OptimizeBlitRoutine(IMAGE image)
{
  // null blit
  bool is_empty = true;
  for (int i = 0; i < image->width * image->height; i++)
    if (image->alpha[i] != 0)
    {
      is_empty = false;
      break;
    }
  if (is_empty)
  {
    image->blit_routine = NullBlit;
    return;
  }

  // tile blit
  bool is_tile = true;
  for (int i = 0; i < image->width * image->height; i++)
    if (image->alpha[i] != 255)
    {
      is_tile = false;
      break;
    }
  if (is_tile)
  {
    image->blit_routine = TileBlit;
    return;
  }

  // sprite blit
  bool is_sprite = true;
  for (int i = 0; i < image->width * image->height; i++)
    if (image->alpha[i] != 0 &&
        image->alpha[i] != 255)
    {
      is_sprite = false;
      break;
    }
  if (is_sprite)
  {
    image->blit_routine = SpriteBlit;
    return;
  }

  // normal blit
  image->blit_routine = NormalBlit;
}

////////////////////////////////////////////////////////////////////////////////

void CreateSurface(IMAGE image)
{
  DDSURFACEDESC ddsd;
  memset(&ddsd, 0, sizeof(ddsd));
  ddsd.dwSize         = sizeof(ddsd);
  ddsd.dwFlags        = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
  ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
  ddsd.dwWidth        = image->width;
  ddsd.dwHeight       = image->height;
  dd->CreateSurface(&ddsd, &image->surface, NULL);
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(bool) FlipScreen()
{
  CLOG("+FlipScreen")

  HRESULT ddrval = ddPrimary->Flip(NULL, DDFLIP_WAIT);
  if (ddrval == DDERR_SURFACELOST) {
    ddPrimary->Restore();
    ddPrimary->Flip(NULL, DDFLIP_WAIT);
  }

  CLOG("-FlipScreen")
  return true;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(bool) ApplyColorMask(RGBA mask)
{
  if (mask.alpha == 0) {
    return true;
  }

  CLOG("+ApplyColorMask")

  DDSURFACEDESC ddsd;
  if ( GetLockedSurface(&ddsd) == false)
    return false;

  // premultiply the alpha
  mask.red   = mask.red   * mask.alpha / 255;
  mask.green = mask.green * mask.alpha / 255;
  mask.blue  = mask.blue  * mask.alpha / 255;

  switch (BitsPerPixel) {
    // 32 bpp --------------------------
    case 32: {
      BGRA* screen = (BGRA*)ddsd.lpSurface;

      // full mask
      if (mask.alpha == 255) {

        BGRA p = { mask.blue, mask.green, mask.red };  // alpha on the destination surface does not matter
        for (int iy = ClippingRectangle.top; iy < ClippingRectangle.bottom; iy++) {
          for (int ix = ClippingRectangle.left; ix < ClippingRectangle.right; ix++) {
            screen[iy * ScreenWidth + ix] = p;
          }
        }

      } else {

        // partial mask
        for (int iy = ClippingRectangle.top; iy < ClippingRectangle.bottom; iy++) {
          for (int ix = ClippingRectangle.left; ix < ClippingRectangle.right; ix++) {
            int i = iy * ScreenWidth + ix;
            screen[i].red   = (mask.red   + screen[i].red    * (256 - mask.alpha)) / 256;
            screen[i].green = (mask.green + screen[i].green  * (256 - mask.alpha)) / 256;
            screen[i].blue  = (mask.blue  + screen[i].blue   * (256 - mask.alpha)) / 256;
          }
        }

      }
      break;
    }

    // 24 bpp --------------------------
    case 24: {
      BGR* screen = (BGR*)ddsd.lpSurface;

      // full mask
      if (mask.alpha == 255) {

        BGR p = { mask.blue, mask.green, mask.red };  // alpha on the destination surface does not matter
        for (int iy = ClippingRectangle.top; iy < ClippingRectangle.bottom; iy++) {
          for (int ix = ClippingRectangle.left; ix < ClippingRectangle.right; ix++) {
            screen[iy * ScreenWidth + ix] = p;
          }
        }

      } else {

        // partial mask
        for (int iy = ClippingRectangle.top; iy < ClippingRectangle.bottom; iy++) {
          for (int ix = ClippingRectangle.left; ix < ClippingRectangle.right; ix++) {
            int i = iy * ScreenWidth + ix;
            screen[i].red   = (mask.red   + screen[i].red    * (256 - mask.alpha)) / 256;
            screen[i].green = (mask.green + screen[i].green  * (256 - mask.alpha)) / 256;
            screen[i].blue  = (mask.blue  + screen[i].blue   * (256 - mask.alpha)) / 256;
          }
        }

      }
      break;
    }

    // 16 bpp --------------------------
    case 16: {
      word* screen = (word*)ddsd.lpSurface;
      
      // full mask
      if (mask.alpha == 255) {

        word p = Pack565(mask);
        for (int iy = ClippingRectangle.top; iy < ClippingRectangle.bottom; iy++) {
          for (int ix = ClippingRectangle.left; ix < ClippingRectangle.right; ix++) {
            screen[iy * ScreenWidth + ix] = p;
          }
        }

      } else {

        // partial mask
        for (int iy = ClippingRectangle.top; iy < ClippingRectangle.bottom; iy++) {
          for (int ix = ClippingRectangle.left; ix < ClippingRectangle.right; ix++) {
            RGBA p = Unpack565(screen[iy * ScreenWidth + ix]);
            p.red   = (mask.red   + p.red    * (256 - mask.alpha)) / 256;
            p.green = (mask.green + p.green  * (256 - mask.alpha)) / 256;
            p.blue  = (mask.blue  + p.blue   * (256 - mask.alpha)) / 256;
            screen[iy * ScreenWidth + ix] = Pack565(p);
          }
        }

      }
      
      break;
    }

    // 15 bpp --------------------------
    case 15: {
      word* screen = (word*)ddsd.lpSurface;
      
      // full mask
      if (mask.alpha == 255) {

        word p = Pack555(mask);
        for (int iy = ClippingRectangle.top; iy < ClippingRectangle.bottom; iy++) {
          for (int ix = ClippingRectangle.left; ix < ClippingRectangle.right; ix++) {
            screen[iy * ScreenWidth + ix] = p;
          }
        }

      } else {

        // partial mask
        for (int iy = ClippingRectangle.top; iy < ClippingRectangle.bottom; iy++) {
          for (int ix = ClippingRectangle.left; ix < ClippingRectangle.right; ix++) {
            RGBA p = Unpack555(screen[iy * ScreenWidth + ix]);
            p.red   = (mask.red   + p.red    * (256 - mask.alpha)) / 256;
            p.green = (mask.green + p.green  * (256 - mask.alpha)) / 256;
            p.blue  = (mask.blue  + p.blue   * (256 - mask.alpha)) / 256;
            screen[iy * ScreenWidth + ix]   = Pack555(p);
          }
        }

      }
      
      break;
    }
  }

  ddSecondary->Unlock(NULL);

  CLOG("-ApplyColorMask")
  return true;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(IMAGE) CreateImage(int width, int height, RGBA* pixels)
{
  CLOG("+CreateImage")

  // allocate the image
  IMAGE image = new IMAGEimp;
  image->width  = width;
  image->height = height;

  FillImagePixels(image, pixels);
  OptimizeBlitRoutine(image);
  CreateSurface(image);

  CLOG("-CreateImage")
  return image;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(IMAGE) GrabImage(int x, int y, int width, int height)
{
  CLOG("+GrabImage")

  if (x < 0 ||
      y < 0 ||
      x + width > ScreenWidth ||
      y + height > ScreenHeight)
    return NULL;

  // create image structure
  IMAGE image = new IMAGEimp;
  image->width        = width;
  image->height       = height;
  image->blit_routine = TileBlit;

  DDSURFACEDESC ddsd;
  if ( GetLockedSurface(&ddsd) == false)
    return false;

  // grab backbuffer pixels and put them in image
  switch (BitsPerPixel) {
    case 32: {
      BGRA* screen = (BGRA*)ddsd.lpSurface;
      image->pixels = new BGRA[width * height];
      for (int iy = 0; iy < height; iy++) {
        memcpy((BGRA*)image->pixels + iy * width,
               screen + (y + iy) * ScreenWidth + x,
               width * 4);
      }
      break;
    }

    case 24: {
      BGR* screen = (BGR*)ddsd.lpSurface;
      image->pixels = new BGR[width * height];
      for (int iy = 0; iy < height; iy++) {
        memcpy((BGR*)image->pixels + iy * width,
               screen + (y + iy) * ScreenWidth + x,
               width * 3);
      }
      break;
    }

    case 16: {
      word* screen = (word*)ddsd.lpSurface;
      image->pixels = new word[width * height];
      for (int iy = 0; iy < height; iy++) {
        memcpy((word*)image->pixels + iy * width,
               screen + (y + iy) * ScreenWidth + x,
               width * 2);
      }
      break;
    }

    case 15: {
      word* screen = (word*)ddsd.lpSurface;
      image->pixels = new word[width * height];
      for (int iy = 0; iy < height; iy++) {
        memcpy((word*)image->pixels + iy * width,
               screen + (y + iy) * ScreenWidth + x,
               width * 2);
      }
    }
  }

  // images grabbed from screen are opaque
  image->alpha = new byte[width * height];
  memset(image->alpha, 255, width * height);

  // unlock backbuffer
  ddSecondary->Unlock(NULL);

  CreateSurface(image);

  CLOG("-GrabImage")
  return image;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(bool) DestroyImage(IMAGE image)
{
  CLOG("+DestroyImage")

  image->surface->Release();
  delete[] (byte*)image->pixels;
  delete[] image->alpha;
  delete image;

  CLOG("-DestroyImage")
  return true;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(bool) BlitImage(IMAGE image, int x, int y)
{
  CLOG("+BlitImage")

  // don't draw it if it's off the screen
  if (x + (int)image->width < ClippingRectangle.left ||
      y + (int)image->height < ClippingRectangle.top ||
      x >= ClippingRectangle.right ||
      y >= ClippingRectangle.bottom)
    return true;

  bool r = image->blit_routine(image, x, y);

  CLOG("-BlitImage")
  return r;
}

////////////////////////////////////////////////////////////////////////////////

bool NullBlit(IMAGE image, int x, int y)
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool TileBlit(IMAGE image, int x, int y)
{
  RECT out = { x, y, x + image->width, y + image->height };
  HRESULT ddrval = ddSecondary->Blt(&out, image->surface, NULL, DDBLT_WAIT, NULL);
  return ddrval == DD_OK;
}

////////////////////////////////////////////////////////////////////////////////

bool SpriteBlit(IMAGE image, int x, int y)
{
  calculate_clipping_metrics(image->width, image->height);

  // lock backbuffer
  DDSURFACEDESC ddsd;
  if ( GetLockedSurface(&ddsd) == false)
    return false;

  switch (BitsPerPixel) {
    case 32: {
      BGRA* screen = (BGRA*)ddsd.lpSurface;
      for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
        for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
          if (image->alpha[iy * image->width + ix])
            screen[(y + iy) * ScreenWidth + (x + ix)] =
              ((BGRA*)image->pixels)[iy * image->width + ix];
      break;
    }

    case 24: {
      BGR* screen = (BGR*)ddsd.lpSurface;
      for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
        for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
          if (image->alpha[iy * image->width + ix])
            screen[(y + iy) * ScreenWidth + (x + ix)] =
              ((BGR*)image->pixels)[iy * image->width + ix];
      break;
    }

    case 16:
    case 15: {
      word* screen = (word*)ddsd.lpSurface;
      for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
        for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
          if (image->alpha[iy * image->width + ix])
            screen[(y + iy) * ScreenWidth + (x + ix)] =
              ((word*)image->pixels)[iy * image->width + ix];
      break;
    }
  }

  ddSecondary->Unlock(NULL);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool NormalBlit(IMAGE image, int x, int y)
{
  calculate_clipping_metrics(image->width, image->height);

  // lock backbuffer
  DDSURFACEDESC ddsd;
  if ( GetLockedSurface(&ddsd) == false)
    return false;

  switch (BitsPerPixel) {
    case 32: {
      for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
        for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
        {
          BGRA* dest  = (BGRA*)ddsd.lpSurface + (y + iy) * ScreenWidth + x + ix;
          BGRA  src   = ((BGRA*)image->pixels)[iy * image->width + ix];
          byte  alpha = image->alpha[iy * image->width + ix];

          if (alpha == 255)
            *dest = src;
          else if (alpha >= 0)
          {
            dest->red   = (dest->red   * (256 - alpha) + src.red   * alpha) / 256;
            dest->green = (dest->green * (256 - alpha) + src.green * alpha) / 256;
            dest->blue  = (dest->blue  * (256 - alpha) + src.blue  * alpha) / 256;
          }
        }

      break;
    }

    case 24: {
      for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
        for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
        {
          BGR* dest  = (BGR*)ddsd.lpSurface + (y + iy) * ScreenWidth + x + ix;
          BGR  src   = ((BGR*)image->pixels)[iy * image->width + ix];
          byte  alpha = image->alpha[iy * image->width + ix];

          if (alpha == 255)
            *dest = src;
          else if (alpha >= 0)
          {
            dest->red   = (dest->red   * (256 - alpha) + src.red   * alpha) / 256;
            dest->green = (dest->green * (256 - alpha) + src.green * alpha) / 256;
            dest->blue  = (dest->blue  * (256 - alpha) + src.blue  * alpha) / 256;
          }
        }

      break;
    }

    case 16: {
      for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
        for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
        {
          word* dest  = (word*)ddsd.lpSurface + (y + iy) * ScreenWidth + x + ix;
          word  src   = ((word*)image->pixels)[iy * image->width + ix];
          byte  alpha = image->alpha[iy * image->width + ix];

          if (alpha == 255)
            *dest = src;
          else if (alpha >= 0)
          {
            RGBA d = Unpack565(*dest);
            RGBA s = Unpack565(src);
            d.red   = (d.red   * (256 - alpha) + s.red   * alpha) / 256;
            d.green = (d.green * (256 - alpha) + s.green * alpha) / 256;
            d.blue  = (d.blue  * (256 - alpha) + s.blue  * alpha) / 256;
            *dest = Pack565(d);
          }
        }

      break;
    }

    case 15: {
      for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
        for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
        {
          word* dest  = (word*)ddsd.lpSurface + (y + iy) * ScreenWidth + x + ix;
          word  src   = ((word*)image->pixels)[iy * image->width + ix];
          byte  alpha = image->alpha[iy * image->width + ix];

          if (alpha == 255)
            *dest = src;
          else if (alpha >= 0)
          {
            RGBA d = Unpack555(*dest);
            RGBA s = Unpack555(src);
            d.red   = (d.red   * (256 - alpha) + s.red   * alpha) / 256;
            d.green = (d.green * (256 - alpha) + s.green * alpha) / 256;
            d.blue  = (d.blue  * (256 - alpha) + s.blue  * alpha) / 256;
            *dest = Pack555(d);
          }
        }

      break;
    }
  }

  ddSecondary->Unlock(NULL);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(int) GetImageWidth(IMAGE image)
{
  return image->width;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(int) GetImageHeight(IMAGE image)
{
  return image->height;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(RGBA*) LockImage(IMAGE image)
{
  image->locked_pixels = new RGBA[image->width * image->height];

  switch (BitsPerPixel) {
    case 32: {
      for (int i = 0; i < image->width * image->height; i++) {
        image->locked_pixels[i].red   = ((BGRA*)image->pixels)[i].red;
        image->locked_pixels[i].green = ((BGRA*)image->pixels)[i].green;
        image->locked_pixels[i].blue  = ((BGRA*)image->pixels)[i].blue;
      }
      break;
    }

    case 24: {
      for (int i = 0; i < image->width * image->height; i++) {
        image->locked_pixels[i].red   = ((BGR*)image->pixels)[i].red;
        image->locked_pixels[i].green = ((BGR*)image->pixels)[i].green;
        image->locked_pixels[i].blue  = ((BGR*)image->pixels)[i].blue;
      }
      break;
    }
    
    case 16: {
      for (int i = 0; i < image->width * image->height; i++) {
        RGBA p = Unpack565(((word*)image->pixels)[i]);
        image->locked_pixels[i].red   = p.red;
        image->locked_pixels[i].green = p.green;
        image->locked_pixels[i].blue  = p.blue;
      }
      break;
    }

    case 15: {
      for (int i = 0; i < image->width * image->height; i++) {
        RGBA p = Unpack555(((word*)image->pixels)[i]);
        image->locked_pixels[i].red   = p.red;
        image->locked_pixels[i].green = p.green;
        image->locked_pixels[i].blue  = p.blue;
      }
      break;
    }
  }

  // alpha
  for (int i = 0; i < image->width * image->height; i++)
    image->locked_pixels[i].alpha = image->alpha[i];

  return image->locked_pixels;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) UnlockImage(IMAGE image)
{
  delete[] (byte*)image->pixels;
  delete[] image->alpha;
  
  FillImagePixels(image, image->locked_pixels);
  OptimizeBlitRoutine(image);
  delete[] image->locked_pixels;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DirectBlit(int x, int y, int w, int h, RGBA* pixels, int method)
{
  if (method == 0)
    return;

  calculate_clipping_metrics(w, h);

  // lock backbuffer
  DDSURFACEDESC ddsd;
  if ( GetLockedSurface(&ddsd) == false)
    return;

  if (method == 1) {   // RGB

    switch (BitsPerPixel) {
      case 32: {
        BGRA* screen = (BGRA*)ddsd.lpSurface;
        for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
          for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
            if (pixels[iy * w + ix].alpha) {
              RGBA s = pixels[iy * w + ix];
              BGRA p = { s.blue, s.green, s.red };
              screen[(y + iy) * ScreenWidth + (x + ix)] = p;
            }
        break;
      }

      case 24: {
        BGR* screen = (BGR*)ddsd.lpSurface;
        for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
          for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
            if (pixels[iy * w + ix].alpha) {
              RGBA s = pixels[iy * w + ix];
              BGR p = { s.blue, s.green, s.red };
              screen[(y + iy) * ScreenWidth + (x + ix)] = p;
            }
        break;
      }

      case 16: {
        word* screen = (word*)ddsd.lpSurface;
        for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
          for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
            if (pixels[iy * w + ix].alpha) {
              word p = Pack555(pixels[iy * w + ix]);
              screen[(y + iy) * ScreenWidth + (x + ix)] = p;
            }
        break;
      }

      case 15: {
        word* screen = (word*)ddsd.lpSurface;
        for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
          for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
            if (pixels[iy * w + ix].alpha) {
              word p = Pack555(pixels[iy * w + ix]);
              screen[(y + iy) * ScreenWidth + (x + ix)] = p;
            }
        break;
      }
    }

  } else if (method == 2) {  // RGBA

    switch (BitsPerPixel) {
      case 32: {
        for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
          for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
          {
            BGRA* dest  = (BGRA*)ddsd.lpSurface + (y + iy) * ScreenWidth + x + ix;
            RGBA  src   = pixels[iy * w + ix];

            if (src.alpha == 255)
            {
              dest->red   = src.red;
              dest->green = src.green;
              dest->blue  = src.blue;
            }
            else if (src.alpha >= 0)
            {
              dest->red   = (dest->red   * (256 - src.alpha) + src.red   * src.alpha) / 256;
              dest->green = (dest->green * (256 - src.alpha) + src.green * src.alpha) / 256;
              dest->blue  = (dest->blue  * (256 - src.alpha) + src.blue  * src.alpha) / 256;
            }
          }

        break;
      }

      case 24: {
        for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
          for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
          {
            BGR* dest  = (BGR*)ddsd.lpSurface + (y + iy) * ScreenWidth + x + ix;
            RGBA src   = pixels[iy * w + ix];

            if (src.alpha == 255)
            {
              dest->red   = src.red;
              dest->green = src.green;
              dest->blue  = src.blue;
            }
            else if (src.alpha >= 0)
            {
              dest->red   = (dest->red   * (256 - src.alpha) + src.red   * src.alpha) / 256;
              dest->green = (dest->green * (256 - src.alpha) + src.green * src.alpha) / 256;
              dest->blue  = (dest->blue  * (256 - src.alpha) + src.blue  * src.alpha) / 256;
            }
          }

        break;
      }

      case 16: {
        for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
          for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
          {
            word* dest  = (word*)ddsd.lpSurface + (y + iy) * ScreenWidth + x + ix;
            RGBA  src   = pixels[iy * w + ix];

            if (src.alpha == 255)
              *dest = Pack565(src);
            else if (src.alpha >= 0)
            {
              RGBA d = Unpack565(*dest);
              d.red   = (d.red   * (256 - src.alpha) + src.red   * src.alpha) / 256;
              d.green = (d.green * (256 - src.alpha) + src.green * src.alpha) / 256;
              d.blue  = (d.blue  * (256 - src.alpha) + src.blue  * src.alpha) / 256;
              *dest = Pack565(d);
            }
          }

        break;
      }

      case 15: {
        for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
          for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
          {
            word* dest  = (word*)ddsd.lpSurface + (y + iy) * ScreenWidth + x + ix;
            RGBA  src   = pixels[iy * w + ix];

            if (src.alpha == 255)
              *dest = Pack555(src);
            else if (src.alpha >= 0)
            {
              RGBA d = Unpack555(*dest);
              d.red   = (d.red   * (256 - src.alpha) + src.red   * src.alpha) / 256;
              d.green = (d.green * (256 - src.alpha) + src.green * src.alpha) / 256;
              d.blue  = (d.blue  * (256 - src.alpha) + src.blue  * src.alpha) / 256;
              *dest = Pack555(d);
            }
          }

        break;
      }
    }

  } // end if

  ddSecondary->Unlock(NULL);
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DirectGrab(int x, int y, int w, int h, RGBA* pixels)
{
  if (x < 0 ||
      y < 0 ||
      x + w > ScreenWidth ||
      y + h > ScreenHeight)
    return;

  // lock backbuffer
  DDSURFACEDESC ddsd;
  if ( GetLockedSurface(&ddsd) == false)
    return;

  switch (BitsPerPixel) {
    case 32: {
      BGRA* screen = (BGRA*)ddsd.lpSurface;
      for (int iy = 0; iy < h; iy++)
        for (int ix = 0; ix < w; ix++)
        {
          pixels[iy * w + ix].red   = screen[(y + iy) * ScreenWidth + x + ix].red;
          pixels[iy * w + ix].green = screen[(y + iy) * ScreenWidth + x + ix].green;
          pixels[iy * w + ix].blue  = screen[(y + iy) * ScreenWidth + x + ix].blue;
          pixels[iy * w + ix].alpha = 255;
        }
      break;
    }

    case 24: {
      BGR* screen = (BGR*)ddsd.lpSurface;
      for (int iy = 0; iy < h; iy++)
        for (int ix = 0; ix < w; ix++)
        {
          pixels[iy * w + ix].red   = screen[(y + iy) * ScreenWidth + x + ix].red;
          pixels[iy * w + ix].green = screen[(y + iy) * ScreenWidth + x + ix].green;
          pixels[iy * w + ix].blue  = screen[(y + iy) * ScreenWidth + x + ix].blue;
          pixels[iy * w + ix].alpha = 255;
        }
      break;
    }

    case 16: {
      word* screen = (word*)ddsd.lpSurface;
      for (int iy = 0; iy < h; iy++) {
        for (int ix = 0; ix < w; ix++) {
          pixels[iy * w + ix] = Unpack565(screen[(y + iy) * ScreenWidth + x + ix]);
        }
      }
      break;
    }

    case 15: {
      word* screen = (word*)ddsd.lpSurface;
      for (int iy = 0; iy < h; iy++) {
        for (int ix = 0; ix < w; ix++) {
          pixels[iy * w + ix] = Unpack555(screen[(y + iy) * ScreenWidth + x + ix]);
        }
      }
      break;
    }
  }

  ddSecondary->Unlock(NULL);
}

////////////////////////////////////////////////////////////////////////////////

inline void blendBGRA(BGRA& dest, RGBA source) {
  Blend3(dest, source, source.alpha);
}

// todo: make these do something...

EXPORT(void) BlitImageMask(IMAGE image, int x, int y, RGBA mask) { }

EXPORT(void) DirectTransformBlit(int x[4], int y[4], int w, int h, RGBA* pixels) { }


///////////////////////////////////////////////////////////////////////////////

void aBlendBGR(struct BGR& d, struct BGR s, int a)
{
  // blit to the dest pixel
  d.red   = (d.red   * (256 - a)) / 256 + s.red;
  d.green = (d.green * (256 - a)) / 256 + s.green;
  d.blue  = (d.blue  * (256 - a)) / 256 + s.blue;
}

void aBlendBGRA(struct BGRA& d, struct BGRA s, int a)
{
  // blit to the dest pixel
  d.red   = (d.red   * (256 - a)) / 256 + s.red;
  d.green = (d.green * (256 - a)) / 256 + s.green;
  d.blue  = (d.blue  * (256 - a)) / 256 + s.blue;
}

///////////////////////////////////////////////////////////////////////////////

EXPORT(void) TransformBlitImage(IMAGE image, int x[4], int y[4]) {

  // lock backbuffer
  DDSURFACEDESC ddsd;
  if ( GetLockedSurface(&ddsd) == false)
    return;

  if (BitsPerPixel == 32) {
    BGRA* screen_buffer = (BGRA*)ddsd.lpSurface;
    primitives::TexturedQuad(
      (BGRA*)screen_buffer,
      ScreenWidth,
      x,
      y,
      (BGRA*) image->locked_pixels,
      image->alpha,
      image->width,
      image->height,
      ClippingRectangle,
      aBlendBGRA
    );
  }

  ddSecondary->Unlock(NULL);
}

///////////////////////////////////////////////////////////////////////////////


template<typename pixelT>
class render_pixel_mask
{
public:
  render_pixel_mask(RGBA mask) : m_mask(mask) { }
  void operator()(pixelT& dst, pixelT src, byte alpha)
  {
    // do the masking on the source pixel
    alpha     = (int)alpha     * m_mask.alpha / 256;
    src.red   = (int)src.red   * m_mask.red   / 256;
    src.green = (int)src.green * m_mask.green / 256;
    src.blue  = (int)src.blue  * m_mask.blue  / 256;

    // blit to the dest pixel
    dst.red   = (dst.red   * (256 - alpha) + src.red   * alpha) / 256;
    dst.green = (dst.green * (256 - alpha) + src.green * alpha) / 256;
    dst.blue  = (dst.blue  * (256 - alpha) + src.blue  * alpha) / 256;
  }

private:
  RGBA m_mask;
};

EXPORT(void) TransformBlitImageMask(IMAGE image, int x[4], int y[4], RGBA mask) {

  // lock backbuffer
  DDSURFACEDESC ddsd;
  if ( GetLockedSurface(&ddsd) == false)
    return;

  if (BitsPerPixel == 32) {
    BGRA* screen_buffer = (BGRA*)ddsd.lpSurface;
    primitives::TexturedQuad(
      (BGRA*)screen_buffer,
      ScreenWidth,
      x,
      y,
      (BGRA*) image->locked_pixels,
      image->alpha,
      image->width,
      image->height,
      ClippingRectangle,
      render_pixel_mask<BGRA>(mask)
    );
  }

  ddSecondary->Unlock(NULL);
}

EXPORT(void) DrawPoint(int x, int y, RGBA color) { }
EXPORT(void) DrawLine(int x[2], int y[2], RGBA color) { }
EXPORT(void) DrawGradientLine(int x[2], int y[2], RGBA colors[2]) { }
EXPORT(void) DrawTriangle(int x[3], int y[3], RGBA color) { }
EXPORT(void) DrawGradientTriangle(int x[3], int y[3], RGBA colors[3]) { }

EXPORT(void) DrawRectangle(int x, int y, int w, int h, RGBA color) {
  if (color.alpha == 0) {          // no mask
    return;
  } else {

  // lock backbuffer
  DDSURFACEDESC ddsd;
  if ( GetLockedSurface(&ddsd) == false)
    return;

    switch (BitsPerPixel) {
      case 32:
        BGRA* screen_buffer = (BGRA*)ddsd.lpSurface;
        primitives::Rectangle((BGRA*)screen_buffer, ScreenWidth, x, y, w, h, color, ClippingRectangle, blendBGRA);
      break;
    }

  }

  ddSecondary->Unlock(NULL);
}

EXPORT(void) DrawGradientRectangle(int x, int y, int w, int h, RGBA colors[4]) { }


////////////////////////////////////////////////////////////////////////////////