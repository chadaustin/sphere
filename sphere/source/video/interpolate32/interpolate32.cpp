#define DIRECTDRAW_VERSION 0x0300
#include <windows.h>
#include <ddraw.h>
#include <stdio.h>
#include "../../common/rgb.hpp"
#include "../../common/primitives.hpp"
#include "../common/video.hpp"
#include "../common/win32x.hpp"
#include "resource.h"


#ifdef USE_ALPHA_TABLE
 unsigned char alpha_new[256][256]={
  #include "../../common/alpha_new.table"
 };

 unsigned char alpha_old[256][256]={
  #include "../../common/alpha_old.table"
 };
#endif


typedef struct _IMAGE
{
  int width;
  int height;

  union
  {
    BGRA* bgra;
    BGR*  bgr;
  };
  byte* alpha;

  void (*blit_routine)(_IMAGE* image, int x, int y);

  RGBA* locked_pixels;
}* IMAGE;


enum BIT_DEPTH
{
  BD_AUTODETECT,
  BD_32,
  BD_24,
};

enum INTERPOLATOR
{
  I_CENTER      = 0,
  I_DIRECTSCALE = 1,
  I_SMOOTH      = 2,
  I_TVMODE      = 3,
};

struct CONFIGURATION
{
  BIT_DEPTH bit_depth;

  bool fullscreen;
  bool vsync;
  int  interpolator;
};


static void LoadConfiguration();
static void SaveConfiguration();
static BOOL CALLBACK ConfigureDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

static bool InitFullscreen();
static bool SetDisplayMode();
static bool CreateSurfaces();
static bool InitWindowed();

static void CloseFullscreen();
static void CloseWindowed();

static void Interpolate(void* dest, int pitch);

static void FillImagePixels(IMAGE image, RGBA* data);
static void OptimizeBlitRoutine(IMAGE image);

static void NullBlit(IMAGE image, int x, int y);
static void TileBlit(IMAGE image, int x, int y);
static void SpriteBlit(IMAGE image, int x, int y);
static void NormalBlit(IMAGE image, int x, int y);


static CONFIGURATION Configuration;
static int           BitsPerPixel;

static HWND  SphereWindow;
static byte* ScreenBuffer;

static LONG OldWindowStyle;
static LONG OldWindowStyleEx;

// windowed output
static HDC     RenderDC;
static HBITMAP RenderBitmap;
static void*   RenderBuffer;

// fullscreen output
static LPDIRECTDRAW dd;
static LPDIRECTDRAWSURFACE ddPrimary;
static LPDIRECTDRAWSURFACE ddSecondary;


////////////////////////////////////////////////////////////////////////////////

EXPORT(void) GetDriverInfo(DRIVERINFO* driverinfo)
{
  driverinfo->name        = "Interpolated 32-bit Color";
  driverinfo->author      = "Chad Austin";
  driverinfo->date        = __DATE__;
  driverinfo->version     = "1.00";
  driverinfo->description = "24/32-bit color output in both windowed and fullscreen modes (2x interpolated)";
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
  int bit_depth = GetPrivateProfileInt("interpolate32", "BitDepth", 0, config_file_name);
  Configuration.bit_depth = (bit_depth == 32 ? BD_32 : (bit_depth == 24 ? BD_24 : BD_AUTODETECT));

  Configuration.fullscreen   = (GetPrivateProfileInt("interpolate32", "Fullscreen",   1, config_file_name) != 0);
  Configuration.vsync        = (GetPrivateProfileInt("interpolate32", "VSync",        1, config_file_name) != 0);
  Configuration.interpolator = GetPrivateProfileInt("interpolate32", "Interpolator", 0, config_file_name);
}

////////////////////////////////////////////////////////////////////////////////

void SaveConfiguration()
{
  char config_file_name[MAX_PATH];
  GetDriverConfigFile(config_file_name);

  // save the fields to the file
  int bit_depth = (Configuration.bit_depth == BD_32 ? 32 : (Configuration.bit_depth == BD_24 ? 24 : 0));
  WritePrivateProfileInt("interpolate32", "BitDepth",     bit_depth,                  config_file_name);
  WritePrivateProfileInt("interpolate32", "Fullscreen",   Configuration.fullscreen,   config_file_name);
  WritePrivateProfileInt("interpolate32", "VSync",        Configuration.vsync,        config_file_name);
  WritePrivateProfileInt("interpolate32", "Interpolator", Configuration.interpolator, config_file_name);
}

