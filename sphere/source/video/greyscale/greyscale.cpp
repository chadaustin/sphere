#define DIRECTDRAW_VERSION 0x0300
#include <windows.h>
#include <commdlg.h>
#include <ddraw.h>
#include "../../common/rgb.hpp"
#include "../../common/primitives.hpp"
#include "../common/win32x.hpp"
#include "../common/video.hpp"
#include "resource.h"



typedef struct _IMAGE
{
  int width;
  int height;

  byte* grey;
  byte* alpha;

  void (*blit_routine)(_IMAGE* image, int x, int y);

  RGBA* locked_pixels;
}* IMAGE;


struct CONFIGURATION
{
  bool     fullscreen;
  bool     vsync;
  COLORREF base_color;
};



static void LoadConfiguration();
static void SaveConfiguration();
static BOOL CALLBACK ConfigureDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

static bool InitFullscreen();
static bool CreateSurfaces();
static bool InitWindowed();

static void CloseFullscreen();
static void CloseWindowed();

static void FillImagePixels(IMAGE image, RGBA* pixels);
static void OptimizeBlitRoutine(IMAGE image);

static void NullBlit(IMAGE image, int x, int y);
static void TileBlit(IMAGE image, int x, int y);
static void SpriteBlit(IMAGE image, int x, int y);
static void NormalBlit(IMAGE image, int x, int y);



static CONFIGURATION Configuration;

static HWND  SphereWindow;
static byte* ScreenBuffer;

static LONG OldWindowStyle;
static LONG OldWindowStyleEx;

// windowed mode
static HDC     RenderDC;
static HBITMAP RenderBitmap;

// fullscreen mode
static LPDIRECTDRAW        dd;
static LPDIRECTDRAWPALETTE ddPalette;
static LPDIRECTDRAWSURFACE ddPrimary;
static LPDIRECTDRAWSURFACE ddSecondary;



////////////////////////////////////////////////////////////////////////////////

EXPORT(void) GetDriverInfo(DRIVERINFO* driverinfo)
{
  driverinfo->name        = "8-bit greyscale";
  driverinfo->author      = "Chad Austin";
  driverinfo->date        = __DATE__;
  driverinfo->version     = "1.00";
  driverinfo->description = "8-bit greyscale (or hued) in both windowed and fullscreen modes";
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

  Configuration.fullscreen = (GetPrivateProfileInt("greyscale", "Fullscreen", 1,        config_file_name) != 0);
  Configuration.vsync      = (GetPrivateProfileInt("greyscale", "VSync",      1,        config_file_name) != 0);
  Configuration.base_color = GetPrivateProfileInt("greyscale",  "BaseColor",  0xFFFFFF, config_file_name);
}

////////////////////////////////////////////////////////////////////////////////

void SaveConfiguration()
{
  char config_file_name[MAX_PATH];
  GetDriverConfigFile(config_file_name);

  WritePrivateProfileInt("greyscale", "Fullscreen", Configuration.fullscreen, config_file_name);
  WritePrivateProfileInt("greyscale", "VSync",      Configuration.vsync,      config_file_name);
  WritePrivateProfileInt("greyscale", "BaseColor",  Configuration.base_color, config_file_name);
}

