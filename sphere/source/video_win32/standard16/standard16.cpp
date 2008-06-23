#define DIRECTDRAW_VERSION 0x0300
#include <windows.h>
#include <ddraw.h>
#include <stdio.h>

#include "2xSaI.h"
#include "hq2x.h"
#include "scale.h"

#include "../../common/rgb.hpp"
#include "../../common/primitives.hpp"
#include "../common/win32x.hpp"
#include "../common/video.hpp"
#include "resource.h"


typedef struct _IMAGE
{
    int width;
    int height;

    word* rgb;
    byte* alpha;

    void (*blit_routine)(_IMAGE* image, int x, int y);

    RGBA* original;

    int pixel_format;

} *IMAGE;

enum SCALING_FILTER
{
    I_NONE = 0,
    I_SCALE2X,
    I_EAGLE,
    I_HQ2X,
    I_2XSAI,
    I_SUPER_2XSAI,
    I_SUPER_EAGLE,
};

typedef struct _screen_border
{
    int top;
    int bottom;
    int left;
    int right;

} screen_border;

class VideoConfiguration
{
    public:

        VideoConfiguration() {};

        int GetExWidth(int width)
        {
            return width + border.left + border.right;
        }

        int GetExHeight(int height)
        {
            return height + border.top + border.bottom;
        }

        int GetOffset(int pitch)
        {
            return pitch * border.top + border.left;
        }

        bool fullscreen;
        bool vsync;

        bool scale;
        int  filter;

        screen_border border;

};

// FUNCTION PROTOTYPES //

static void LoadConfiguration();

static bool InitFullscreen();
static bool SetDisplayMode();
static bool CreateSurfaces();
static bool InitWindowed();

static void CloseFullscreen();
static void CloseWindowed();

static void Scale(word* dst, int dst_pitch);

static void FillImagePixels(IMAGE image, RGBA* pixels);
static void RefillImagePixels(IMAGE image);
static void OptimizeBlitRoutine(IMAGE image);

static void NullBlit(IMAGE image, int x, int y);
static void TileBlit(IMAGE image, int x, int y);
static void SpriteBlit(IMAGE image, int x, int y);
static void NormalBlit(IMAGE image, int x, int y);



// INLINE FUNCTIONS //

// like 255 - a only faster
inline byte InvertAlpha(byte a)
{
    return a ^ 0xFF;
}

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

static enum
{
    RGB565,
    RGB555,

} PixelFormat;


static VideoConfiguration Config;

static HWND  SphereWindow        = NULL;
static word* ScreenBuffer        = NULL;
static word* ScreenBufferSection = NULL;

static int   ScreenBufferWidth  = 0;
static int   ScreenBufferHeight = 0;
static int   ScreenScaleFactor  = 1;

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
    driverinfo->name   = "Standard16";
    driverinfo->author = "Chad Austin, Anatoli Steinmark";
    driverinfo->date   = __DATE__;
    driverinfo->version = "v1.1";
    driverinfo->description = "16-bit Software Sphere Video Driver";
}

////////////////////////////////////////////////////////////////////////////////
void LoadConfiguration()
{
    char config_file_name[MAX_PATH];
    GetDriverConfigFile(config_file_name);

    // load the fields from the file
    Config.fullscreen = GetPrivateProfileInt("standard16", "Fullscreen", 0, config_file_name) != 0;
    Config.vsync      = GetPrivateProfileInt("standard16", "VSync",      1, config_file_name) != 0;
    Config.scale      = GetPrivateProfileInt("standard16", "Scale",      0, config_file_name) != 0;
    Config.filter     = GetPrivateProfileInt("standard16", "Filter",     0, config_file_name);

    int vexpand              = GetPrivateProfileInt("standard16", "VExpand",    0, config_file_name);
    int hexpand              = GetPrivateProfileInt("standard16", "HExpand",    0, config_file_name);

    if (vexpand < 0 || vexpand > 1024)
        vexpand = 0;

    if (hexpand < 0 || hexpand > 1024)
        hexpand = 0;

    Config.border.top    = (int)(hexpand / 2);
    Config.border.bottom = (int)(hexpand / 2) + (hexpand % 2);
    Config.border.left   = (int)(vexpand / 2);
    Config.border.right  = (int)(vexpand / 2) + (vexpand % 2);

}