////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK ConfigureDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  switch (message)
  {
    case WM_INITDIALOG:
      // set the bit depth radio buttons
      if (Configuration.bit_depth == BD_AUTODETECT)
        SendDlgItemMessage(window, IDC_BITDEPTH_AUTODETECT, BM_SETCHECK, BST_CHECKED, 0);
      else if (Configuration.bit_depth == BD_32)
        SendDlgItemMessage(window, IDC_BITDEPTH_32, BM_SETCHECK, BST_CHECKED, 0);
      else if (Configuration.bit_depth == BD_24)
        SendDlgItemMessage(window, IDC_BITDEPTH_24, BM_SETCHECK, BST_CHECKED, 0);

      // set the check boxes
      CheckDlgButton(window, IDC_FULLSCREEN, Configuration.fullscreen ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(window, IDC_VSYNC,      Configuration.vsync      ? BST_CHECKED : BST_UNCHECKED);

      // set the interpolation engine
      switch (Configuration.interpolator)
      {
        case I_CENTER:      CheckDlgButton(window, IDC_CENTER,      BST_CHECKED); break;
        case I_DIRECTSCALE: CheckDlgButton(window, IDC_DIRECTSCALE, BST_CHECKED); break;
        case I_SMOOTH:      CheckDlgButton(window, IDC_SMOOTH,      BST_CHECKED); break;
        case I_TVMODE:      CheckDlgButton(window, IDC_TVMODE,      BST_CHECKED); break;
      }

      // update the check states
      SendMessage(window, WM_COMMAND, MAKEWPARAM(IDC_FULLSCREEN, BN_PUSHED), 0);

      return TRUE;

    ////////////////////////////////////////////////////////////////////////////

    case WM_COMMAND:
      switch (LOWORD(wparam))
      {
        case IDOK:
          if (IsDlgButtonChecked(window, IDC_BITDEPTH_32))
            Configuration.bit_depth = BD_32;
          else if (IsDlgButtonChecked(window, IDC_BITDEPTH_24))
            Configuration.bit_depth = BD_24;
          else
            Configuration.bit_depth = BD_AUTODETECT;

          Configuration.fullscreen = (IsDlgButtonChecked(window, IDC_FULLSCREEN) != FALSE);
          Configuration.vsync      = (IsDlgButtonChecked(window, IDC_VSYNC)      != FALSE);

          if (IsDlgButtonChecked(window, IDC_CENTER))
            Configuration.interpolator = I_CENTER;
          else if (IsDlgButtonChecked(window, IDC_DIRECTSCALE))
            Configuration.interpolator = I_DIRECTSCALE;
          else if (IsDlgButtonChecked(window, IDC_SMOOTH))
            Configuration.interpolator = I_SMOOTH;
          else if (IsDlgButtonChecked(window, IDC_TVMODE))
            Configuration.interpolator = I_TVMODE;

          EndDialog(window, 1);
          return TRUE;

        case IDCANCEL:
          EndDialog(window, 0);
          return TRUE;

        case IDC_FULLSCREEN:
          EnableWindow(GetDlgItem(window, IDC_VSYNC), IsDlgButtonChecked(window, IDC_FULLSCREEN));
          EnableWindow(GetDlgItem(window, IDC_BITDEPTH_AUTODETECT), IsDlgButtonChecked(window, IDC_FULLSCREEN));
          if (IsDlgButtonChecked(window, IDC_BITDEPTH_AUTODETECT) && !IsDlgButtonChecked(window, IDC_FULLSCREEN))
          {
            CheckDlgButton(window, IDC_BITDEPTH_AUTODETECT, BST_UNCHECKED);
            CheckDlgButton(window, IDC_BITDEPTH_32,         BST_CHECKED);
          }
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
  SphereWindow = window;
  ScreenWidth  = screen_width;
  ScreenHeight = screen_height;

  // set default clipping rectangle
  SetClippingRectangle(0, 0, screen_width, screen_height);

  LoadConfiguration();

  bool retval;
  if (Configuration.fullscreen)
    retval = InitFullscreen();
  else
    retval = InitWindowed();
  if (!retval)
    return false;

  // allocate a blitting buffer
  ScreenBuffer = new byte[ScreenWidth * ScreenHeight * (BitsPerPixel / 8)];

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool InitFullscreen()
{
  HRESULT ddrval;
  bool    retval;

  // store old window styles
  OldWindowStyle = GetWindowLong(SphereWindow, GWL_STYLE);
  OldWindowStyleEx = GetWindowLong(SphereWindow, GWL_EXSTYLE);

  SetWindowLong(SphereWindow, GWL_STYLE, WS_POPUP);
  SetWindowLong(SphereWindow, GWL_EXSTYLE, 0);

  // create DirectDraw object
  ddrval = DirectDrawCreate(NULL, &dd, NULL);
  if (ddrval != DD_OK)
  {
    MessageBox(SphereWindow, "DirectDrawCreate() failed", "interpolate32", MB_OK);
    return false;
  }

  // set application behavior
  ddrval = dd->SetCooperativeLevel(SphereWindow, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
  if (ddrval != DD_OK)
  {
    dd->Release();
    MessageBox(SphereWindow, "SetCooperativeLevel() failed", "interpolate32", MB_OK);
    return false;
  }

  // set display mode
  retval = SetDisplayMode();
  if (retval == false)
  {
    dd->Release();
    MessageBox(SphereWindow, "SetDisplayMode() failed", "interpolate32", MB_OK);
    return false;
  }

  // create surfaces
  retval = CreateSurfaces();
  if (retval == false)
  {
    dd->Release();
    MessageBox(SphereWindow, "CreateSurfaces() failed", "interpolate32", MB_OK);
    return false;
  }

  ShowCursor(FALSE);

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

  switch (Configuration.bit_depth)
  {
    case BD_AUTODETECT:
      ddrval = dd->SetDisplayMode(ScreenWidth * 2, ScreenHeight * 2, 32);
      BitsPerPixel = 32;
      if (ddrval != DD_OK)
      {
        ddrval = dd->SetDisplayMode(ScreenWidth * 2, ScreenHeight * 2, 24);
        BitsPerPixel = 24;
      }
      return ddrval == DD_OK;

    case BD_32:
      ddrval = dd->SetDisplayMode(ScreenWidth * 2, ScreenHeight * 2, 32);
      BitsPerPixel = 32;
      return ddrval == DD_OK;

    case BD_24:
      ddrval = dd->SetDisplayMode(ScreenWidth * 2, ScreenHeight * 2, 24);
      BitsPerPixel = 24;
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
  ddsd.dwSize = sizeof(ddsd);
  
  if (Configuration.vsync)
  {
    ddsd.dwFlags           = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps    = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
    ddsd.dwBackBufferCount = 1;
  }
  else
  {
    ddsd.dwFlags        = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
  }

  // create the primary surface
  HRESULT ddrval = dd->CreateSurface(&ddsd, &ddPrimary, NULL);
  if (ddrval != DD_OK)
    return false;

  if (Configuration.vsync)
  {
    ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
    ddrval = ddPrimary->GetAttachedSurface(&ddsd.ddsCaps, &ddSecondary);
    if (ddrval != DD_OK)
    {
      ddPrimary->Release();
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool InitWindowed()
{
  // calculate bits per pixel
  BitsPerPixel = (Configuration.bit_depth == BD_32 ? 32 : 24);

  // create the render DC
  RenderDC = CreateCompatibleDC(NULL);
  if (RenderDC == NULL)
    return false;

  // define/create the render DIB section
  BITMAPINFO bmi;
  memset(&bmi, 0, sizeof(bmi));
  BITMAPINFOHEADER& bmih = bmi.bmiHeader;
  bmih.biSize        = sizeof(bmih);
  bmih.biWidth       = ScreenWidth * 2;
  bmih.biHeight      = -ScreenHeight * 2;
  bmih.biPlanes      = 1;
  bmih.biBitCount    = BitsPerPixel;
  bmih.biCompression = BI_RGB;
  RenderBitmap = CreateDIBSection(RenderDC, &bmi, DIB_RGB_COLORS, (void**)&RenderBuffer, NULL, 0);
  if (RenderBitmap == NULL)
  {
    DeleteDC(RenderDC);
    return false;
  }

  SelectObject(RenderDC, RenderBitmap);

  // center the window
  RECT WindowRect = { 0, 0, ScreenWidth * 2, ScreenHeight * 2 };
  AdjustWindowRectEx(
    &WindowRect,
    GetWindowLong(SphereWindow, GWL_STYLE),
    (GetMenu(SphereWindow) ? TRUE : FALSE),
    GetWindowLong(SphereWindow, GWL_EXSTYLE));

  int window_width  = WindowRect.right - WindowRect.left;
  int window_height = WindowRect.bottom - WindowRect.top;

  MoveWindow(
    SphereWindow,
    (GetSystemMetrics(SM_CXSCREEN) - window_width) / 2,
    (GetSystemMetrics(SM_CYSCREEN) - window_height) / 2,
    window_width,
    window_height,
    TRUE);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) CloseVideoDriver()
{
  if (Configuration.fullscreen)
    CloseFullscreen();
  else
    CloseWindowed();
}

////////////////////////////////////////////////////////////////////////////////

void CloseFullscreen()
{
  SetWindowLong(SphereWindow, GWL_STYLE, OldWindowStyle);
  SetWindowLong(SphereWindow, GWL_EXSTYLE, OldWindowStyleEx);

  ShowCursor(TRUE);
  delete[] ScreenBuffer;
  dd->Release();
}

////////////////////////////////////////////////////////////////////////////////

void CloseWindowed()
{
  DeleteDC(RenderDC);
  DeleteObject(RenderBitmap);
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) FlipScreen()
{
  if (Configuration.fullscreen)
  {
    LPDIRECTDRAWSURFACE surface;
    if (Configuration.vsync)
      surface = ddSecondary;
    else
      surface = ddPrimary;

    // lock the surface
    DDSURFACEDESC ddsd;
    ddsd.dwSize = sizeof(ddsd);
    HRESULT ddrval = surface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);

    // if the surface was lost, restore it
    if (ddrval == DDERR_SURFACELOST)
    {
      surface->Restore();
      if (surface == ddSecondary)
        ddPrimary->Restore();

      // attempt to lock again
      ddrval = surface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
      if (ddrval != DD_OK)
      {
        Sleep(100);
        return;
      }
    }

    Interpolate(ddsd.lpSurface, ddsd.lPitch);

    // unlock the surface and do the flip!
    surface->Unlock(NULL);
    if (Configuration.vsync)
      ddPrimary->Flip(NULL, DDFLIP_WAIT);
  }
  else
  {
    // make sure the lines are on dword boundaries
    int bytes_per_pixel = BitsPerPixel / 8;
    int pitch = ((ScreenWidth * 2) * bytes_per_pixel + bytes_per_pixel - 1) / 4 * 4;
    Interpolate(RenderBuffer, pitch);

    // blit the render buffer to the window
    HDC dc = GetDC(SphereWindow);
    BitBlt(dc, 0, 0, ScreenWidth * 2, ScreenHeight * 2, RenderDC, 0, 0, SRCCOPY);
    ReleaseDC(SphereWindow, dc);
  }
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
void Interpolate_Center(T* dest, int pitch)
{
  // top quarter
  memset(dest, 0, pitch * (ScreenHeight / 4) * sizeof(T));

  // bottom quarter
  memset(dest + pitch * ScreenHeight * 6 / 4, 0, pitch * (ScreenHeight / 4) * sizeof(T));

  
  T* dst = dest + pitch * ScreenHeight / 2;
  T* src = (T*)ScreenBuffer;

  int i = ScreenHeight;
  while (i--)
  {
    // clear left and right sides
    memset(dst, 0, ScreenWidth / 4 * sizeof(T));
    memset(dst + ScreenWidth * 6 / 4, 0, ScreenWidth / 4 * sizeof(T));

    memcpy(dst + ScreenWidth / 2, src, ScreenWidth * sizeof(T));

    src += ScreenWidth;
    dst += pitch;
  }
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
void Interpolate_DirectScale(T* dest, int pitch)
{
  T* src = (T*)ScreenBuffer;
  int i = ScreenHeight;
  while (i--)
  {
    T* line1 = dest;
    T* line2 = dest + pitch;
    int j = ScreenWidth;
    while (j--)
    {
      *line1++ = *src;
      *line1++ = *src;
      *line2++ = *src;
      *line2++ = *src;
      src++;
    }
    dest += pitch * 2;
  }
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
void Interpolate_Smooth(T* dest, int pitch)
{
  T* src = (T*)ScreenBuffer;
  int i = ScreenHeight - 1; // don't draw last row
  while (i--)
  {
    T* line1 = dest;
    T* line2 = dest + pitch;
    int j = ScreenWidth - 1; // don't draw last column
    while (j--)
    {
      T average;

      // block is [ab]
      //          [cd]

      // a
      *line1++ = *src;

      average.red   = (src->red   + (src + 1)->red)   / 2;
      average.green = (src->green + (src + 1)->green) / 2;
      average.blue  = (src->blue  + (src + 1)->blue)  / 2;
      *line1++ = average;

      average.red   = (src->red   + (src + ScreenWidth)->red)   / 2;
      average.green = (src->green + (src + ScreenWidth)->green) / 2;
      average.blue  = (src->blue  + (src + ScreenWidth)->blue)  / 2;
      *line2++ = average;

      average.red   = (src->red   + (src + ScreenWidth + 1)->red)   / 2;
      average.green = (src->green + (src + ScreenWidth + 1)->green) / 2;
      average.blue  = (src->blue  + (src + ScreenWidth + 1)->blue)  / 2;
      *line2++ = average;

      src++;
    }

    // make up for the lost column
    src++;
    dest += pitch * 2;
  }
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
void Interpolate_TVMode(T* dest, int pitch)
{
  T* src = (T*)ScreenBuffer;
  int i = ScreenHeight;
  while (i--)
  {
    T* line1 = dest;
    T* line2 = dest + pitch;
    int j = ScreenWidth - 1; // skip last column
    while (j--)
    {
      T a = *src;
      T b;
      b.red   = (src->red   + (src + 1)->red)   / 2;
      b.green = (src->green + (src + 1)->green) / 2;
      b.blue  = (src->blue  + (src + 1)->blue)  / 2;

      *line1++ = a;
      *line1++ = b;

      a.red   /= 2;
      a.green /= 2;
      a.blue  /= 2;

      b.red   /= 2;
      b.green /= 2;
      b.blue  /= 2;

      *line2++ = a;
      *line2++ = b;

      src++;
    }

    // make up for last column
    src++;
    dest += pitch * 2;
  }
}

////////////////////////////////////////////////////////////////////////////////

void Interpolate(void* dest, int pitch)
{
  if (BitsPerPixel == 32)
  {
    switch (Configuration.interpolator)
    {
      case I_CENTER:      Interpolate_Center     ((BGRA*)dest, pitch / 4); break;
      case I_DIRECTSCALE: Interpolate_DirectScale((BGRA*)dest, pitch / 4); break;
      case I_SMOOTH:      Interpolate_Smooth     ((BGRA*)dest, pitch / 4); break;
      case I_TVMODE:      Interpolate_TVMode     ((BGRA*)dest, pitch / 4); break;
    }
  }
  else if (BitsPerPixel == 24)
  {
    switch (Configuration.interpolator)
    {
      case I_CENTER:      Interpolate_Center     ((BGR*)dest, pitch / 3); break;
      case I_DIRECTSCALE: Interpolate_DirectScale((BGR*)dest, pitch / 3); break;
      case I_SMOOTH:      Interpolate_Smooth     ((BGR*)dest, pitch / 3); break;
      case I_TVMODE:      Interpolate_TVMode     ((BGR*)dest, pitch / 3); break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(IMAGE) CreateImage(int width, int height, RGBA* pixels)
{
  // allocate the image
  IMAGE image = new _IMAGE;
  image->width  = width;
  image->height = height;

  FillImagePixels(image, pixels);
  OptimizeBlitRoutine(image);
  return image;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(IMAGE) GrabImage(int x, int y, int width, int height)
{
  if (x < 0 ||
      y < 0 ||
      x + width > ScreenWidth ||
      y + height > ScreenHeight)
    return NULL;

  IMAGE image = new _IMAGE;
  image->width        = width;
  image->height       = height;
  image->blit_routine = TileBlit;

  if (BitsPerPixel == 32)
  {
    BGRA* Screen = (BGRA*)ScreenBuffer;
    image->bgra = new BGRA[width * height];
    for (int iy = 0; iy < height; iy++)
      memcpy(image->bgra + iy * width,
             Screen + (y + iy) * ScreenWidth + x,
             width * 4);
  }
  else
  {
    BGR* Screen = (BGR*)ScreenBuffer;
    image->bgr = new BGR[width * height];
    for (int iy = 0; iy < height; iy++)
      memcpy(image->bgr + iy * width,
             Screen + (y + iy) * ScreenWidth + x,
             width * 3);
  }

  image->alpha = new byte[width * height];
  memset(image->alpha, 255, width * height);

  return image;
}

////////////////////////////////////////////////////////////////////////////////

void FillImagePixels(IMAGE image, RGBA* pixels)
{
  // fill the image pixels
  if (BitsPerPixel == 32)
  {
    image->bgra = new BGRA[image->width * image->height];
    for (int i = 0; i < image->width * image->height; i++)
    {
      image->bgra[i].red   = pixels[i].red;
      image->bgra[i].green = pixels[i].green;
      image->bgra[i].blue  = pixels[i].blue;
    }
  }
  else
  {
    image->bgr  = new BGR[image->width * image->height];
    for (int i = 0; i < image->width * image->height; i++)
    {
      image->bgr[i].red   = pixels[i].red;
      image->bgr[i].green = pixels[i].green;
      image->bgr[i].blue  = pixels[i].blue;
    }
  }

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

EXPORT(void) DestroyImage(IMAGE image)
{
  if (BitsPerPixel == 32)
    delete[] image->bgra;
  else
    delete[] image->bgr;
  delete[] image->alpha;
  delete image;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) BlitImage(IMAGE image, int x, int y)
{
  // don't draw it if it's off the screen
  if (x + (int)image->width < ClippingRectangle.left ||
      y + (int)image->height < ClippingRectangle.top ||
      x > ClippingRectangle.right ||
      y > ClippingRectangle.bottom)
    return;

  image->blit_routine(image, x, y);
}

////////////////////////////////////////////////////////////////////////////////

template<typename pixelT>
class render_pixel_mask
{
public:
  render_pixel_mask(RGBA mask) : m_mask(mask) { }
  void operator()(pixelT& dst, pixelT src, byte alpha)
  {
    // do the masking on the source pixel
    alpha     = alpha     * m_mask.alpha / 256;
    src.red   = src.red   * m_mask.red   / 256;
    src.green = src.green * m_mask.green / 256;
    src.blue  = src.blue  * m_mask.blue  / 256;

    // blit to the dest pixel
    dst.red   = (dst.red   * (256 - alpha) + src.red   * alpha) / 256;
    dst.green = (dst.green * (256 - alpha) + src.green * alpha) / 256;
    dst.blue  = (dst.blue  * (256 - alpha) + src.blue  * alpha) / 256;
  }

private:
  RGBA m_mask;
};


EXPORT(void) BlitImageMask(IMAGE image, int x, int y, RGBA mask)
{
  if (BitsPerPixel == 32) {

    primitives::Blit(
      (BGRA*)ScreenBuffer,
      ScreenWidth,
      x,
      y,
      image->bgra,
      image->alpha,
      image->width,
      image->height,
      ClippingRectangle,
      render_pixel_mask<BGRA>(mask)
    );

  } else {

    primitives::Blit(
      (BGR*)ScreenBuffer,
      ScreenWidth,
      x,
      y,
      image->bgr,
      image->alpha,
      image->width,
      image->height,
      ClippingRectangle,
      render_pixel_mask<BGR>(mask)
    );

  }
}

////////////////////////////////////////////////////////////////////////////////

void aBlendBGR(struct BGR& d, struct BGR s, int a)
{
  Blend3(d, s, a);
}

void aBlendBGRA(struct BGRA& d, struct BGRA s, int a)
{
  Blend3(d, s, a);
}

EXPORT(void) TransformBlitImage(IMAGE image, int x[4], int y[4])
{
  if (BitsPerPixel == 32) {
    primitives::TexturedQuad(
      (BGRA*)ScreenBuffer,
      ScreenWidth,
      x,
      y,
      image->bgra,
      image->alpha,
      image->width,
      image->height,
      ClippingRectangle,
      aBlendBGRA
    );
  } else {
    primitives::TexturedQuad(
      (BGR*)ScreenBuffer,
      ScreenWidth,
      x,
      y,
      image->bgr,
      image->alpha,
      image->width,
      image->height,
      ClippingRectangle,
      aBlendBGR
    );
  }
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) TransformBlitImageMask(IMAGE image, int x[4], int y[4], RGBA mask)
{
  if (BitsPerPixel == 32) {
    primitives::TexturedQuad(
      (BGRA*)ScreenBuffer,
      ScreenWidth,
      x,
      y,
      image->bgra,
      image->alpha,
      image->width,
      image->height,
      ClippingRectangle,
      render_pixel_mask<BGRA>(mask)
    );
  } else {
    primitives::TexturedQuad(
      (BGR*)ScreenBuffer,
      ScreenWidth,
      x,
      y,
      image->bgr,
      image->alpha,
      image->width,
      image->height,
      ClippingRectangle,
      render_pixel_mask<BGR>(mask)
    );
  }
}

////////////////////////////////////////////////////////////////////////////////

void NullBlit(IMAGE image, int x, int y)
{
}

////////////////////////////////////////////////////////////////////////////////

void TileBlit(IMAGE image, int x, int y)
{
  calculate_clipping_metrics(image->width, image->height);

  if (BitsPerPixel == 32)
  {
    BGRA* Screen = (BGRA*)ScreenBuffer;
    for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
      memcpy(Screen + ((y + iy) * ScreenWidth + x + image_offset_x),
             image->bgra + iy * image->width + image_offset_x,
             image_blit_width * sizeof(BGRA));
  }
  else
  {
    BGR* Screen = (BGR*)ScreenBuffer;
    for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
      memcpy(Screen + ((y + iy) * ScreenWidth + x + image_offset_x),
             image->bgr + iy * image->width + image_offset_x,
             image_blit_width * sizeof(BGR));
  }
}

////////////////////////////////////////////////////////////////////////////////

void SpriteBlit(IMAGE image, int x, int y)
{
  calculate_clipping_metrics(image->width, image->height);

  if (BitsPerPixel == 32)
  {
    BGRA* Screen = (BGRA*)ScreenBuffer;
    for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
      for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
        if (image->alpha[iy * image->width + ix])
          Screen[(y + iy) * ScreenWidth + (x + ix)] =
            image->bgra[iy * image->width + ix];
  }
  else
  {
    BGR* Screen = (BGR*)ScreenBuffer;
    for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
      for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
        if (image->alpha[iy * image->width + ix])
          Screen[(y + iy) * ScreenWidth + (x + ix)] =
            image->bgr[iy * image->width + ix];
  }
}

////////////////////////////////////////////////////////////////////////////////

void NormalBlit(IMAGE image, int x, int y)
{
  calculate_clipping_metrics(image->width, image->height);

  if (BitsPerPixel == 32)
  {
    for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
      for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
      {
        BGRA* dest  = (BGRA*)ScreenBuffer + (y + iy) * ScreenWidth + x + ix;
        BGRA  src   = image->bgra[iy * image->width + ix];
        byte  alpha = image->alpha[iy * image->width + ix];

        if (alpha == 255)
          *dest = src;
        else if (alpha > 0)
        {
          dest->red   = (dest->red   * (256 - alpha) + src.red   * alpha) / 256;
          dest->green = (dest->green * (256 - alpha) + src.green * alpha) / 256;
          dest->blue  = (dest->blue  * (256 - alpha) + src.blue  * alpha) / 256;
        }
      }
  }
  else
  {
    for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
      for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
      {
        BGR* dest  = (BGR*)ScreenBuffer + (y + iy) * ScreenWidth + x + ix;
        BGR  src   = image->bgr[iy * image->width + ix];
        byte alpha = image->alpha[iy * image->width + ix];

        if (alpha == 255)
          *dest = src;
        else if (alpha > 0)
        {
          dest->red   = (dest->red   * (256 - alpha) + src.red   * alpha) / 256;
          dest->green = (dest->green * (256 - alpha) + src.green * alpha) / 256;
          dest->blue  = (dest->blue  * (256 - alpha) + src.blue  * alpha) / 256;
        }
      }
  }
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

  // rgb
  if (BitsPerPixel == 32)
  {
    for (int i = 0; i < image->width * image->height; i++)
    {
      image->locked_pixels[i].red   = image->bgra[i].red;
      image->locked_pixels[i].green = image->bgra[i].green;
      image->locked_pixels[i].blue  = image->bgra[i].blue;
    }
  }
  else
  {
    for (int i = 0; i < image->width * image->height; i++)
    {
      image->locked_pixels[i].red   = image->bgr[i].red;
      image->locked_pixels[i].green = image->bgr[i].green;
      image->locked_pixels[i].blue  = image->bgr[i].blue;
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
  if (BitsPerPixel == 32)
    delete[] image->bgra;
  else
    delete[] image->bgr;
  delete[] image->alpha;
  
  FillImagePixels(image, image->locked_pixels);
  OptimizeBlitRoutine(image);
  delete[] image->locked_pixels;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DirectBlit(int x, int y, int w, int h, RGBA* pixels)
{
  calculate_clipping_metrics(w, h);

  if (BitsPerPixel == 32)
  {
    for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
      for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
      {
        BGRA* dest  = (BGRA*)ScreenBuffer + (y + iy) * ScreenWidth + x + ix;
        RGBA  src   = pixels[iy * w + ix];

        if (src.alpha == 255)
        {
          dest->red   = src.red;
          dest->green = src.green;
          dest->blue  = src.blue;
        }
        else if (src.alpha > 0)
        {
          dest->red   = (dest->red   * (256 - src.alpha) + src.red   * src.alpha) / 256;
          dest->green = (dest->green * (256 - src.alpha) + src.green * src.alpha) / 256;
          dest->blue  = (dest->blue  * (256 - src.alpha) + src.blue  * src.alpha) / 256;
        }
      }
  }
  else
  {
    for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
      for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
      {
        BGR* dest  = (BGR*)ScreenBuffer + (y + iy) * ScreenWidth + x + ix;
        RGBA src   = pixels[iy * w + ix];

        if (src.alpha == 255)
        {
          dest->red   = src.red;
          dest->green = src.green;
          dest->blue  = src.blue;
        }
        else if (src.alpha > 0)
        {
          dest->red   = (dest->red   * (256 - src.alpha) + src.red   * src.alpha) / 256;
          dest->green = (dest->green * (256 - src.alpha) + src.green * src.alpha) / 256;
          dest->blue  = (dest->blue  * (256 - src.alpha) + src.blue  * src.alpha) / 256;
        } // end for
      } // end for
  } // end if (BitsPerPixel)
}

////////////////////////////////////////////////////////////////////////////////

inline void BlendRGBAtoBGRA(BGRA& d, RGBA src, RGBA alpha)
{
  Blend3(d, src, alpha.alpha);
}

inline void BlendRGBAtoBGR(BGR& d, RGBA src, RGBA alpha)
{
  Blend3(d, src, alpha.alpha);
}

EXPORT(void) DirectTransformBlit(int x[4], int y[4], int w, int h, RGBA* pixels)
{
  if (BitsPerPixel == 32) {
    primitives::TexturedQuad(
      (BGRA*)ScreenBuffer,
      ScreenWidth,
      x,
      y,
      pixels,
      pixels,
      w,
      h,
      ClippingRectangle,
      BlendRGBAtoBGRA
    );
  } else {
    primitives::TexturedQuad(
      (BGR*)ScreenBuffer,
      ScreenWidth,
      x,
      y,
      pixels,
      pixels,
      w,
      h,
      ClippingRectangle,
      BlendRGBAtoBGR
    );
  }
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DirectGrab(int x, int y, int w, int h, RGBA* pixels)
{
  if (x < 0 ||
      y < 0 ||
      x + w > ScreenWidth ||
      y + h > ScreenHeight)
    return;

  if (BitsPerPixel == 32)
  {
    BGRA* Screen = (BGRA*)ScreenBuffer;
    for (int iy = 0; iy < h; iy++)
      for (int ix = 0; ix < w; ix++)
      {
        pixels[iy * w + ix].red   = Screen[(y + iy) * ScreenWidth + x + ix].red;
        pixels[iy * w + ix].green = Screen[(y + iy) * ScreenWidth + x + ix].green;
        pixels[iy * w + ix].blue  = Screen[(y + iy) * ScreenWidth + x + ix].blue;
        pixels[iy * w + ix].alpha = 255;
      }
  }
  else
  {
    BGR* Screen = (BGR*)ScreenBuffer;
    for (int iy = 0; iy < h; iy++)
      for (int ix = 0; ix < w; ix++)
      {
        pixels[iy * w + ix].red   = Screen[(y + iy) * ScreenWidth + x + ix].red;
        pixels[iy * w + ix].green = Screen[(y + iy) * ScreenWidth + x + ix].green;
        pixels[iy * w + ix].blue  = Screen[(y + iy) * ScreenWidth + x + ix].blue;
        pixels[iy * w + ix].alpha = 255;
      }
  }
}

////////////////////////////////////////////////////////////////////////////////

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


inline void copyBGRA(BGRA& dest, BGRA source) {
  dest = source;
}

inline void copyBGR(BGR& dest, BGR source) {
  dest = source;
}

inline void blendBGRA(BGRA& dest, RGBA source) {
  Blend3(dest, source, source.alpha);
}

inline void blendBGR(BGR& dest, RGBA source) {
  Blend3(dest, source, source.alpha);
}


EXPORT(void) DrawPoint(int x, int y, RGBA color)
{
  if (BitsPerPixel == 32) {
    primitives::Point((BGRA*)ScreenBuffer, ScreenWidth, x, y, color, ClippingRectangle, blendBGRA);
  } else {
    primitives::Point((BGR*)ScreenBuffer, ScreenWidth, x, y, color, ClippingRectangle, blendBGR);
  }
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DrawLine(int x[2], int y[2], RGBA color)
{
  if (BitsPerPixel == 32) {
    primitives::Line((BGRA*)ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], constant_color(color), ClippingRectangle, blendBGRA);
  } else {
    primitives::Line((BGR*)ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], constant_color(color), ClippingRectangle, blendBGR);
  }
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DrawGradientLine(int x[2], int y[2], RGBA colors[2])
{
  if (BitsPerPixel == 32) {
    primitives::Line((BGRA*)ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], gradient_color(colors[0], colors[1]), ClippingRectangle, blendBGRA);
  } else {
    primitives::Line((BGR*)ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], gradient_color(colors[0], colors[1]), ClippingRectangle, blendBGR);
  }
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DrawTriangle(int x[3], int y[3], RGBA color)
{
  if (BitsPerPixel == 32) {
    primitives::Triangle((BGRA*)ScreenBuffer, ScreenWidth, x, y, color, ClippingRectangle, blendBGRA);
  } else {
    primitives::Triangle((BGR*)ScreenBuffer, ScreenWidth, x, y, color, ClippingRectangle, blendBGR);
  }
}

////////////////////////////////////////////////////////////////////////////////

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

EXPORT(void) DrawGradientTriangle(int x[3], int y[3], RGBA colors[3])
{
  if (BitsPerPixel == 32) {
    primitives::GradientTriangle((BGRA*)ScreenBuffer, ScreenWidth, x, y, colors, ClippingRectangle, blendBGRA, interpolateRGBA);
  } else {
    primitives::GradientTriangle((BGR*)ScreenBuffer, ScreenWidth, x, y, colors, ClippingRectangle, blendBGR, interpolateRGBA);
  }
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DrawRectangle(int x, int y, int w, int h, RGBA color)
{
  if (color.alpha == 0) {          // no mask

    return;

  } else if (color.alpha == 255) { // full mask
    
    if (BitsPerPixel == 32) {
      BGRA bgra = { color.blue, color.green, color.red };
      primitives::Rectangle((BGRA*)ScreenBuffer, ScreenWidth, x, y, w, h, bgra, ClippingRectangle, copyBGRA);
    } else {
      BGR bgr = { color.blue, color.green, color.red };
      primitives::Rectangle((BGR*)ScreenBuffer, ScreenWidth, x, y, w, h, bgr, ClippingRectangle, copyBGR);
    }

  } else {

    if (BitsPerPixel == 32) {
      primitives::Rectangle((BGRA*)ScreenBuffer, ScreenWidth, x, y, w, h, color, ClippingRectangle, blendBGRA);
    } else {
      primitives::Rectangle((BGR*)ScreenBuffer, ScreenWidth, x, y, w, h, color, ClippingRectangle, blendBGR);
    }

  }
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DrawGradientRectangle(int x, int y, int w, int h, RGBA colors[4])
{
  if (BitsPerPixel == 32) {
    primitives::GradientRectangle((BGRA*)ScreenBuffer, ScreenWidth, x, y, w, h, colors, ClippingRectangle, blendBGRA, interpolateRGBA);
  } else {
    primitives::GradientRectangle((BGR*)ScreenBuffer, ScreenWidth, x, y, w, h, colors, ClippingRectangle, blendBGR, interpolateRGBA);
  }
}

////////////////////////////////////////////////////////////////////////////////
