#define DIRECTDRAW_VERSION 0x0300
#include <windows.h>
#include <ddraw.h>

#include "2xSaI.h"
#include "hq2x.h"
#include "scale.h"

#include "../../common/rgb.hpp"
#include "../../common/primitives.hpp"
#include "../common/video.hpp"
#include "../common/win32x.hpp"
#include "resource.h"

/**
   @todo maybe this should go to rgb.cpp or some such...  but then
   every video driver depends on libcommon?
 */
#ifdef USE_ALPHA_TABLE
unsigned char alpha_new[256][256]={
#include "../../common/alpha_new.table"
                                  };
unsigned char alpha_old[256][256]={
#include "../../common/alpha_old.table"
                                  };
#endif

/////////////////////////////////////////////////
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
    RGBA* original;

    void (*blit_routine)(_IMAGE* image, int x, int y);

#ifdef USE_CLIP_IMAGE
    int clip_x;
    int clip_y;
    int clip_width;
    int clip_height;
#endif

} *IMAGE;

enum BIT_DEPTH
{
    BD_AUTODETECT,
    BD_32,
    BD_24,
};

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
    BIT_DEPTH bit_depth;

    bool fullscreen;
    bool vsync;

    bool scale;
    int  algorithm;
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

static void Scale(void* dst, int dst_pitch);

static bool FillImagePixels(IMAGE image, RGBA* data);
static bool RefillImagePixels(IMAGE image);
static void OptimizeBlitRoutine(IMAGE image);

static void NullBlit(IMAGE image, int x, int y);
static void TileBlit(IMAGE image, int x, int y);
static void SpriteBlit(IMAGE image, int x, int y);
static void NormalBlit(IMAGE image, int x, int y);

static CONFIGURATION Configuration;
static int           BitsPerPixel = 0;

static HWND  SphereWindow = NULL;
static byte* ScreenBuffer = NULL;

static LONG OldWindowStyle   = 0;
static LONG OldWindowStyleEx = 0;

// windowed output
static HDC     RenderDC = NULL;
static HBITMAP RenderBitmap = NULL;
static void*   RenderBuffer;

// fullscreen output
static LPDIRECTDRAW dd = NULL;
static LPDIRECTDRAWSURFACE ddPrimary   = NULL;
static LPDIRECTDRAWSURFACE ddSecondary = NULL;

