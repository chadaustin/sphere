#define DIRECTDRAW_VERSION 0x0300

#include <windows.h>
#include <ddraw.h>
#include <stdio.h>

#include "2xSaI.h"
#include "hq2x.h"

#include "../../common/rgb.hpp"
#include "../../common/primitives.hpp"
#include "../common/video.hpp"
#include "../common/win32x.hpp"
#include "resource.h"

// DATA TYPES //
typedef struct _IMAGE
{
    int width;
    int height;

    word* rgb;
    byte* alpha;

    void (*blit_routine)(_IMAGE* image, int x, int y);

    RGBA* original;
}
* IMAGE;

enum SCALE_ALGORITHM
{
    I_DIRECT_SCALE = 0,
    I_SCALE2X      = 1,
    I_EAGLE        = 2,
    I_HQ2X         = 3,
    I_2XSAI        = 4,
    I_SUPER_2XSAI  = 5,
    I_SUPER_EAGLE  = 6,
};

struct CONFIGURATION
{
    bool fullscreen;
    bool vsync;
    int  algorithm;
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
static void Interpolate(word* dest);
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
static enum
{
    RGB565,
    RGB555,
}
PixelFormat;

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
EXPORT(void) GetDriverInfo(DRIVERINFO* driverinfo)
{
    driverinfo->name        = "Scaled 16-bit Color";
    driverinfo->author      = "Chad Austin\nAnatoli Steinmark";
    driverinfo->date        = __DATE__;
    driverinfo->version     = "v1.1";
    driverinfo->description = "Scaled (2x) 15/16-bit color output in both windowed and fullscreen modes";
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
    Configuration.fullscreen        = (GetPrivateProfileInt("scale16", "Fullscreen",   1, config_file_name) != 0);
    Configuration.vsync             = (GetPrivateProfileInt("scale16", "VSync",        1, config_file_name) != 0);
    Configuration.algorithm         =  GetPrivateProfileInt("scale16", "Algorithm",    0, config_file_name);
}

////////////////////////////////////////////////////////////////////////////////
void SaveConfiguration()
{
    char config_file_name[MAX_PATH];
    GetDriverConfigFile(config_file_name);
    WritePrivateProfileInt("scale16", "Fullscreen",       Configuration.fullscreen,        config_file_name);
    WritePrivateProfileInt("scale16", "VSync",            Configuration.vsync,             config_file_name);
    WritePrivateProfileInt("scale16", "Algorithm",        Configuration.algorithm,         config_file_name);
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

        switch (Configuration.algorithm)
        {
            case I_DIRECT_SCALE:
                CheckDlgButton(window, IDC_DIRECT_SCALE,  BST_CHECKED);
                break;

            case I_SCALE2X:
                CheckDlgButton(window, IDC_SCALE2X,       BST_CHECKED);
                break;

            case I_EAGLE:
                CheckDlgButton(window, IDC_EAGLE,         BST_CHECKED);
                break;

            case I_HQ2X:
                CheckDlgButton(window, IDC_HQ2X,          BST_CHECKED);
                break;

            case I_2XSAI:
                CheckDlgButton(window, IDC_2XSAI,         BST_CHECKED);
                break;

            case I_SUPER_2XSAI:
                CheckDlgButton(window, IDC_SUPER_2XSAI,   BST_CHECKED);
                break;

            case I_SUPER_EAGLE:
                CheckDlgButton(window, IDC_SUPER_EAGLE,   BST_CHECKED);
                break;

        }

        // update the check states
        SendMessage(window, WM_COMMAND, MAKEWPARAM(IDC_FULLSCREEN, BN_PUSHED), 0);
        return TRUE;

        ////////////////////////////////////////////////////////////////////////////

    case WM_COMMAND:

        switch (LOWORD(wparam))
        {
            case IDOK:

                Configuration.fullscreen = (IsDlgButtonChecked(window, IDC_FULLSCREEN) != FALSE);
                Configuration.vsync      = (IsDlgButtonChecked(window, IDC_VSYNC)      != FALSE);

                if (IsDlgButtonChecked(window, IDC_DIRECT_SCALE) == BST_CHECKED)
                    Configuration.algorithm =    I_DIRECT_SCALE;

                if (IsDlgButtonChecked(window, IDC_SCALE2X)      == BST_CHECKED)
                    Configuration.algorithm =    I_SCALE2X;

                if (IsDlgButtonChecked(window, IDC_EAGLE)        == BST_CHECKED)
                    Configuration.algorithm =    I_EAGLE;

                if (IsDlgButtonChecked(window, IDC_HQ2X)         == BST_CHECKED)
                    Configuration.algorithm =    I_HQ2X;

                if (IsDlgButtonChecked(window, IDC_2XSAI)        == BST_CHECKED)
                    Configuration.algorithm =    I_2XSAI;

                if (IsDlgButtonChecked(window, IDC_SUPER_2XSAI)  == BST_CHECKED)
                    Configuration.algorithm =    I_SUPER_2XSAI;

                if (IsDlgButtonChecked(window, IDC_SUPER_EAGLE)  == BST_CHECKED)
                    Configuration.algorithm =    I_SUPER_EAGLE;


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

    ScreenBuffer = new word[ScreenWidth * ScreenHeight];

    if (!ScreenBuffer)
        return false;

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
        MessageBox(SphereWindow, "DirectDrawCreate() failed", "scale16", MB_OK);
        return false;
    }

    // set application behavior
    ddrval = dd->SetCooperativeLevel(SphereWindow, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
    if (ddrval != DD_OK)
    {
        dd->Release();
        MessageBox(SphereWindow, "SetCooperativeLevel() failed", "scale16", MB_OK);
        return false;
    }

    // set display mode
    retval = SetDisplayMode();
    if (retval == false)
    {
        dd->Release();
        MessageBox(SphereWindow, "SetDisplayMode() failed", "scale16", MB_OK);
        return false;
    }

    // create surfaces
    retval = CreateSurfaces();
    if (retval == false)
    {
        dd->Release();
        MessageBox(SphereWindow, "CreateSurfaces() failed", "scale16", MB_OK);
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
    HRESULT ddrval = dd->SetDisplayMode(ScreenWidth  * 2, ScreenHeight * 2, 16);

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
    {
        PixelFormat = RGB565;
    }
    else if (ddpf.dwRBitMask == 0x7C00)
    {
        PixelFormat = RGB555;
    }
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
    bmih.biWidth       =  ScreenWidth  * 2;
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
    AdjustWindowRectEx(&WindowRect,
                       GetWindowLong(SphereWindow, GWL_STYLE),
                       (GetMenu(SphereWindow) ? TRUE : FALSE),
                       GetWindowLong(SphereWindow, GWL_EXSTYLE));

    int window_width  = WindowRect.right  - WindowRect.left;
    int window_height = WindowRect.bottom - WindowRect.top;

    MoveWindow(SphereWindow,
               (GetSystemMetrics(SM_CXSCREEN) - window_width)  / 2,
               (GetSystemMetrics(SM_CYSCREEN) - window_height) / 2,
               window_width,
               window_height,
               TRUE);

    // we know that 16-bit color DIBs are always 5:5:5
    PixelFormat = RGB555;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) CloseVideoDriver()
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

        Interpolate((word*)ddsd.lpSurface);

        // unlock the surface and do the flip!
        surface->Unlock(NULL);
        if (Configuration.vsync)
            ddPrimary->Flip(NULL, DDFLIP_WAIT);
    }
    else
    {

        Interpolate(RenderBuffer);

        // blit the render buffer to the window
        HDC dc = GetDC(SphereWindow);
        BitBlt(dc, 0, 0, ScreenWidth  * 2, ScreenHeight * 2, RenderDC, 0, 0, SRCCOPY);
        ReleaseDC(SphereWindow, dc);
    }
    return;
}

////////////////////////////////////////////////////////////////////////////////
void DirectScale(word* dst, word* src, int src_width, int src_height)
{
    /*
    +--+
    |E |
    +--+

    +--+--+
    |E0|E1|
    +--+--+
    |E2|E3|
    +--+--+

    E: src[0]

    E0: dst0[0]
    E2: dst1[0]
    */

    int dst_pitch = src_width * 2;
    word* dst0 = dst;
    word* dst1 = dst + dst_pitch;

    int iy = src_height;
    int ix;

    while (iy--)
    {
        ix = src_width;

        while (ix--)
        {
            dst0[0] = src[0];
            dst0[1] = src[0];
            dst1[0] = src[0];
            dst1[1] = src[0];

            ++src;
            dst0 += 2;
            dst1 += 2;
        }

        dst0 += dst_pitch;
        dst1 += dst_pitch;
    }
}

////////////////////////////////////////////////////////////////////////////////
void Scale2x(word* dst, word* src, int src_width, int src_height)
{
    /*
    +--+--+--+
    |A |B |C |
    +--+--+--+
    |D |E |F |
    +--+--+--+
    |G |H |I |
    +--+--+--+

    +--+--+
    |E0|E1|
    +--+--+
    |E2|E3|
    +--+--+

    B: src0[0]
    E: src1[0]
    H: src2[0]

    E0: dst0[0]
    E2: dst1[0]
    */

    int src_pitch = src_width;
    int dst_pitch = src_width * 2;

    word* dst0 = dst;
    word* dst1 = dst + dst_pitch;
    word* src0;
    word* src1 = src;
    word* src2;

    int iy = src_height;
    int ix;

    // left and right columns
    while (iy--)
    {
        dst0[0] = src1[0];
        dst1[0] = src1[0];
        dst0[1] = src1[0];
        dst1[1] = src1[0];

        src1 += src_pitch - 1;
        dst0 += dst_pitch - 2;
        dst1 += dst_pitch - 2;

        dst0[0] = src1[0];
        dst1[0] = src1[0];
        dst0[1] = src1[0];
        dst1[1] = src1[0];

        src1 += 1;
        dst0 += dst_pitch + 2;
        dst1 += dst_pitch + 2;
    }

    dst0 = dst + 2;
    dst1 = dst + 2 + dst_pitch;
    src1 = src + 1;

    int src_helper = src_pitch * (src_height - 1);
    int dst_helper = src_helper * 4;
    ix = src_pitch - 2;

    // top and bottom rows
    while (ix--)
    {
        dst0[0] = src1[0];
        dst1[0] = src1[0];
        dst0[1] = src1[0];
        dst1[1] = src1[0];

        src1 += src_helper;
        dst0 += dst_helper;
        dst1 += dst_helper;

        dst0[0] = src1[0];
        dst1[0] = src1[0];
        dst0[1] = src1[0];
        dst1[1] = src1[0];

        src1 -= src_helper - 1;
        dst0 -= dst_helper - 2;
        dst1 -= dst_helper - 2;
    }

    dst0 = dst + 2 + dst_pitch * 2;
    dst1 = dst + 2 + dst_pitch * 3;
    src0 = src + 1;
    src1 = src + 1 + src_pitch;
    src2 = src + 1 + src_pitch * 2;

    iy = src_height - 2;

    // middle
    while (iy--)
    {
        ix = src_pitch - 2;

        while (ix--)
        {
            if (src0[0] != src2[0] && src1[-1] != src1[1])
            {
                dst0[0] = src1[-1] == src0[0] ? src1[-1] : src1[0];
                dst0[1] = src0[0]  == src1[1] ? src1[1]  : src1[0];
                dst1[0] = src1[-1] == src2[0] ? src1[-1] : src1[0];
                dst1[1] = src2[0]  == src1[1] ? src1[1]  : src1[0];
            }
            else
            {
                dst0[0] = src1[0];
                dst0[1] = src1[0];
                dst1[0] = src1[0];
                dst1[1] = src1[0];
            }

            ++src0;
            ++src1;
            ++src2;
            dst0 += 2;
            dst1 += 2;
        }

        src0 += 2;
        src1 += 2;
        src2 += 2;
        dst0 += dst_pitch + 4;
        dst1 += dst_pitch + 4;
    }
}

////////////////////////////////////////////////////////////////////////////////
void Eagle(word* dst, word* src, int src_width, int src_height)
{
    int src_pitch = src_width;
    int dst_pitch = src_width * 2;

    word* dst0 = dst;
    word* dst1 = dst + dst_pitch;
    word* src0;
    word* src1 = src;
    word* src2;

    int iy = src_height;
    int ix;

    // left and right columns
    while (iy--)
    {
        dst0[0] = src1[0];
        dst1[0] = src1[0];
        dst0[1] = src1[0];
        dst1[1] = src1[0];

        src1 += src_pitch - 1;
        dst0 += dst_pitch - 2;
        dst1 += dst_pitch - 2;

        dst0[0] = src1[0];
        dst1[0] = src1[0];
        dst0[1] = src1[0];
        dst1[1] = src1[0];

        src1 += 1;
        dst0 += dst_pitch + 2;
        dst1 += dst_pitch + 2;
    }

    dst0 = dst + 2;
    dst1 = dst + 2 + dst_pitch;
    src1 = src + 1;

    int src_helper = src_pitch * (src_height - 1);
    int dst_helper = src_helper * 4;
    ix = src_pitch - 2;

    // top and bottom rows
    while (ix--)
    {
        dst0[0] = src1[0];
        dst1[0] = src1[0];
        dst0[1] = src1[0];
        dst1[1] = src1[0];

        src1 += src_helper;
        dst0 += dst_helper;
        dst1 += dst_helper;

        dst0[0] = src1[0];
        dst1[0] = src1[0];
        dst0[1] = src1[0];
        dst1[1] = src1[0];

        src1 -= src_helper - 1;
        dst0 -= dst_helper - 2;
        dst1 -= dst_helper - 2;
    }

    dst0 = dst + 2 + dst_pitch * 2;
    dst1 = dst + 2 + dst_pitch * 3;
    src0 = src + 1;
    src1 = src + 1 + src_pitch;
    src2 = src + 1 + src_pitch * 2;

    iy = src_height - 2;

    // middle
    while (iy--)
    {
        ix = src_pitch - 2;

        while (ix--)
        {

            if (src0[0] == src0[-1] && src0[-1] == src1[-1])
                dst0[0] = src1[-1];
            else
                dst0[0] = src1[0];

            if (src0[0] == src0[1] && src0[1] == src1[1])
                dst0[1] = src1[1];
            else
                dst0[1] = src1[0];

            if (src2[0] == src2[-1] && src2[-1] == src1[-1])
                dst1[0] = src1[-1];
            else
                dst1[0] = src1[0];

            if (src2[0] == src2[1] && src2[1] == src1[1])
                dst1[1] = src1[1];
            else
                dst1[1] = src1[0];

            ++src0;
            ++src1;
            ++src2;
            dst0 += 2;
            dst1 += 2;
        }

        src0 += 2;
        src1 += 2;
        src2 += 2;
        dst0 += dst_pitch + 4;
        dst1 += dst_pitch + 4;
    }
}

////////////////////////////////////////////////////////////////////////////////
void Interpolate(word* dst)
{
    if (PixelFormat == RGB565)
    {
        switch (Configuration.algorithm)
        {
            case I_DIRECT_SCALE:
                DirectScale(dst, ScreenBuffer, ScreenWidth, ScreenHeight);
                break;

            case I_SCALE2X:
                Scale2x(dst, ScreenBuffer, ScreenWidth, ScreenHeight);
                break;

            case I_EAGLE:
                Eagle(dst, ScreenBuffer, ScreenWidth, ScreenHeight);
                break;

            case I_HQ2X:
                hq2x(dst, ScreenBuffer, ScreenWidth, ScreenHeight, 16);
                break;

            case I_2XSAI:
                _2xSaI(dst, ScreenBuffer, ScreenWidth, ScreenHeight, 16);
                break;

            case I_SUPER_2XSAI:
                Super2xSaI(dst, ScreenBuffer, ScreenWidth, ScreenHeight, 16);
                break;

            case I_SUPER_EAGLE:
                SuperEagle(dst, ScreenBuffer, ScreenWidth, ScreenHeight, 16);
                break;

        }
    }
    else
    {
        switch (Configuration.algorithm)
        {
            case I_DIRECT_SCALE:
                DirectScale(dst, ScreenBuffer, ScreenWidth, ScreenHeight);
                break;

            case I_SCALE2X:
                Scale2x(dst, ScreenBuffer, ScreenWidth, ScreenHeight);
                break;

            case I_EAGLE:
                Eagle(dst, ScreenBuffer, ScreenWidth, ScreenHeight);
                break;

            case I_HQ2X:
                hq2x(dst, ScreenBuffer, ScreenWidth, ScreenHeight, 15);
                break;

            case I_2XSAI:
                _2xSaI(dst, ScreenBuffer, ScreenWidth, ScreenHeight, 15);
                break;

            case I_SUPER_2XSAI:
                Super2xSaI(dst, ScreenBuffer, ScreenWidth, ScreenHeight, 15);
                break;

            case I_SUPER_EAGLE:
                SuperEagle(dst, ScreenBuffer, ScreenWidth, ScreenHeight, 15);
                break;

        }
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(IMAGE) CreateImage(int width, int height, RGBA* pixels)
{
    IMAGE image = new _IMAGE;

    if (image)
    {
        image->width  = width;
        image->height = height;

        FillImagePixels(image, pixels);
        OptimizeBlitRoutine(image);
    }

    return image;
}

////////////////////////////////////////////////////////////////////////////////
void FillImagePixels(IMAGE image, RGBA* pixels)
{
    int pixels_total = image->width * image->height;

    // fill the original data
    if (pixels != image->original)
    {
        image->original = new RGBA[pixels_total];

        if (!image->original)
            return;

        memcpy(image->original, pixels, pixels_total * sizeof(RGBA));
    }

    // fill the premultiplied data
    RGBA pixel;
    image->rgb = new word[pixels_total];

    if (!image->rgb)
        return;

    if (PixelFormat == RGB565)
    {
        for (int i = 0; i < image->width * image->height; i++)
        {
            pixel = pixels[i];

            // premultiply
            pixel.red   = (pixel.red   * pixel.alpha) / 255;
            pixel.green = (pixel.green * pixel.alpha) / 255;
            pixel.blue  = (pixel.blue  * pixel.alpha) / 255;

            image->rgb[i] = PackPixel565(pixel);
        }
    }
    else
    {
        for (int i = 0; i < image->width * image->height; i++)
        {
            pixel = pixels[i];

            // premultiply
            pixel.red   = (pixel.red   * pixel.alpha) / 255;
            pixel.green = (pixel.green * pixel.alpha) / 255;
            pixel.blue  = (pixel.blue  * pixel.alpha) / 255;

            image->rgb[i] = PackPixel555(pixel);
        }
    }

    // alpha
    image->alpha = new byte[pixels_total];

    if (!image->alpha)
        return;

    for (int i = 0; i < pixels_total; i++)
        image->alpha[i] = pixels[i].alpha;

}

////////////////////////////////////////////////////////////////////////////////
void OptimizeBlitRoutine(IMAGE image)
{
    int pixels_total = image->width * image->height;

    // null blit
    bool is_empty = true;
    for (int i = 0; i < pixels_total; i++)
    {
        if (image->alpha[i])
        {
            is_empty = false;
            break;
        }
    }

    if (is_empty)
    {
        image->blit_routine = NullBlit;
        return;
    }

    // tile blit
    bool is_tile = true;
    for (int i = 0; i < pixels_total; i++)
    {
        if (image->alpha[i] < 255)
        {
            is_tile = false;
            break;
        }
    }

    if (is_tile)
    {
        image->blit_routine = TileBlit;
        return;
    }

    // sprite blit
    bool is_sprite = true;
    for (int i = 0; i < pixels_total; i++)
    {
        if (image->alpha[i] > 0 && image->alpha[i] < 255)
        {
            is_sprite = false;
            break;
        }
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
        x + width  > ScreenWidth ||
        y + height > ScreenHeight)
        return NULL;

    int pixels_total = width * height;

    IMAGE image         = new _IMAGE;
    image->width        = width;
    image->height       = height;
    image->blit_routine = TileBlit;
    image->rgb          = new word[pixels_total];
    image->alpha        = new byte[pixels_total];
    image->original     = new RGBA[pixels_total];

    for (int iy = 0; iy < height; iy++)
        memcpy(image->rgb + iy * width, ScreenBuffer + (y + iy) * ScreenWidth + x, width * 2);

    memset(image->alpha, 255, pixels_total);

    if (PixelFormat == RGB565)
    {
        for (int i = 0; i < pixels_total; ++i)
        {
            image->original[i]       = UnpackPixel565(image->rgb[i]);
            image->original[i].alpha = 255;
        }
    }
    else
    {
        for (int i = 0; i < pixels_total; ++i)
        {
            image->original[i]       = UnpackPixel555(image->rgb[i]);
            image->original[i].alpha = 255;
        }
    }

    return image;
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DestroyImage(IMAGE image)
{
    delete[] image->rgb;
    delete[] image->alpha;
    delete[] image->original;
    delete image;
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) BlitImage(IMAGE image, int x, int y)
{
    // don't draw it if it's off the screen
    if (x + image->width  < ClippingRectangle.left  ||
        y + image->height < ClippingRectangle.top   ||
        x                 > ClippingRectangle.right ||
        y                 > ClippingRectangle.bottom)
        return;

    image->blit_routine(image, x, y);
}

////////////////////////////////////////////////////////////////////////////////
class render_pixel_mask_565
{
public:
    render_pixel_mask_565(RGBA mask) : m_mask(mask)
    { }

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
    render_pixel_mask_555(RGBA mask) : m_mask(mask)
    { }

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
EXPORT(void) BlitImageMask(IMAGE image, int x, int y, RGBA mask)
{
    if (PixelFormat == RGB565)
    {

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
    }
    else
    {

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
EXPORT(void) TransformBlitImage(IMAGE image, int x[4], int y[4])
{
    if (PixelFormat == RGB565)
    {

        primitives::TexturedQuad(ScreenBuffer, ScreenWidth, x, y, image->rgb, image->alpha, image->width, image->height, ClippingRectangle, renderpixel565);
    }
    else
    {

        primitives::TexturedQuad(ScreenBuffer, ScreenWidth, x, y, image->rgb, image->alpha, image->width, image->height, ClippingRectangle, renderpixel555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) TransformBlitImageMask(IMAGE image, int x[4], int y[4], RGBA mask)
{
    if (PixelFormat == RGB565)
    {

        primitives::TexturedQuad(ScreenBuffer, ScreenWidth, x, y, image->rgb, image->alpha, image->width, image->height, ClippingRectangle, render_pixel_mask_565(mask));
    }
    else
    {

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

    word* dst   = (word*)ScreenBuffer + (y + image_offset_y) * ScreenWidth  + image_offset_x + x;
    word* src   = (word*)image->rgb   +      image_offset_y  * image->width + image_offset_x;
    byte* alpha = image->alpha        +      image_offset_y  * image->width + image_offset_x;

    int dst_inc = ScreenWidth  - image_blit_width;
    int src_inc = image->width - image_blit_width;
    int iy = image_blit_height;
    int ix;

    while (iy-- > 0)
    {
        ix = image_blit_width;
        while (ix-- > 0)
        {
            if (*alpha)
                *dst = *src;

            ++dst;
            ++src;
            ++alpha;
        }

        dst   += dst_inc;
        src   += src_inc;
        alpha += src_inc;
    }
}

////////////////////////////////////////////////////////////////////////////////
void NormalBlit(IMAGE image, int x, int y)
{
    calculate_clipping_metrics(image->width, image->height);

    int a;
    word result;

    word* dst   = (word*)ScreenBuffer + (y + image_offset_y) * ScreenWidth  + image_offset_x + x;
    word* src   = (word*)image->rgb   +      image_offset_y  * image->width + image_offset_x;
    byte* alpha = image->alpha        +      image_offset_y  * image->width + image_offset_x;

    int dst_inc = ScreenWidth  - image_blit_width;
    int src_inc = image->width - image_blit_width;
    int iy = image_blit_height;
    int ix;

    if (PixelFormat == RGB565)
    {
        while (iy-- > 0)
        {
            ix = image_blit_width;
            while (ix-- > 0)
            {
                a = 256 - *alpha;

                result  =  ((dst[0] & 0x001F) * a >> 8) + (src[0] & 0x001F);
                result |= (((dst[0] & 0x07E0) * a >> 8) + (src[0] & 0x07E0)) & 0x07E0;
                result |= (((dst[0] & 0xF800) * a >> 8) + (src[0] & 0xF800)) & 0xF800;

                dst[0] = result;
                ++dst;
                ++src;
                ++alpha;
            }

            dst   += dst_inc;
            src   += src_inc;
            alpha += src_inc;
        }
    }
    else
    {
        while (iy-- > 0)
        {
            ix = image_blit_width;
            while (ix-- > 0)
            {
                a = 256 - *alpha;

                result  =  ((dst[0] & 0x001F) * a >> 8) + (src[0] & 0x001F);
                result |= (((dst[0] & 0x03E0) * a >> 8) + (src[0] & 0x03E0)) & 0x03E0;
                result |= (((dst[0] & 0x7C00) * a >> 8) + (src[0] & 0x7C00)) & 0x7C00;

                dst[0] = result;
                ++dst;
                ++src;
                ++alpha;
            }

            dst   += dst_inc;
            src   += src_inc;
            alpha += src_inc;
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
    return image->original;
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) UnlockImage(IMAGE image, bool pixels_changed)
{
    if (pixels_changed)
    {
        delete[] image->rgb;
        delete[] image->alpha;

        FillImagePixels(image, image->original);
        OptimizeBlitRoutine(image);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DirectBlit(int x, int y, int w, int h, RGBA* pixels)
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
EXPORT(void) DirectTransformBlit(int x[4], int y[4], int w, int h, RGBA* pixels)
{
    if (PixelFormat == RGB565)
    {

        primitives::TexturedQuad(ScreenBuffer, ScreenWidth, x, y, pixels, pixels, w, h, ClippingRectangle, blendRGBAto565);
    }
    else
    {

        primitives::TexturedQuad(ScreenBuffer, ScreenWidth, x, y, pixels, pixels, w, h, ClippingRectangle, blendRGBAto555);
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
                pixels[iy * w + ix]       = UnpackPixel555(ScreenBuffer[(y + iy) * ScreenWidth + x + ix]);
                pixels[iy * w + ix].alpha = 255;
            }
    }
}

////////////////////////////////////////////////////////////////////////////////
class constant_color
{
public:
    constant_color(RGBA color)
            : m_color(color)
    {
    }

    RGBA operator()(int i, int range)
    {

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
            , m_color2(color2)
    {
    }

    RGBA operator()(int i, int range)
    {

        if (range == 0)
        {

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
inline void copyWord(word& dest, word source)
{

    dest = source;
}
inline void blend565(word& dest, RGBA source)
{

    RGBA out = UnpackPixel565(dest);
    out.red   = (source.red   * source.alpha + out.red   * (256 - source.alpha)) / 256;
    out.green = (source.green * source.alpha + out.green * (256 - source.alpha)) / 256;
    out.blue  = (source.blue  * source.alpha + out.blue  * (256 - source.alpha)) / 256;
    dest = PackPixel565(out);
}
inline void blend555(word& dest, RGBA source)
{

    RGBA out = UnpackPixel555(dest);
    out.red   = (source.red   * source.alpha + out.red   * (256 - source.alpha)) / 256;
    out.green = (source.green * source.alpha + out.green * (256 - source.alpha)) / 256;
    out.blue  = (source.blue  * source.alpha + out.blue  * (256 - source.alpha)) / 256;
    dest = PackPixel555(out);
}
EXPORT(void) DrawPoint(int x, int y, RGBA color)
{
    if (PixelFormat == RGB565)
    {

        primitives::Point(ScreenBuffer, ScreenWidth, x, y, color, ClippingRectangle, blend565);
    }
    else
    {

        primitives::Point(ScreenBuffer, ScreenWidth, x, y, color, ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawPointSeries(VECTOR_INT** points, int length, RGBA color)
{
    if (PixelFormat == RGB565)
    {

        primitives::PointSeries(ScreenBuffer, ScreenWidth, points, length, color, ClippingRectangle, blend565);
    }
    else
    {

        primitives::PointSeries(ScreenBuffer, ScreenWidth, points, length, color, ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawLine(int x[2], int y[2], RGBA color)
{
    if (PixelFormat == RGB565)
    {

        primitives::Line(ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], constant_color(color), ClippingRectangle, blend565);
    }
    else
    {

        primitives::Line(ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], constant_color(color), ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawGradientLine(int x[2], int y[2], RGBA colors[2])
{
    if (PixelFormat == RGB565)
    {

        primitives::Line(ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], gradient_color(colors[0], colors[1]), ClippingRectangle, blend565);
    }
    else
    {

        primitives::Line(ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], gradient_color(colors[0], colors[1]), ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawLineSeries(VECTOR_INT** points, int length, RGBA color, int type)
{
    if (PixelFormat == RGB565)
    {

        primitives::LineSeries(ScreenBuffer, ScreenWidth, points, length, color, type, ClippingRectangle, blend565);
    }
    else
    {

        primitives::LineSeries(ScreenBuffer, ScreenWidth, points, length, color, type, ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawBezierCurve(int x[4], int y[4], double step, RGBA color, int cubic)
{
    if (PixelFormat == RGB565)
    {

        primitives::BezierCurve(ScreenBuffer, ScreenWidth, x, y, step, color, cubic, ClippingRectangle, blend565);
    }
    else
    {

        primitives::BezierCurve(ScreenBuffer, ScreenWidth, x, y, step, color, cubic, ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawTriangle(int x[3], int y[3], RGBA color)
{
    if (PixelFormat == RGB565)
    {

        primitives::Triangle(ScreenBuffer, ScreenWidth, x, y, color, ClippingRectangle, blend565);
    }
    else
    {

        primitives::Triangle(ScreenBuffer, ScreenWidth, x, y, color, ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
inline RGBA interpolateRGBA(RGBA a, RGBA b, int i, int range)
{
    if (range == 0)
    {

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
    if (PixelFormat == RGB565)
    {

        primitives::GradientTriangle(ScreenBuffer, ScreenWidth, x, y, colors, ClippingRectangle, blend565, interpolateRGBA);
    }
    else
    {

        primitives::GradientTriangle(ScreenBuffer, ScreenWidth, x, y, colors, ClippingRectangle, blend555, interpolateRGBA);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawPolygon(VECTOR_INT** points, int length, int invert, RGBA color)
{
    if (PixelFormat == RGB565)
    {

        primitives::Polygon(ScreenBuffer, ScreenWidth, points, length, invert, color, ClippingRectangle, blend565);
    }
    else
    {

        primitives::Polygon(ScreenBuffer, ScreenWidth, points, length, invert, color, ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawOutlinedRectangle(int x, int y, int w, int h, int size, RGBA color)
{
    if (color.alpha == 0)
    {          // no mask

        return;
    }
    else if (color.alpha == 255)
    { // full mask

        if (PixelFormat == RGB565)
        {

            word c = PackPixel565(color);
            primitives::OutlinedRectangle(ScreenBuffer, ScreenWidth, x, y, w, h, size, c, ClippingRectangle, copyWord);
        }
        else
        {

            word c = PackPixel555(color);
            primitives::OutlinedRectangle(ScreenBuffer, ScreenWidth, x, y, w, h, size, c, ClippingRectangle, copyWord);
        }
    }
    else
    {

        if (PixelFormat == RGB565)
        {

            primitives::OutlinedRectangle(ScreenBuffer, ScreenWidth, x, y, w, h, size, color, ClippingRectangle, blend565);
        }
        else
        {

            primitives::OutlinedRectangle(ScreenBuffer, ScreenWidth, x, y, w, h, size, color, ClippingRectangle, blend555);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawRectangle(int x, int y, int w, int h, RGBA color)
{
    if (color.alpha == 0)
    {          // no mask

        return;
    }
    else if (color.alpha == 255)
    { // full mask

        if (PixelFormat == RGB565)
        {

            word c = PackPixel565(color);
            primitives::Rectangle(ScreenBuffer, ScreenWidth, x, y, w, h, c, ClippingRectangle, copyWord);
        }
        else
        {

            word c = PackPixel555(color);
            primitives::Rectangle(ScreenBuffer, ScreenWidth, x, y, w, h, c, ClippingRectangle, copyWord);
        }
    }
    else
    {

        if (PixelFormat == RGB565)
        {

            primitives::Rectangle(ScreenBuffer, ScreenWidth, x, y, w, h, color, ClippingRectangle, blend565);
        }
        else
        {

            primitives::Rectangle(ScreenBuffer, ScreenWidth, x, y, w, h, color, ClippingRectangle, blend555);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawGradientRectangle(int x, int y, int w, int h, RGBA colors[4])
{
    if (PixelFormat == RGB565)
    {

        primitives::GradientRectangle(ScreenBuffer, ScreenWidth, x, y, w, h, colors, ClippingRectangle, blend565, interpolateRGBA);
    }
    else
    {

        primitives::GradientRectangle(ScreenBuffer, ScreenWidth, x, y, w, h, colors, ClippingRectangle, blend555, interpolateRGBA);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawOutlinedComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, RGBA color, int antialias)
{
    if (PixelFormat == RGB565)
    {

        primitives::OutlinedComplex(ScreenBuffer, ScreenWidth, r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, color, antialias, ClippingRectangle, blend565);
    }
    else
    {

        primitives::OutlinedComplex(ScreenBuffer, ScreenWidth, r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, color, antialias, ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawFilledComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, float angle, float frac_size, int fill_empty, RGBA colors[2])
{
    if (PixelFormat == RGB565)
    {

        primitives::FilledComplex(ScreenBuffer, ScreenWidth, r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, angle, frac_size, fill_empty, colors, ClippingRectangle, blend565);
    }
    else
    {

        primitives::FilledComplex(ScreenBuffer, ScreenWidth, r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, angle, frac_size, fill_empty, colors, ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawGradientComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, float angle, float frac_size, int fill_empty, RGBA colors[3])
{
    if (PixelFormat == RGB565)
    {

        primitives::GradientComplex(ScreenBuffer, ScreenWidth, r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, angle, frac_size, fill_empty, colors, ClippingRectangle, blend565);
    }
    else
    {

        primitives::GradientComplex(ScreenBuffer, ScreenWidth, r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, angle, frac_size, fill_empty, colors, ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawOutlinedEllipse(int x, int y, int rx, int ry, RGBA color)
{
    if (PixelFormat == RGB565)
    {

        primitives::OutlinedEllipse(ScreenBuffer, ScreenWidth, x, y, rx, ry, color, ClippingRectangle, blend565);
    }
    else
    {

        primitives::OutlinedEllipse(ScreenBuffer, ScreenWidth, x, y, rx, ry, color, ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawFilledEllipse(int x, int y, int rx, int ry, RGBA color)
{
    if (PixelFormat == RGB565)
    {

        primitives::FilledEllipse(ScreenBuffer, ScreenWidth, x, y, rx, ry, color, ClippingRectangle, blend565);
    }
    else
    {

        primitives::FilledEllipse(ScreenBuffer, ScreenWidth, x, y, rx, ry, color, ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawOutlinedCircle(int x, int y, int r, RGBA color, int antialias)
{
    if (PixelFormat == RGB565)
    {

        primitives::OutlinedCircle(ScreenBuffer, ScreenWidth, x, y, r, color, antialias, ClippingRectangle, blend565);
    }
    else
    {

        primitives::OutlinedCircle(ScreenBuffer, ScreenWidth, x, y, r, color, antialias, ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawFilledCircle(int x, int y, int r, RGBA color, int antialias)
{
    if (PixelFormat == RGB565)
    {

        primitives::FilledCircle(ScreenBuffer, ScreenWidth, x, y, r, color, antialias, ClippingRectangle, blend565);
    }
    else
    {

        primitives::FilledCircle(ScreenBuffer, ScreenWidth, x, y, r, color, antialias, ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawGradientCircle(int x, int y, int r, RGBA colors[2], int antialias)
{
    if (PixelFormat == RGB565)
    {

        primitives::GradientCircle(ScreenBuffer, ScreenWidth, x, y, r, colors, antialias, ClippingRectangle, blend565);
    }
    else
    {

        primitives::GradientCircle(ScreenBuffer, ScreenWidth, x, y, r, colors, antialias, ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
