#define DIRECTDRAW_VERSION 0x0300
#include <windows.h>
#include <ddraw.h>
#include <stdio.h>
#include "2xSaIWin.h"
#include "../../common/rgb.hpp"
#include "../../common/primitives.hpp"
#include "../common/video.hpp"
#include "../common/win32x.hpp"
#include "resource.h"



// these masks are used to calculate averages of 15-bit and 16-bit color pixels
#define MASK565 0xF7DE  // 11110 111110 11110
#define MASK555 0x7BDE  // 0 11110 11110 11110



// DATA TYPES //

typedef struct _IMAGE
{
  int width;
  int height;

  word* rgb;
  byte* alpha;

  void (*blit_routine)(_IMAGE* image, int x, int y);

  RGBA* locked_pixels;
}* IMAGE;


enum INTERPOLATOR
{
  I_CENTER      = 0,
  I_DIRECTSCALE = 1,
  I_EAGLE       = 2,
  I_2XSAI       = 3,
  I_TVMODE      = 4,
};


struct CONFIGURATION
{
  bool fullscreen;
  bool vsync;
  int  interpolator;
};



// FUNCTION PROTOTYPES //

static void LoadConfiguration();
static void SaveConfiguration();
static BOOL CALLBACK ConfigureDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

static bool InitFullscreen();
static bool SetDisplayMode();
static bool CreateSurfaces();
static bool InitWindowed();

static void CloseFullscreen();
static void CloseWindowed();

static void Interpolate(word* dest, int pitch);

static void FillImagePixels(IMAGE image, RGBA* pixels);
static void OptimizeBlitRoutine(IMAGE image);

static void NullBlit(IMAGE image, int x, int y);
static void TileBlit(IMAGE image, int x, int y);
static void SpriteBlit(IMAGE image, int x, int y);
static void NormalBlit(IMAGE image, int x, int y);



// INLINE FUNCTIONS //

inline word PackPixel565(RGBA pixel)
{
  return (word)(((pixel.red   >> 3) << 11) +
                ((pixel.green >> 2) <<  5) +
                ((pixel.blue  >> 3) <<  0));
}

inline RGBA UnpackPixel565(word pixel)
{
  RGBA rgba = {
    ((pixel & 0xF800) >> 11) << 3, // 11111 000000 00000
    ((pixel & 0x07E0) >>  5) << 2, // 00000 111111 00000
    ((pixel & 0x001F) >>  0) << 3, // 00000 000000 11111
  };
  return rgba;
}

inline word PackPixel555(RGBA pixel)
{
  return (word)(
    ((pixel.red   >> 3) << 10) + 
    ((pixel.green >> 3) << 5) +
    ((pixel.blue  >> 3) << 0));
}

inline RGBA UnpackPixel555(word pixel)
{
  RGBA rgba = {
    ((pixel & 0x7C00) >> 10) << 3, // 0 11111 00000 00000
    ((pixel & 0x03E0) >>  5) << 3, // 0 00000 11111 00000
    ((pixel & 0x001F) >>  0) << 3, // 0 00000 00000 11111
  };
  return rgba;
}



// GLOBAL VARIABLES

static CONFIGURATION Configuration;
static enum { RGB565, RGB555 } PixelFormat;

static HWND  SphereWindow;
static word* ScreenBuffer;

static LONG OldWindowStyle;
static LONG OldWindowStyleEx;

// fullscreen output
static LPDIRECTDRAW        dd;
static LPDIRECTDRAWSURFACE ddPrimary;
static LPDIRECTDRAWSURFACE ddSecondary;

// windowed output
static HDC     RenderDC;
static HBITMAP RenderBitmap;
static word*   RenderBuffer;



////////////////////////////////////////////////////////////////////////////////

void EXPORT GetDriverInfo(DRIVERINFO* driverinfo)
{
  driverinfo->name   = "Standard 16-bit Color";
  driverinfo->author = "Chad Austin";
  driverinfo->date   = __DATE__;
  driverinfo->version = "1.00";
  driverinfo->description = "15/16-bit color output in both windowed and fullscreen modes (2x interpolate)";
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT ConfigureDriver(HWND parent)
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

  Configuration.fullscreen        = (GetPrivateProfileInt("interpolate16", "Fullscreen",       1, config_file_name) != 0);
  Configuration.vsync             = (GetPrivateProfileInt("interpolate16", "VSync",            1, config_file_name) != 0);
  Configuration.interpolator      = GetPrivateProfileInt("interpolate16", "Interpolator", 0, config_file_name);
}

