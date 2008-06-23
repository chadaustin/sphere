#include <SDL.h>
#include <SDL_getenv.h>
#include <string>

#include "../../common/rgb.hpp"
#include "../../common/primitives.hpp"
#include "../../common/configfile.hpp"

#include "scale.h"
#include "hq2x.h"
#include "2xSaI.h"

#define EXPORT(ret) extern "C" ret __attribute__((stdcall))

#define calculate_clipping_metrics(width, height)   /* EVIL! */      \
  int image_offset_x = 0;                                            \
  int image_offset_y = 0;                                            \
  int image_blit_width = width;                                      \
  int image_blit_height = height;                                    \
                                                                     \
  if (x < ClippingRectangle.left) {                                  \
    image_offset_x = (ClippingRectangle.left - x);                   \
    image_blit_width -= image_offset_x;                              \
  }                                                                  \
  if (y < ClippingRectangle.top) {                                   \
    image_offset_y = (ClippingRectangle.top - y);                    \
    image_blit_height -= image_offset_y;                             \
  }                                                                  \
  if (x + (int)width - 1 > ClippingRectangle.right)                  \
    image_blit_width -= (x + width - ClippingRectangle.right - 1);   \
  if (y + (int)height - 1 > ClippingRectangle.bottom)                \
    image_blit_height -= (y + height - ClippingRectangle.bottom - 1)


struct DRIVERINFO
{
    const char* name;
    const char* author;
    const char* date;
    const char* version;
    const char* description;
};

typedef struct _IMAGE
{
    int width;
    int height;

    BGRA* bgra;

    byte* alpha;
    RGBA* original;

    void (*blit_routine)(_IMAGE* image, int x, int y);

} *IMAGE;

typedef struct _clipper
{
    int left;
    int top;
    int right;
    int bottom;

} clipper;

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


// function prototypes
static bool   InitVideo(int w, int h);
EXPORT(bool)  InitVideo(int w, int h, std::string sphere_dir);

static void   NullBlit(IMAGE image, int x, int y);
static void   TileBlit(IMAGE image, int x, int y);
static void   SpriteBlit(IMAGE image, int x, int y);
static void   NormalBlit(IMAGE image, int x, int y);


// globals
std::string    WindowTitle;

static VideoConfiguration Config;

static clipper ClippingRectangle;

static SDL_Surface* SDLScreenBuffer;

static BGRA*   ScreenBuffer        = NULL;
static BGRA*   ScreenBufferSection = NULL;

static int     ScreenBufferWidth  = 0;
static int     ScreenBufferHeight = 0;

static int     ScreenWidth        = 0;
static int     ScreenHeight       = 0;


////////////////////////////////////////////////////////////////////////////////
EXPORT(void) GetDriverInfo(DRIVERINFO* driverinfo)
{
    driverinfo->name        = "SDL32";
    driverinfo->author      = "Chad Austin, Anatoli Steinmark";
    driverinfo->date        = __DATE__;
    driverinfo->version     = "v1.1";
    driverinfo->description = "32-bit Software Sphere Video Driver";
}

