#define DIRECTDRAW_VERSION 0x0300
#include <windows.h>
#include <commdlg.h>
#include <ddraw.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include "../../common/common_palettes.hpp"
#include "../../common/primitives.hpp"
#include "../common/win32x.hpp"
#include "../common/video.hpp"
#include "resource.h"



typedef struct _IMAGE
{
  int width;
  int height;

  byte* pixels;
  byte* alpha;

  void (*blit_routine)(_IMAGE* image, int x, int y);

  RGBA* locked_pixels;
}* IMAGE;


struct CONFIGURATION
{
  bool fullscreen;
  bool vsync;
  char palette_file[FILENAME_MAX];
};


static void LoadConfiguration();
static void SaveConfiguration();
static BOOL CALLBACK ConfigureDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

static bool LoadPalette();
static void FillLUTs();
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

const int DOWNCAST_BITS   = 5;
const int DOWNCAST_SHIFT  = 8 - DOWNCAST_BITS;
const int DOWNCAST_COLORS = 1 << DOWNCAST_BITS;

RGB  Palette[256];
byte ReductionTable[DOWNCAST_COLORS * DOWNCAST_COLORS * DOWNCAST_COLORS];

// alpha tables

const int ALPHA_LEVELS = 8;
// result(alphalevel, dst, src)
byte AlphaLUTs[ALPHA_LEVELS][256][256];


// optimized blender with LUTs!
inline byte blend(byte dst, byte src, int alpha) {
  return AlphaLUTs[alpha / (256 / ALPHA_LEVELS)][dst][src];
}


// 24 -> 8
inline byte Pack(RGBA c) {
  c.red   >>= DOWNCAST_SHIFT;
  c.green >>= DOWNCAST_SHIFT;
  c.blue  >>= DOWNCAST_SHIFT;
  int index = (c.red << (DOWNCAST_BITS * 2)) + (c.green << DOWNCAST_BITS) + c.blue;
  return ReductionTable[index];
}


inline RGBA Unpack(byte b) {
  RGBA rgba;
  rgba.red   = Palette[b].red;
  rgba.green = Palette[b].green;
  rgba.blue  = Palette[b].blue;
  return rgba;
}


////////////////////////////////////////////////////////////////////////////////

EXPORT(void) GetDriverInfo(DRIVERINFO* driverinfo)
{
  driverinfo->name        = "Standard 8-bit Color";
  driverinfo->author      = "Chad Austin";
  driverinfo->date        = __DATE__;
  driverinfo->version     = "1.00";
  driverinfo->description = "8-bit palettized color in both windowed and fullscreen modes";
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

  Configuration.fullscreen = (GetPrivateProfileInt("standard8", "fullscreen", 1, config_file_name) != 0);
  Configuration.vsync      = (GetPrivateProfileInt("standard8", "vsync",      1, config_file_name) != 0);
  GetPrivateProfileString("standard8", "palette", "(default)", Configuration.palette_file, FILENAME_MAX, config_file_name);
}

////////////////////////////////////////////////////////////////////////////////

void SaveConfiguration()
{
  char config_file_name[MAX_PATH];
  GetDriverConfigFile(config_file_name);

  WritePrivateProfileInt("standard8", "fullscreen", Configuration.fullscreen,   config_file_name);
  WritePrivateProfileInt("standard8", "vsync",      Configuration.vsync,        config_file_name);
  WritePrivateProfileString("standard8", "palette", Configuration.palette_file, config_file_name);
}