////////////////////////////////////////////////////////////////////////////////

void SaveConfiguration()
{
  char config_file_name[MAX_PATH];
  GetDriverConfigFile(config_file_name);

  WritePrivateProfileInt("interpolate16", "Fullscreen",       Configuration.fullscreen,        config_file_name);
  WritePrivateProfileInt("interpolate16", "VSync",            Configuration.vsync,             config_file_name);
  WritePrivateProfileInt("interpolate16", "Interpolator",     Configuration.interpolator,      config_file_name);
}

////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK ConfigureDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  switch (message)
  {
    case WM_INITDIALOG:
      // set check boxes
      SendDlgItemMessage(window, IDC_FULLSCREEN,       BM_SETCHECK, (Configuration.fullscreen        ? BST_CHECKED : BST_UNCHECKED), 0);
      SendDlgItemMessage(window, IDC_VSYNC,            BM_SETCHECK, (Configuration.vsync             ? BST_CHECKED : BST_UNCHECKED), 0);

      switch (Configuration.interpolator)
      {
        case I_CENTER:      CheckDlgButton(window, IDC_CENTER,      BST_CHECKED); break;
        case I_DIRECTSCALE: CheckDlgButton(window, IDC_DIRECTSCALE, BST_CHECKED); break;
        case I_EAGLE:       CheckDlgButton(window, IDC_EAGLE,       BST_CHECKED); break;
        case I_2XSAI:       CheckDlgButton(window, IDC_2XSAI,       BST_CHECKED); break;
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
          Configuration.fullscreen        = (IsDlgButtonChecked(window, IDC_FULLSCREEN) != FALSE);
          Configuration.vsync             = (IsDlgButtonChecked(window, IDC_VSYNC) != FALSE);

          if (IsDlgButtonChecked(window, IDC_CENTER) == BST_CHECKED)
            Configuration.interpolator = I_CENTER;
          else if (IsDlgButtonChecked(window, IDC_DIRECTSCALE) == BST_CHECKED)
            Configuration.interpolator = I_DIRECTSCALE;
          else if (IsDlgButtonChecked(window, IDC_EAGLE) == BST_CHECKED)
            Configuration.interpolator = I_EAGLE;
          else if (IsDlgButtonChecked(window, IDC_2XSAI) == BST_CHECKED)
            Configuration.interpolator = I_2XSAI;
          else if (IsDlgButtonChecked(window, IDC_TVMODE) == BST_CHECKED)
            Configuration.interpolator = I_TVMODE;

          EndDialog(window, 1);
          return TRUE;

        case IDCANCEL:
          EndDialog(window, 0);
          return TRUE;

        case IDC_FULLSCREEN:
          EnableWindow(GetDlgItem(window, IDC_VSYNC), IsDlgButtonChecked(window, IDC_FULLSCREEN));
          return TRUE;
      }
      return FALSE;

    ////////////////////////////////////////////////////////////////////////////

    default:
      return FALSE;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool EXPORT InitVideoDriver(HWND window, int screen_width, int screen_height)
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

  ScreenBuffer = new word[ScreenWidth * ScreenHeight];

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
    MessageBox(SphereWindow, "DirectDrawCreate() failed", "interpolate16", MB_OK);
    return false;
  }

  // set application behavior
  ddrval = dd->SetCooperativeLevel(SphereWindow, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
  if (ddrval != DD_OK)
  {
    dd->Release();
    MessageBox(SphereWindow, "SetCooperativeLevel() failed", "interpolate16", MB_OK);
    return false;
  }

  // set display mode
  retval = SetDisplayMode();
  if (retval == false)
  {
    dd->Release();
    MessageBox(SphereWindow, "SetDisplayMode() failed", "interpolate16", MB_OK);
    return false;
  }

  // create surfaces
  retval = CreateSurfaces();
  if (retval == false)
  {
    dd->Release();
    MessageBox(SphereWindow, "CreateSurfaces() failed", "interpolate16", MB_OK);
    return false;
  }

  ShowCursor(FALSE);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool SetDisplayMode()
{
  HRESULT ddrval = dd->SetDisplayMode(ScreenWidth * 2, ScreenHeight * 2, 16);
  if (ddrval != DD_OK)
    return false;

  return true;
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

  // determine bits per pixel
  DDPIXELFORMAT ddpf;
  ddpf.dwSize = sizeof(ddpf);
  ddpf.dwFlags = DDPF_RGB;
  ddrval = ddPrimary->GetPixelFormat(&ddpf);
  if (ddrval != DD_OK)
  {
    dd->Release();
    return false;
  }

  // 5:6:5 -- F800 07E0 001F
  // 5:5:5 -- 7C00 03E0 001F

  if (ddpf.dwRBitMask == 0xF800)
    PixelFormat = RGB565;
  else if (ddpf.dwRBitMask == 0x7C00)
    PixelFormat = RGB555;
  else
  {
    dd->Release();
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool InitWindowed()
{
  // create the render DC
  RenderDC = CreateCompatibleDC(NULL);
  if (RenderDC == NULL)
    return false;

  // define/create the DIB section
  BITMAPINFO bmi;
  memset(&bmi, 0, sizeof(bmi));
  BITMAPINFOHEADER& bmih = bmi.bmiHeader;
  bmih.biSize        = sizeof(bmih);
  bmih.biWidth       = ScreenWidth * 2;
  bmih.biHeight      = -ScreenHeight * 2;
  bmih.biPlanes      = 1;
  bmih.biBitCount    = 16;
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

  // we know that 16-bit color DIBs are always 5:5:5
  PixelFormat = RGB555;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT CloseVideoDriver()
{
  delete[] ScreenBuffer;
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
  dd->Release();
}

////////////////////////////////////////////////////////////////////////////////

void CloseWindowed()
{
  DeleteDC(RenderDC);
  DeleteObject(RenderBitmap);
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT FlipScreen()
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

    Interpolate((word*)ddsd.lpSurface, ddsd.lPitch / 2);

    // unlock the surface and do the flip!
    surface->Unlock(NULL);
    if (Configuration.vsync)
      ddPrimary->Flip(NULL, DDFLIP_WAIT);
  }
  else
  {
    // make sure all lines start on a dword boundary
    int pitch = ((ScreenWidth * 4) + 3) / 4 * 4;
    Interpolate(RenderBuffer, pitch / 2);

    // blit the render buffer to the window
    HDC dc = GetDC(SphereWindow);
    BitBlt(dc, 0, 0, ScreenWidth * 2, ScreenHeight * 2, RenderDC, 0, 0, SRCCOPY);
    ReleaseDC(SphereWindow, dc);
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////

void Interpolate_Center(word* dest, int pitch)
{
  // top quarter
  memset(dest, 0, pitch * (ScreenHeight / 4) * 2);

  // bottom quarter
  memset(dest + pitch * ScreenHeight * 6 / 4, 0, pitch * (ScreenHeight / 4) *2);

  
  word* dst = dest + pitch * ScreenHeight / 2;
  word* src = ScreenBuffer;

  int i = ScreenHeight;
  while (i--)
  {
    // clear left and right sides
    memset(dst, 0, ScreenWidth / 4 * 2);
    memset(dst + ScreenWidth * 6 / 4, 0, ScreenWidth / 4 * 2);

    memcpy(dst + ScreenWidth / 2, src, ScreenWidth * 2);

    src += ScreenWidth;
    dst += pitch;
  }
}

////////////////////////////////////////////////////////////////////////////////

void Interpolate_DirectScale(word* dest, int pitch)
{
  word* src = ScreenBuffer;
  int i = ScreenHeight;
  while (i--)
  {
    word* line1 = dest;
    word* line2 = dest + pitch;
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

void Interpolate_Eagle(word* dest, int pitch)
{
  // do left and right columns
  for (int iy = 0; iy < ScreenHeight; iy++)
  {
    int ix = 0;

    dest[(iy * 2 + 0) * pitch + (ix * 2 + 0)] =
    dest[(iy * 2 + 1) * pitch + (ix * 2 + 0)] =
    dest[(iy * 2 + 0) * pitch + (ix * 2 + 1)] =
    dest[(iy * 2 + 1) * pitch + (ix * 2 + 1)] =
      ScreenBuffer[iy * ScreenWidth + ix];

    ix = ScreenWidth - 1;

    dest[(iy * 2 + 0) * pitch + (ix * 2 + 0)] =
    dest[(iy * 2 + 1) * pitch + (ix * 2 + 0)] =
    dest[(iy * 2 + 0) * pitch + (ix * 2 + 1)] =
    dest[(iy * 2 + 1) * pitch + (ix * 2 + 1)] =
      ScreenBuffer[iy * ScreenWidth + ix];
  }

  // do top and bottom rows
  for (int ix = 1; ix < ScreenWidth - 1; ix++)
  {
    int iy = 0;

    dest[(iy * 2 + 0) * pitch + (ix * 2 + 0)] =
    dest[(iy * 2 + 1) * pitch + (ix * 2 + 0)] =
    dest[(iy * 2 + 0) * pitch + (ix * 2 + 1)] =
    dest[(iy * 2 + 1) * pitch + (ix * 2 + 1)] =
      ScreenBuffer[iy * ScreenWidth + ix];

    iy = ScreenHeight - 1;

    dest[(iy * 2 + 0) * pitch + (ix * 2 + 0)] =
    dest[(iy * 2 + 1) * pitch + (ix * 2 + 0)] =
    dest[(iy * 2 + 0) * pitch + (ix * 2 + 1)] =
    dest[(iy * 2 + 1) * pitch + (ix * 2 + 1)] =
      ScreenBuffer[iy * ScreenWidth + ix];
  }

  // do middle
  for (int iy = 1; iy < ScreenHeight - 1; iy++)
    for (int ix = 1; ix < ScreenWidth - 1; ix++)
    {
      #define scr(x, y) ScreenBuffer[(iy + y) * ScreenWidth + (ix + x)]

      // upper left pixel
      if (scr(0, -1) == scr(-1, -1) &&
          scr(-1, -1) == scr(-1, 0))
      {
        dest[(iy * 2 + 0) * pitch + (ix * 2 + 0)] = scr(-1, 0);
      }
      else
      {
        dest[(iy * 2 + 0) * pitch + (ix * 2 + 0)] = scr(0, 0);
      }

      // upper right pixel
      if (scr(0, -1) == scr(1, -1) &&
          scr(1, -1) == scr(1, 0))
      {
        dest[(iy * 2 + 0) * pitch + (ix * 2 + 1)] = scr(1, 0);
      }
      else
      {
        dest[(iy * 2 + 0) * pitch + (ix * 2 + 1)] = scr(0, 0);
      }

      // lower left pixel
      if (scr(0, 1) == scr(-1, 1) &&
          scr(-1, 1) == scr(-1, 0))
      {
        dest[(iy * 2 + 1) * pitch + (ix * 2 + 0)] = scr(-1, 0);
      }
      else
      {
        dest[(iy * 2 + 1) * pitch + (ix * 2 + 0)] = scr(0, 0);
      }

      // lower right pixel
      if (scr(0, 1) == scr(1, 1) &&
          scr(1, 1) == scr(1, 0))
      {
        dest[(iy * 2 + 1) * pitch + (ix * 2 + 1)] = scr(1, 0);
      }
      else
      {
        dest[(iy * 2 + 1) * pitch + (ix * 2 + 1)] = scr(0, 0);
      }
    }
}

////////////////////////////////////////////////////////////////////////////////

void Interpolate_TVMode(word* dest, int pitch, word mask)
{
  word* src = ScreenBuffer;
  int i = ScreenHeight;
  while (i--)
  {
    word* line1 = dest;
    word* line2 = dest + pitch;
    int j = ScreenWidth - 1; // skip last column
    while (j--)
    {
      word a = src[0];
      word b = ((a & mask) + (src[1] & mask)) / 2;
      *line1++ = a;
      *line1++ = b;

      *line2++ = (a & mask) / 2;
      *line2++ = (b & mask) / 2;

      src++;
    }

    // make up for last column
    src++;
    dest += pitch * 2;
  }
}

////////////////////////////////////////////////////////////////////////////////

void Interpolate(word* dest, int pitch)
{
  if (PixelFormat == RGB565)
  {
    switch (Configuration.interpolator)
    {
      case I_CENTER:      Interpolate_Center     (dest, pitch); break;
      case I_DIRECTSCALE: Interpolate_DirectScale(dest, pitch); break;
      case I_EAGLE:       Interpolate_Eagle      (dest, pitch); break;
      case I_2XSAI:       _2xSaIBitmap(16, ScreenBuffer, ScreenWidth, ScreenHeight, dest, pitch); break;
      case I_TVMODE:      Interpolate_TVMode     (dest, pitch, MASK565); break;
    }
  }
  else
  {
    switch (Configuration.interpolator)
    {
      case I_CENTER:      Interpolate_Center     (dest, pitch); break;
      case I_DIRECTSCALE: Interpolate_DirectScale(dest, pitch); break;
      case I_EAGLE:       Interpolate_Eagle      (dest, pitch); break;
      case I_2XSAI:       _2xSaIBitmap(15, ScreenBuffer, ScreenWidth, ScreenHeight, dest, pitch); break;
      case I_TVMODE:      Interpolate_TVMode     (dest, pitch, MASK555); break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

IMAGE EXPORT CreateImage(int width, int height, RGBA* pixels)
{
  IMAGE image = new _IMAGE;
  image->width  = width;
  image->height = height;

  FillImagePixels(image, pixels);
  OptimizeBlitRoutine(image);
  return image;
}

////////////////////////////////////////////////////////////////////////////////

void FillImagePixels(IMAGE image, RGBA* pixels)
{
  // rgb
  image->rgb = new word[image->width * image->height];
  if (PixelFormat == RGB565)
  {
    for (int i = 0; i < image->width * image->height; i++)
      image->rgb[i] = PackPixel565(pixels[i]);
  }
  else
  {
    for (int i = 0; i < image->width * image->height; i++)
      image->rgb[i] = PackPixel555(pixels[i]);
  }

  // alpha
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

IMAGE EXPORT GrabImage(int x, int y, int width, int height)
{
  if (x < 0 ||
      y > 0 ||
      x + width > ScreenWidth ||
      y + height > ScreenHeight)
    return NULL;

  IMAGE image = new _IMAGE;
  image->width        = width;
  image->height       = height;
  image->blit_routine = TileBlit;
  
  image->rgb = new word[width * height];
  image->alpha = new byte[width * height];

  for (int iy = 0; iy < height; iy++)
    memcpy(image->rgb + iy * width,
           ScreenBuffer + (y + iy) * ScreenWidth + x,
           width * 2);
  
  memset(image->alpha, 255, width * height);

  return image;
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DestroyImage(IMAGE image)
{
  delete[] image->rgb;
  delete[] image->alpha;
  delete image;
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT BlitImage(IMAGE image, int x, int y)
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

class render_pixel_mask_565
{
public:
  render_pixel_mask_565(RGBA mask) : m_mask(mask) { }
  void operator()(word& dst, word src, byte alpha)
  {
    RGBA d = UnpackPixel565(dst);
    RGBA s = UnpackPixel565(src);

    // do the masking on the source pixel
    alpha   = alpha   * m_mask.alpha / 256;
    s.red   = s.red   * m_mask.red   / 256;
    s.green = s.green * m_mask.green / 256;
    s.blue  = s.blue  * m_mask.blue  / 256;

    // blit to the dest pixel
    d.red   = (d.red   * (256 - alpha) + s.red   * alpha) / 256;
    d.green = (d.green * (256 - alpha) + s.green * alpha) / 256;
    d.blue  = (d.blue  * (256 - alpha) + s.blue  * alpha) / 256;

    dst = PackPixel565(d);
  }

private:
  RGBA m_mask;
};


class render_pixel_mask_555
{
public:
  render_pixel_mask_555(RGBA mask) : m_mask(mask) { }
  void operator()(word& dst, word src, byte alpha)
  {
    RGBA d = UnpackPixel555(dst);
    RGBA s = UnpackPixel555(src);

    // do the masking on the source pixel
    alpha   = alpha   * m_mask.alpha / 256;
    s.red   = s.red   * m_mask.red   / 256;
    s.green = s.green * m_mask.green / 256;
    s.blue  = s.blue  * m_mask.blue  / 256;

    // blit to the dest pixel
    d.red   = (d.red   * (256 - alpha) + s.red   * alpha) / 256;
    d.green = (d.green * (256 - alpha) + s.green * alpha) / 256;
    d.blue  = (d.blue  * (256 - alpha) + s.blue  * alpha) / 256;

    dst = PackPixel555(d);
  }

private:
  RGBA m_mask;
};


void EXPORT BlitImageMask(IMAGE image, int x, int y, RGBA mask)
{
  if (PixelFormat == RGB565) {

    primitives::Blit(
      ScreenBuffer,
      ScreenWidth,
      x,
      y,
      image->rgb,
      image->alpha,
      image->width,
      image->height,
      ClippingRectangle,
      render_pixel_mask_565(mask)
    );

  } else {

    primitives::Blit(
      ScreenBuffer,
      ScreenWidth,
      x,
      y,
      image->rgb,
      image->alpha,
      image->width,
      image->height,
      ClippingRectangle,
      render_pixel_mask_555(mask)
    );

  }
}

////////////////////////////////////////////////////////////////////////////////

inline void renderpixel565(word& d, const word& s, int a)
{
  RGBA out = UnpackPixel565(d);
  RGBA in  = UnpackPixel565(s);
  out.red   = (in.red   * a + out.red   * (256 - a)) / 256;
  out.green = (in.green * a + out.green * (256 - a)) / 256;
  out.blue  = (in.blue  * a + out.blue  * (256 - a)) / 256;
  d = PackPixel565(out);
}

inline void renderpixel555(word& d, const word& s, int a)
{
  RGBA out = UnpackPixel555(d);
  RGBA in  = UnpackPixel555(s);
  out.red   = (in.red   * a + out.red   * (256 - a)) / 256;
  out.green = (in.green * a + out.green * (256 - a)) / 256;
  out.blue  = (in.blue  * a + out.blue  * (256 - a)) / 256;
  d = PackPixel555(out);
}

void EXPORT TransformBlitImage(IMAGE image, int x[4], int y[4])
{
  if (PixelFormat == RGB565) {
    primitives::TexturedQuad(ScreenBuffer, ScreenWidth, x, y, image->rgb, image->alpha, image->width, image->height, ClippingRectangle, renderpixel565);
  } else {
    primitives::TexturedQuad(ScreenBuffer, ScreenWidth, x, y, image->rgb, image->alpha, image->width, image->height, ClippingRectangle, renderpixel555);
  }
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT TransformBlitImageMask(IMAGE image, int x[4], int y[4], RGBA mask)
{
  if (PixelFormat == RGB565) {
    primitives::TexturedQuad(ScreenBuffer, ScreenWidth, x, y, image->rgb, image->alpha, image->width, image->height, ClippingRectangle, render_pixel_mask_565(mask));
  } else {
    primitives::TexturedQuad(ScreenBuffer, ScreenWidth, x, y, image->rgb, image->alpha, image->width, image->height, ClippingRectangle, render_pixel_mask_555(mask));
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

  for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
    memcpy(ScreenBuffer + (y + iy) * ScreenWidth + x + image_offset_x,
           image->rgb + iy * image->width + image_offset_x,
           image_blit_width * 2);
}

////////////////////////////////////////////////////////////////////////////////

void SpriteBlit(IMAGE image, int x, int y)
{
  calculate_clipping_metrics(image->width, image->height);

  for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
    for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
    {
      if (image->alpha[iy * image->width + ix])
        ScreenBuffer[(y + iy) * ScreenWidth + (x + ix)] =
          image->rgb[iy * image->width + ix];
    }
}

////////////////////////////////////////////////////////////////////////////////

void NormalBlit(IMAGE image, int x, int y)
{
  calculate_clipping_metrics(image->width, image->height);

  if (PixelFormat == RGB565)
  {
    // 5:6:5
    for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
      for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
      {
        word* dest   = ScreenBuffer + (y + iy) * ScreenWidth + (x + ix);
        word  src    = image->rgb[iy * image->width + ix];
        int   alpha  = image->alpha[iy * image->width + ix];
    
        RGBA out = UnpackPixel565(*dest);
        RGBA in  = UnpackPixel565(src);
        out.red   = (in.red   * alpha + out.red   * (256 - alpha)) / 256;
        out.green = (in.green * alpha + out.green * (256 - alpha)) / 256;
        out.blue  = (in.blue  * alpha + out.blue  * (256 - alpha)) / 256;

        *dest = PackPixel565(out);
      }
  }
  else
  {
    // 5:5:5
    for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
      for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
      {
        word* dest   = ScreenBuffer + (y + iy) * ScreenWidth + (x + ix);
        word  src    = image->rgb[iy * image->width + ix];
        int   alpha  = image->alpha[iy * image->width + ix];
    
        RGBA out = UnpackPixel555(*dest);
        RGBA in  = UnpackPixel555(src);
        out.red   = (in.red   * alpha + out.red   * (256 - alpha)) / 256;
        out.green = (in.green * alpha + out.green * (256 - alpha)) / 256;
        out.blue  = (in.blue  * alpha + out.blue  * (256 - alpha)) / 256;

        *dest = PackPixel555(out);
      }
  }
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
  image->locked_pixels = new RGBA[image->width * image->height];

  // rgb
  if (PixelFormat == RGB565)
  {
    // 5:6:5
    for (int i = 0; i < image->width * image->height; i++)
      image->locked_pixels[i] = UnpackPixel565(image->rgb[i]);
  }
  else
  {
    // 5:5:5
    for (int i = 0; i < image->width * image->height; i++)
      image->locked_pixels[i] = UnpackPixel555(image->rgb[i]);
  }

  // alpha
  for (int i = 0; i < image->width * image->height; i++)
    image->locked_pixels[i].alpha = image->alpha[i];

  return image->locked_pixels;
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT UnlockImage(IMAGE image)
{
  delete[] image->rgb;
  delete[] image->alpha;

  FillImagePixels(image, image->locked_pixels);
  OptimizeBlitRoutine(image);
  delete[] image->locked_pixels;
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DirectBlit(int x, int y, int w, int h, RGBA* pixels)
{
  calculate_clipping_metrics(w, h);

  if (PixelFormat == RGB565)
  {
    // 5:6:5
    for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
      for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
      {
        word* dest  = ScreenBuffer + (y + iy) * ScreenWidth + (x + ix);
        int   alpha = pixels[iy * w + ix].alpha;
  
        RGBA out = UnpackPixel565(*dest);
        RGBA in  = pixels[iy * w + ix];
        out.red   = (in.red   * alpha + out.red   * (256 - alpha)) / 256;
        out.green = (in.green * alpha + out.green * (256 - alpha)) / 256;
        out.blue  = (in.blue  * alpha + out.blue  * (256 - alpha)) / 256;

        *dest = PackPixel565(out);
      }
  }
  else
  {
    // 5:5:5
    for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
      for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
      {
        word* dest  = ScreenBuffer + (y + iy) * ScreenWidth + (x + ix);
        int   alpha = pixels[iy * w + ix].alpha;
  
        RGBA out = UnpackPixel555(*dest);
        RGBA in  = pixels[iy * w + ix];
        out.red   = (in.red   * alpha + out.red   * (256 - alpha)) / 256;
        out.green = (in.green * alpha + out.green * (256 - alpha)) / 256;
        out.blue  = (in.blue  * alpha + out.blue  * (256 - alpha)) / 256;

        *dest = PackPixel555(out);
      }
  }
}

////////////////////////////////////////////////////////////////////////////////

inline void blendRGBAto565(word& d, RGBA s, RGBA alpha)
{
  RGBA out = UnpackPixel565(d);
  byte a = alpha.alpha;
  out.red   = (s.red   * a + out.red   * (256 - a)) / 256;
  out.green = (s.green * a + out.green * (256 - a)) / 256;
  out.blue  = (s.blue  * a + out.blue  * (256 - a)) / 256;
  d = PackPixel565(out);
}

inline void blendRGBAto555(word& d, RGBA s, RGBA alpha)
{
  RGBA out = UnpackPixel555(d);
  byte a = alpha.alpha;
  out.red   = (s.red   * a + out.red   * (256 - a)) / 256;
  out.green = (s.green * a + out.green * (256 - a)) / 256;
  out.blue  = (s.blue  * a + out.blue  * (256 - a)) / 256;
  d = PackPixel555(out);
}

void EXPORT DirectTransformBlit(int x[4], int y[4], int w, int h, RGBA* pixels)
{
  if (PixelFormat == RGB565) {
    primitives::TexturedQuad(ScreenBuffer, ScreenWidth, x, y, pixels, pixels, w, h, ClippingRectangle, blendRGBAto565);
  } else {
    primitives::TexturedQuad(ScreenBuffer, ScreenWidth, x, y, pixels, pixels, w, h, ClippingRectangle, blendRGBAto555);
  }
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DirectGrab(int x, int y, int w, int h, RGBA* pixels)
{
  if (x < 0 ||
      y < 0 ||
      x + w > ScreenWidth ||
      y + h > ScreenHeight)
    return;

  // 5:6:5
  if (PixelFormat == RGB565)
  {
    for (int iy = 0; iy < h; iy++)
      for (int ix = 0; ix < w; ix++)
      {
        pixels[iy * w + ix]       = UnpackPixel565(ScreenBuffer[(y + iy) * ScreenWidth + x + ix]);
        pixels[iy * w + ix].alpha = 255;
      }
  }
  // 5:5:5
  else
  {
    for (int iy = 0; iy < h; iy++)
      for (int ix = 0; ix < w; ix++)
      {
        pixels[iy * w + ix]       = UnpackPixel555(ScreenBuffer[(y + iy) * ScreenHeight + x + x]);
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


inline void copyWord(word& dest, word source) {
  dest = source;
}

inline void blend565(word& dest, RGBA source) {
  RGBA out = UnpackPixel565(dest);
  out.red   = (source.red   * source.alpha + out.red   * (256 - source.alpha)) / 256;
  out.green = (source.green * source.alpha + out.green * (256 - source.alpha)) / 256;
  out.blue  = (source.blue  * source.alpha + out.blue  * (256 - source.alpha)) / 256;
  dest = PackPixel565(out);
}

inline void blend555(word& dest, RGBA source) {
  RGBA out = UnpackPixel555(dest);
  out.red   = (source.red   * source.alpha + out.red   * (256 - source.alpha)) / 256;
  out.green = (source.green * source.alpha + out.green * (256 - source.alpha)) / 256;
  out.blue  = (source.blue  * source.alpha + out.blue  * (256 - source.alpha)) / 256;
  dest = PackPixel555(out);
}


void EXPORT DrawPoint(int x, int y, RGBA color)
{
  if (PixelFormat == RGB565) {
    primitives::Point(ScreenBuffer, ScreenWidth, x, y, color, ClippingRectangle, blend565);
  } else {
    primitives::Point(ScreenBuffer, ScreenWidth, x, y, color, ClippingRectangle, blend555);
  }
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DrawLine(int x[2], int y[2], RGBA color)
{
  if (PixelFormat == RGB565) {
    primitives::Line(ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], constant_color(color), ClippingRectangle, blend565);
  } else {
    primitives::Line(ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], constant_color(color), ClippingRectangle, blend555);
  }
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DrawGradientLine(int x[2], int y[2], RGBA colors[2])
{
  if (PixelFormat == RGB565) {
    primitives::Line(ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], gradient_color(colors[0], colors[1]), ClippingRectangle, blend565);
  } else {
    primitives::Line(ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], gradient_color(colors[0], colors[1]), ClippingRectangle, blend555);
  }
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DrawTriangle(int x[3], int y[3], RGBA color)
{
  if (PixelFormat == RGB565) {
    primitives::Triangle(ScreenBuffer, ScreenWidth, x, y, color, ClippingRectangle, blend565);
  } else {
    primitives::Triangle(ScreenBuffer, ScreenWidth, x, y, color, ClippingRectangle, blend555);
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

void EXPORT DrawGradientTriangle(int x[3], int y[3], RGBA colors[3])
{
  if (PixelFormat == RGB565) {
    primitives::GradientTriangle(ScreenBuffer, ScreenWidth, x, y, colors, ClippingRectangle, blend565, interpolateRGBA);
  } else {
    primitives::GradientTriangle(ScreenBuffer, ScreenWidth, x, y, colors, ClippingRectangle, blend555, interpolateRGBA);
  }
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DrawRectangle(int x, int y, int w, int h, RGBA color)
{
  if (color.alpha == 0) {          // no mask

    return;

  } else if (color.alpha == 255) { // full mask
    
    if (PixelFormat == RGB565) {
      word c = PackPixel565(color);
      primitives::Rectangle(ScreenBuffer, ScreenWidth, x, y, w, h, c, ClippingRectangle, copyWord);
    } else {
      word c = PackPixel555(color);
      primitives::Rectangle(ScreenBuffer, ScreenWidth, x, y, w, h, c, ClippingRectangle, copyWord);
    }

  } else {

    if (PixelFormat == RGB565) {
      primitives::Rectangle(ScreenBuffer, ScreenWidth, x, y, w, h, color, ClippingRectangle, blend565);
    } else {
      primitives::Rectangle(ScreenBuffer, ScreenWidth, x, y, w, h, color, ClippingRectangle, blend555);
    }

  }
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DrawGradientRectangle(int x, int y, int w, int h, RGBA colors[4])
{
  if (PixelFormat == RGB565) {
    primitives::GradientRectangle(ScreenBuffer, ScreenWidth, x, y, w, h, colors, ClippingRectangle, blend565, interpolateRGBA);
  } else {
    primitives::GradientRectangle(ScreenBuffer, ScreenWidth, x, y, w, h, colors, ClippingRectangle, blend555, interpolateRGBA);
  }
}

////////////////////////////////////////////////////////////////////////////////