static bool s_fullscreen = false;
static int  scale_factor = 1;
static bool firstcall    = true;

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) GetDriverInfo(DRIVERINFO* driverinfo)
{

    if (driverinfo == NULL)
        return;
    driverinfo->name        = "Standard 32-bit Color";
    driverinfo->author      = "Chad Austin\nAnatoli Steinmark";
    driverinfo->date        = __DATE__;
    driverinfo->version     = "v1.1";
    driverinfo->description = "24/32-bit color output in both windowed and fullscreen modes";

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
    int bit_depth            = GetPrivateProfileInt("standard32", "BitDepth", 0, config_file_name);
    Configuration.bit_depth  = (bit_depth == 32 ? BD_32 : (bit_depth == 24 ? BD_24 : BD_AUTODETECT));

    Configuration.fullscreen = GetPrivateProfileInt("standard32", "Fullscreen", 1, config_file_name) != 0;
    Configuration.vsync      = GetPrivateProfileInt("standard32", "VSync",      1, config_file_name) != 0;

    Configuration.scale      = GetPrivateProfileInt("standard32", "Scale",      1, config_file_name) != 0;
    Configuration.algorithm  = GetPrivateProfileInt("standard32", "Algorithm",  0, config_file_name);
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

    WritePrivateProfileInt("standard32", "Scale",      Configuration.scale,      config_file_name);
    WritePrivateProfileInt("standard32", "Algorithm",  Configuration.algorithm,  config_file_name);
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
        CheckDlgButton(window, IDC_SCALE,      Configuration.scale      ? BST_CHECKED : BST_UNCHECKED);

        // set the scaling algorithm
        switch (Configuration.algorithm)
        {
            case I_DIRECT_SCALE:
                CheckDlgButton(window, IDC_DIRECT_SCALE, BST_CHECKED);
                break;

            case I_SCALE2X:
                CheckDlgButton(window, IDC_SCALE2X,      BST_CHECKED);
                break;

            case I_EAGLE:
                CheckDlgButton(window, IDC_EAGLE,        BST_CHECKED);
                break;

            case I_HQ2X:
                CheckDlgButton(window, IDC_HQ2X,         BST_CHECKED);
                break;

            case I_2XSAI:
                CheckDlgButton(window, IDC_2XSAI,        BST_CHECKED);
                break;

            case I_SUPER_2XSAI:
                CheckDlgButton(window, IDC_SUPER_2XSAI,  BST_CHECKED);
                break;

            case I_SUPER_EAGLE:
                CheckDlgButton(window, IDC_SUPER_EAGLE,  BST_CHECKED);
                break;

        }

        // update the check states
        SendMessage(window, WM_COMMAND, MAKEWPARAM(IDC_FULLSCREEN, BN_PUSHED), 0);
        SendMessage(window, WM_COMMAND, MAKEWPARAM(IDC_SCALE, BN_PUSHED), 0);
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
                Configuration.scale      = (IsDlgButtonChecked(window, IDC_SCALE)      != FALSE);

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

            case IDC_SCALE:

                EnableWindow(GetDlgItem(window, IDC_DIRECT_SCALE), IsDlgButtonChecked(window, IDC_SCALE));
                EnableWindow(GetDlgItem(window, IDC_SCALE2X),      IsDlgButtonChecked(window, IDC_SCALE));
                EnableWindow(GetDlgItem(window, IDC_EAGLE),        IsDlgButtonChecked(window, IDC_SCALE));
                EnableWindow(GetDlgItem(window, IDC_HQ2X),         IsDlgButtonChecked(window, IDC_SCALE));
                EnableWindow(GetDlgItem(window, IDC_2XSAI),        IsDlgButtonChecked(window, IDC_SCALE));
                EnableWindow(GetDlgItem(window, IDC_SUPER_2XSAI),  IsDlgButtonChecked(window, IDC_SCALE));
                EnableWindow(GetDlgItem(window, IDC_SUPER_EAGLE),  IsDlgButtonChecked(window, IDC_SCALE));

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

    if (firstcall)
    {
        SetClippingRectangle(0, 0, screen_width, screen_height);
        LoadConfiguration();
        s_fullscreen = Configuration.fullscreen;
        scale_factor = Configuration.scale ? 2 : 1;
        firstcall = false;
    }

    if (s_fullscreen)
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

    // allocate a blitting buffer
    ScreenBuffer = new byte[ScreenWidth * ScreenHeight * (BitsPerPixel / 8)];

    if (ScreenBuffer == NULL)
        return false;

    ShowCursor(FALSE);

    SetWindowPos(SphereWindow, HWND_TOPMOST, 0, 0,
                 ScreenWidth * scale_factor, ScreenHeight * scale_factor,
                 SWP_SHOWWINDOW);

    return true;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * Switches from fullscreen to windowed or vice-versa,
 * updates the fullscreen flag as needed
 * returns whether the engine should be *not* shutdown
 */
EXPORT(bool) ToggleFullScreen()
{
    int x, y, w, h;
    GetClippingRectangle(&x, &y, &w, &h);

    // if we have a screen size, close the old driver
    if (ScreenWidth != 0 || ScreenHeight != 0)
    {

        if (s_fullscreen)
        {
            CloseFullscreen();
        }
        else
        {
            CloseWindowed();
        }
    }

    s_fullscreen = !s_fullscreen;

    if (InitVideoDriver(SphereWindow, ScreenWidth, ScreenHeight) == true)
    {
        SetClippingRectangle(x, y, w, h);
        return true;
    }
    else
    {

        // switching failed, try to revert to what it was
        s_fullscreen = !s_fullscreen;
        if (InitVideoDriver(SphereWindow, ScreenWidth, ScreenHeight) == true)
        {
            SetClippingRectangle(x, y, w, h);
            return true;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool SetDisplayMode()
{
    HRESULT ddrval;

    switch (Configuration.bit_depth)
    {

        case BD_AUTODETECT:

            if (BitsPerPixel == 0 || BitsPerPixel == 32)
            {
                ddrval = dd->SetDisplayMode(ScreenWidth * scale_factor, ScreenHeight * scale_factor, 32);
                if (ddrval == DD_OK)
                {
                    BitsPerPixel = 32;
                    return true;
                }
            }
            if (BitsPerPixel == 0 || BitsPerPixel == 24)
            {
                ddrval = dd->SetDisplayMode(ScreenWidth * scale_factor, ScreenHeight * scale_factor, 24);
                if (ddrval == DD_OK)
                {
                    BitsPerPixel = 24;
                    return true;
                }
            }
            return false;

        case BD_32:

            ddrval = dd->SetDisplayMode(ScreenWidth * scale_factor, ScreenHeight * scale_factor, 32);
            if (ddrval == DD_OK)
            {

                BitsPerPixel = 32;
                return true;
            }
            return false;

        case BD_24:

            ddrval = dd->SetDisplayMode(ScreenWidth * scale_factor, ScreenHeight * scale_factor, 24);

            if (ddrval == DD_OK)
            {
                return true;

            }
            return false;

        default:
            return false;
    }

    return false;
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
    if (BitsPerPixel != 32 && BitsPerPixel != 24)
        BitsPerPixel = 32;

    // create the render DC
    RenderDC = CreateCompatibleDC(NULL);

    if (RenderDC == NULL)
        return false;

    // define/create the render DIB section
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    BITMAPINFOHEADER& bmih = bmi.bmiHeader;
    bmih.biSize        = sizeof(bmih);
    bmih.biWidth       =  ScreenWidth  * scale_factor;
    bmih.biHeight      = -ScreenHeight * scale_factor;
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
    CenterWindow(SphereWindow, ScreenWidth * scale_factor, ScreenHeight * scale_factor);

    // allocate a blitting buffer
    ScreenBuffer = new byte[ScreenWidth * ScreenHeight * (BitsPerPixel / 8)];

    if (ScreenBuffer == NULL)
        return false;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) CloseVideoDriver()
{
    if (s_fullscreen)
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
    if (ScreenBuffer != NULL)
    {

        delete[] ScreenBuffer;
        ScreenBuffer = NULL;
    }
    if (dd != NULL)
    {

        dd->Release();
        dd = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////
void CloseWindowed()
{
    DeleteDC(RenderDC);
    RenderDC = NULL;
    DeleteObject(RenderBitmap);
    RenderBitmap = NULL;

    if (ScreenBuffer != NULL)
    {

        delete[] ScreenBuffer;
        ScreenBuffer = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) FlipScreen()
{

    if (s_fullscreen)
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

        if (Configuration.scale)
        {
            Scale(ddsd.lpSurface, ddsd.lPitch / (BitsPerPixel / 8));
        }
        else if (BitsPerPixel == 32)
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

        if (Configuration.scale)
            Scale(RenderBuffer, ScreenWidth * 2);
        else if (BitsPerPixel == 32)
            memcpy((byte*)RenderBuffer, (byte*)ScreenBuffer, ScreenWidth * ScreenHeight * 4);
        else
            memcpy((byte*)RenderBuffer, (byte*)ScreenBuffer, ScreenWidth * ScreenHeight * 3);

        // blit the render buffer to the window
        HDC dc = GetDC(SphereWindow);
        BitBlt(dc, 0, 0, ScreenWidth * scale_factor, ScreenHeight * scale_factor, RenderDC, 0, 0, SRCCOPY);
        ReleaseDC(SphereWindow, dc);
    }
}

////////////////////////////////////////////////////////////////////////////////
void Scale(void* dst, int dst_pitch)
{
    if (BitsPerPixel == 32)
    {
        switch (Configuration.algorithm)
        {
            case I_DIRECT_SCALE:
                DirectScale((dword*)dst, dst_pitch, (dword*)ScreenBuffer, ScreenWidth, ScreenHeight);
                break;

            case I_SCALE2X:
                Scale2x((dword*)dst, dst_pitch, (dword*)ScreenBuffer, ScreenWidth, ScreenHeight);
                break;

            case I_EAGLE:
                Eagle((dword*)dst, dst_pitch, (dword*)ScreenBuffer, ScreenWidth, ScreenHeight);
                break;

            case I_HQ2X:
                hq2x((dword*)dst, dst_pitch, (dword*)ScreenBuffer, ScreenWidth, ScreenHeight);
                break;

            case I_2XSAI:
                _2xSaI((dword*)dst, dst_pitch, (dword*)ScreenBuffer, ScreenWidth, ScreenHeight);
                break;

            case I_SUPER_2XSAI:
                Super2xSaI((dword*)dst, dst_pitch, (dword*)ScreenBuffer, ScreenWidth, ScreenHeight);
                break;

            case I_SUPER_EAGLE:
                SuperEagle((dword*)dst, dst_pitch, (dword*)ScreenBuffer, ScreenWidth, ScreenHeight);
                break;

        }
    }
    else if (BitsPerPixel == 24)
    {
        switch (Configuration.algorithm)
        {
            case I_DIRECT_SCALE:
                DirectScale((BGR*)dst, dst_pitch, (BGR*)ScreenBuffer, ScreenWidth, ScreenHeight);
                break;

            case I_SCALE2X:
                Scale2x((BGR*)dst, dst_pitch, (BGR*)ScreenBuffer, ScreenWidth, ScreenHeight);
                break;

            case I_EAGLE:
                Eagle((BGR*)dst, dst_pitch, (BGR*)ScreenBuffer, ScreenWidth, ScreenHeight);
                break;

            case I_HQ2X:
                hq2x((BGR*)dst, dst_pitch, (BGR*)ScreenBuffer, ScreenWidth, ScreenHeight);
                break;

            case I_2XSAI:
                _2xSaI((BGR*)dst, dst_pitch, (BGR*)ScreenBuffer, ScreenWidth, ScreenHeight);
                break;

            case I_SUPER_2XSAI:
                Super2xSaI((BGR*)dst, dst_pitch, (BGR*)ScreenBuffer, ScreenWidth, ScreenHeight);
                break;

            case I_SUPER_EAGLE:
                SuperEagle((BGR*)dst, dst_pitch, (BGR*)ScreenBuffer, ScreenWidth, ScreenHeight);
                break;

        }
    }
}

////////////////////////////////////////////////////////////////////////////////
bool RefillImagePixels(IMAGE image)
{
    int pixels_total = image->width * image->height;

    RGBA* pixels = image->original;

    // fill the image pixels
    if (BitsPerPixel == 32)
    {
        image->bgra = new BGRA[pixels_total];

        if (image->bgra == NULL)
            return false;

        for (int i = 0; i < pixels_total; ++i)
        {
            image->bgra[i].red   = (pixels[i].red   * pixels[i].alpha) / 255;
            image->bgra[i].green = (pixels[i].green * pixels[i].alpha) / 255;
            image->bgra[i].blue  = (pixels[i].blue  * pixels[i].alpha) / 255;
        }
    }
    else
    {
        image->bgr  = new BGR[pixels_total];

        if (image->bgr == NULL)
            return false;

        for (int i = 0; i < pixels_total; ++i)
        {
            image->bgr[i].red   = (pixels[i].red   * pixels[i].alpha) / 255;
            image->bgr[i].green = (pixels[i].green * pixels[i].alpha) / 255;
            image->bgr[i].blue  = (pixels[i].blue  * pixels[i].alpha) / 255;
        }
    }

    // fill the alpha array
    image->alpha = new byte[pixels_total];

    if (image->alpha == NULL)
        return false;

    for (int i = 0; i < pixels_total; i++)
        image->alpha[i] = pixels[i].alpha;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool FillImagePixels(IMAGE image, RGBA* pixels)
{
    int pixels_total = image->width * image->height;

    // fill the original data
    image->original = new RGBA[pixels_total];

    if (!image->original)
        return false;

    memcpy(image->original, pixels, pixels_total * sizeof(RGBA));

    // fill the image pixels
    if (BitsPerPixel == 32)
    {
        image->bgra = new BGRA[pixels_total];

        if (image->bgra == NULL)
            return false;

        for (int i = 0; i < pixels_total; ++i)
        {
            image->bgra[i].red   = (pixels[i].red   * pixels[i].alpha) / 255;
            image->bgra[i].green = (pixels[i].green * pixels[i].alpha) / 255;
            image->bgra[i].blue  = (pixels[i].blue  * pixels[i].alpha) / 255;
        }
    }
    else
    {
        image->bgr  = new BGR[pixels_total];

        if (image->bgr == NULL)
            return false;

        for (int i = 0; i < pixels_total; ++i)
        {
            image->bgr[i].red   = (pixels[i].red   * pixels[i].alpha) / 255;
            image->bgr[i].green = (pixels[i].green * pixels[i].alpha) / 255;
            image->bgr[i].blue  = (pixels[i].blue  * pixels[i].alpha) / 255;
        }
    }

    // fill the alpha array
    image->alpha = new byte[pixels_total];

    if (image->alpha == NULL)
        return false;

    for (int i = 0; i < pixels_total; i++)
        image->alpha[i] = pixels[i].alpha;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
#if USE_CLIP_IMAGE
/**
  Images can often have big blocks of alpha around them (e.g. fonts, spritesets)
  This works out what parts of the image itself can be clipped from the blit
*/
void
ClipImage(IMAGE image, int& clip_x, int& clip_y, int& clip_width, int& clip_height)
{
    clip_x = 0;
    clip_y = 0;
    clip_width = image->width;
    clip_height = image->height;
    bool done = false;
    for (int x = 0; x < image->width; x++)
    {

        for (int y = 0; y < image->height; y++)
        {

            if (image->alpha[y * image->width + x] != 0)
            {

                done = true;
                break;
            }
        }
        if (done)
        {
            break;
        }
        else
        {
            clip_x++;
        }

    }
    done = false;
    for (int y = 0; y < image->height; y++)
    {

        for (int x = 0; x < image->width; x++)
        {

            if (image->alpha[y * image->width + x] != 0)
            {

                done = true;
                break;
            }
        }
        if (done)
        {
            break;
        }
        else
        {
            clip_y++;
        }

    }
    done = false;
    for (int x = image->width - 1; x >= clip_x; x--)
    {

        for (int y = 0; y < image->height; y++)
        {

            if (image->alpha[y * image->width + x] != 0)
            {

                done = true;
                break;
            }
        }
        if (done)
        {
            break;
        }
        else
        {
            clip_width--;
        }

    }
    done = false;
    for (int y = image->height - 1; y >= clip_y; y--)
    {

        for (int x = 0; x < image->width; x++)
        {

            if (image->alpha[y * image->width + x] != 0)
            {

                done = true;
                break;
            }
        }
        if (done)
        {
            break;
        }
        else
        {
            clip_height--;
        }

    }
}
#endif
////////////////////////////////////////////////////////////////////////////////
/**
  This works out the best way to draw the image.
  nullblit = alpha is all zero, therefore nothing to draw
  tileblit = alpha is all 255, therefore totally opaque
  spriteblit = alpha is all zero or 255, therefore each pixel is opaque or blank
  normal = alpha values range from zero to 255, so it has to do alpha blending
*/
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

#ifdef USE_CLIP_IMAGE
    ClipImage(image, image->clip_x, image->clip_y, image->clip_width, image->clip_height);
#endif
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
    {
        delete[] image->bgra;
        image->bgra = NULL;
    }
    else
    {
        delete[] image->bgr;
        image->bgr = NULL;
    }

    delete[] image->alpha;
    delete[] image->original;
    delete image;
}
////////////////////////////////////////////////////////////////////////////////
EXPORT(IMAGE) CreateImage(int width, int height, RGBA* pixels)
{
    // allocate the image
    IMAGE image = new _IMAGE;

    if (!image)
        return NULL;

    image->width  = width;
    image->height = height;

    if (!FillImagePixels(image, pixels))
    {
        DestroyImage(image);
        return NULL;
    }

    OptimizeBlitRoutine(image);

    return image;
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

    IMAGE image = new _IMAGE;
    if (!image)
        return NULL;

    image->width        = width;
    image->height       = height;
    image->blit_routine = TileBlit;

    if (BitsPerPixel == 32)
    {
        BGRA* Screen = (BGRA*)ScreenBuffer;

        image->bgra  = new BGRA[pixels_total];
        if (image->bgra == NULL)
        {
            delete image;
            return NULL;
        }

        image->original = new RGBA[pixels_total];
        if (image->original == NULL)
        {
            delete image;
            delete [] image->bgra;
            return NULL;
        }

        for (int iy = 0; iy < height; iy++)
            memcpy(image->bgra + iy * width, Screen + (y + iy) * ScreenWidth + x, width * 4);

        for (int i = 0; i < pixels_total; ++i)
        {
            image->original[i].red   = image->bgra[i].red;
            image->original[i].green = image->bgra[i].green;
            image->original[i].blue  = image->bgra[i].blue;
            image->original[i].alpha = 255;
        }

    }
    else
    {
        BGR* Screen = (BGR*)ScreenBuffer;

        image->bgr = new BGR[width * height];
        if (image->bgr == NULL)
        {

            delete image;
            image = NULL;
            return NULL;
        }

        image->original = new RGBA[pixels_total];
        if (image->original == NULL)
        {
            delete image;
            delete [] image->bgr;
            return NULL;
        }

        for (int iy = 0; iy < height; iy++)
            memcpy(image->bgr + iy * width, Screen + (y + iy) * ScreenWidth + x, width * 3);

        for (int i = 0; i < pixels_total; ++i)
        {
            image->original[i].red   = image->bgr[i].red;
            image->original[i].green = image->bgr[i].green;
            image->original[i].blue  = image->bgr[i].blue;
            image->original[i].alpha = 255;
        }
    }

    image->alpha = new byte[pixels_total];
    if (image->alpha == NULL)
    {
        if (BitsPerPixel == 32)
        {
            delete[] image->bgra;
            image->bgra = NULL;
        }

        if (BitsPerPixel == 24)
        {
            delete[] image->bgr;
            image->bgr = NULL;
        }

        delete [] image->original;
        delete image;

        return NULL;
    }

    memset(image->alpha, 255, pixels_total);

    return image;
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
template<typename pixelT>
class render_pixel_mask
{
public:
    render_pixel_mask(RGBA mask) : m_mask(mask)
    { }
    void operator()(pixelT& dst, pixelT src, byte alpha)
    {
        // do the masking on the source pixel
#ifdef USE_ALPHA_TABLE
        alpha     = alpha_new[m_mask.alpha][alpha];
        src.red   = alpha_new[m_mask.red][src.red];
        src.green = alpha_new[m_mask.green][src.green];
        src.blue  = alpha_new[m_mask.blue][src.blue];
#else
        alpha     = (int)alpha     * m_mask.alpha / 256;
        src.red   = (int)src.red   * m_mask.red   / 256;
        src.green = (int)src.green * m_mask.green / 256;
        src.blue  = (int)src.blue  * m_mask.blue  / 256;
#endif

        // blit to the dest pixel
#ifdef USE_ALPHA_TABLE
        dst.red   = alpha_old[alpha][dst.red]   + alpha_new[m_mask.alpha][src.red];
        dst.green = alpha_old[alpha][dst.green] + alpha_new[m_mask.alpha][src.green];
        dst.blue  = alpha_old[alpha][dst.blue]  + alpha_new[m_mask.alpha][src.blue];
#else
        dst.red   = (dst.red   * (256 - alpha) + src.red   * m_mask.alpha) / 256;
        dst.green = (dst.green * (256 - alpha) + src.green * m_mask.alpha) / 256;
        dst.blue  = (dst.blue  * (256 - alpha) + src.blue  * m_mask.alpha) / 256;
#endif
    }

private:
    RGBA m_mask;
};

EXPORT(void) BlitImageMask(IMAGE image, int x, int y, RGBA mask)
{

    if (mask.alpha == 0)
    {

        return;
    }
    if (mask.alpha == 255)
    {

        // if the mask doesn't affect the imageblit, fallback onto BlitImage
        if (mask.red == 255 && mask.green == 255 && mask.blue == 255)
        {

            BlitImage(image, x, y);
            return;
        }
    }
    if (BitsPerPixel == 32)
    {

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

    }
    else
    {

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
#ifdef USE_ALPHA_TABLE
    d.red   = alpha_old[a][d.red]   + s.red;
    d.green = alpha_old[a][d.green] + s.green;
    d.blue  = alpha_old[a][d.blue]  + s.blue;
#else
    d.red   = (d.red   * (256 - a)) / 256 + s.red;
    d.green = (d.green * (256 - a)) / 256 + s.green;
    d.blue  = (d.blue  * (256 - a)) / 256 + s.blue;
#endif
}

void aBlendBGRA(struct BGRA& d, struct BGRA s, int a)
{
    // blit to the dest pixel
#ifdef USE_ALPHA_TABLE
    d.red   = alpha_old[a][d.red]   + s.red;
    d.green = alpha_old[a][d.green] + s.green;
    d.blue  = alpha_old[a][d.blue]  + s.blue;
#else
    d.red   = (d.red   * (256 - a)) / 256 + s.red;
    d.green = (d.green * (256 - a)) / 256 + s.green;
    d.blue  = (d.blue  * (256 - a)) / 256 + s.blue;
#endif
}

EXPORT(void) TransformBlitImage(IMAGE image, int x[4], int y[4])
{

    // fallback onto BlitImage if possible
    if (x[0] == x[3] && x[1] == x[2] && y[0] == y[1] && y[2] == y[3])
    {
        int dw = x[2] - x[0] + 1;
        int dh = y[2] - y[0] + 1;
        if (dw == image->width && dh == image->height)
        {
            BlitImage(image, x[0], y[0]);
            return;
        }
    }

    if (BitsPerPixel == 32)
    {
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
    }
    else
    {
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

    // if the mask doesn't affect the imageblit, fallback onto the non-mask blit
    if (mask.red == 255 && mask.green == 255 && mask.blue == 255 && mask.alpha == 255)
    {

        TransformBlitImage(image, x, y);
        return;
    }
    if (BitsPerPixel == 32)
    {
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
    }
    else
    {
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
{}

////////////////////////////////////////////////////////////////////////////////
void TileBlit(IMAGE image, int x, int y)
{
    calculate_clipping_metrics(image->width, image->height);

    if (BitsPerPixel == 32)
    {

        BGRA* dest  = (BGRA*)ScreenBuffer + (y + image_offset_y) * ScreenWidth  + image_offset_x + x;
        BGRA* src   = (BGRA*)image->bgra  +       image_offset_y * image->width + image_offset_x;

        int iy = image_blit_height;
        while (iy-- > 0)
        {

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
        while (iy-- > 0)
        {

            memcpy(dest, src, image_blit_width * sizeof(BGR));
            dest += ScreenWidth;
            src += image->width;

        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void SpriteBlit(IMAGE image, int x, int y)
{

#ifdef USE_CLIP_IMAGE
    calculate_clipping_metrics(image->clip_width, image->clip_height);
#else
    calculate_clipping_metrics(image->width, image->height);
#endif
    if (BitsPerPixel == 32)
    {

        BGRA* dst   = (BGRA*)ScreenBuffer + (y + image_offset_y) * ScreenWidth  + image_offset_x + x;
        BGRA* src   = (BGRA*)image->bgra  +      image_offset_y  * image->width + image_offset_x;
        byte* alpha = image->alpha        +      image_offset_y  * image->width + image_offset_x;

        int dst_inc = ScreenWidth  - image_blit_width;
        int src_inc = image->width - image_blit_width;

        int iy = image_blit_height;
        while (iy-- > 0)
        {
            int ix = image_blit_width;
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
    else
    {

        BGR*  dst   = (BGR*)ScreenBuffer + (y + image_offset_y) * ScreenWidth  + image_offset_x + x;
        BGR*  src   = (BGR*)image->bgra  +       image_offset_y * image->width + image_offset_x;
        byte* alpha = image->alpha       +       image_offset_y * image->width + image_offset_x;

        int dst_inc = ScreenWidth  - image_blit_width;
        int src_inc = image->width - image_blit_width;

        int iy = image_blit_height;
        while (iy-- > 0)
        {
            int ix = image_blit_width;
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
}

////////////////////////////////////////////////////////////////////////////////
void NormalBlit(IMAGE image, int x, int y)
{
#ifdef USE_CLIP_IMAGE

    calculate_clipping_metrics(image->clip_width, image->clip_height);
#else
    calculate_clipping_metrics(image->width, image->height);
#endif

    int a;

    if (BitsPerPixel == 32)
    {
        BGRA* dst  = (BGRA*)ScreenBuffer + (y + image_offset_y) * ScreenWidth  + image_offset_x + x;
        BGRA* src  = (BGRA*)image->bgra  +      image_offset_y  * image->width + image_offset_x;
        byte* alpha = image->alpha       +      image_offset_y  * image->width + image_offset_x;

        int dst_inc = ScreenWidth  - image_blit_width;
        int src_inc = image->width - image_blit_width;

        int iy = image_blit_height;
        int ix;
        while (iy-- > 0)
        {
            ix = image_blit_width;
            while (ix-- > 0)
            {
                a = 256 - *alpha;

                dst->red   = ((dst->red   * a) >> 8) + src->red;
                dst->green = ((dst->green * a) >> 8) + src->green;
                dst->blue  = ((dst->blue  * a) >> 8) + src->blue;

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
        BGR*  dst   = (BGR*)ScreenBuffer + (y + image_offset_y) * ScreenWidth  + image_offset_x + x;
        BGR*  src   = (BGR*)image->bgr   +      image_offset_y  * image->width + image_offset_x;
        byte* alpha = image->alpha       +      image_offset_y  * image->width + image_offset_x;

        int dst_inc = ScreenWidth  - image_blit_width;
        int src_inc = image->width - image_blit_width;

        int iy = image_blit_height;
        int ix;
        while (iy-- > 0)
        {
            ix = image_blit_width;
            while (ix-- > 0)
            {
                a = 256 - *alpha;

                dst->red   = ((dst->red   * a) >> 8) + src->red;
                dst->green = ((dst->green * a) >> 8) + src->green;
                dst->blue  = ((dst->blue  * a) >> 8) + src->blue;

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
        if (BitsPerPixel == 32)
            delete[] image->bgra;
        else
            delete[] image->bgr;

        delete[] image->alpha;

        RefillImagePixels(image);
        OptimizeBlitRoutine(image);
    }
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
#ifdef USE_ALPHA_TABLE
                    dest->red   = alpha_old[src.alpha][dest->red]   + alpha_new[src.alpha][src.red];
                    dest->green = alpha_old[src.alpha][dest->green] + alpha_new[src.alpha][src.green];
                    dest->blue  = alpha_old[src.alpha][dest->blue]  + alpha_new[src.alpha][src.blue];
#else
                    dest->red   = (dest->red   * (256 - src.alpha) + src.red   * src.alpha) / 256;
                    dest->green = (dest->green * (256 - src.alpha) + src.green * src.alpha) / 256;
                    dest->blue  = (dest->blue  * (256 - src.alpha) + src.blue  * src.alpha) / 256;
#endif
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
#ifdef USE_ALPHA_TABLE
                    dest->red   = alpha_old[src.alpha][dest->red]   + alpha_new[src.alpha][src.red];
                    dest->green = alpha_old[src.alpha][dest->green] + alpha_new[src.alpha][src.green];
                    dest->blue  = alpha_old[src.alpha][dest->blue]  + alpha_new[src.alpha][src.blue];
#else
                    dest->red   = (dest->red   * (256 - src.alpha) + src.red   * src.alpha) / 256;
                    dest->green = (dest->green * (256 - src.alpha) + src.green * src.alpha) / 256;
                    dest->blue  = (dest->blue  * (256 - src.alpha) + src.blue  * src.alpha) / 256;
#endif
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
    if (BitsPerPixel == 32)
    {
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
    }
    else
    {
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
    if (x     < 0 ||
        y     < 0 ||
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
            : m_color(color)
    {}

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
    {}

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

inline void copyBGRA(BGRA& dest, BGRA source)
{
    dest = source;
}

inline void copyBGR(BGR& dest, BGR source)
{
    dest = source;
}

inline void blendBGRA(BGRA& dest, RGBA source)
{
    Blend3(dest, source, source.alpha);
}

inline void blendBGR(BGR& dest, RGBA source)
{
    Blend3(dest, source, source.alpha);
}

EXPORT(void) DrawPoint(int x, int y, RGBA color)
{
    if (BitsPerPixel == 32)
    {
        primitives::Point((BGRA*)ScreenBuffer, ScreenWidth, x, y, color, ClippingRectangle, blendBGRA);
    }
    else
    {
        primitives::Point((BGR*)ScreenBuffer, ScreenWidth, x, y, color, ClippingRectangle, blendBGR);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawPointSeries(VECTOR_INT** points, int length, RGBA color)
{
    if (BitsPerPixel == 32)
    {
        primitives::PointSeries((BGRA*)ScreenBuffer, ScreenWidth, points, length, color, ClippingRectangle, blendBGRA);
    }
    else
    {
        primitives::PointSeries((BGR*)ScreenBuffer, ScreenWidth, points, length, color, ClippingRectangle, blendBGR);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawLine(int x[2], int y[2], RGBA color)
{
    if (BitsPerPixel == 32)
    {
        primitives::Line((BGRA*)ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], constant_color(color), ClippingRectangle, blendBGRA);
    }
    else
    {
        primitives::Line((BGR*)ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], constant_color(color), ClippingRectangle, blendBGR);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawGradientLine(int x[2], int y[2], RGBA colors[2])
{
    if (BitsPerPixel == 32)
    {
        primitives::Line((BGRA*)ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], gradient_color(colors[0], colors[1]), ClippingRectangle, blendBGRA);
    }
    else
    {
        primitives::Line((BGR*)ScreenBuffer, ScreenWidth, x[0], y[0], x[1], y[1], gradient_color(colors[0], colors[1]), ClippingRectangle, blendBGR);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawLineSeries(VECTOR_INT** points, int length, RGBA color, int type)
{
    if (color.alpha == 0)
    {          // no mask

        return;
    }
    else if (color.alpha == 255)
    { // full mask

        if (BitsPerPixel == 32)
        {
            BGRA bgra = { color.blue, color.green, color.red };
            primitives::LineSeries((BGRA*)ScreenBuffer, ScreenWidth, points, length, bgra, type, ClippingRectangle, copyBGRA);
        }
        else
        {
            BGR bgr = { color.blue, color.green, color.red };
            primitives::LineSeries((BGR*)ScreenBuffer, ScreenWidth, points, length, bgr, type, ClippingRectangle, copyBGR);
        }

    }
    else
    {

        if (BitsPerPixel == 32)
        {
            primitives::LineSeries((BGRA*)ScreenBuffer, ScreenWidth, points, length, color, type, ClippingRectangle, blendBGRA);
        }
        else
        {
            primitives::LineSeries((BGR*)ScreenBuffer, ScreenWidth, points, length, color, type, ClippingRectangle, blendBGR);
        }

    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawBezierCurve(int x[4], int y[4], double step, RGBA color, int cubic)
{
    if (BitsPerPixel == 32)
    {
        primitives::BezierCurve((BGRA*)ScreenBuffer, ScreenWidth, x, y, step, color, cubic, ClippingRectangle, blendBGRA);
    }
    else
    {
        primitives::BezierCurve((BGR*)ScreenBuffer, ScreenWidth, x, y, step, color, cubic, ClippingRectangle, blendBGR);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawTriangle(int x[3], int y[3], RGBA color)
{
    if (BitsPerPixel == 32)
    {
        primitives::Triangle((BGRA*)ScreenBuffer, ScreenWidth, x, y, color, ClippingRectangle, blendBGRA);
    }
    else
    {
        primitives::Triangle((BGR*)ScreenBuffer, ScreenWidth, x, y, color, ClippingRectangle, blendBGR);
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
    if (BitsPerPixel == 32)
    {
        primitives::GradientTriangle((BGRA*)ScreenBuffer, ScreenWidth, x, y, colors, ClippingRectangle, blendBGRA, interpolateRGBA);
    }
    else
    {
        primitives::GradientTriangle((BGR*)ScreenBuffer, ScreenWidth, x, y, colors, ClippingRectangle, blendBGR, interpolateRGBA);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawPolygon(VECTOR_INT** points, int length, int invert, RGBA color)
{
    if (color.alpha == 0)
    {          // no mask

        return;
    }
    else if (color.alpha == 255)
    { // full mask

        if (BitsPerPixel == 32)
        {
            BGRA bgra = { color.blue, color.green, color.red };
            primitives::Polygon((BGRA*)ScreenBuffer, ScreenWidth, points, length, invert, bgra, ClippingRectangle, copyBGRA);
        }
        else
        {
            BGR bgr = { color.blue, color.green, color.red };
            primitives::Polygon((BGR*)ScreenBuffer, ScreenWidth, points, length, invert, bgr, ClippingRectangle, copyBGR);
        }

    }
    else
    {

        if (BitsPerPixel == 32)
        {
            primitives::Polygon((BGRA*)ScreenBuffer, ScreenWidth, points, length, invert, color, ClippingRectangle, blendBGRA);
        }
        else
        {
            primitives::Polygon((BGR*)ScreenBuffer, ScreenWidth, points, length, invert, color, ClippingRectangle, blendBGR);
        }

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

        if (BitsPerPixel == 32)
        {
            BGRA bgra = { color.blue, color.green, color.red };
            primitives::OutlinedRectangle((BGRA*)ScreenBuffer, ScreenWidth, x, y, w, h, size, bgra, ClippingRectangle, copyBGRA);
        }
        else
        {
            BGR bgr = { color.blue, color.green, color.red };
            primitives::OutlinedRectangle((BGR*)ScreenBuffer, ScreenWidth, x, y, w, h, size, bgr, ClippingRectangle, copyBGR);
        }

    }
    else
    {

        if (BitsPerPixel == 32)
        {
            primitives::OutlinedRectangle((BGRA*)ScreenBuffer, ScreenWidth, x, y, w, h, size, color, ClippingRectangle, blendBGRA);
        }
        else
        {
            primitives::OutlinedRectangle((BGR*)ScreenBuffer, ScreenWidth, x, y, w, h, size, color, ClippingRectangle, blendBGR);
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

        if (BitsPerPixel == 32)
        {
            BGRA bgra = { color.blue, color.green, color.red };
            primitives::Rectangle((BGRA*)ScreenBuffer, ScreenWidth, x, y, w, h, bgra, ClippingRectangle, copyBGRA);
        }
        else
        {
            BGR bgr = { color.blue, color.green, color.red };
            primitives::Rectangle((BGR*)ScreenBuffer, ScreenWidth, x, y, w, h, bgr, ClippingRectangle, copyBGR);
        }

    }
    else
    {

        if (BitsPerPixel == 32)
        {
            primitives::Rectangle((BGRA*)ScreenBuffer, ScreenWidth, x, y, w, h, color, ClippingRectangle, blendBGRA);
        }
        else
        {
            primitives::Rectangle((BGR*)ScreenBuffer, ScreenWidth, x, y, w, h, color, ClippingRectangle, blendBGR);
        }

    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawGradientRectangle(int x, int y, int w, int h, RGBA colors[4])
{
    if (BitsPerPixel == 32)
    {
        primitives::GradientRectangle((BGRA*)ScreenBuffer, ScreenWidth, x, y, w, h, colors, ClippingRectangle, blendBGRA, interpolateRGBA);
    }
    else
    {
        primitives::GradientRectangle((BGR*)ScreenBuffer, ScreenWidth, x, y, w, h, colors, ClippingRectangle, blendBGR, interpolateRGBA);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawOutlinedComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, RGBA color, int antialias)
{
    if (color.alpha == 0)
    {          // no mask

        return;
    }
    else
    {
        if (BitsPerPixel == 32)
        {
            primitives::OutlinedComplex((BGRA*)ScreenBuffer, ScreenWidth, r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, color, antialias, ClippingRectangle, blendBGRA);
        }
        else
        {
            primitives::OutlinedComplex((BGR*)ScreenBuffer, ScreenWidth, r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, color, antialias, ClippingRectangle, blendBGR);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawFilledComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, float angle, float frac_size, int fill_empty, RGBA colors[2])
{
    if (colors[0].alpha == 0 && colors[1].alpha == 0)
    {          // no mask

        return;
    }
    else
    {
        if (BitsPerPixel == 32)
        {
            primitives::FilledComplex((BGRA*)ScreenBuffer, ScreenWidth, r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, angle, frac_size, fill_empty, colors, ClippingRectangle, blendBGRA);
        }
        else
        {
            primitives::FilledComplex((BGR*)ScreenBuffer, ScreenWidth, r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r,  angle, frac_size, fill_empty, colors, ClippingRectangle, blendBGR);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawGradientComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, float angle, float frac_size, int fill_empty, RGBA colors[3])
{
    if (colors[0].alpha == 0 && colors[1].alpha == 0 && colors[2].alpha == 0)
    {          // no mask

        return;
    }
    else
    {
        if (BitsPerPixel == 32)
        {
            primitives::GradientComplex((BGRA*)ScreenBuffer, ScreenWidth, r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, angle, frac_size, fill_empty, colors, ClippingRectangle, blendBGRA);
        }
        else
        {
            primitives::GradientComplex((BGR*)ScreenBuffer, ScreenWidth, r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r,  angle, frac_size, fill_empty, colors, ClippingRectangle, blendBGR);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawOutlinedEllipse(int x, int y, int rx, int ry, RGBA color)
{
    if (color.alpha == 0)
    {          // no mask

        return;
    }
    else
    {
        if (BitsPerPixel == 32)
        {
            primitives::OutlinedEllipse((BGRA*)ScreenBuffer, ScreenWidth, x, y, rx, ry, color, ClippingRectangle, blendBGRA);
        }
        else
        {
            primitives::OutlinedEllipse((BGR*)ScreenBuffer, ScreenWidth, x, y, rx, ry, color, ClippingRectangle, blendBGR);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawFilledEllipse(int x, int y, int rx, int ry, RGBA color)
{
    if (color.alpha == 0)
    {          // no mask

        return;
    }
    else
    {
        if (BitsPerPixel == 32)
        {
            primitives::FilledEllipse((BGRA*)ScreenBuffer, ScreenWidth, x, y, rx, ry, color, ClippingRectangle, blendBGRA);
        }
        else
        {
            primitives::FilledEllipse((BGR*)ScreenBuffer, ScreenWidth, x, y, rx, ry, color, ClippingRectangle, blendBGR);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawOutlinedCircle(int x, int y, int r, RGBA color, int antialias)
{
    if (color.alpha == 0)
    {          // no mask

        return;
    }
    else
    {
        if (BitsPerPixel == 32)
        {
            primitives::OutlinedCircle((BGRA*)ScreenBuffer, ScreenWidth, x, y, r, color, antialias, ClippingRectangle, blendBGRA);
        }
        else
        {
            primitives::OutlinedCircle((BGR*)ScreenBuffer, ScreenWidth, x, y, r, color, antialias, ClippingRectangle, blendBGR);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawFilledCircle(int x, int y, int r, RGBA color, int antialias)
{
    if (color.alpha == 0)
    {          // no mask

        return;
    }
    else
    {
        if (BitsPerPixel == 32)
        {
            primitives::FilledCircle((BGRA*)ScreenBuffer, ScreenWidth, x, y, r, color, antialias, ClippingRectangle, blendBGRA);
        }
        else
        {
            primitives::FilledCircle((BGR*)ScreenBuffer, ScreenWidth, x, y, r, color, antialias, ClippingRectangle, blendBGR);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawGradientCircle(int x, int y, int r, RGBA colors[2], int antialias)
{
    if (colors[0].alpha == 0 && colors[1].alpha == 0)
    {          // no mask

        return;
    }
    else
    {
        if (BitsPerPixel == 32)
        {
            primitives::GradientCircle((BGRA*)ScreenBuffer, ScreenWidth, x, y, r, colors, antialias, ClippingRectangle, blendBGRA);
        }
        else
        {
            primitives::GradientCircle((BGR*)ScreenBuffer, ScreenWidth, x, y, r, colors, antialias, ClippingRectangle, blendBGR);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