////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK ConfigureDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  switch (message)
  {
    case WM_INITDIALOG:
      // add a default palette
      SendDlgItemMessage(window, IDC_PALETTE, CB_ADDSTRING, 0, (LPARAM)"(default)");

      char path[FILENAME_MAX];
      GetModuleFileName(DriverInstance, path, FILENAME_MAX);
      *strrchr(path, '\\') = 0;
      strcat(path, "\\*.pal");
      SendDlgItemMessage(window, IDC_PALETTE, CB_DIR, 0, (LPARAM)path);

      // now select the current palette
      SendDlgItemMessage(window, IDC_PALETTE, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)Configuration.palette_file);

      CheckDlgButton(window, IDC_FULLSCREEN,  Configuration.fullscreen ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(window, IDC_VSYNC,       Configuration.vsync      ? BST_CHECKED : BST_UNCHECKED);

      // update the check states
      SendMessage(window, WM_COMMAND, MAKEWPARAM(IDC_FULLSCREEN, BN_PUSHED), 0);
                  
      return TRUE;

    ////////////////////////////////////////////////////////////////////////////

    case WM_COMMAND:
      switch (LOWORD(wparam))
      {
        case IDOK:
        {
          Configuration.fullscreen = IsDlgButtonChecked(window, IDC_FULLSCREEN) != FALSE;
          Configuration.vsync      = IsDlgButtonChecked(window, IDC_VSYNC)      != FALSE;

          int sel = SendDlgItemMessage(window, IDC_PALETTE, CB_GETCURSEL, 0, 0);
          SendDlgItemMessage(window, IDC_PALETTE, CB_GETLBTEXT, sel, (LPARAM)Configuration.palette_file);

          EndDialog(window, 1);
          return TRUE;
        }

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

EXPORT(bool) InitVideoDriver(HWND window, int screen_width, int screen_height)
{
  SphereWindow = window;
  ScreenWidth  = screen_width;
  ScreenHeight = screen_height;

  // set default clipping rectangle
  SetClippingRectangle(0, 0, screen_width, screen_height);

  LoadConfiguration();
  if (!LoadPalette()) {
    MessageBox(window, "Error: Could not load palette", "standard8", MB_OK);
    return false;
  }

  // fill the look-up tables
  FillLUTs();

  if (Configuration.fullscreen)
    return InitFullscreen();
  else
    return InitWindowed();
}

////////////////////////////////////////////////////////////////////////////////

bool LoadPalette()
{
  if (strcmp(Configuration.palette_file, "(default)") == 0) {

    // use the default DOS palette
    memcpy(Palette, dos_palette, 256 * sizeof(RGB));
    return true;
  }

  // determine the palette filename
  char filename[FILENAME_MAX];
  GetModuleFileName(DriverInstance, filename, FILENAME_MAX);
  *strrchr(filename, '\\') = 0;
  strcat(filename, "\\");
  strcat(filename, Configuration.palette_file);

  FILE* file = fopen(filename, "rb");
  char jasc_sig[8];
  fread(jasc_sig, 1, 8, file);
  if (memcmp(jasc_sig, "JASC-PAL", 8) == 0) {
    // it's a PSP palette file

    // reopen as text
    fclose(file);
    file = fopen(filename, "r");

    // skip the first three lines
    char dummy[80];
    fgets(dummy, 80, file);
    fgets(dummy, 80, file);
    fgets(dummy, 80, file);

    // read the colors
    for (int i = 0; i < 256; i++) {
      int r = 0;
      int g = 0;
      int b = 0;
      fscanf(file, "%d %d %d", &r, &g, &b);

      Palette[i].red   = r;
      Palette[i].green = g;
      Palette[i].blue  = b;
    }

  } else {
    // it's probably a raw palette file
    fseek(file, 0, SEEK_SET);
    fread(Palette, 3, 256, file);
  }

  fclose(file);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

inline int sqr(int t) {
  return t * t;
}

////////////////////////////////////////////////////////////////////////////////

void FillLUTs()
{
  // square root look-up table
  short sqr_root[3 * 256 * 256];
  for (unsigned int i = 0; i < sizeof(sqr_root) / sizeof(*sqr_root); i++) {
    sqr_root[i] = (short)sqrt(i);
  }

  // fill the downcast look-up tables
  const int step = 256 / DOWNCAST_COLORS;
  byte* p = ReductionTable;
  for (byte r = 0; r < DOWNCAST_COLORS; r++) {
    for (byte g = 0; g < DOWNCAST_COLORS; g++) {
      for (byte b = 0; b < DOWNCAST_COLORS; b++) {

        // find out which entry in the palette most closely resembles this color
        int distance = INT_MAX;
        int index = 0;
        for (int i = 0; i < 256; i++) {
          int new_distance = sqr_root[
            sqr(r * step - Palette[i].red) +
            sqr(g * step - Palette[i].green) +
            sqr(b * step - Palette[i].blue)
          ];

          if (new_distance < distance) {
            distance = new_distance;
            index = i;
            if (distance == 0) {
              break;
            }
          }
        }

        *p++ = index;
      }
    }
  }

  // the alpha look-up tables
  for (int i = 0; i < ALPHA_LEVELS; i++) {
    int alpha = i * ALPHA_LEVELS + ALPHA_LEVELS / 2;

    if (i == 0) {

      // transparent
      for (int j = 0; j < 256; j++) {
        for (int k = 0; k < 256; k++) {
          AlphaLUTs[i][j][k] = j;
        }
      }

    } else if (i == ALPHA_LEVELS - 1) {

      // transparent
      for (int j = 0; j < 256; j++) {
        for (int k = 0; k < 256; k++) {
          AlphaLUTs[i][j][k] = k;
        }
      }
    
    } else {

      // translucent
      for (int j = 0; j < 256; j++) {
        RGBA dst = Unpack(j);

        for (int k = 0; k < 256; k++) {
          RGBA src = Unpack(k);

          // calculate destination color
          dst.red   = (dst.red   * (256 - alpha) + src.red   * alpha) / 256;
          dst.green = (dst.green * (256 - alpha) + src.green * alpha) / 256;
          dst.blue  = (dst.blue  * (256 - alpha) + src.blue  * alpha) / 256;

          // now repack that
          AlphaLUTs[i][j][k] = Pack(dst);
        }
      }

    } // end if
  } // end for alpha levels
}

////////////////////////////////////////////////////////////////////////////////

bool InitFullscreen()
{
  // store old window styles
  OldWindowStyle = GetWindowLong(SphereWindow, GWL_STYLE);
  OldWindowStyleEx = GetWindowLong(SphereWindow, GWL_EXSTYLE);

  SetWindowLong(SphereWindow, GWL_STYLE, WS_POPUP);
  SetWindowLong(SphereWindow, GWL_EXSTYLE, 0);

  // create DirectDraw object
  HRESULT ddrval = DirectDrawCreate(NULL, &dd, NULL);
  if (ddrval != DD_OK)
  {
    MessageBox(SphereWindow, "DirectDrawCreate() failed", "standard8", MB_OK);
    return false;
  }

  // set application behavior
  ddrval = dd->SetCooperativeLevel(SphereWindow, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
  if (ddrval != DD_OK)
  {
    dd->Release();
    MessageBox(SphereWindow, "SetCooperativeLevel() failed", "standard8", MB_OK);
    return false;
  }

  // set display mode
  ddrval = dd->SetDisplayMode(ScreenWidth, ScreenHeight, 8);
  if (ddrval != DD_OK)
  {
    dd->Release();
    MessageBox(SphereWindow, "SetDisplayMode() failed", "standard8", MB_OK);
    return false;
  }

  // create surfaces
  bool retval = CreateSurfaces();
  if (retval == false)
  {
    dd->Release();
    MessageBox(SphereWindow, "CreateSurfaces() failed", "standard8", MB_OK);
    return false;
  }

  // create a palette
  PALETTEENTRY palette_entries[256];
  for (int i = 0; i < 256; i++)
  {
    RGBA color = Unpack(i);
    palette_entries[i].peRed   = color.red;
    palette_entries[i].peGreen = color.green;
    palette_entries[i].peBlue  = color.blue;
  }
  
  ddrval = dd->CreatePalette(
    DDPCAPS_8BIT | DDPCAPS_ALLOW256,
    palette_entries,
    &ddPalette,
    NULL);
  if (ddrval != DD_OK)
  {
    dd->Release();
    MessageBox(SphereWindow, "CreatePalette() failed", "standard8", MB_OK);
    return false;
  }

  // assign the palette to the primary surface
  ddrval = ddPrimary->SetPalette(ddPalette);
  if (ddrval != DD_OK)
  {
    dd->Release();
    MessageBox(SphereWindow, "SetPalette() failed", "standard8", MB_OK);
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
    RGBA color = Unpack(i);
    bmi->bmiColors[i].rgbRed   = color.red;
    bmi->bmiColors[i].rgbGreen = color.green;
    bmi->bmiColors[i].rgbBlue  = color.blue;
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
  image->pixels  = new byte[image->width * image->height];
  image->alpha = new byte[image->width * image->height];

  for (int i = 0; i < image->width * image->height; i++)
  {
    image->pixels[i] = Pack(pixels[i]);
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
  image->pixels = new byte[width * height];
  for (int iy = 0; iy < height; iy++)
    memcpy(image->pixels + iy * width,
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
  delete[] image->pixels;
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
  render_pixel_mask(RGBA mask) : m_mask(mask) { }
  void operator()(byte& dst, byte src, byte alpha)
  {
    RGBA d = Unpack(dst);
    RGBA s = Unpack(src);

    // do the masking on the source pixel
    alpha   = alpha   * m_mask.alpha / 256;
    s.red   = s.red   * m_mask.red   / 256;
    s.green = s.green * m_mask.green / 256;
    s.blue  = s.blue  * m_mask.blue  / 256;

    // blit to the dest pixel
    d.red   = (d.red   * (256 - alpha) + s.red   * alpha) / 256;
    d.green = (d.green * (256 - alpha) + s.green * alpha) / 256;
    d.blue  = (d.blue  * (256 - alpha) + s.blue  * alpha) / 256;

    dst = Pack(d);
  }

private:
  RGBA m_mask;
};


EXPORT(void) BlitImageMask(IMAGE image, int x, int y, RGBA mask)
{
  primitives::Blit(
    ScreenBuffer,
    ScreenWidth,
    x,
    y,
    image->pixels,
    image->alpha,
    image->width,
    image->height,
    ClippingRectangle,
    render_pixel_mask(mask)
  );
}

////////////////////////////////////////////////////////////////////////////////

inline void render_pixel(byte& d, byte s, byte a)
{
  d = blend(d, s, a);
}

EXPORT(void) TransformBlitImage(IMAGE image, int x[4], int y[4])
{
  primitives::TexturedQuad(
    ScreenBuffer,
    ScreenWidth,
    x,
    y,
    image->pixels,
    image->alpha,
    image->width,
    image->height,
    ClippingRectangle,
    render_pixel
  );
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) TransformBlitImageMask(IMAGE image, int x[4], int y[4], RGBA mask)
{
  primitives::TexturedQuad(
    ScreenBuffer,
    ScreenWidth,
    x,
    y,
    image->pixels,
    image->alpha,
    image->width,
    image->height,
    ClippingRectangle,
    render_pixel_mask(mask)
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
           image->pixels + iy * image->width + image_offset_x,
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
          image->pixels[iy * image->width + ix];
}

////////////////////////////////////////////////////////////////////////////////

void NormalBlit(IMAGE image, int x, int y)
{
  calculate_clipping_metrics(image->width, image->height);

  for (int iy = image_offset_y; iy < image_offset_y + image_blit_height; iy++)
    for (int ix = image_offset_x; ix < image_offset_x + image_blit_width; ix++)
    {
      byte* dest  = ScreenBuffer + (y + iy) * ScreenWidth + x + ix;
      byte  src   = image->pixels[iy * image->width + ix];
      byte  alpha = image->alpha[iy * image->width + ix];

      if (alpha == 255)
        *dest = src;
      else if (alpha > 0)
        *dest = blend(*dest, src, alpha);
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
    image->locked_pixels[i] = Unpack(image->pixels[i]);

  // alpha
  for (int i = 0; i < image->width * image->height; i++)
    image->locked_pixels[i].alpha = image->alpha[i];

  return image->locked_pixels;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) UnlockImage(IMAGE image, bool pixels_changed)
{
  if (pixels_changed) {
    delete[] image->pixels;
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
      byte  src   = Pack(pixels[iy * w + ix]);
      byte  alpha = pixels[iy * w + ix].alpha;

      if (alpha == 255)
        *dest = src;
      else if (alpha > 0)
        *dest = blend(*dest, src, alpha);
    }
}

////////////////////////////////////////////////////////////////////////////////

void render_rgba(byte& d, RGBA s, RGBA a)
{
  d = blend(d, Pack(s), a.alpha);
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
    render_rgba
  );
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
      pixels[iy * w + ix] = Unpack(ScreenBuffer[(y + iy) * ScreenWidth + (x + ix)]);
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
    color.red   = (i * m_color1.red   + (range - i) * m_color2.red)   / range;
    color.green = (i * m_color1.green + (range - i) * m_color2.green) / range;
    color.blue  = (i * m_color1.blue  + (range - i) * m_color2.blue)  / range;
    color.alpha = (i * m_color1.alpha + (range - i) * m_color2.alpha) / range;
    return color;
  }

private:
  RGBA m_color1;
  RGBA m_color2;
};


inline void copyByte(byte& dest, byte source) {
  dest = source;
}

inline void blendRGBA(byte& dest, RGBA source) {
  dest = blend(dest, Pack(source), source.alpha);
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

EXPORT(void) DrawRectangle(int x, int y, int w, int h, RGBA color)
{
  if (color.alpha == 0) {          // no mask

    return;

  } else if (color.alpha == 255) { // full mask
    
    byte b = Pack(color);
    primitives::Rectangle(ScreenBuffer, ScreenWidth, x, y, w, h, b, ClippingRectangle, copyByte);

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