////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK ConfigureDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  static COLORREF BaseColor;
  static HBRUSH   BaseBrush;

  switch (message)
  {
    case WM_INITDIALOG:
      CheckDlgButton(window, IDC_FULLSCREEN, Configuration.fullscreen ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(window, IDC_VSYNC,      Configuration.vsync      ? BST_CHECKED : BST_UNCHECKED);

      BaseColor = Configuration.base_color;
      
      // update the check states
      SendMessage(window, WM_COMMAND, MAKEWPARAM(IDC_FULLSCREEN, BN_PUSHED), 0);
                  
      return TRUE;

    ////////////////////////////////////////////////////////////////////////////

    case WM_DESTROY:
      if (BaseBrush)
      {
        DeleteObject(BaseBrush);
        BaseBrush = NULL;
      }
      return TRUE;

    ////////////////////////////////////////////////////////////////////////////

    case WM_COMMAND:
      switch (LOWORD(wparam))
      {
        case IDOK:
          Configuration.fullscreen = IsDlgButtonChecked(window, IDC_FULLSCREEN) != FALSE;
          Configuration.vsync      = IsDlgButtonChecked(window, IDC_VSYNC)      != FALSE;
          Configuration.base_color = BaseColor;
          EndDialog(window, 1);
          return TRUE;

        case IDCANCEL:
          EndDialog(window, 0);
          return TRUE;

        case IDC_FULLSCREEN:
          EnableWindow(GetDlgItem(window, IDC_VSYNC), IsDlgButtonChecked(window, IDC_FULLSCREEN));
          return TRUE;

        case IDC_HUE:
        {
          CHOOSECOLOR cc;
          COLORREF colors[16];
          memset(colors, 0, sizeof(colors));
          cc.lStructSize  = sizeof(cc);
          cc.hwndOwner    = window;
          cc.rgbResult    = BaseColor;
          cc.lpCustColors = colors;
          cc.Flags        = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
          if (ChooseColor(&cc))
          {
            BaseColor = cc.rgbResult;
            InvalidateRect(GetDlgItem(window, IDC_HUE_DISPLAY), NULL, TRUE);
          }
          
          return TRUE;
        }
      }
      return FALSE;

    ////////////////////////////////////////////////////////////////////////////

    case WM_CTLCOLORSTATIC:
      if (lparam == (LPARAM)GetDlgItem(window, IDC_HUE_DISPLAY))
      {
        if (BaseBrush)
        {
          DeleteObject(BaseBrush);
          BaseBrush = NULL;
        }

        BaseBrush = CreateSolidBrush(BaseColor);

        return (BOOL)BaseBrush;
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
    MessageBox(SphereWindow, "DirectDrawCreate() failed", "greyscale", MB_OK);
    return false;
  }

  // set application behavior
  ddrval = dd->SetCooperativeLevel(SphereWindow, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
  if (ddrval != DD_OK)
  {
    dd->Release();
    MessageBox(SphereWindow, "SetCooperativeLevel() failed", "greyscale", MB_OK);
    return false;
  }

  // set display mode
  ddrval = dd->SetDisplayMode(ScreenWidth, ScreenHeight, 8);
  if (ddrval != DD_OK)
  {
    dd->Release();
    MessageBox(SphereWindow, "SetDisplayMode() failed", "greyscale", MB_OK);
    return false;
  }

  // create surfaces
  retval = CreateSurfaces();
  if (retval == false)
  {
    dd->Release();
    MessageBox(SphereWindow, "CreateSurfaces() failed", "greyscale", MB_OK);
    return false;
  }

  // create a palette
  PALETTEENTRY palette_entries[256];
  for (int i = 0; i < 256; i++)
  {
    palette_entries[i].peRed   = i * GetRValue(Configuration.base_color) / 256;
    palette_entries[i].peGreen = i * GetGValue(Configuration.base_color) / 256;
    palette_entries[i].peBlue  = i * GetBValue(Configuration.base_color) / 256;
  }
  
  ddrval = dd->CreatePalette(
    DDPCAPS_8BIT | DDPCAPS_ALLOW256,
    palette_entries,
    &ddPalette,
    NULL);
  if (ddrval != DD_OK)
  {
    dd->Release();
    MessageBox(SphereWindow, "CreatePalette() failed", "Greyscale", MB_OK);
    return false;
  }

  // assign the palette to the primary surface
  ddrval = ddPrimary->SetPalette(ddPalette);
  if (ddrval != DD_OK)
  {
    dd->Release();
    MessageBox(SphereWindow, "SetPalette() failed", "Greyscale", MB_OK);
    return false;
  }

  ShowCursor(FALSE);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(bool) ToggleFullScreen() {
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

  // allocate a blitting buffer
  ScreenBuffer = new byte[ScreenWidth * ScreenHeight];

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool InitWindowed()
{
  // create the render DC
  RenderDC = CreateCompatibleDC(NULL);
  if (RenderDC == NULL)
    return false;

  // define the render DIB section
  byte* data = new byte[4096];
  memset(data, 0, 4096);
  BITMAPINFO* bmi = (BITMAPINFO*)data;

  BITMAPINFOHEADER& bmih = bmi->bmiHeader;
  bmih.biSize        = sizeof(bmih);
  bmih.biWidth       = ScreenWidth;
  bmih.biHeight      = -ScreenHeight;
  bmih.biPlanes      = 1;
  bmih.biBitCount    = 8;
  bmih.biCompression = BI_RGB;

  // define the palette
  for (int i = 0; i < 256; i++)
  {
    bmi->bmiColors[i].rgbRed   = i * GetRValue(Configuration.base_color) / 256;
    bmi->bmiColors[i].rgbGreen = i * GetGValue(Configuration.base_color) / 256;
    bmi->bmiColors[i].rgbBlue  = i * GetBValue(Configuration.base_color) / 256;
  }

  // create DIB section
  RenderBitmap = CreateDIBSection(
    RenderDC,
    bmi,
    DIB_RGB_COLORS,
    (void**)&ScreenBuffer,
    NULL,
    0);
  if (RenderBitmap == NULL)
  {
    DeleteDC(RenderDC);
    return false;
  }

  SelectObject(RenderDC, RenderBitmap);

  delete[] data;

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

    // copy the screen buffer to the surface
    byte* dst = (byte*)ddsd.lpSurface;
    byte* src = ScreenBuffer;
    for (int i = 0; i < ScreenHeight; i++)
    {
      memcpy(dst, src, ScreenWidth);
      dst += ddsd.lPitch;
      src += ScreenWidth;
    }

    // unlock the surface and do the flip!
    surface->Unlock(NULL);
    if (Configuration.vsync)
      ddPrimary->Flip(NULL, DDFLIP_WAIT);
  }
  else
  {
    // make sure the lines are on dword boundaries
    if (ScreenWidth % 4 != 0)
    {
      int pitch = (ScreenWidth + 3) * 4 / 4;
      for (int i = ScreenHeight - 1; i >= 0; i--)
      {
        memmove(ScreenBuffer + i * pitch,
                ScreenBuffer + i * ScreenWidth,
                ScreenWidth);
      }
    }

    // blit the render buffer to the window
    HDC dc = GetDC(SphereWindow);
    BitBlt(dc, 0, 0, ScreenWidth, ScreenHeight, RenderDC, 0, 0, SRCCOPY);
    ReleaseDC(SphereWindow, dc);
  }
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(IMAGE) CreateImage(int width, int height, RGBA* pixels)
{
  IMAGE image = new _IMAGE;
  image->width = width;
  image->height = height;

  FillImagePixels(image, pixels);
  OptimizeBlitRoutine(image);
  return image;
}

////////////////////////////////////////////////////////////////////////////////

void FillImagePixels(IMAGE image, RGBA* pixels)
{
  image->grey  = new byte[image->width * image->height];
  image->alpha = new byte[image->width * image->height];

  for (int i = 0; i < image->width * image->height; i++)
  {
    image->grey[i] = (pixels[i].red + pixels[i].green + pixels[i].blue) / 3;
    image->alpha[i] = pixels[i].alpha;
  }
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

  // grab grey
  image->grey = new byte[width * height];
  for (int iy = 0; iy < height; iy++)
    memcpy(image->grey + iy * width,
             ScreenBuffer + (y + iy) * ScreenWidth + x,
             width);

  // grab alpha
  image->alpha = new byte[width * height];
  memset(image->alpha, 255, width * height);

  return image;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DestroyImage(IMAGE image)
{
  delete[] image->grey;
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

class render_pixel_mask
{
public:
  render_pixel_mask(byte mask, byte alpha) : m_mask(mask), m_alpha(alpha) { }
  void operator()(byte& dst, byte src, byte alpha)
  {
    // do the masking on the source pixel
    src   = m_mask  * src   / 256;
    alpha = m_alpha * alpha / 256;

    // blit to the dest pixel
    dst = (dst * (256 - alpha) + src * alpha) / 256;
  }

private:
  byte m_mask;
  byte m_alpha;
};


EXPORT(void) BlitImageMask(IMAGE image, int x, int y, RGBA mask)
{
  byte m = (mask.red + mask.green + mask.blue) / 3;

  primitives::Blit(
    ScreenBuffer,
    ScreenWidth,
    x,
    y,
    image->grey,
    image->alpha,
    image->width,
    image->height,
    ClippingRectangle,
    render_pixel_mask(m, mask.alpha)
  );
}

////////////////////////////////////////////////////////////////////////////////

inline void render_pixel(byte& d, byte s, int a)
{
  d = (d * (256 - a) + s * a) / 256;
}

EXPORT(void) TransformBlitImage(IMAGE image, int x[4], int y[4])
{
  primitives::TexturedQuad(
    ScreenBuffer,
    ScreenWidth,
    x,
    y,
    image->grey,
    image->alpha,
    image->width,
    image->height,
    ClippingRectangle,
    render_pixel);
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) TransformBlitImageMask(IMAGE image, int x[4], int y[4], RGBA mask)
{
  byte m = (mask.red + mask.green + mask.blue) / 3;

  primitives::TexturedQuad(
    ScreenBuffer,
    ScreenWidth,
    x,
    y,
    image->grey,
    image->alpha,
    image->width,
    image->height,
    ClippingRectangle,
    render_pixel_mask(m, mask.alpha)
  );
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
    memcpy(ScreenBuffer + ((y + iy) * ScreenWidth + x + image_offset_x),
           image->grey + iy * image->width + image_offset_x,
           image_blit_width);
}

////////////////////////////////////////////////////////////////////////////////

void SpriteBlit(IMAGE image, int x, int y)
{
  calculate_clipping_metrics(image->width, image->height);

  for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
    for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
      if (image->alpha[iy * image->width + ix])
        ScreenBuffer[(y + iy) * ScreenWidth + (x + ix)] =
          image->grey[iy * image->width + ix];
}

////////////////////////////////////////////////////////////////////////////////

void NormalBlit(IMAGE image, int x, int y)
{
  calculate_clipping_metrics(image->width, image->height);

  for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
    for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
    {
      byte* dest  = ScreenBuffer + (y + iy) * ScreenWidth + x + ix;
      byte  src   = image->grey[iy * image->width + ix];
      byte  alpha = image->alpha[iy * image->width + ix];

      if (alpha == 255)
        *dest = src;
      else if (alpha > 0)
        *dest = (*dest * (256 - alpha) + src * alpha) / 256;
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
  for (int i = 0; i < image->width * image->height; i++)
  {
    image->locked_pixels[i].red   = image->grey[i];
    image->locked_pixels[i].green = image->grey[i];
    image->locked_pixels[i].blue  = image->grey[i];
  }

  // alpha
  for (int i = 0; i < image->width * image->height; i++)
    image->locked_pixels[i].alpha = image->alpha[i];

  return image->locked_pixels;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) UnlockImage(IMAGE image, bool pixels_changed)
{
  if (pixels_changed) {
    delete[] image->grey;
    delete[] image->alpha;
  
    FillImagePixels(image, image->locked_pixels);
    OptimizeBlitRoutine(image);
  }

  delete[] image->locked_pixels;
  image->locked_pixels = NULL;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DirectBlit(int x, int y, int w, int h, RGBA* pixels)
{
  calculate_clipping_metrics(w, h);
  
  for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
    for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
    {
      byte* dest  = ScreenBuffer + (y + iy) * ScreenWidth + x + ix;
      byte  src   = (pixels[iy * w + ix].red + pixels[iy * w + ix].green + pixels[iy * w + ix].blue) / 3;
      byte  alpha = pixels[iy * w + ix].alpha;

      if (alpha == 255)
        *dest = src;
      else if (alpha > 0)
      {
        *dest = (*dest * (256 - alpha) + src * alpha) / 256;
      }
    }
}

////////////////////////////////////////////////////////////////////////////////

inline void renderRGBA(byte& d, RGBA s, RGBA a)
{
  byte src = (s.red + s.green + s.blue) / 3;
  byte alpha = s.alpha;
  d = (d * (256 - alpha) + src * alpha) / 256;
}

EXPORT(void) DirectTransformBlit(int x[4], int y[4], int w, int h, RGBA* pixels)
{
  primitives::TexturedQuad(
    ScreenBuffer,
    ScreenWidth,
    x,
    y,
    pixels,
    pixels,
    w,
    h,
    ClippingRectangle,
    renderRGBA);
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DirectGrab(int x, int y, int w, int h, RGBA* pixels)
{
  if (x < 0 ||
      y < 0 ||
      x + w > ScreenWidth ||
      y + h > ScreenHeight)
    return;

  for (int iy = 0; iy < h; iy++) {
    for (int ix = 0; ix < w; ix++) {
      byte b = ScreenBuffer[(y + iy) * ScreenWidth + (x + ix)];
      pixels[iy * w + ix] = CreateRGBA(b / 3, b / 3, b / 3, 255);
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


inline void blendRGBA(byte& dest, RGBA source) {
  int color = (source.red + source.green + source.blue) / 3;
  dest = (byte)((color * source.alpha + dest * (256 - source.alpha)) / 256);
}

EXPORT(void) DrawPoint(int x, int y, RGBA color)
{
  primitives::Point(ScreenBuffer, ScreenWidth, x, y, color, ClippingRectangle, blendRGBA);
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DrawLine(int x[2], int y[2], RGBA color)
{
  primitives::Line(ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], constant_color(color), ClippingRectangle, blendRGBA);
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DrawGradientLine(int x[2], int y[2], RGBA colors[2])
{
  primitives::Line(ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], gradient_color(colors[0], colors[1]), ClippingRectangle, blendRGBA);
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DrawTriangle(int x[3], int y[3], RGBA color)
{
  primitives::Triangle(ScreenBuffer, ScreenWidth, x, y, color, ClippingRectangle, blendRGBA);
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
  primitives::GradientTriangle(ScreenBuffer, ScreenWidth, x, y, colors, ClippingRectangle, blendRGBA, interpolateRGBA);
}

////////////////////////////////////////////////////////////////////////////////

inline void copyByte(byte& dest, byte source) {
  dest = source;
}

EXPORT(void) DrawRectangle(int x, int y, int w, int h, RGBA color)
{
  if (color.alpha == 0) {          // no mask

    return;

  } else if (color.alpha == 255) { // full mask
    
    byte c = (color.red + color.green + color.blue) / 3;
    primitives::Rectangle(ScreenBuffer, ScreenWidth, x, y, w, h, c, ClippingRectangle, copyByte);

  } else {

    primitives::Rectangle(ScreenBuffer, ScreenWidth, x, y, w, h, color, ClippingRectangle, blendRGBA);

  }
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DrawGradientRectangle(int x, int y, int w, int h, RGBA colors[4])
{
  primitives::GradientRectangle(ScreenBuffer, ScreenWidth, x, y, w, h, colors, ClippingRectangle, blendRGBA, interpolateRGBA);
}

////////////////////////////////////////////////////////////////////////////////