////////////////////////////////////////////////////////////////////////////////
void LoadConfiguration(std::string sphere_dir)
{
    CConfigFile file;
    file.Load((sphere_dir + "/system/video/sdl32.cfg").c_str());

    Config.fullscreen = file.ReadBool("sdl32", "Fullscreen", true);
    Config.vsync      = file.ReadBool("sdl32", "VSync",      true);
    Config.scale      = file.ReadBool("sdl32", "Scale",      true);
    Config.filter     = file.ReadInt ("sdl32", "Filter",        0);

    int vexpand       = file.ReadInt ("sdl32", "VExpand",       0);
    int hexpand       = file.ReadInt ("sdl32", "HExpand",       0);

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
EXPORT(void) SetClippingRectangle(int x, int y, int w, int h)
{
    int x1 = x;
    int y1 = y;
    int x2 = x + w - 1;
    int y2 = y + h - 1;

    // validate x values
    if (x1 < 0)
        x1 = 0;
    else if (x1 > ScreenWidth - 1)
        x1 = ScreenWidth - 1;

    if (x2 < 0)
        x2 = 0;
    else if (x2 > ScreenWidth - 1)
        x2 = ScreenWidth - 1;

    // validate y values
    if (y1 < 0)
        y1 = 0;
    else if (y1 > ScreenHeight - 1)
        y1 = ScreenHeight - 1;

    if (y2 < 0)
        y2 = 0;
    else if (y2 > ScreenHeight - 1)
        y2 = ScreenHeight - 1;

    SDL_Rect rect;
    rect.x = x1;
    rect.y = y1;
    rect.w = w;
    rect.h = h;

    SDL_SetClipRect(SDLScreenBuffer, &rect);

    ClippingRectangle.left   = x1;
    ClippingRectangle.right  = x2;
    ClippingRectangle.top    = y1;
    ClippingRectangle.bottom = y2;
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) GetClippingRectangle(int* x, int* y, int* w, int* h)
{
    *x = ClippingRectangle.left;
    *y = ClippingRectangle.top;
    *w = ClippingRectangle.right  - ClippingRectangle.left + 1;
    *h = ClippingRectangle.bottom - ClippingRectangle.top  + 1;
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(bool) SetWindowTitle(const char* text)
{
    WindowTitle = text;

    SDL_WM_SetCaption(text, NULL);

    return true;
}

////////////////////////////////////////////////////////////////////////////////
inline bool InitScreenBuffer()
{
    if (ScreenBuffer)
    {
        delete [] ScreenBuffer;
        ScreenBuffer        = NULL;
        ScreenBufferSection = NULL;
    }

    if (Config.fullscreen)
    {
        ScreenBufferWidth   = Config.GetExWidth(ScreenWidth);
        ScreenBufferHeight  = Config.GetExHeight(ScreenHeight);
        ScreenBuffer        = new BGRA[ScreenBufferWidth * ScreenBufferHeight];
        ScreenBufferSection = ScreenBuffer + Config.GetOffset(ScreenBufferWidth);
    }
    else
    {
        ScreenBufferWidth   = ScreenWidth;
        ScreenBufferHeight  = ScreenHeight;
        ScreenBuffer        = new BGRA[ScreenBufferWidth * ScreenBufferHeight];
        ScreenBufferSection = ScreenBuffer;
    }

    if (!ScreenBuffer)
        return false;

    // clear the buffer
    memset((byte*)ScreenBuffer, 0, ScreenBufferWidth * ScreenBufferHeight * sizeof(BGRA));


    return true;
}

////////////////////////////////////////////////////////////////////////////////
static bool InitVideo(int w, int h)
{
    return InitVideo(w, h, "");
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(bool) InitVideo(int w, int h, std::string sphere_dir)
{

    ScreenWidth  = w;
    ScreenHeight = h;

    static bool firstcall = true;

    // Center the window on the display
    putenv("SDL_VIDEO_CENTERED=1");

    if (firstcall)
    {
        LoadConfiguration(sphere_dir);

        if (!InitScreenBuffer())
            return false;

        // initialize SDL
        // Note: SDL_INIT_EVENTTHREAD is currently not supported on Mac OS X
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) == -1)
        {
            fprintf(stderr, "Could not initialize SDL:\n%s\n", SDL_GetError());
            return false;
        }

        SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

        firstcall  = false;

    }
    else
    {
        // reinitialize the screen buffer, because the new resolution can differ
        if (!InitScreenBuffer())
        {
            SDL_Quit();
            return false;
        }

        // reinitialize the SDL video subsystem
        SDL_QuitSubSystem(SDL_INIT_VIDEO);

        if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
        {
            fprintf(stderr, "Could not initialize video:\n%s\n", SDL_GetError());
            return false;
        }

        // keep the window title as what it was
        SetWindowTitle(WindowTitle.c_str());
    }

    int s_width  = ScreenBufferWidth  * (Config.scale ? 2 : 1);
    int s_height = ScreenBufferHeight * (Config.scale ? 2 : 1);

    dword flags = 0;
    if (Config.fullscreen)  flags |= SDL_FULLSCREEN;
    if (Config.vsync)       flags |= SDL_HWSURFACE | SDL_DOUBLEBUF;
    else                    flags |= SDL_SWSURFACE;

    // set up the video surface
    SDLScreenBuffer = SDL_SetVideoMode(s_width, s_height, 32, flags);

    if (SDLScreenBuffer == NULL)
    {
        fprintf(stderr, "Could not set video mode:\n%s\n", SDL_GetError());
        return false;
    }

    SDL_ShowCursor(false);
    SetClippingRectangle(0, 0, SDLScreenBuffer->w, SDLScreenBuffer->h);

    return true;
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) CloseVideo()
{
    // clean up
    if (ScreenBuffer)
    {
        delete [] ScreenBuffer;
        ScreenBuffer        = NULL;
        ScreenBufferSection = NULL;
    }

    SDL_Quit();

}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) ToggleFullScreen()
{
    int x, y, w, h;
    GetClippingRectangle(&x, &y, &w, &h);

    Config.fullscreen = !Config.fullscreen;

    bool succeeded = InitVideo(ScreenWidth, ScreenHeight);

    // if we failed, try to restore the original video mode
    if (!succeeded)
    {
        Config.fullscreen = !Config.fullscreen;
        InitVideo(ScreenWidth, ScreenHeight);
    }

    SetClippingRectangle(x, y, w, h);
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) FlipScreen()
{
    if (SDL_MUSTLOCK(SDLScreenBuffer))
        if (SDL_LockSurface(SDLScreenBuffer) != 0)
            return;

    int pitch = SDLScreenBuffer->pitch / sizeof(BGRA);

    if (Config.scale)
    {

        switch (Config.filter)
        {

            case I_NONE:
                DirectScale((dword*)SDLScreenBuffer->pixels, pitch, (dword*)ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight);
                break;

            case I_SCALE2X:
                Scale2x(    (dword*)SDLScreenBuffer->pixels, pitch, (dword*)ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight);
                break;

            case I_EAGLE:
                Eagle(      (dword*)SDLScreenBuffer->pixels, pitch, (dword*)ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight);
                break;

            case I_HQ2X:
                hq2x(       (dword*)SDLScreenBuffer->pixels, pitch, (dword*)ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight);
                break;

            case I_2XSAI:
                _2xSaI(     (dword*)SDLScreenBuffer->pixels, pitch, (dword*)ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight);
                break;

            case I_SUPER_2XSAI:
                Super2xSaI( (dword*)SDLScreenBuffer->pixels, pitch, (dword*)ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight);
                break;

            case I_SUPER_EAGLE:
                SuperEagle( (dword*)SDLScreenBuffer->pixels, pitch, (dword*)ScreenBuffer, ScreenBufferWidth, ScreenBufferHeight);
                break;

        }
    }
    else
    {
        BGRA* dst = (BGRA*)SDLScreenBuffer->pixels;
        BGRA* src = ScreenBuffer;

        for (int i = 0; i < ScreenBufferHeight; ++i)
        {
            memcpy(dst, src, ScreenBufferWidth * sizeof(BGRA));
            dst += pitch;
            src += ScreenBufferWidth;
        }
    }

    if (SDL_MUSTLOCK(SDLScreenBuffer))
        SDL_UnlockSurface(SDLScreenBuffer);

    SDL_Flip(SDLScreenBuffer);

}

