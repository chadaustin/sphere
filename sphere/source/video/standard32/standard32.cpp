#define DIRECTDRAW_VERSION 0x0300
#include <windows.h>
#include <ddraw.h>
#include <stdio.h>
#include "../../common/rgb.hpp"
#include "../../common/primitives.hpp"
#include "../common/video.hpp"
#include "../common/win32x.hpp"
#include "resource.h"


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

struct CONFIGURATION
{
  BIT_DEPTH bit_depth;

  bool fullscreen;
  bool vsync;
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

static void FillImagePixels(IMAGE image, RGBA* data);
static void OptimizeBlitRoutine(IMAGE image);

static void NullBlit(IMAGE image, int x, int y);
static void TileBlit(IMAGE image, int x, int y);
static void SpriteBlit(IMAGE image, int x, int y);
static void NormalBlit(IMAGE image, int x, int y);


static CONFIGURATION Configuration;
static int           BitsPerPixel;

static HWND  SphereWindow;
static void* ScreenBuffer;

static LONG OldWindowStyle;
static LONG OldWindowStyleEx;

// windowed output
static HDC     RenderDC;
static HBITMAP RenderBitmap;

// fullscreen output
static LPDIRECTDRAW dd;
static LPDIRECTDRAWSURFACE ddPrimary;
static LPDIRECTDRAWSURFACE ddSecondary;


////////////////////////////////////////////////////////////////////////////////

void EXPORT GetDriverInfo(DRIVERINFO* driverinfo)
{
  driverinfo->name        = "Standard 32-bit Color";
  driverinfo->author      = "Chad Austin";
  driverinfo->date        = __DATE__;
  driverinfo->version     = "1.00";
  driverinfo->description = "24/32-bit color output in both windowed and fullscreen modes";
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

  // load the fields from the file
  int bit_depth = GetPrivateProfileInt("standard32", "BitDepth", 0, config_file_name);
  Configuration.bit_depth = (bit_depth == 32 ? BD_32 : (bit_depth == 24 ? BD_24 : BD_AUTODETECT));

  Configuration.fullscreen = GetPrivateProfileInt("standard32", "Fullscreen", 1, config_file_name) != 0;
  Configuration.vsync      = GetPrivateProfileInt("standard32", "VSync",      1, config_file_name) != 0;
}

////////////////////////////////////////////////////////////////////////////////

void SaveConfiguration()
{
  char config_file_name[MAX_PATH];
  GetDriverConfigFile(config_file_name);

  // save the fields to the file
  int bit_depth = (Configuration.bit_depth == BD_32 ? 32 : (Configuration.bit_depth == BD_24 ? 24 : 0));
  WritePrivateProfileInt("standard32", "BitDepth",   bit_depth,                config_file_name);
  WritePrivateProfileInt("standard32", "Fullscreen", Configuration.fullscreen, config_file_name);
  WritePrivateProfileInt("standard32", "VSync",      Configuration.vsync,      config_file_name);
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

bool EXPORT InitVideoDriver(HWND window, int screen_width, int screen_height)
{
  SphereWindow = window;
  ScreenWidth  = screen_width;
  ScreenHeight = screen_height;

  // set default clipping rectangle
  SetClippingRectangle(0, 0, screen_width, screen_height);

  LoadConfiguration();
  if (Configuration.fullscreen)
    return InitFullscreen();
  else
    return InitWindowed();
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
    MessageBox(SphereWindow, "DirectDrawCreate() failed", "standard32", MB_OK);
    return false;
  }

  // set application behavior
  ddrval = dd->SetCooperativeLevel(SphereWindow, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
  if (ddrval != DD_OK)
  {
    dd->Release();
    MessageBox(SphereWindow, "SetCooperativeLevel() failed", "standard32", MB_OK);
    return false;
  }

  // set display mode
  retval = SetDisplayMode();
  if (retval == false)
  {
    dd->Release();
    MessageBox(SphereWindow, "SetDisplayMode() failed", "standard32", MB_OK);
    return false;
  }

  // create surfaces
  retval = CreateSurfaces();
  if (retval == false)
  {
    dd->Release();
    MessageBox(SphereWindow, "CreateSurfaces() failed", "standard32", MB_OK);
    return false;
  }

  ShowCursor(FALSE);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool SetDisplayMode()
{
  HRESULT ddrval;

  switch (Configuration.bit_depth)
  {
    case BD_AUTODETECT:
      ddrval = dd->SetDisplayMode(ScreenWidth, ScreenHeight, 32);
      BitsPerPixel = 32;
      if (ddrval != DD_OK)
      {
        ddrval = dd->SetDisplayMode(ScreenWidth, ScreenHeight, 24);
        BitsPerPixel = 24;
      }
      return ddrval == DD_OK;

    case BD_32:
      ddrval = dd->SetDisplayMode(ScreenWidth, ScreenHeight, 32);
      BitsPerPixel = 32;
      return ddrval == DD_OK;

    case BD_24:
      ddrval = dd->SetDisplayMode(ScreenWidth, ScreenHeight, 24);
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

  // allocate a blitting buffer
  ScreenBuffer = new byte[ScreenWidth * ScreenHeight * (BitsPerPixel / 8)];

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
  bmih.biWidth       = ScreenWidth;
  bmih.biHeight      = -ScreenHeight;
  bmih.biPlanes      = 1;
  bmih.biBitCount    = BitsPerPixel;
  bmih.biCompression = BI_RGB;
  RenderBitmap = CreateDIBSection(RenderDC, &bmi, DIB_RGB_COLORS, (void**)&ScreenBuffer, NULL, 0);
  if (RenderBitmap == NULL)
  {
    DeleteDC(RenderDC);
    return false;
  }

  SelectObject(RenderDC, RenderBitmap);

  // center the window
  RECT WindowRect = { 0, 0, ScreenWidth, ScreenHeight };
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

void EXPORT CloseVideoDriver()
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

void FillImagePixels(IMAGE image, RGBA* pixels)
{
  // fill the image pixels
  if (BitsPerPixel == 32)
  {
    image->bgra = new BGRA[image->width * image->height];
    for (int i = 0; i < image->width * image->height; i++)
    {
      image->bgra[i].red   = (pixels[i].red   * pixels[i].alpha) / 256;
      image->bgra[i].green = (pixels[i].green * pixels[i].alpha) / 256;
      image->bgra[i].blue  = (pixels[i].blue  * pixels[i].alpha) / 256;;
    }
  }
  else
  {
    image->bgr  = new BGR[image->width * image->height];
    for (int i = 0; i < image->width * image->height; i++)
    {
      image->bgr[i].red   = (pixels[i].red   * pixels[i].alpha) / 256;
      image->bgr[i].green = (pixels[i].green * pixels[i].alpha) / 256;
      image->bgr[i].blue  = (pixels[i].blue  * pixels[i].alpha) / 256;
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

    // render backbuffer to the screen
    if (BitsPerPixel == 32)
    {
      BGRA* dst = (BGRA*)ddsd.lpSurface;
      BGRA* src = (BGRA*)ScreenBuffer;
      for (int i = 0; i < ScreenHeight; i++)
      {
        memcpy(dst, src, ScreenWidth * 4);
        dst += ddsd.lPitch / 4;
        src += ScreenWidth;
      }
    }
    else
    {
      BGR* dst = (BGR*)ddsd.lpSurface;
      BGR* src = (BGR*)ScreenBuffer;
      for (int i = 0; i < ScreenHeight; i++)
      {
        memcpy(dst, src, ScreenWidth * 3);
        dst += ddsd.lPitch / 3;
        src += ScreenWidth;
      }
    }

    // unlock the surface and do the flip!
    surface->Unlock(NULL);
    if (Configuration.vsync)
      ddPrimary->Flip(NULL, DDFLIP_WAIT);
  }
  else
  {
    // make sure the lines are on dword boundaries
    if (BitsPerPixel == 24)
    {
      int pitch = (ScreenWidth * 3 + 3) / 4 * 4;
      byte* dst = (byte*)ScreenBuffer;
      BGR*  src = (BGR*)ScreenBuffer;
      for (int i = ScreenHeight - 1; i >= 0; i--)
      {
        memmove(dst + i * pitch,
                src + i * ScreenWidth,
                ScreenWidth * 3);
      }
    }

    // blit the render buffer to the window
    HDC dc = GetDC(SphereWindow);
    BitBlt(dc, 0, 0, ScreenWidth, ScreenHeight, RenderDC, 0, 0, SRCCOPY);
    ReleaseDC(SphereWindow, dc);
  }
}

////////////////////////////////////////////////////////////////////////////////

IMAGE EXPORT CreateImage(int width, int height, RGBA* pixels)
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

IMAGE EXPORT GrabImage(int x, int y, int width, int height)
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

void EXPORT DestroyImage(IMAGE image)
{
  if (BitsPerPixel == 32)
    delete[] image->bgra;
  else
    delete[] image->bgr;
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


void EXPORT BlitImageMask(IMAGE image, int x, int y, RGBA mask)
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

void EXPORT TransformBlitImage(IMAGE image, int x[4], int y[4])
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

void EXPORT TransformBlitImageMask(IMAGE image, int x[4], int y[4], RGBA mask)
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

    BGRA* dest  = (BGRA*)ScreenBuffer + (y + image_offset_y) * ScreenWidth  + image_offset_x + x;
    BGRA* src   = (BGRA*)image->bgra  +       image_offset_y * image->width + image_offset_x;

    int iy = image_blit_height;
    while (iy-- > 0) {

      memcpy(dest, src, image_blit_width * sizeof(BGRA));
      dest += ScreenWidth;
      src += image->width;

    }

  }
  else
  {

    BGR* dest  = (BGR*)ScreenBuffer + (y + image_offset_y) * ScreenWidth  + image_offset_x + x;
    BGR* src   = (BGR*)image->bgra  +       image_offset_y * image->width + image_offset_x;

    int iy = image_blit_height;
    while (iy-- > 0) {

      memcpy(dest, src, image_blit_width * sizeof(BGR));
      dest += ScreenWidth;
      src += image->width;

    }

  }
}

////////////////////////////////////////////////////////////////////////////////

void SpriteBlit(IMAGE image, int x, int y)
{
  calculate_clipping_metrics(image->width, image->height);

  if (BitsPerPixel == 32) {

    BGRA* dest  = (BGRA*)ScreenBuffer + (y + image_offset_y) * ScreenWidth  + image_offset_x + x;
    BGRA* src   = (BGRA*)image->bgra  +       image_offset_y * image->width + image_offset_x;
    byte* alpha = image->alpha        +       image_offset_y * image->width + image_offset_x;

    int dest_inc = ScreenWidth  - image_blit_width;
    int src_inc  = image->width - image_blit_width;

    int iy = image_blit_height;
    while (iy-- > 0) {
      int ix = image_blit_width;
      while (ix-- > 0) {
        
        if (*alpha) {
          *dest = *src;
        }

        dest++;
        src++;
        alpha++;
      }

      dest += dest_inc;
      src += src_inc;
      alpha += src_inc;
    }

  } else {

    BGR*  dest  = (BGR*)ScreenBuffer + (y + image_offset_y) * ScreenWidth  + image_offset_x + x;
    BGR*  src   = (BGR*)image->bgra  +       image_offset_y * image->width + image_offset_x;
    byte* alpha = image->alpha       +       image_offset_y * image->width + image_offset_x;

    int dest_inc = ScreenWidth  - image_blit_width;
    int src_inc  = image->width - image_blit_width;

    int iy = image_blit_height;
    while (iy-- > 0) {
      int ix = image_blit_width;
      while (ix-- > 0) {
        
        if (*alpha) {
          *dest = *src;
        }

        dest++;
        src++;
        alpha++;
      }

      dest += dest_inc;
      src += src_inc;
      alpha += src_inc;
    }

  }
}

////////////////////////////////////////////////////////////////////////////////

void NormalBlit(IMAGE image, int x, int y)
{
  calculate_clipping_metrics(image->width, image->height);

  if (BitsPerPixel == 32) {

/*/
    BGRA* dest  = (BGRA*)ScreenBuffer + (y + image_offset_y) * ScreenWidth  + image_offset_x + x;
    BGRA* src   = (BGRA*)image->bgra  +       image_offset_y * image->width + image_offset_x;
    byte* alpha = image->alpha        +       image_offset_y * image->width + image_offset_x;

    int dest_inc = ScreenWidth  - image_blit_width;
    int src_inc  = image->width - image_blit_width;

    int iy = image_blit_height;
    while (iy-- > 0) {
      int ix = image_blit_width;
      while (ix-- > 0) {

        word a = *alpha;
        word b = 256 - a;

        dest->red   = (dest->red   * b) / 256 + src->red;
        dest->green = (dest->green * b) / 256 + src->green;
        dest->blue  = (dest->blue  * b) / 256 + src->blue;

        dest++;
        src++;
        alpha++;
      }

      dest  += dest_inc;
      src   += src_inc;
      alpha += src_inc;
    }
/*/
    dword* dest = (dword*)ScreenBuffer + (y + image_offset_y) * ScreenWidth  + image_offset_x + x;
    dword* src  = (dword*)image->bgra  +       image_offset_y * image->width + image_offset_x;
    byte* alpha = image->alpha         +       image_offset_y * image->width + image_offset_x;

    int dest_inc = ScreenWidth  - image_blit_width;
    int src_inc  = image->width - image_blit_width;

    int iy = image_blit_height;
    while (iy-- > 0) {
      int ix = image_blit_width;
      while (ix-- > 0) {

        word a = *alpha;
        word b = 256 - a;

        dword d = *dest;
        dword s = *src;

        dword result;
        result = ((d & 0xFF) * b >> 8) + (s & 0xFF);

        d >>= 8;
        s >>= 8;

        result |= ((d & 0xFF) * b + ((s & 0xFF) << 8)) & 0xFF00;

        d >>= 8;
        s >>= 8;

        result |= (((d & 0xFF) * b + ((s & 0xFF) << 8)) & 0xFF00) << 8;

        *dest = result;

        dest++;
        src++;
        alpha++;
      }

      dest  += dest_inc;
      src   += src_inc;
      alpha += src_inc;
    }
//*/

  }
  else
  {
    BGR*  dest  = (BGR*)ScreenBuffer + (y + image_offset_y) * ScreenWidth  + image_offset_x + x;
    BGR*  src   = (BGR*)image->bgr   +       image_offset_y * image->width + image_offset_x;
    byte* alpha = image->alpha       +       image_offset_y * image->width + image_offset_x;

    int dest_inc = ScreenWidth  - image_blit_width;
    int src_inc  = image->width - image_blit_width;

    int iy = image_blit_height;
    while (iy-- > 0) {
      int ix = image_blit_width;
      while (ix-- > 0) {

        dest->red   = (dest->red   * (256 - *alpha)) / 256 + src->red;
        dest->green = (dest->green * (256 - *alpha)) / 256 + src->green;
        dest->blue  = (dest->blue  * (256 - *alpha)) / 256 + src->blue;

        dest++;
        src++;
        alpha++;
      }

      dest  += dest_inc;
      src   += src_inc;
      alpha += src_inc;
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
  if (BitsPerPixel == 32)
  {
    for (int i = 0; i < image->width * image->height; i++)
    {
      image->locked_pixels[i].red   = (image->bgra[i].red   * 256) / image->alpha[i];
      image->locked_pixels[i].green = (image->bgra[i].green * 256) / image->alpha[i];
      image->locked_pixels[i].blue  = (image->bgra[i].blue  * 256) / image->alpha[i];
    }
  }
  else
  {
    for (int i = 0; i < image->width * image->height; i++)
    {
      image->locked_pixels[i].red   = (image->bgr[i].red   * 256) / image->alpha[i];
      image->locked_pixels[i].green = (image->bgr[i].green * 256) / image->alpha[i];
      image->locked_pixels[i].blue  = (image->bgr[i].blue  * 256) / image->alpha[i];
    }
  }

  // alpha
  for (int i = 0; i < image->width * image->height; i++)
    image->locked_pixels[i].alpha = image->alpha[i];

  return image->locked_pixels;
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT UnlockImage(IMAGE image)
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

void EXPORT DirectBlit(int x, int y, int w, int h, RGBA* pixels)
{
  calculate_clipping_metrics(w, h);

  if (BitsPerPixel == 32)
  {
    for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
      for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
      {
        BGRA* dest = (BGRA*)ScreenBuffer + (y + iy) * ScreenWidth + x + ix;
        RGBA  src  = pixels[iy * w + ix];

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

void EXPORT DirectTransformBlit(int x[4], int y[4], int w, int h, RGBA* pixels)
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

void EXPORT DirectGrab(int x, int y, int w, int h, RGBA* pixels)
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


void EXPORT DrawPoint(int x, int y, RGBA color)
{
  if (BitsPerPixel == 32) {
    primitives::Point((BGRA*)ScreenBuffer, ScreenWidth, x, y, color, ClippingRectangle, blendBGRA);
  } else {
    primitives::Point((BGR*)ScreenBuffer, ScreenWidth, x, y, color, ClippingRectangle, blendBGR);
  }
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DrawLine(int x[2], int y[2], RGBA color)
{
  if (BitsPerPixel == 32) {
    primitives::Line((BGRA*)ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], constant_color(color), ClippingRectangle, blendBGRA);
  } else {
    primitives::Line((BGR*)ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], constant_color(color), ClippingRectangle, blendBGR);
  }
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DrawGradientLine(int x[2], int y[2], RGBA colors[2])
{
  if (BitsPerPixel == 32) {
    primitives::Line((BGRA*)ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], gradient_color(colors[0], colors[1]), ClippingRectangle, blendBGRA);
  } else {
    primitives::Line((BGR*)ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], gradient_color(colors[0], colors[1]), ClippingRectangle, blendBGR);
  }
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DrawTriangle(int x[3], int y[3], RGBA color)
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

void EXPORT DrawGradientTriangle(int x[3], int y[3], RGBA colors[3])
{
  if (BitsPerPixel == 32) {
    primitives::GradientTriangle((BGRA*)ScreenBuffer, ScreenWidth, x, y, colors, ClippingRectangle, blendBGRA, interpolateRGBA);
  } else {
    primitives::GradientTriangle((BGR*)ScreenBuffer, ScreenWidth, x, y, colors, ClippingRectangle, blendBGR, interpolateRGBA);
  }
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT DrawRectangle(int x, int y, int w, int h, RGBA color)
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

void EXPORT DrawGradientRectangle(int x, int y, int w, int h, RGBA colors[4])
{
  if (BitsPerPixel == 32) {
    primitives::GradientRectangle((BGRA*)ScreenBuffer, ScreenWidth, x, y, w, h, colors, ClippingRectangle, blendBGRA, interpolateRGBA);
  } else {
    primitives::GradientRectangle((BGR*)ScreenBuffer, ScreenWidth, x, y, w, h, colors, ClippingRectangle, blendBGR, interpolateRGBA);
  }
}

////////////////////////////////////////////////////////////////////////////////