////////////////////////////////////////////////////////////////////////////////
EXPORT(bool) InitVideoDriver(HWND window, int screen_width, int screen_height)
{
    SphereWindow = window;
    ScreenWidth  = screen_width;
    ScreenHeight = screen_height;

    SetClippingRectangle(0, 0, screen_width, screen_height);

    static bool firstcall = true;

    if (firstcall)
    {
        LoadConfiguration();
        ScreenScaleFactor  = Config.scale ? 2 : 1;
        firstcall = false;
    }

    if (Config.fullscreen)
        return InitFullscreen();
    else
        return InitWindowed();

    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool InitFullscreen()
{
    // initialize the screen buffer variables and allocate a blitting buffer
    ScreenBufferWidth   = Config.GetExWidth(ScreenWidth);
    ScreenBufferHeight  = Config.GetExHeight(ScreenHeight);
    ScreenBuffer        = new word[ScreenBufferWidth * ScreenBufferHeight];
    ScreenBufferSection = ScreenBuffer + Config.GetOffset(ScreenBufferWidth);

    if (ScreenBuffer == NULL)
        return false;

    memset((byte*)ScreenBuffer, 0, ScreenBufferWidth * ScreenBufferHeight * 2);


    HRESULT ddrval;
    bool    retval;

    // store old window styles
    OldWindowStyle   = GetWindowLong(SphereWindow, GWL_STYLE);
    OldWindowStyleEx = GetWindowLong(SphereWindow, GWL_EXSTYLE);

    SetWindowLong(SphereWindow, GWL_STYLE, WS_POPUP);
    SetWindowLong(SphereWindow, GWL_EXSTYLE, 0);

    // create DirectDraw object
    ddrval = DirectDrawCreate(NULL, &dd, NULL);
    if (ddrval != DD_OK)
    {
        MessageBox(SphereWindow, "DirectDrawCreate() failed", "standard16", MB_OK);
        return false;
    }

    // set application behavior
    ddrval = dd->SetCooperativeLevel(SphereWindow, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
    if (ddrval != DD_OK)
    {
        dd->Release();
        MessageBox(SphereWindow, "SetCooperativeLevel() failed", "standard16", MB_OK);
        return false;
    }

    // set display mode
    retval = SetDisplayMode();
    if (retval == false)
    {
        dd->Release();
        MessageBox(SphereWindow, "SetDisplayMode() failed", "standard16", MB_OK);
        return false;
    }

    // create surfaces
    retval = CreateSurfaces();
    if (retval == false)
    {
        dd->Release();
        MessageBox(SphereWindow, "CreateSurfaces() failed", "standard16", MB_OK);
        return false;
    }

    ShowCursor(FALSE);

    SetWindowPos(SphereWindow, HWND_TOPMOST, 0, 0,
                 ScreenBufferWidth  * ScreenScaleFactor,
                 ScreenBufferHeight * ScreenScaleFactor,
                 SWP_SHOWWINDOW);

    return true;
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(bool) ToggleFullScreen()
{
    int x, y, w, h;
    GetClippingRectangle(&x, &y, &w, &h);

    // if we have a screen size, close the old driver
    if (ScreenWidth != 0 || ScreenHeight != 0)
    {

        if (Config.fullscreen)
        {
            CloseFullscreen();
        }
        else
        {
            CloseWindowed();
        }
    }

    Config.fullscreen = !Config.fullscreen;

    if (InitVideoDriver(SphereWindow, ScreenWidth, ScreenHeight) == true)
    {
        SetClippingRectangle(x, y, w, h);
        return true;
    }
    else
    {

        // switching failed, try to revert to what it was
        Config.fullscreen = !Config.fullscreen;
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

    ddrval = dd->SetDisplayMode(ScreenBufferWidth  * ScreenScaleFactor,
                                ScreenBufferHeight * ScreenScaleFactor,
                                16);

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

    if (Config.vsync)
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

    if (Config.vsync)
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
    // initialize the screen buffer variables and allocate a blitting buffer
    ScreenBufferWidth   = ScreenWidth;
    ScreenBufferHeight  = ScreenHeight;
    ScreenBuffer        = new word[ScreenBufferWidth * ScreenBufferHeight];
    ScreenBufferSection = ScreenBuffer;

    if (ScreenBuffer == NULL)
        return false;

    memset((byte*)ScreenBuffer, 0, ScreenBufferWidth * ScreenBufferHeight * 2);


    // create the render DC
    RenderDC = CreateCompatibleDC(NULL);
    if (RenderDC == NULL)
        return false;

    // define/create the DIB section
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    BITMAPINFOHEADER& bmih = bmi.bmiHeader;
    bmih.biSize        = sizeof(bmih);
    bmih.biWidth       =  ScreenBufferWidth  * ScreenScaleFactor;
    bmih.biHeight      = -ScreenBufferHeight * ScreenScaleFactor;
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

    CenterWindow(SphereWindow,
                 ScreenWidth  * ScreenScaleFactor,
                 ScreenHeight * ScreenScaleFactor);

    // we know that 16-bit color DIBs are always 5:5:5
    PixelFormat = RGB555;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) CloseVideoDriver()
{
    if (Config.fullscreen)
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
        ScreenBuffer        = NULL;
        ScreenBufferSection = NULL;
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
        ScreenBuffer        = NULL;
        ScreenBufferSection = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) FlipScreen()
{

    if (Config.fullscreen)
    {
        LPDIRECTDRAWSURFACE surface;

        if (Config.vsync)
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

        int dst_pitch = ddsd.lPitch / 2;

        if (Config.scale)
        {
            Scale((word*)ddsd.lpSurface, dst_pitch);
        }
        else
        {
            word* dst = (word*)ddsd.lpSurface;
            word* src = ScreenBuffer;

            for (int i = 0; i < ScreenBufferHeight; i++)
            {
                memcpy(dst, src, ScreenBufferWidth * 2);
                dst += dst_pitch;
                src += ScreenBufferWidth;
            }
        }

        // unlock the surface and do the flip!
        surface->Unlock(NULL);

        if (Config.vsync)
            ddPrimary->Flip(NULL, DDFLIP_WAIT);

    }
    else
    {

        if (Config.scale)
            Scale(RenderBuffer, ScreenBufferWidth * 2);
        else
            memcpy((byte*)RenderBuffer,
                   (byte*)ScreenBuffer,
                   ScreenBufferWidth * ScreenBufferHeight * 2);

        // blit the render buffer to the window
        HDC dc = GetDC(SphereWindow);

        BitBlt(dc, 0, 0,
               ScreenBufferWidth  * ScreenScaleFactor,
               ScreenBufferHeight * ScreenScaleFactor,
               RenderDC, 0, 0, SRCCOPY);

        ReleaseDC(SphereWindow, dc);
    }
}

////////////////////////////////////////////////////////////////////////////////
void Scale(word* dst, int dst_pitch)
{
    if (PixelFormat == RGB565)
    {

        switch (Config.filter)
        {

            case I_NONE:
                DirectScale(dst, dst_pitch, ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight);
                break;

            case I_SCALE2X:
                Scale2x(    dst, dst_pitch, ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight);
                break;

            case I_EAGLE:
                Eagle(      dst, dst_pitch, ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight);
                break;

            case I_HQ2X:
                hq2x(       dst, dst_pitch, ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight, 16);
                break;

            case I_2XSAI:
                _2xSaI(     dst, dst_pitch, ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight, 16);
                break;

            case I_SUPER_2XSAI:
                Super2xSaI( dst, dst_pitch, ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight, 16);
                break;

            case I_SUPER_EAGLE:
                SuperEagle( dst, dst_pitch, ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight, 16);
                break;

        }
    }
    else
    {
        switch (Config.filter)
        {

            case I_NONE:
                DirectScale(dst, dst_pitch, ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight);
                break;

            case I_SCALE2X:
                Scale2x(    dst, dst_pitch, ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight);
                break;

            case I_EAGLE:
                Eagle(      dst, dst_pitch, ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight);
                break;

            case I_HQ2X:
                hq2x(       dst, dst_pitch, ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight, 15);
                break;

            case I_2XSAI:
                _2xSaI(     dst, dst_pitch, ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight, 15);
                break;

            case I_SUPER_2XSAI:
                Super2xSaI( dst, dst_pitch, ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight, 15);
                break;

            case I_SUPER_EAGLE:
                SuperEagle( dst, dst_pitch, ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight, 15);
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
        image->pixel_format = PixelFormat;

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
    image->original = new RGBA[pixels_total];

    if (!image->original)
        return;

    memcpy(image->original, pixels, pixels_total * sizeof(RGBA));

    // fill the premultiplied data
    RGBA pixel;
    image->rgb = new word[pixels_total];

    if (!image->rgb)
        return;

    if (PixelFormat == RGB565)
    {
        for (int i = 0; i < pixels_total; i++)
        {
            pixel = pixels[i];

            // premultiply
            pixel.red   = (pixel.red   * pixel.alpha) >> 8;
            pixel.green = (pixel.green * pixel.alpha) >> 8;
            pixel.blue  = (pixel.blue  * pixel.alpha) >> 8;

            image->rgb[i] = PackPixel565(pixel);
        }
    }
    else
    {
        for (int i = 0; i < pixels_total; i++)
        {
            pixel = pixels[i];

            // premultiply
            pixel.red   = (pixel.red   * pixel.alpha) >> 8;
            pixel.green = (pixel.green * pixel.alpha) >> 8;
            pixel.blue  = (pixel.blue  * pixel.alpha) >> 8;

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
void RefillImagePixels(IMAGE image)
{
    int pixels_total = image->width * image->height;

    RGBA* pixels = image->original;

    // fill the premultiplied data
    RGBA pixel;
    image->rgb = new word[pixels_total];

    if (!image->rgb)
        return;

    if (PixelFormat == RGB565)
    {
        for (int i = 0; i < pixels_total; i++)
        {
            pixel = pixels[i];

            // premultiply
            pixel.red   = (pixel.red   * pixel.alpha) >> 8;
            pixel.green = (pixel.green * pixel.alpha) >> 8;
            pixel.blue  = (pixel.blue  * pixel.alpha) >> 8;

            image->rgb[i] = PackPixel565(pixel);
        }
    }
    else
    {
        for (int i = 0; i < pixels_total; i++)
        {
            pixel = pixels[i];

            // premultiply
            pixel.red   = (pixel.red   * pixel.alpha) >> 8;
            pixel.green = (pixel.green * pixel.alpha) >> 8;
            pixel.blue  = (pixel.blue  * pixel.alpha) >> 8;

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
    if (x          < 0           ||
        y          < 0           ||
        x + width  > ScreenWidth ||
        y + height > ScreenHeight)
    {
        return NULL;
    }

    int pixels_total = width * height;

    IMAGE image         = new _IMAGE;

    if (!image)
        return NULL;

    image->width        = width;
    image->height       = height;
    image->blit_routine = TileBlit;
    image->pixel_format = PixelFormat;

    image->rgb = new word[pixels_total];
    if (!image->rgb)
    {
        delete image;
        return NULL;
    }

    image->alpha = new byte[pixels_total];
    if (!image->alpha)
    {
        delete [] image->rgb;
        delete image;
        return NULL;
    }

    image->original = new RGBA[pixels_total];
    if (!image->original)
    {
        delete [] image->rgb;
        delete [] image->alpha;
        delete image;
        return NULL;
    }

    for (int iy = 0; iy < height; iy++)
        memcpy(image->rgb + iy * width, ScreenBufferSection + (y + iy) * ScreenBufferWidth + x, width * 2);

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
    // if toggled from/to fullscreen, convert the image to the new pixel format first
    if (image->pixel_format != PixelFormat)
    {
        image->pixel_format = PixelFormat;
        FillImagePixels(image, image->original);
    }

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

        alpha   = (alpha   * m_mask.alpha) / 255;
        s.blue  = (s.blue  * m_mask.blue)  / 255;
        s.green = (s.green * m_mask.green) / 255;
        s.red   = (s.red   * m_mask.red)   / 255;

        // blit to the dest pixel
        byte b = InvertAlpha(alpha);
        d.blue = ((s.blue * m_mask.alpha) + (d.blue * b)) / 255;
        d.green = ((s.green * m_mask.alpha) + (d.green * b)) / 255;
        d.red = ((s.red * m_mask.alpha) + (d.red * b)) / 255;

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

        alpha   = (alpha   * m_mask.alpha) / 255;
        s.blue  = (s.blue  * m_mask.blue)  / 255;
        s.green = (s.green * m_mask.green) / 255;
        s.red   = (s.red   * m_mask.red)   / 255;

        // blit to the dest pixel
        byte b = InvertAlpha(alpha);
        d.blue = ((s.blue * m_mask.alpha) + (d.blue * b)) / 255;
        d.green = ((s.green * m_mask.alpha) + (d.green * b)) / 255;
        d.red = ((s.red * m_mask.alpha) + (d.red * b)) / 255;

        dst = PackPixel555(d);
    }

private:
    RGBA m_mask;
};


EXPORT(void) BlitImageMask(IMAGE image, int x, int y, RGBA mask)
{
    // if toggled from/to fullscreen, convert the image to the new pixel format first
    if (image->pixel_format != PixelFormat)
    {
        image->pixel_format = PixelFormat;
        FillImagePixels(image, image->original);
    }

    if (PixelFormat == RGB565)
    {

        primitives::Blit(
            ScreenBufferSection,
            ScreenBufferWidth,
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
            ScreenBufferSection,
            ScreenBufferWidth,
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
    out.red   = (in.red)   + ((out.red * (InvertAlpha(a))) / 255);
    out.green = (in.green) + ((out.green * (InvertAlpha(a))) / 255);
    out.blue  = (in.blue)  + ((out.blue  * (InvertAlpha(a))) / 255);
    d = PackPixel565(out);
}

inline void renderpixel555(word& d, const word& s, int a)
{
    RGBA out = UnpackPixel555(d);
    RGBA in  = UnpackPixel555(s);
    out.red   = (in.red)   + ((out.red * (InvertAlpha(a))) / 255);
    out.green = (in.green) + ((out.green * (InvertAlpha(a))) / 255);
    out.blue  = (in.blue)  + ((out.blue  * (InvertAlpha(a))) / 255);
    d = PackPixel555(out);
}

EXPORT(void) TransformBlitImage(IMAGE image, int x[4], int y[4])
{
    // if toggled from/to fullscreen, convert the image to the new pixel format first
    if (image->pixel_format != PixelFormat)
    {
        image->pixel_format = PixelFormat;
        FillImagePixels(image, image->original);
    }

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

    if (PixelFormat == RGB565)
    {
        primitives::TexturedQuad(ScreenBufferSection, ScreenBufferWidth, x, y, image->rgb, image->alpha, image->width, image->height, ClippingRectangle, renderpixel565);
    }
    else
    {
        primitives::TexturedQuad(ScreenBufferSection, ScreenBufferWidth, x, y, image->rgb, image->alpha, image->width, image->height, ClippingRectangle, renderpixel555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) TransformBlitImageMask(IMAGE image, int x[4], int y[4], RGBA mask)
{
    // if toggled from/to fullscreen, convert the image to the new pixel format first
    if (image->pixel_format != PixelFormat)
    {
        image->pixel_format = PixelFormat;
        FillImagePixels(image, image->original);
    }

    if (PixelFormat == RGB565)
    {
        primitives::TexturedQuad(ScreenBufferSection, ScreenBufferWidth, x, y, image->rgb, image->alpha, image->width, image->height, ClippingRectangle, render_pixel_mask_565(mask));
    }
    else
    {
        primitives::TexturedQuad(ScreenBufferSection, ScreenBufferWidth, x, y, image->rgb, image->alpha, image->width, image->height, ClippingRectangle, render_pixel_mask_555(mask));
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

    word* dest  = (word*)ScreenBufferSection + (y + image_offset_y) * ScreenBufferWidth  + image_offset_x + x;
    word* src   = (word*)image->rgb          +       image_offset_y * image->width       + image_offset_x;

    int iy = image_blit_height;
    while (iy-- > 0)
    {

        memcpy(dest, src, image_blit_width * sizeof(word));
        dest += ScreenBufferWidth;
        src  += image->width;

    }
}

////////////////////////////////////////////////////////////////////////////////
void SpriteBlit(IMAGE image, int x, int y)
{
    calculate_clipping_metrics(image->width, image->height);

    word* dst   = (word*)ScreenBufferSection + (y + image_offset_y) * ScreenBufferWidth  + image_offset_x + x;
    word* src   = (word*)image->rgb          +      image_offset_y  * image->width       + image_offset_x;
    byte* alpha = image->alpha               +      image_offset_y  * image->width       + image_offset_x;

    int dst_inc = ScreenBufferWidth  - image_blit_width;
    int src_inc = image->width       - image_blit_width;

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

    word* dst   = (word*)ScreenBufferSection + (y + image_offset_y) * ScreenBufferWidth + image_offset_x + x;
    word* src   = (word*)image->rgb          +      image_offset_y  * image->width      + image_offset_x;
    byte* alpha = image->alpha               +      image_offset_y  * image->width      + image_offset_x;

    int dst_inc = ScreenBufferWidth - image_blit_width;
    int src_inc = image->width      - image_blit_width;

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
/**
 * ClipByte clamps an integer value to the range 0-255 without
 * any branch operations.
 */

inline int ClipByte(int value)
{
    int iClipped;
    value = (value & (-(int)!(value < 0)));
    iClipped = -(int)(value > 255);
    return (255 & iClipped) | (value & ~iClipped);
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

    word* dst = ScreenBufferSection + (y + image_offset_y) * ScreenBufferWidth + image_offset_x + x;
    RGBA* src = pixels              +      image_offset_y  * w                 + image_offset_x;

    int dst_inc = ScreenBufferWidth - image_blit_width;
    int src_inc = w                 - image_blit_width;

    int a;
    RGBA temp;
    word result;

    int iy = image_blit_height;
    int ix;

    if (PixelFormat == RGB565)
    {
        while (iy-- > 0)
        {
            ix = image_blit_width;

            while (ix-- > 0)
            {

                if (src[0].alpha == 255)
                {
                    dst[0] = PackPixel565(src[0]);
                }
                else if (src[0].alpha > 0)
                {

                    temp.red   = (src[0].red   * src[0].alpha) >> 8;
                    temp.green = (src[0].green * src[0].alpha) >> 8;
                    temp.blue  = (src[0].blue  * src[0].alpha) >> 8;

                    a = 256 - src[0].alpha;

                    result  =  ((dst[0] & 0x001F) * a >> 8) + ( temp.blue  >> 3);
                    result |= (((dst[0] & 0x07E0) * a >> 8) + ((temp.green >> 2) <<  5)) & 0x07E0;
                    result |= (((dst[0] & 0xF800) * a >> 8) + ((temp.red   >> 3) << 11)) & 0xF800;

                    dst[0] = result;
                }

                ++dst;
                ++src;
            }

            dst += dst_inc;
            src += src_inc;
        }
    }
    else
    {
        while (iy-- > 0)
        {
            ix = image_blit_width;

            while (ix-- > 0)
            {

                if (src[0].alpha == 255)
                {
                    dst[0] = PackPixel555(src[0]);
                }
                else if (src[0].alpha > 0)
                {

                    temp.red   = (src[0].red   * src[0].alpha) >> 8;
                    temp.green = (src[0].green * src[0].alpha) >> 8;
                    temp.blue  = (src[0].blue  * src[0].alpha) >> 8;

                    a = 256 - src[0].alpha;

                    result  =  ((dst[0] & 0x001F) * a >> 8) + ( temp.blue  >> 3);
                    result |= (((dst[0] & 0x03E0) * a >> 8) + ((temp.green >> 3) <<  5)) & 0x03E0;
                    result |= (((dst[0] & 0x7C00) * a >> 8) + ((temp.red   >> 3) << 10)) & 0x7C00;

                    dst[0] = result;
                }

                ++dst;
                ++src;
            }

            dst += dst_inc;
            src += src_inc;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
inline void blendRGBAto565(word& d, RGBA s, RGBA alpha)
{
    int  a;
    RGBA temp;
    word result;

    temp.red   = (s.red   * alpha.alpha) >> 8;
    temp.green = (s.green * alpha.alpha) >> 8;
    temp.blue  = (s.blue  * alpha.alpha) >> 8;

    a = 256 - alpha.alpha;

    result  =  ((d & 0x001F) * a >> 8) + ( temp.blue  >> 3);
    result |= (((d & 0x07E0) * a >> 8) + ((temp.green >> 2) <<  5)) & 0x07E0;
    result |= (((d & 0xF800) * a >> 8) + ((temp.red   >> 3) << 11)) & 0xF800;

    d = result;
}

inline void blendRGBAto555(word& d, RGBA s, RGBA alpha)
{
    int  a;
    RGBA temp;
    word result;

    temp.red   = (s.red   * alpha.alpha) >> 8;
    temp.green = (s.green * alpha.alpha) >> 8;
    temp.blue  = (s.blue  * alpha.alpha) >> 8;

    a = 256 - alpha.alpha;

    result  =  ((d & 0x001F) * a >> 8) + ( temp.blue  >> 3);
    result |= (((d & 0x03E0) * a >> 8) + ((temp.green >> 3) <<  5)) & 0x03E0;
    result |= (((d & 0x7C00) * a >> 8) + ((temp.red   >> 3) << 10)) & 0x7C00;

    d = result;
}

EXPORT(void) DirectTransformBlit(int x[4], int y[4], int w, int h, RGBA* pixels)
{
    if (PixelFormat == RGB565)
    {
        primitives::TexturedQuad(ScreenBufferSection,
                                 ScreenBufferWidth,
                                 x,
                                 y,
                                 pixels,
                                 pixels,
                                 w,
                                 h,
                                 ClippingRectangle,
                                 blendRGBAto565);
    }
    else
    {
        primitives::TexturedQuad(ScreenBufferSection,
                                 ScreenBufferWidth,
                                 x,
                                 y,
                                 pixels,
                                 pixels,
                                 w,
                                 h,
                                 ClippingRectangle,
                                 blendRGBAto555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DirectGrab(int x, int y, int w, int h, RGBA* pixels)
{
    if (x     < 0           ||
        y     < 0           ||
        x + w > ScreenWidth ||
        y + h > ScreenHeight)
    {
        return;
    }

    memset((byte*)pixels, 255, w * h * sizeof(RGBA));

    word* Screen = ScreenBufferSection + y * ScreenBufferWidth + x;

    int scr_inc = ScreenBufferWidth - w;

    int iy = h;
    int ix;

    if (PixelFormat == RGB565)
    {
        while (iy-- > 0)
        {
            ix = w;

            while (ix-- > 0)
            {
                pixels->red   = ((Screen[0] & 0xF800) >> 11) << 3;
                pixels->green = ((Screen[0] & 0x07E0) >>  5) << 2;
                pixels->blue  = ((Screen[0] & 0x001F) >>  0) << 3;

                ++Screen;
                ++pixels;

            }

            Screen += scr_inc;
        }
    }
    else
    {
        while (iy-- > 0)
        {
            ix = w;

            while (ix-- > 0)
            {
                pixels->red   = ((Screen[0] & 0x7C00) >> 10) << 3;
                pixels->green = ((Screen[0] & 0x03E0) >>  5) << 3;
                pixels->blue  = ((Screen[0] & 0x001F) >>  0) << 3;

                ++Screen;
                ++pixels;

            }

            Screen += scr_inc;
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

inline void copyWord(word& dest, word source)
{
    dest = source;
}


EXPORT(void) DrawPoint(int x, int y, RGBA color)
{
    if (PixelFormat == RGB565)
    {
        primitives::Point(ScreenBufferSection, ScreenBufferWidth, x, y, color, ClippingRectangle, blend565);
    }
    else
    {
        primitives::Point(ScreenBufferSection, ScreenBufferWidth, x, y, color, ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawPointSeries(VECTOR_INT** points, int length, RGBA color)
{
    if (PixelFormat == RGB565)
    {
        primitives::PointSeries(ScreenBufferSection, ScreenBufferWidth, points, length, color, ClippingRectangle, blend565);
    }
    else
    {
        primitives::PointSeries(ScreenBufferSection, ScreenBufferWidth, points, length, color, ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawLine(int x[2], int y[2], RGBA color)
{
    if (PixelFormat == RGB565)
    {
        primitives::Line(ScreenBufferSection, ScreenBufferWidth, x[0], y[0], x[1], y[1], constant_color(color), ClippingRectangle, blend565);
    }
    else
    {
        primitives::Line(ScreenBufferSection, ScreenBufferWidth, x[0], y[0], x[1], y[1], constant_color(color), ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawGradientLine(int x[2], int y[2], RGBA colors[2])
{
    if (PixelFormat == RGB565)
    {
        primitives::Line(ScreenBufferSection, ScreenBufferWidth, x[0], y[0], x[1], y[1], gradient_color(colors[0], colors[1]), ClippingRectangle, blend565);
    }
    else
    {
        primitives::Line(ScreenBufferSection, ScreenBufferWidth, x[0], y[0], x[1], y[1], gradient_color(colors[0], colors[1]), ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawLineSeries(VECTOR_INT** points, int length, RGBA color, int type)
{
    if (PixelFormat == RGB565)
    {
        primitives::LineSeries(ScreenBufferSection, ScreenBufferWidth, points, length, color, type, ClippingRectangle, blend565);
    }
    else
    {
        primitives::LineSeries(ScreenBufferSection, ScreenBufferWidth, points, length, color, type, ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawBezierCurve(int x[4], int y[4], double step, RGBA color, int cubic)
{
    if (PixelFormat == RGB565)
    {
        primitives::BezierCurve(ScreenBufferSection, ScreenBufferWidth, x, y, step, color, cubic, ClippingRectangle, blend565);
    }
    else
    {
        primitives::BezierCurve(ScreenBufferSection, ScreenBufferWidth, x, y, step, color, cubic, ClippingRectangle, blend555);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawTriangle(int x[3], int y[3], RGBA color)
{
    if (PixelFormat == RGB565)
    {
        primitives::Triangle(ScreenBufferSection, ScreenBufferWidth, x, y, color, ClippingRectangle, blend565);
    }
    else
    {
        primitives::Triangle(ScreenBufferSection, ScreenBufferWidth, x, y, color, ClippingRectangle, blend555);
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
        primitives::GradientTriangle(ScreenBufferSection, ScreenBufferWidth, x, y, colors, ClippingRectangle, blend565, interpolateRGBA);
    }
    else
    {
        primitives::GradientTriangle(ScreenBufferSection, ScreenBufferWidth, x, y, colors, ClippingRectangle, blend555, interpolateRGBA);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawPolygon(VECTOR_INT** points, int length, int invert, RGBA color)
{
    if (PixelFormat == RGB565)
    {
        primitives::Polygon(ScreenBufferSection, ScreenBufferWidth, points, length, invert, color, ClippingRectangle, blend565);
    }
    else
    {
        primitives::Polygon(ScreenBufferSection, ScreenBufferWidth, points, length, invert, color, ClippingRectangle, blend555);
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
            primitives::OutlinedRectangle(ScreenBufferSection, ScreenBufferWidth, x, y, w, h, size, c, ClippingRectangle, copyWord);
        }
        else
        {
            word c = PackPixel555(color);
            primitives::OutlinedRectangle(ScreenBufferSection, ScreenBufferWidth, x, y, w, h, size, c, ClippingRectangle, copyWord);
        }

    }
    else
    {

        if (PixelFormat == RGB565)
        {
            primitives::OutlinedRectangle(ScreenBufferSection, ScreenBufferWidth, x, y, w, h, size, color, ClippingRectangle, blend565);
        }
        else
        {
            primitives::OutlinedRectangle(ScreenBufferSection, ScreenBufferWidth, x, y, w, h, size, color, ClippingRectangle, blend555);
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
            primitives::Rectangle(ScreenBufferSection, ScreenBufferWidth, x, y, w, h, c, ClippingRectangle, copyWord);
        }
        else
        {
            word c = PackPixel555(color);
            primitives::Rectangle(ScreenBufferSection, ScreenBufferWidth, x, y, w, h, c, ClippingRectangle, copyWord);
        }

    }
    else
    {

        if (PixelFormat == RGB565)
        {
            primitives::Rectangle(ScreenBufferSection, ScreenBufferWidth, x, y, w, h, color, ClippingRectangle, blend565);
        }
        else
        {
            primitives::Rectangle(ScreenBufferSection, ScreenBufferWidth, x, y, w, h, color, ClippingRectangle, blend555);
        }

    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawGradientRectangle(int x, int y, int w, int h, RGBA colors[4])
{
    if (PixelFormat == RGB565)
    {
        primitives::GradientRectangle(ScreenBufferSection, ScreenBufferWidth, x, y, w, h, colors, ClippingRectangle, blend565, interpolateRGBA);
    }
    else
    {
        primitives::GradientRectangle(ScreenBufferSection, ScreenBufferWidth, x, y, w, h, colors, ClippingRectangle, blend555, interpolateRGBA);
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
        if (PixelFormat == RGB565)
        {
            primitives::OutlinedComplex(ScreenBufferSection, ScreenBufferWidth, r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, color, antialias, ClippingRectangle, blend565);
        }
        else
        {
            primitives::OutlinedComplex(ScreenBufferSection, ScreenBufferWidth, r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, color, antialias, ClippingRectangle, blend555);
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
        if (PixelFormat == RGB565)
        {
            primitives::FilledComplex(ScreenBufferSection, ScreenBufferWidth, r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, angle, frac_size, fill_empty, colors, ClippingRectangle, blend565);
        }
        else
        {
            primitives::FilledComplex(ScreenBufferSection, ScreenBufferWidth, r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, angle, frac_size, fill_empty, colors, ClippingRectangle, blend555);
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
        if (PixelFormat == RGB565)
        {
            primitives::GradientComplex(ScreenBufferSection, ScreenBufferWidth, r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, angle, frac_size, fill_empty, colors, ClippingRectangle, blend565);
        }
        else
        {
            primitives::GradientComplex(ScreenBufferSection, ScreenBufferWidth, r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r, angle, frac_size, fill_empty, colors, ClippingRectangle, blend555);
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
        if (PixelFormat == RGB565)
        {
            primitives::OutlinedEllipse(ScreenBufferSection, ScreenBufferWidth, x, y, rx, ry, color, ClippingRectangle, blend565);
        }
        else
        {
            primitives::OutlinedEllipse(ScreenBufferSection, ScreenBufferWidth, x, y, rx, ry, color, ClippingRectangle, blend555);
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
        if (PixelFormat == RGB565)
        {
            primitives::FilledEllipse(ScreenBufferSection, ScreenBufferWidth, x, y, rx, ry, color, ClippingRectangle, blend565);
        }
        else
        {
            primitives::FilledEllipse(ScreenBufferSection, ScreenBufferWidth, x, y, rx, ry, color, ClippingRectangle, blend555);
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
        if (PixelFormat == RGB565)
        {
            primitives::OutlinedCircle(ScreenBufferSection, ScreenBufferWidth, x, y, r, color, antialias, ClippingRectangle, blend565);
        }
        else
        {
            primitives::OutlinedCircle(ScreenBufferSection, ScreenBufferWidth, x, y, r, color, antialias, ClippingRectangle, blend555);
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
        if (PixelFormat == RGB565)
        {
            primitives::FilledCircle(ScreenBufferSection, ScreenBufferWidth, x, y, r, color, antialias, ClippingRectangle, blend565);
        }
        else
        {
            primitives::FilledCircle(ScreenBufferSection, ScreenBufferWidth, x, y, r, color, antialias, ClippingRectangle, blend555);
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
        if (PixelFormat == RGB565)
        {
            primitives::GradientCircle(ScreenBufferSection, ScreenBufferWidth, x, y, r, colors, antialias, ClippingRectangle, blend565);
        }
        else
        {
            primitives::GradientCircle(ScreenBufferSection, ScreenBufferWidth, x, y, r, colors, antialias, ClippingRectangle, blend555);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////