////////////////////////////////////////////////////////////////////////////////
bool FillImagePixels(IMAGE image, const RGBA* pixels)
{
    int pixels_total = image->width * image->height;

    // fill the original data
    image->original = new RGBA[pixels_total];

    if (!image->original)
        return false;

    memcpy(image->original, pixels, pixels_total * sizeof(RGBA));


    // fill the image pixels
    image->bgra = new BGRA[pixels_total];

    if (image->bgra == NULL)
        return false;

    for (int i = 0; i < pixels_total; ++i)
    {
        image->bgra[i].red   = (pixels[i].red   * pixels[i].alpha) >> 8;
        image->bgra[i].green = (pixels[i].green * pixels[i].alpha) >> 8;
        image->bgra[i].blue  = (pixels[i].blue  * pixels[i].alpha) >> 8;
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
bool RefillImagePixels(IMAGE image)
{
    int pixels_total = image->width * image->height;

    RGBA* pixels = image->original;


    // fill the image pixels
    image->bgra = new BGRA[pixels_total];

    if (image->bgra == NULL)
        return false;

    for (int i = 0; i < pixels_total; ++i)
    {
        image->bgra[i].red   = (pixels[i].red   * pixels[i].alpha) >> 8;
        image->bgra[i].green = (pixels[i].green * pixels[i].alpha) >> 8;
        image->bgra[i].blue  = (pixels[i].blue  * pixels[i].alpha) >> 8;
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
void OptimizeBlitRoutine(IMAGE image)
{
    // null blit
    bool is_empty = true;
    for (int i = 0; i < image->width * image->height; i++)
    {
        if (image->alpha[i] != 0)
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
    for (int i = 0; i < image->width * image->height; i++)
    {
        if (image->alpha[i] != 255)
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
    for (int i = 0; i < image->width * image->height; i++)
    {
        if (image->alpha[i] != 0 && image->alpha[i] != 255)
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
EXPORT(void) DestroyImage(IMAGE image)
{
    delete[] image->bgra;
    delete[] image->original;
    delete[] image->alpha;
    delete image;
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(IMAGE) CreateImage(int width, int height, const RGBA* pixels)
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

    // fill bgra and original rgba data
    image->bgra  = new BGRA[pixels_total];
    if (image->bgra == NULL)
    {
        delete image;
        return NULL;
    }

    image->original = new RGBA[pixels_total];
    if (image->original == NULL)
    {
        delete [] image->bgra;
        delete image;
        return NULL;
    }

    for (int iy = 0; iy < height; iy++)
        memcpy(image->bgra + iy * width, ScreenBufferSection + (y + iy) * ScreenBufferWidth + x, width * 4);

    for (int i = 0; i < pixels_total; ++i)
    {
        image->original[i].red   = image->bgra[i].red;
        image->original[i].green = image->bgra[i].green;
        image->original[i].blue  = image->bgra[i].blue;
        image->original[i].alpha = 255;
    }

    // fill alpha
    image->alpha = new byte[pixels_total];
    if (image->alpha == NULL)
    {
        delete [] image->bgra;
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
    if (x + (int)image->width  < ClippingRectangle.left  ||
        y + (int)image->height < ClippingRectangle.top   ||
        x                      > ClippingRectangle.right ||
        y                      > ClippingRectangle.bottom)
    {
        return;
    }

    image->blit_routine(image, x, y);
}

////////////////////////////////////////////////////////////////////////////////
template<typename pixelT>
class render_pixel_mask
{
    public:
        render_pixel_mask(RGBA mask) : m_mask(mask) {};

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

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) BlitImageMask(IMAGE image, int x, int y, RGBA mask)
{
    primitives::Blit(ScreenBufferSection,
                     ScreenBufferWidth,
                     x,
                     y,
                     image->bgra,
                     image->alpha,
                     image->width,
                     image->height,
                     ClippingRectangle,
                     render_pixel_mask<BGRA>(mask));

}

////////////////////////////////////////////////////////////////////////////////
void aBlendBGRA(struct BGRA& d, struct BGRA s, int a)
{
    // blit to the dest pixel
    d.red   = s.red   + (d.red   * (256 - a)) / 256;
    d.green = s.green + (d.green * (256 - a)) / 256;
    d.blue  = s.blue  + (d.blue  * (256 - a)) / 256;
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) TransformBlitImage(IMAGE image, int x[4], int y[4])
{
    primitives::TexturedQuad(ScreenBufferSection,
                             ScreenBufferWidth,
                             x,
                             y,
                             image->bgra,
                             image->alpha,
                             image->width,
                             image->height,
                             ClippingRectangle,
                             aBlendBGRA);

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

    primitives::TexturedQuad(ScreenBufferSection,
                             ScreenBufferWidth,
                             x,
                             y,
                             image->bgra,
                             image->alpha,
                             image->width,
                             image->height,
                             ClippingRectangle,
                             render_pixel_mask<BGRA>(mask));

}

////////////////////////////////////////////////////////////////////////////////
void NullBlit(IMAGE image, int x, int y)
{
}

////////////////////////////////////////////////////////////////////////////////
void TileBlit(IMAGE image, int x, int y)
{
    calculate_clipping_metrics(image->width, image->height);

    BGRA* dst = ScreenBufferSection + (y + image_offset_y) * ScreenBufferWidth  + image_offset_x + x;
    BGRA* src = image->bgra         + image_offset_y       * image->width       + image_offset_x;

    int iy = image_blit_height;

    while (iy-- > 0)
    {
        memcpy(dst, src, image_blit_width * sizeof(BGRA));
        dst += ScreenBufferWidth;
        src += image->width;
    }

}

////////////////////////////////////////////////////////////////////////////////
void SpriteBlit(IMAGE image, int x, int y)
{
    calculate_clipping_metrics(image->width, image->height);

    BGRA* dst   = ScreenBufferSection + (y + image_offset_y) * ScreenBufferWidth  + image_offset_x + x;
    BGRA* src   = image->bgra         + image_offset_y       * image->width       + image_offset_x;
    byte* alpha = image->alpha        + image_offset_y       * image->width       + image_offset_x;

    int dst_inc = ScreenBufferWidth  - image_blit_width;
    int src_inc = image->width       - image_blit_width;

    int iy = image_blit_height;
    while (iy-- > 0)
    {
        int ix = image_blit_width;

        while (ix-- > 0)
        {
            if (*alpha)
                *dst = *src;

            dst++;
            src++;
            alpha++;
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

    BGRA* dst   = ScreenBufferSection + (y + image_offset_y) * ScreenBufferWidth  + image_offset_x + x;
    BGRA* src   = image->bgra         + image_offset_y       * image->width       + image_offset_x;
    byte* alpha = image->alpha        + image_offset_y       * image->width       + image_offset_x;

    int dst_inc = ScreenBufferWidth  - image_blit_width;
    int src_inc = image->width       - image_blit_width;

    int iy = image_blit_height;
    int ix;
    int a;

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
        delete[] image->bgra;
        delete[] image->alpha;

        RefillImagePixels(image);
        OptimizeBlitRoutine(image);
    }
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DirectBlit(int x, int y, int w, int h, RGBA* pixels)
{
    calculate_clipping_metrics(w, h);

    BGRA* dst = ScreenBufferSection + (y + image_offset_y) * ScreenBufferWidth + image_offset_x + x;
    RGBA* src = pixels              +      image_offset_y  * w                 + image_offset_x;

    int dst_inc = ScreenBufferWidth - image_blit_width;
    int src_inc = w                 - image_blit_width;

    int a;

    int iy = image_blit_height;
    int ix;

    while (iy-- > 0)
    {
        ix = image_blit_width;

        while (ix-- > 0)
        {

            if (src->alpha == 255)
            {
                dst->red   = src->red;
                dst->green = src->green;
                dst->blue  = src->blue;
            }
            else if (src->alpha > 0)
            {
                a = 256 - src->alpha;

                dst->red   = (dst->red   * a + src->red   * src->alpha) >> 8;
                dst->green = (dst->green * a + src->green * src->alpha) >> 8;
                dst->blue  = (dst->blue  * a + src->blue  * src->alpha) >> 8;
            }

            ++dst;
            ++src;
        }

        dst   += dst_inc;
        src   += src_inc;
    }
}

////////////////////////////////////////////////////////////////////////////////
inline void BlendRGBAtoBGRA(BGRA& dst, RGBA src, RGBA alpha)
{
    int a = 256 - alpha.alpha;

    dst.red   = (dst.red   * a + src.red   * alpha.alpha) >> 8;
    dst.green = (dst.green * a + src.green * alpha.alpha) >> 8;
    dst.blue  = (dst.blue  * a + src.blue  * alpha.alpha) >> 8;
}

EXPORT(void) DirectTransformBlit(int x[4], int y[4], int w, int h, RGBA* pixels)
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
                             BlendRGBAtoBGRA);

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

    BGRA* Screen = ScreenBufferSection + y * ScreenBufferWidth + x;

    int scr_inc = ScreenBufferWidth - w;

    int iy = h;
    int ix;

    while (iy-- > 0)
    {
        ix = w;

        while (ix-- > 0)
        {
            pixels->red   = Screen->red;
            pixels->green = Screen->green;
            pixels->blue  = Screen->blue;

            ++Screen;
            ++pixels;

        }

        Screen += scr_inc;
    }

}

////////////////////////////////////////////////////////////////////////////////
class constant_color
{
    public:
        constant_color(RGBA color) : m_color(color) {};

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
        gradient_color(RGBA color1, RGBA color2) : m_color1(color1), m_color2(color2) {};

        RGBA operator()(int i, int range)
        {
            if (range == 0)
                return m_color1;

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

inline void copyBGRA(BGRA& dest, BGRA source)
{
  dest = source;
}

inline void blendBGRA(BGRA& dest, RGBA source)
{
  Blend3(dest, source, source.alpha);
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawPoint(int x, int y, RGBA color)
{
    primitives::Point(ScreenBufferSection, ScreenBufferWidth, x, y, color,
                       ClippingRectangle, blendBGRA);

}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawPointSeries(VECTOR_INT** points, int length, RGBA color)
{
    primitives::PointSeries(ScreenBufferSection, ScreenBufferWidth, points, length, color,
                            ClippingRectangle, blendBGRA);

}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawLine(int x[2], int y[2], RGBA color)
{
    primitives::Line(ScreenBufferSection, ScreenBufferWidth, x[0], y[0],
                     x[1], y[1], constant_color(color),
                     ClippingRectangle, blendBGRA);

}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawGradientLine(int x[2], int y[2], RGBA colors[2])
{
    primitives::Line(ScreenBufferSection, ScreenBufferWidth, x[0], y[0],
                     x[1], y[1], gradient_color(colors[0], colors[1]),
                     ClippingRectangle, blendBGRA);

}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawLineSeries(VECTOR_INT** points, int length, RGBA color, int type)
{
    if (color.alpha == 0)
        return;

    primitives::LineSeries(ScreenBufferSection, ScreenBufferWidth, points,
                           length, color, type, ClippingRectangle, blendBGRA);

}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawBezierCurve(int x[4], int y[4], double step, RGBA color, int cubic)
{
    if (color.alpha == 0)
        return;

    primitives::BezierCurve(ScreenBufferSection, ScreenBufferWidth, x, y, step,
                            color, cubic, ClippingRectangle, blendBGRA);

}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawTriangle(int x[3], int y[3], RGBA color)
{
    if (color.alpha == 0)
        return;

    primitives::Triangle(ScreenBufferSection, ScreenBufferWidth, x, y,
                         color, ClippingRectangle, blendBGRA);

}

////////////////////////////////////////////////////////////////////////////////
inline RGBA interpolateRGBA(RGBA a, RGBA b, int i, int range)
{
    if (range == 0)
        return a;

    RGBA result =
    {
        (a.red   * (range - i) + b.red   * i) / range,
        (a.green * (range - i) + b.green * i) / range,
        (a.blue  * (range - i) + b.blue  * i) / range,
        (a.alpha * (range - i) + b.alpha * i) / range,
    };

    return result;
}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawGradientTriangle(int x[3], int y[3], RGBA colors[3])
{
    primitives::GradientTriangle(ScreenBufferSection, ScreenBufferWidth,
                                 x, y, colors, ClippingRectangle,
                                 blendBGRA, interpolateRGBA);

}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawPolygon(VECTOR_INT** points, int length, int invert, RGBA color)
{
    if (color.alpha == 0)
        return;

    primitives::Polygon(ScreenBufferSection, ScreenBufferWidth, points,
                        length, invert, color, ClippingRectangle, blendBGRA);

}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawOutlinedRectangle(int x, int y, int w, int h, int size, RGBA color)
{
    if (color.alpha == 0)
        return;

    primitives::OutlinedRectangle(ScreenBufferSection, ScreenBufferWidth, x, y,
                                  w, h, size, color, ClippingRectangle, blendBGRA);

}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawRectangle(int x, int y, int w, int h, RGBA color)
{
    if (color.alpha == 0)
        return;

    primitives::Rectangle(ScreenBufferSection, ScreenBufferWidth, x, y,
                          w, h, color, ClippingRectangle, blendBGRA);

}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawGradientRectangle(int x, int y, int w, int h, RGBA colors[4])
{
    primitives::GradientRectangle(ScreenBufferSection, ScreenBufferWidth,
                                  x, y, w, h, colors, ClippingRectangle,
                                  blendBGRA, interpolateRGBA);

}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawOutlinedComplex(int r_x, int r_y, int r_w, int r_h,
                                 int circ_x, int circ_y, int circ_r,
                                 RGBA color, int antialias)
{
    if (color.alpha == 0)
        return;

    primitives::OutlinedComplex(ScreenBufferSection, ScreenBufferWidth,
                                r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r,
                                color, antialias, ClippingRectangle,
                                blendBGRA);

}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawFilledComplex(int r_x, int r_y, int r_w, int r_h,
                               int circ_x, int circ_y, int circ_r,
                               float angle, float frac_size, int fill_empty, RGBA colors[2])
{
    primitives::FilledComplex(ScreenBufferSection, ScreenBufferWidth,
                              r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r,
                              angle, frac_size, fill_empty, colors, ClippingRectangle,
                              blendBGRA);

}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawGradientComplex(int r_x, int r_y, int r_w, int r_h,
                                 int circ_x, int circ_y, int circ_r,
                                 float angle, float frac_size, int fill_empty, RGBA colors[3])
{
    primitives::GradientComplex(ScreenBufferSection, ScreenBufferWidth,
                                r_x, r_y, r_w, r_h, circ_x, circ_y, circ_r,
                                angle, frac_size, fill_empty, colors, ClippingRectangle,
                                blendBGRA);

}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawOutlinedEllipse(int x, int y, int rx, int ry, RGBA color)
{
    if (color.alpha == 0)
        return;

    primitives::OutlinedEllipse(ScreenBufferSection, ScreenBufferWidth, x, y,
                                rx, ry, color, ClippingRectangle, blendBGRA);

}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawFilledEllipse(int x, int y, int rx, int ry, RGBA color)
{
    if (color.alpha == 0)
        return;

    primitives::FilledEllipse(ScreenBufferSection, ScreenBufferWidth, x, y,
                              rx, ry, color, ClippingRectangle, blendBGRA);

}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawOutlinedCircle(int x, int y, int r, RGBA color, int antialias)
{
    if (color.alpha == 0)
        return;

    primitives::OutlinedCircle(ScreenBufferSection, ScreenBufferWidth, x, y,
                               r, color, antialias, ClippingRectangle, blendBGRA);

}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawFilledCircle(int x, int y, int r, RGBA color, int antialias)
{
    if (color.alpha == 0)
        return;

    primitives::FilledCircle(ScreenBufferSection, ScreenBufferWidth, x, y,
                             r, color, antialias, ClippingRectangle, blendBGRA);

}

////////////////////////////////////////////////////////////////////////////////
EXPORT(void) DrawGradientCircle(int x, int y, int r, RGBA colors[2], int antialias)
{
    primitives::GradientCircle(ScreenBufferSection, ScreenBufferWidth, x, y,
                               r, colors, antialias, ClippingRectangle, blendBGRA);

}

////////////////////////////////////////////////////////////////////////////////





















































