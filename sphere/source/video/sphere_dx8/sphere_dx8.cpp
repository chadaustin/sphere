#include <d3d8.h>
#include <d3dx8tex.h>
#include <d3dx8math.h>
#include <stdio.h>
#include <math.h>
#include "../../common/rgb.hpp"
#include "../../common/ParticleStructs.hpp"
#include "../common/win32x.hpp"
#include "resource.h"

// macros
#define EXPORT __stdcall

// types
struct CONFIGURATION
{
    bool fullscreen;
};

struct DRIVERINFO
{
    const char* name;
    const char* author;
    const char* date;
    const char* version;
    const char* description;
};

typedef struct IMAGE_STRUCT
{
    int width;
    int height;
    RGBA* pixels;
    IDirect3DTexture8* texture;
    float tex_width;
    float tex_height;
}* IMAGE;

#define COLOR_VERTEX_FORMAT (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
struct COLOR_VERTEX
{
    FLOAT x, y, z, rhw;
    DWORD color;
};

#define TEXTURE_VERTEX_FORMAT (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
struct TEXTURE_VERTEX
{
    FLOAT x, y, z, rhw;
    DWORD color;
    FLOAT tu, tv;
};

// function prototypes
static void GetConfigFile(char filename[MAX_PATH]);
static void LoadConfig();
static void SaveConfig();
static BOOL CALLBACK ConfigureDriverDialogProc(HWND dialog, UINT message, WPARAM wparam, LPARAM lparam);
static bool CreateTexture(IMAGE image);

void EXPORT DirectGrab(int x, int y, int w, int h, RGBA* pixels);
void EXPORT BlitImageMask(IMAGE image, int x, int y, RGBA mask);
void EXPORT TransformBlitImageMask(IMAGE image, int x[4], int y[4], RGBA mask);

// globals
static HINSTANCE         s_DriverInstance;
static CONFIGURATION     s_Configuration;
static IDirect3D8*       s_Direct3D;
static IDirect3DDevice8* s_Direct3DDevice;
static int               s_ScreenWidth;
static int               s_ScreenHeight;
static bool              s_Fullscreen;

////////////////////////////////////////////////////////////////////////////////
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    s_DriverInstance = instance;
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
void EXPORT GetDriverInfo(DRIVERINFO* driverinfo)
{
    driverinfo->name        = "DirectX 8";
    driverinfo->author      = "Chad Austin";
    driverinfo->date        = __DATE__;
    driverinfo->version     = "0.50";
    driverinfo->description = "DirectX 8 hardware accelerated driver";
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT ConfigureDriver(HWND parent)
{
    LoadConfig();
    DialogBox(
        s_DriverInstance,
        MAKEINTRESOURCE(IDD_CONFIGURE),
        parent,
        ConfigureDriverDialogProc
    );
}
////////////////////////////////////////////////////////////////////////////////
void GetConfigFile(char filename[MAX_PATH])
{
    GetModuleFileName(s_DriverInstance, filename, MAX_PATH);
    if (strrchr(filename, '\\'))
    {

        *strrchr(filename, '\\') = 0;
        strcat(filename, "\\");
    }
    else
    {

        filename[0] = 0;
    }
    strcat(filename, "sphere_dx8.cfg");
}
////////////////////////////////////////////////////////////////////////////////
void LoadConfig()
{
    char filename[MAX_PATH];
    GetConfigFile(filename);
    s_Configuration.fullscreen = (0 != GetPrivateProfileInt("sphere_dx8", "fullscreen", 0, filename));
}
////////////////////////////////////////////////////////////////////////////////
// BOOL WritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nInt, LPCTSTR lpFileName)
// {
//   char str[80];
//   sprintf(str, "%d", nInt);
//   return WritePrivateProfileString(lpAppName, lpKeyName, str, lpFileName);
// }

////////////////////////////////////////////////////////////////////////////////
void SaveConfig()
{
    char filename[MAX_PATH];
    GetConfigFile(filename);
    WritePrivateProfileInt("sphere_dx8", "fullscreen", s_Configuration.fullscreen, filename);
}
////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK ConfigureDriverDialogProc(HWND dialog, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {

    case WM_INITDIALOG:
    {

        // set fullscreen flag
        if (s_Configuration.fullscreen)
        {

            CheckDlgButton(dialog, IDC_FULLSCREEN, BST_CHECKED);
        }
        return TRUE;
    }
    case WM_COMMAND:
    {

        switch (LOWORD(wparam))
        {

        case IDOK:
        {

            // store fullscreen setting
            s_Configuration.fullscreen = (IsDlgButtonChecked(dialog, IDC_FULLSCREEN) == BST_CHECKED);
            SaveConfig();
            EndDialog(dialog, 0);
            return TRUE;
        }
        case IDCANCEL:
        {

            EndDialog(dialog, 0);
            return TRUE;
        }
        }
    }
    default:
    {

        return FALSE;
    }
    }
}
////////////////////////////////////////////////////////////////////////////////
bool EXPORT InitVideoDriver(HWND window, int screen_width, int screen_height)
{
    static bool firstcall = true;
    LoadConfig();
    // create the Direct3D object
    s_Direct3D = Direct3DCreate8(D3D_SDK_VERSION);
    if (s_Direct3D == NULL)
    {

        MessageBox(window, "Direct3D object creation failed", "sphere_dx8", MB_OK | MB_ICONERROR);
        return false;
    }
    if (firstcall)
    {

        s_Fullscreen = s_Configuration.fullscreen;
        firstcall = false;
    }
    // set the display mode
    D3DDISPLAYMODE display_mode;
    if (s_Fullscreen)
    {   // FULLSCREEN

        display_mode.RefreshRate = 0;  // use default
        display_mode.Format      = D3DFMT_A8R8G8B8;
    }
    else
    {                            // WINDOWED

        // ask Windows for the current settings
        if (FAILED(s_Direct3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &display_mode)))
        {

            MessageBox(window, "Direct3D GetAdapterDisplayMode failed", "sphere_dx8", MB_OK | MB_ICONERROR);
            s_Direct3D->Release();
            s_Direct3D = NULL;
            return false;
        }
        CenterWindow(window, screen_width, screen_height);
    }
    display_mode.Width  = screen_width;
    display_mode.Height = screen_height;
    // define how we want our application displayed
    D3DPRESENT_PARAMETERS d3dpp;
    memset(&d3dpp, 0, sizeof(d3dpp));
    d3dpp.Windowed         = (s_Fullscreen ? FALSE : TRUE);
    d3dpp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = display_mode.Format;
    d3dpp.BackBufferWidth  = display_mode.Width;
    d3dpp.BackBufferHeight = display_mode.Height;
    // create the device object
    HRESULT result = s_Direct3D->CreateDevice(
                         D3DADAPTER_DEFAULT,
                         D3DDEVTYPE_HAL,
                         window,
                         D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                         &d3dpp,
                         &s_Direct3DDevice
                     );
    if (FAILED(result))
    {

        s_Direct3D->Release();
        s_Direct3D = NULL;
        MessageBox(window, "Direct3D device creation failed", "sphere_dx8", MB_OK | MB_ICONERROR);
        return false;
    }
    // device setup
    BOOL succeeded = TRUE;
    // disable lighting
    succeeded &= !FAILED(s_Direct3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE));
    succeeded &= !FAILED(s_Direct3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
    succeeded &= !FAILED(s_Direct3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
    succeeded &= !FAILED(s_Direct3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
    if (!succeeded)
    {

        MessageBox(window, "Initial device setup failed", "sphere_dx8", MB_OK | MB_ICONERROR);
        return false;
    }
    // start the initial scene
    s_Direct3DDevice->BeginScene();
    s_ScreenWidth  = screen_width;
    s_ScreenHeight = screen_height;
    return true;
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT CloseVideoDriver()
{
    // end the current scene
    s_Direct3DDevice->EndScene();
    // release the device
    if (s_Direct3DDevice)
    {

        s_Direct3DDevice->Release();
        s_Direct3DDevice = NULL;
    }
    // release the Direct3D object
    if (s_Direct3D)
    {

        s_Direct3D->Release();
        s_Direct3D = NULL;
    }
}
////////////////////////////////////////////////////////////////////////////////
bool EXPORT ToggleFullScreen()
{
    return true;
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT FlipScreen()
{
    // end the scene
    s_Direct3DDevice->EndScene();
    // flip
    s_Direct3DDevice->Present(NULL, NULL, NULL, NULL);
    s_Direct3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    // start a new scene
    s_Direct3DDevice->BeginScene();
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT SetClippingRectangle(int x, int y, int w, int h)
{
    D3DVIEWPORT8 viewport;
    viewport.X      = x;
    viewport.Y      = y;
    viewport.Width  = w;
    viewport.Height = h;
    viewport.MinZ   = 0.0f;
    viewport.MaxZ   = 1.0f;
    s_Direct3DDevice->SetViewport(&viewport);
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT GetClippingRectangle(int* x, int* y, int* w, int* h)
{
    D3DVIEWPORT8 viewport;
    s_Direct3DDevice->GetViewport(&viewport);
    *x = viewport.X;
    *y = viewport.Y;
    *w = viewport.Width;
    *h = viewport.Height;
}
////////////////////////////////////////////////////////////////////////////////
IMAGE EXPORT CreateImage(int width, int height, RGBA* pixels)
{
    IMAGE i = new IMAGE_STRUCT;
    i->width  = width;
    i->height = height;
    i->pixels = new RGBA[width * height];
    memcpy(i->pixels, pixels, sizeof(RGBA) * width * height);
    if (!CreateTexture(i))
    {

        delete[] i->pixels;
        delete i;
        return NULL;
    }
    return i;
}
////////////////////////////////////////////////////////////////////////////////
bool CreateTexture(IMAGE image)
{
    HRESULT result = D3DXCreateTexture(
                         s_Direct3DDevice,
                         image->width,
                         image->height,
                         1,
                         0,
                         D3DFMT_A8R8G8B8,
                         D3DPOOL_MANAGED,
                         &image->texture
                     );
    if (FAILED(result))
    {

        return false;
    }
    // now find out the real texture size
    D3DSURFACE_DESC ddsd;
    image->texture->GetLevelDesc(0, &ddsd);
    image->tex_width  = (float)image->width  / ddsd.Width;
    image->tex_height = (float)image->height / ddsd.Height;
    // lock
    D3DLOCKED_RECT locked_rect;
    result = image->texture->LockRect(0, &locked_rect, NULL, 0);
    if (FAILED(result))
    {

        image->texture->Release();
        return false;
    }
    byte* pixels = (byte*)locked_rect.pBits;
    RGBA* src = image->pixels;
    int iy = image->height;
    while (iy--)
    {

        BGRA* dst = (BGRA*)pixels;
        int ix = image->width;
        while (ix--)
        {

            dst->red   = src->red;
            dst->green = src->green;
            dst->blue  = src->blue;
            dst->alpha = src->alpha;
            dst++;
            src++;
        }
        pixels += locked_rect.Pitch;
    }
    // unlock
    image->texture->UnlockRect(0);
    return true;
}
////////////////////////////////////////////////////////////////////////////////
IMAGE EXPORT GrabImage(int x, int y, int width, int height)
{
    RGBA* pixels = new RGBA[width * height];
    DirectGrab(x, y, width, height, pixels);
    IMAGE result = CreateImage(width, height, pixels);
    delete[] pixels;
    return result;
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DestroyImage(IMAGE image)
{
    image->texture->Release();
    delete[] image->pixels;
    delete image;
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT BlitImage(IMAGE image, int x, int y)
{
    BlitImageMask(image, x, y, CreateRGBA(255, 255, 255, 255));
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT BlitImageMask(IMAGE image, int x, int y, RGBA mask)
{
    D3DCOLOR c = D3DCOLOR_RGBA(mask.red, mask.green, mask.blue, mask.alpha);
    int w = image->width;
    int h = image->height;
    float u = image->tex_width;
    float v = image->tex_height;
    TEXTURE_VERTEX vertex[] = {
                                  { (float)x,     (float)y,      0.0f, 1.0f, c, 0.0f, 0.0f, },
                                  { (float)x + w, (float)y,      0.0f, 1.0f, c, u,    0.0f, },
                                  { (float)x + w, (float)y + h,  0.0f, 1.0f, c, u,    v, },
                                  { (float)x,     (float)y + h,  0.0f, 1.0f, c, 0.0f, v, },
                              };
    s_Direct3DDevice->SetVertexShader(TEXTURE_VERTEX_FORMAT);
    s_Direct3DDevice->SetTexture(0, image->texture);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertex, sizeof(*vertex));
    s_Direct3DDevice->SetTexture(0, NULL);
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT TransformBlitImage(IMAGE image, int x[4], int y[4])
{
    TransformBlitImageMask(image, x, y, CreateRGBA(255, 255, 255, 255));
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT TransformBlitImageMask(IMAGE image, int x[4], int y[4], RGBA mask)
{
    D3DCOLOR c = D3DCOLOR_RGBA(mask.red, mask.green, mask.blue, mask.alpha);
    int w = image->width;
    int h = image->height;
    float u = image->tex_width;
    float v = image->tex_height;
    TEXTURE_VERTEX vertex[] = {
                                  { (float)x[0],     (float)y[0],      0.0f, 1.0f, c, 0.0f, 0.0f, },
                                  { (float)x[1],     (float)y[1],      0.0f, 1.0f, c, u,    0.0f, },
                                  { (float)x[2],     (float)y[2],  0.0f, 1.0f, c, u,    v, },
                                  { (float)x[3],     (float)y[3],  0.0f, 1.0f, c, 0.0f, v, },
                              };
    s_Direct3DDevice->SetVertexShader(TEXTURE_VERTEX_FORMAT);
    s_Direct3DDevice->SetTexture(0, image->texture);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertex, sizeof(*vertex));
    s_Direct3DDevice->SetTexture(0, NULL);
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
    return image->pixels;
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT UnlockImage(IMAGE image, bool pixels_changed)
{
    if (pixels_changed)
    {

        image->texture->Release();
        if (!CreateTexture(image))
        {

            // uh oh...
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DirectBlit(int x, int y, int w, int h, RGBA* pixels)
{
    IMAGE i = CreateImage(w, h, pixels);
    if (i)
    {

        BlitImage(i, x, y);
        DestroyImage(i);
    }
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DirectTransformBlit(int x[4], int y[4], int w, int h, RGBA* pixels)
{
    IMAGE i = CreateImage(w, h, pixels);
    if (i)
    {

        TransformBlitImage(i, x, y);
        DestroyImage(i);
    }
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DirectGrab(int x, int y, int w, int h, RGBA* pixels)
{
    if (x < 0 || y < 0 || x + w > s_ScreenWidth || y + h > s_ScreenHeight)
    {

        return;
    }
    // get the backbuffer
    IDirect3DSurface8* backbuffer;
    HRESULT result = s_Direct3DDevice->GetBackBuffer(
                         0,
                         D3DBACKBUFFER_TYPE_MONO,
                         &backbuffer
                     );
    if (FAILED(result))
    {

        return;
    }
    // now lock the backbuffer
    D3DLOCKED_RECT locked_rect;
    result = backbuffer->LockRect(&locked_rect, NULL, 0);
    if (FAILED(result))
    {

        backbuffer->Release();
        return;
    }
    // copy the pixels out
    RGBA* dst = pixels;
    RGBA* src = (RGBA*)locked_rect.pBits + w;
    int iy = h;
    while (iy--)
    {

        int ix = w;
        while (ix--)
        {

            dst->red   = src->red;
            dst->green = src->green;
            dst->blue  = src->blue;
            dst->alpha = 255;
            pixels++;
            src++;
        }
        src += locked_rect.Pitch / sizeof(RGBA) - w;
    }
    backbuffer->UnlockRect();
    backbuffer->Release();
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DrawPoint(int x, int y, RGBA color)
{
    COLOR_VERTEX vertex = {
                              (float)x,
                              (float)y,
                              0.0f,
                              1.0f,
                              D3DCOLOR_RGBA(color.red, color.green, color.blue, color.alpha),
                          };
    s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, 1, &vertex, sizeof(vertex));
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DrawPointSeries(VECTOR_INT** points, int length, RGBA color)
{
    if (color.alpha == 0)
    {
        return;
    }
    D3DCOLOR c = D3DCOLOR_RGBA(color.red, color.green, color.blue, color.alpha);
    COLOR_VERTEX* vertex = new COLOR_VERTEX[length];
    for (int i = 0; i < length; ++i)
    {
        vertex[i].x     = (float)points[i]->x;
        vertex[i].y     = (float)points[i]->y;
        vertex[i].z     = 0.0f;
        vertex[i].rhw   = 1.0f;
        vertex[i].color = c;
    }
    s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, length, vertex, sizeof(*vertex));
    delete [] vertex;
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DrawLine(int x[2], int y[2], RGBA color)
{
    D3DCOLOR c = D3DCOLOR_RGBA(color.red, color.green, color.blue, color.alpha);
    COLOR_VERTEX vertex[] = {
                                { (float)x[0], (float)y[0], 0.0f, 1.0f, c },
                                { (float)x[1], (float)y[1], 0.0f, 1.0f, c },
                            };
    s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, vertex, sizeof(*vertex));
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DrawGradientLine(int x[2], int y[2], RGBA colors[2])
{
    D3DCOLOR color1 = D3DCOLOR_RGBA(colors[0].red, colors[0].green, colors[0].blue, colors[0].alpha);
    D3DCOLOR color2 = D3DCOLOR_RGBA(colors[1].red, colors[1].green, colors[1].blue, colors[1].alpha);
    COLOR_VERTEX vertex[] = {
                                { (float)x[0], (float)y[0], 0.0f, 1.0f, color1 },
                                { (float)x[1], (float)y[1], 0.0f, 1.0f, color2 },
                            };
    s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, vertex, sizeof(*vertex));
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DrawLineSeries(VECTOR_INT** points, int length, RGBA color, int type)
{
    if (color.alpha == 0)
    {
        return;
    }
    D3DCOLOR c = D3DCOLOR_RGBA(color.red, color.green, color.blue, color.alpha);
    COLOR_VERTEX* vertex = new COLOR_VERTEX[length+1];
    int i;
    for (i = 0; i < length; ++i)
    {
        vertex[i].x     = (float)points[i]->x;
        vertex[i].y     = (float)points[i]->y;
        vertex[i].z     = 0.0f;
        vertex[i].rhw   = 1.0f;
        vertex[i].color = c;
    }
    s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
    if (type == 0)
    {
        s_Direct3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, length-1, vertex, sizeof(*vertex));
    }
    else if (type == 1)
    {
        s_Direct3DDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, length-1, vertex, sizeof(*vertex));
    }
    else
    {
        vertex[i].x     = (float)points[0]->x;
        vertex[i].y     = (float)points[0]->y;
        vertex[i].z     = 0.0f;
        vertex[i].rhw   = 1.0f;
        vertex[i].color = c;
        s_Direct3DDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, length, vertex, sizeof(*vertex));
    }
    delete [] vertex;
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DrawBezierCurve(int x[4], int y[4], double step, RGBA color, int cubic)
{
    if (color.alpha == 0)
    {
        return;
    }
    D3DCOLOR c = D3DCOLOR_RGBA(color.red, color.green, color.blue, color.alpha);
    COLOR_VERTEX* vertex = new COLOR_VERTEX[abs(x[2] - x[0]) * abs(y[2] - y[0])];

    // make sure step is in a valid range
    if (step <= 0)
    {
        step = 0.001;
    }
    else if (step > 1.0)
    {
        step = 1.0;
    }
    // draw the Bezier curve
    int count = 0, new_x, new_y, old_x, old_y;
    double b;
    for (double a = 1.0; a >= 0; a -= step)
    {
        b = 1.0 - a;
        if (cubic)
        {               // calculate the cubic Bezier curve
            new_x = (int)(x[0]*a*a*a + x[1]*3*a*a*b + x[3]*3*a*b*b + x[2]*b*b*b);
            new_y = (int)(y[0]*a*a*a + y[1]*3*a*a*b + y[3]*3*a*b*b + y[2]*b*b*b);
        }
        else
        {               // calculate the quadratic Bezier curve
            new_x = (int)(x[0]*a*a + x[1]*2*a*b + x[2]*b*b);
            new_y = (int)(y[0]*a*a + y[1]*2*a*b + y[2]*b*b);
        }
        if (a != 1.0)
        {
            if (new_x != old_x || new_y != old_y)
            {
                vertex[count].x     = (float)new_x;
                vertex[count].y     = (float)new_y;
                vertex[count].z     = 0.0f;
                vertex[count].rhw   = 1.0f;
                vertex[count].color = c;
                count++;
            }
        }
        else
        {
            vertex[count].x     = (float)new_x;
            vertex[count].y     = (float)new_y;
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = c;
            count++;
        }
        old_x = new_x;
        old_y = new_y;
    }
    s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, count, vertex, sizeof(*vertex));
    delete [] vertex;
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DrawTriangle(int x[3], int y[3], RGBA color)
{
    D3DCOLOR c = D3DCOLOR_RGBA(color.red, color.green, color.blue, color.alpha);
    COLOR_VERTEX vertex[] = {
                                { (float)x[0], (float)y[0], 0.0f, 1.0f, c },
                                { (float)x[1], (float)y[1], 0.0f, 1.0f, c },
                                { (float)x[2], (float)y[2], 0.0f, 1.0f, c },
                            };
    s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, vertex, sizeof(*vertex));
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DrawGradientTriangle(int x[3], int y[3], RGBA colors[3])
{
    D3DCOLOR color1 = D3DCOLOR_RGBA(colors[0].red, colors[0].green, colors[0].blue, colors[0].alpha);
    D3DCOLOR color2 = D3DCOLOR_RGBA(colors[1].red, colors[1].green, colors[1].blue, colors[1].alpha);
    D3DCOLOR color3 = D3DCOLOR_RGBA(colors[2].red, colors[2].green, colors[2].blue, colors[2].alpha);
    COLOR_VERTEX vertex[] = {
                                { (float)x[0], (float)y[0], 0.0f, 1.0f, color1 },
                                { (float)x[1], (float)y[1], 0.0f, 1.0f, color2 },
                                { (float)x[2], (float)y[2], 0.0f, 1.0f, color3 },
                            };
    s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, vertex, sizeof(*vertex));
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DrawPolygon(VECTOR_INT** points, int length, int invert, RGBA color)
{
    if (color.alpha == 0)
    {
        return;
    }
    // find polygon's bounds
    int i, bound_x1 = points[0]->x, bound_x2 = points[0]->x, bound_y1 = points[0]->y, bound_y2 = points[0]->y;
    for (i = 1; i < length; i++)
    {
        if (points[i]->x > bound_x2) bound_x2 = points[i]->x;
        if (points[i]->x < bound_x1) bound_x1 = points[i]->x;
        if (points[i]->y > bound_y2) bound_y2 = points[i]->y;
        if (points[i]->y < bound_y1) bound_y1 = points[i]->y;
    }
    D3DCOLOR c = D3DCOLOR_RGBA(color.red, color.green, color.blue, color.alpha);
    COLOR_VERTEX* vertex = new COLOR_VERTEX[(bound_x2 - bound_x1) * (bound_y2 - bound_y1)];

    // draw the polygon with the crossing number algorithm
    int count = 0, point_in, c_x, c_y, j;
    for (c_y = bound_y1; c_y <= bound_y2; c_y++)
    {
        for (c_x = bound_x1; c_x <= bound_x2; c_x++)
        {
            point_in = 0;
            j = length-1;
            for (i = 0; i < length; i++)
            {
                if (points[i]->y <= c_y && points[j]->y > c_y ||
                    points[j]->y <= c_y && points[i]->y > c_y)
                {
                    if (points[i]->x >= c_x && points[j]->x >= c_x)
                    {
                        point_in = !point_in;
                    }
                    else if ((float)(points[i]->x) +
                            ((float)(c_y - points[i]->y) /
                             (float)(points[j]->y - points[i]->y)) *
                             (float)(points[j]->x - points[i]->x) > c_x)
                    {
                        point_in = !point_in;
                    }
                }
                j = i;
            }
            if (invert)
            {
                point_in = !point_in;
            }
            if (point_in)
            {
                vertex[count].x     = (float)c_x;
                vertex[count].y     = (float)c_y;
                vertex[count].z     = 0.0f;
                vertex[count].rhw   = 1.0f;
                vertex[count].color = c;
                count++;
            }
        }
    }
    s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, count, vertex, sizeof(*vertex));
    delete [] vertex;
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DrawOutlinedRectangle(int x, int y, int w, int h, int size, RGBA color)
{
    if (color.alpha == 0 || size <= 0 || h / 2 < 1)
    {
        return;
    }
    // make sure size is in a valid range
    if (size > h / 2)
    {
        size = h / 2;
    }
    D3DCOLOR c = D3DCOLOR_RGBA(color.red, color.green, color.blue, color.alpha);
    COLOR_VERTEX* vertex = new COLOR_VERTEX[w*h];
    int ix, iy, tx, ty, count = 0;

    for (iy = y; iy < y + size; iy++)
    {
        for (ix = x; ix < x+w; ix++)
        {
            vertex[count].x     = (float)ix;
            vertex[count].y     = (float)iy;
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = c;
            count++;
            ty = y + h - 1 - (iy - y);
            vertex[count].x     = (float)ix;
            vertex[count].y     = (float)ty;
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = c;
            count++;
        }
    }
    ty = y + h - size;
    for (iy = y + size; iy < ty; iy++)
    {
        for (ix = x; ix < x + size; ix++)
        {
            vertex[count].x     = (float)ix;
            vertex[count].y     = (float)iy;
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = c;
            count++;
            tx = x + w - 1 - (ix - x);
            vertex[count].x     = (float)tx;
            vertex[count].y     = (float)iy;
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = c;
            count++;
        }
    }
    s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, count, vertex, sizeof(*vertex));
    delete [] vertex;
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DrawRectangle(int x, int y, int w, int h, RGBA color)
{
    D3DCOLOR c = D3DCOLOR_RGBA(color.red, color.green, color.blue, color.alpha);
    COLOR_VERTEX vertex[] = {
                                { (float)x,     (float)y,      0.0f, 1.0f, c, },
                                { (float)x + w, (float)y,      0.0f, 1.0f, c, },
                                { (float)x + w, (float)y + h,  0.0f, 1.0f, c, },
                                { (float)x,     (float)y + h,  0.0f, 1.0f, c, }
                            };
    s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertex, sizeof(*vertex));
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DrawGradientRectangle(int x, int y, int w, int h, RGBA colors[4])
{
    D3DCOLOR color1 = D3DCOLOR_RGBA(colors[0].red, colors[0].green, colors[0].blue, colors[0].alpha);
    D3DCOLOR color2 = D3DCOLOR_RGBA(colors[1].red, colors[1].green, colors[1].blue, colors[1].alpha);
    D3DCOLOR color3 = D3DCOLOR_RGBA(colors[2].red, colors[2].green, colors[2].blue, colors[2].alpha);
    D3DCOLOR color4 = D3DCOLOR_RGBA(colors[3].red, colors[3].green, colors[3].blue, colors[3].alpha);
    COLOR_VERTEX vertex[] = {
                                { (float)x,     (float)y,      0.0f, 1.0f, color1, },
                                { (float)x + w, (float)y,      0.0f, 1.0f, color2, },
                                { (float)x + w, (float)y + h,  0.0f, 1.0f, color3, },
                                { (float)x,     (float)y + h,  0.0f, 1.0f, color4, }
                            };
    s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertex, sizeof(*vertex));
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DrawOutlinedComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, RGBA color, int antialias)
{
    D3DCOLOR c = D3DCOLOR_RGBA(color.red, color.green, color.blue, color.alpha);
    COLOR_VERTEX* vertex = new COLOR_VERTEX[r_w*r_h];
    int x, y, dist, ca = color.alpha, crr = circ_r * circ_r, count = 0;
    float fdist, fcr = (float)(circ_r), fca = (float)(ca);

    for (y = r_y; y < r_y + r_h; y++)
    {
        for (x = r_x; x < r_x + r_w; x++)
        {
            dist = abs(circ_x-x)*abs(circ_x-x) + abs(circ_y-y)*abs(circ_y-y);
            if (dist >= crr)
            {
                vertex[count].x     = (float)x;
                vertex[count].y     = (float)y;
                vertex[count].z     = 0.0f;
                vertex[count].rhw   = 1.0f;
                vertex[count].color = c;
                count++;
            }
            else if (antialias)
            {
                fdist = fcr - sqrt((float)(dist));
                if (fdist < 1)
                {
                    vertex[count].x     = (float)x;
                    vertex[count].y     = (float)y;
                    vertex[count].z     = 0.0f;
                    vertex[count].rhw   = 1.0f;
                    vertex[count].color = D3DCOLOR_RGBA(color.red,
                                                        color.green,
                                                        color.blue,
                                                        (byte)(fca * (1.0 - fdist)));
                    count++;
                }
            }
        }
    }
    s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, count, vertex, sizeof(*vertex));
    delete [] vertex;
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DrawFilledComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, float angle, float frac_size, int fill_empty, RGBA colors[2])
{
    D3DCOLOR c0 = D3DCOLOR_RGBA(colors[0].red, colors[0].green, colors[0].blue, colors[0].alpha);
    D3DCOLOR c1 = D3DCOLOR_RGBA(colors[1].red, colors[1].green, colors[1].blue, colors[1].alpha);
    COLOR_VERTEX* vertex = new COLOR_VERTEX[r_w*r_h];
    int x, y, dist, crr = circ_r * circ_r, count = 0;
    float fang_p;
    const float PI = (float)(3.1415927);

    // make sure frac_size is in a valid range
    if (frac_size < 0 || frac_size >= PI)
    {
        frac_size = 0;
    }

    for (y = r_y; y < r_y + r_h; y++)
    {
        for (x = r_x; x < r_x + r_w; x++)
        {
            // check if point is outside of the circle
            dist = abs(x-circ_x)*abs(x-circ_x) + abs(y-circ_y)*abs(y-circ_y);
            if (dist >= crr)
            {
                vertex[count].x     = (float)x;
                vertex[count].y     = (float)y;
                vertex[count].z     = 0.0f;
                vertex[count].rhw   = 1.0f;
                vertex[count].color = c0;
                count++;
            }
            else
            {
                if (frac_size != 0)
                {
                    // check if point is located in fraction
                    fang_p = atan2(float(y-circ_y), float(x-circ_x));
                    if (fang_p < 0)
                    {
                        fang_p = PI + (PI + fang_p);
                    }
                    fang_p = fabs(angle - fang_p);
                    if (fang_p >= PI)
                    {
                        fang_p = 2*PI - fang_p;
                    }
                    if (fang_p <= frac_size)
                    {
                        // it is, so draw the point with circle's color
                        vertex[count].x     = (float)x;
                        vertex[count].y     = (float)y;
                        vertex[count].z     = 0.0f;
                        vertex[count].rhw   = 1.0f;
                        vertex[count].color = c1;
                        count++;
                    }
                    else if (fill_empty)
                    {
                        // it is not, so draw the point with rectangle's color
                        vertex[count].x     = (float)x;
                        vertex[count].y     = (float)y;
                        vertex[count].z     = 0.0f;
                        vertex[count].rhw   = 1.0f;
                        vertex[count].color = c0;
                        count++;
                    }
                }
                else
                {
                    vertex[count].x     = (float)x;
                    vertex[count].y     = (float)y;
                    vertex[count].z     = 0.0f;
                    vertex[count].rhw   = 1.0f;
                    vertex[count].color = c1;
                    count++;
                }
            }
        }
    }
    s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, count, vertex, sizeof(*vertex));
    delete [] vertex;
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DrawGradientComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, float angle, float frac_size, int fill_empty, RGBA colors[3])
{
    D3DCOLOR c0 = D3DCOLOR_RGBA(colors[0].red, colors[0].green, colors[0].blue, colors[0].alpha);
    COLOR_VERTEX* vertex = new COLOR_VERTEX[r_w*r_h];
    int x, y, dist, crr = circ_r * circ_r, count = 0;
    float fdr = (float)(colors[2].red - colors[1].red);
    float fdg = (float)(colors[2].green - colors[1].green);
    float fdb = (float)(colors[2].blue - colors[1].blue);
    float fda = (float)(colors[2].alpha - colors[1].alpha);
    float fang_p, factor, fdist, fr = (float)(circ_r);
    const float PI = (float)(3.1415927), PI_H = PI / 2;

    // make sure frac_size is in a valid range
    if (frac_size < 0 || frac_size >= PI)
    {
        frac_size = 0;
    }

    for (y = r_y; y < r_y + r_h; y++)
    {
        for (x = r_x; x < r_x + r_w; x++)
        {
            // check if point is outside of the circle
            dist = abs(x-circ_x)*abs(x-circ_x) + abs(y-circ_y)*abs(y-circ_y);
            if (dist >= crr)
            {
                vertex[count].x     = (float)x;
                vertex[count].y     = (float)y;
                vertex[count].z     = 0.0f;
                vertex[count].rhw   = 1.0f;
                vertex[count].color = c0;
                count++;
            }
            else
            {
                if (frac_size != 0)
                {
                    // check if point is located in fraction
                    fang_p = atan2(float(y-circ_y), float(x-circ_x));
                    if (fang_p < 0)
                    {
                        fang_p = PI + (PI + fang_p);
                    }
                    fang_p = abs(angle - fang_p);
                    if (fang_p >= PI)
                    {
                        fang_p = 2*PI - fang_p;
                    }
                    if (fang_p <= frac_size)
                    {
                        // it is, so draw the point with circle's color
                        fdist = sqrt((float)(dist));
                        factor = sin((float(1) - fdist / fr) * PI_H);

                        vertex[count].x     = (float)x;
                        vertex[count].y     = (float)y;
                        vertex[count].z     = 0.0f;
                        vertex[count].rhw   = 1.0f;
                        vertex[count].color = D3DCOLOR_RGBA((byte)(colors[2].red - fdr * factor),
                                                            (byte)(colors[2].green - fdg * factor),
                                                            (byte)(colors[2].blue - fdb * factor),
                                                            (byte)(colors[2].alpha - fda * factor));
                        count++;
                    }
                    else if (fill_empty)
                    {
                        // it is not, so draw the point with rectangle's color
                        vertex[count].x     = (float)x;
                        vertex[count].y     = (float)y;
                        vertex[count].z     = 0.0f;
                        vertex[count].rhw   = 1.0f;
                        vertex[count].color = c0;
                        count++;
                    }
                }
                else
                {
                    fdist = sqrt((float)(dist));
                    factor = sin((float(1) - fdist / fr) * PI_H);

                    vertex[count].x     = (float)x;
                    vertex[count].y     = (float)y;
                    vertex[count].z     = 0.0f;
                    vertex[count].rhw   = 1.0f;
                    vertex[count].color = D3DCOLOR_RGBA((byte)(colors[2].red - fdr * factor),
                                                        (byte)(colors[2].green - fdg * factor),
                                                        (byte)(colors[2].blue - fdb * factor),
                                                        (byte)(colors[2].alpha - fda * factor));
                    count++;
                }
            }
        }
    }
    s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, count, vertex, sizeof(*vertex));
    delete [] vertex;
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DrawOutlinedEllipse(int xc, int yc, int rx, int ry, RGBA color)
{
    if (color.alpha == 0)
    {
        return;
    }

    D3DCOLOR c = D3DCOLOR_RGBA(color.red, color.green, color.blue, color.alpha);
    COLOR_VERTEX* vertex = new COLOR_VERTEX[(rx*2)*(ry*2)];
    int xcm1 = xc - 1;
    int ycm1 = yc - 1;
    int count = 0;

    // draw ellipse with bresenham's ellipse algorithm
    int x = rx;
    int y = 0;
    int tworx2 = 2 * rx * rx;
    int twory2 = 2 * ry * ry;
    int xchange = ry * ry * (1 - 2 * rx);
    int ychange = rx * rx;
    int error = 0;
    int xstop = twory2 * rx;
    int ystop = 0;

    // draw first set of points
    while (xstop >= ystop)
    {
        vertex[count].x     = (float)(xc + x);
        vertex[count].y     = (float)(yc + y);
        vertex[count].z     = 0.0f;
        vertex[count].rhw   = 1.0f;
        vertex[count].color = c;
        count++;
        vertex[count].x     = (float)(xcm1 - x);
        vertex[count].y     = (float)(yc + y);
        vertex[count].z     = 0.0f;
        vertex[count].rhw   = 1.0f;
        vertex[count].color = c;
        count++;
        vertex[count].x     = (float)(xcm1 - x);
        vertex[count].y     = (float)(ycm1 - y);
        vertex[count].z     = 0.0f;
        vertex[count].rhw   = 1.0f;
        vertex[count].color = c;
        count++;
        vertex[count].x     = (float)(xc + x);
        vertex[count].y     = (float)(ycm1 - y);
        vertex[count].z     = 0.0f;
        vertex[count].rhw   = 1.0f;
        vertex[count].color = c;
        count++;

        y++;
        ystop   += tworx2;
        error   += ychange;
        ychange += tworx2;
        if (2 * error + xchange > 0)
        {
            x--;
            xstop -= twory2;
            error += xchange;
            xchange += twory2;
        }
    }

    x = 0;
    y = ry;
    xchange = ry * ry;
    ychange = rx * rx * (1 - 2 * ry);
    error = 0;
    xstop = 0;
    ystop = tworx2 * ry;

    // draw second set of points
    while (xstop <= ystop)
    {
        vertex[count].x     = (float)(xc + x);
        vertex[count].y     = (float)(yc + y);
        vertex[count].z     = 0.0f;
        vertex[count].rhw   = 1.0f;
        vertex[count].color = c;
        count++;
        vertex[count].x     = (float)(xcm1 - x);
        vertex[count].y     = (float)(yc + y);
        vertex[count].z     = 0.0f;
        vertex[count].rhw   = 1.0f;
        vertex[count].color = c;
        count++;
        vertex[count].x     = (float)(xcm1 - x);
        vertex[count].y     = (float)(ycm1 - y);
        vertex[count].z     = 0.0f;
        vertex[count].rhw   = 1.0f;
        vertex[count].color = c;
        count++;
        vertex[count].x     = (float)(xc + x);
        vertex[count].y     = (float)(ycm1 - y);
        vertex[count].z     = 0.0f;
        vertex[count].rhw   = 1.0f;
        vertex[count].color = c;
        count++;

        x++;
        xstop   += twory2;
        error   += xchange;
        xchange += twory2;
        if (2 * error + ychange > 0)
        {
            y--;
            ystop -= tworx2;
            error += ychange;
            ychange += tworx2;
        }
    }
    s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, count, vertex, sizeof(*vertex));
    delete [] vertex;
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DrawFilledEllipse(int xc, int yc, int rx, int ry, RGBA color)
{
    if (color.alpha == 0)
    {
        return;
    }

    D3DCOLOR c = D3DCOLOR_RGBA(color.red, color.green, color.blue, color.alpha);
    COLOR_VERTEX* vertex = new COLOR_VERTEX[(rx*2)*(ry*2)];

    int xcm1 = xc - 1;
    int ycm1 = yc - 1;
    int count = 0;

    // draw ellipse with bresenham's ellipse algorithm
    int x = rx;
    int y = 0;
    int tworx2 = 2 * rx * rx;
    int twory2 = 2 * ry * ry;
    int xchange = ry * ry * (1 - 2 * rx);
    int ychange = rx * rx;
    int error = 0;
    int xstop = twory2 * rx;
    int ystop = 0;

    // first set of points
    while (xstop >= ystop)
    {
        vertex[count].x     = (float)(xcm1 - x);
        vertex[count].y     = (float)(ycm1 - y);
        vertex[count].z     = 0.0f;
        vertex[count].rhw   = 1.0f;
        vertex[count].color = c;
        count++;
        vertex[count].x     = (float)(xc + x);
        vertex[count].y     = (float)(ycm1 - y);
        vertex[count].z     = 0.0f;
        vertex[count].rhw   = 1.0f;
        vertex[count].color = c;
        count++;
        vertex[count].x     = (float)(xcm1 - x);
        vertex[count].y     = (float)(yc + y);
        vertex[count].z     = 0.0f;
        vertex[count].rhw   = 1.0f;
        vertex[count].color = c;
        count++;
        vertex[count].x     = (float)(xc + x);
        vertex[count].y     = (float)(yc + y);
        vertex[count].z     = 0.0f;
        vertex[count].rhw   = 1.0f;
        vertex[count].color = c;
        count++;

        y++;
        ystop   += tworx2;
        error   += ychange;
        ychange += tworx2;
        if (2 * error + xchange > 0)
        {
            x--;
            xstop -= twory2;
            error += xchange;
            xchange += twory2;
        }
    }

    x = 0;
    y = ry;
    xchange = ry * ry;
    ychange = rx * rx * (1 - 2 * ry);
    error = 0;
    xstop = 0;
    ystop = tworx2 * ry;

    // second set of points
    while (xstop <= ystop)
    {
        x++;
        xstop   += twory2;
        error   += xchange;
        xchange += twory2;
        if (2 * error + ychange > 0)
        {
            vertex[count].x     = (float)(xcm1 - x);
            vertex[count].y     = (float)(ycm1 - y);
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = c;
            count++;
            vertex[count].x     = (float)(xc + x);
            vertex[count].y     = (float)(ycm1 - y);
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = c;
            count++;
            vertex[count].x     = (float)(xcm1 - x);
            vertex[count].y     = (float)(yc + y);
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = c;
            count++;
            vertex[count].x     = (float)(xc + x);
            vertex[count].y     = (float)(yc + y);
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = c;
            count++;

            y--;
            ystop -= tworx2;
            error += ychange;
            ychange += tworx2;
        }
    }
    s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, count, vertex, sizeof(*vertex));
    delete [] vertex;
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DrawOutlinedCircle(int x, int y, int r, RGBA color, int antialias)
{
    if (color.alpha == 0)
    {
        return;
    }

    D3DCOLOR c = D3DCOLOR_RGBA(color.red, color.green, color.blue, color.alpha);
    D3DCOLOR tc;

    COLOR_VERTEX* vertex = new COLOR_VERTEX[4*r*r];
    int count = 0, ix = 1, iy = r, dist, n, rr = r*r, rr_m2 = (r-2)*(r-2), ca = color.alpha;
    float fr = (float)(r), fca = (float)(ca);
    const float PI_H = (float)(3.1415927 / 2.0);

    while (ix <= iy)
    {
        if (antialias == 1)
        {
            n = iy + 1;
            while (--n >= ix)
            {
                dist = ix*ix + n*n;
                if (dist > rr) dist = rr;
                if (dist > rr_m2)
                {
                    tc = D3DCOLOR_RGBA(color.red,
                                       color.green,
                                       color.blue,
                                       (byte)(fca * sin(sin((1.0 - fabs(sqrt((float)(dist)) - fr + 1.0)) * PI_H) * PI_H)));

                    vertex[count].x     = (float)(x-1+ix);
                    vertex[count].y     = (float)(y-n);
                    vertex[count].z     = 0.0f;
                    vertex[count].rhw   = 1.0f;
                    vertex[count].color = tc;
                    count++;
                    vertex[count].x     = (float)(x-ix);
                    vertex[count].y     = (float)(y-n);
                    vertex[count].z     = 0.0f;
                    vertex[count].rhw   = 1.0f;
                    vertex[count].color = tc;
                    count++;
                    vertex[count].x     = (float)(x-1+ix);
                    vertex[count].y     = (float)(y-1+n);
                    vertex[count].z     = 0.0f;
                    vertex[count].rhw   = 1.0f;
                    vertex[count].color = tc;
                    count++;
                    vertex[count].x     = (float)(x-ix);
                    vertex[count].y     = (float)(y-1+n);
                    vertex[count].z     = 0.0f;
                    vertex[count].rhw   = 1.0f;
                    vertex[count].color = tc;
                    count++;
                    if (ix != n)
                    {
                        vertex[count].x     = (float)(x-1+n);
                        vertex[count].y     = (float)(y-ix);
                        vertex[count].z     = 0.0f;
                        vertex[count].rhw   = 1.0f;
                        vertex[count].color = tc;
                        count++;
                        vertex[count].x     = (float)(x-n);
                        vertex[count].y     = (float)(y-ix);
                        vertex[count].z     = 0.0f;
                        vertex[count].rhw   = 1.0f;
                        vertex[count].color = tc;
                        count++;
                        vertex[count].x     = (float)(x-1+n);
                        vertex[count].y     = (float)(y-1+ix);
                        vertex[count].z     = 0.0f;
                        vertex[count].rhw   = 1.0f;
                        vertex[count].color = tc;
                        count++;
                        vertex[count].x     = (float)(x-n);
                        vertex[count].y     = (float)(y-1+ix);
                        vertex[count].z     = 0.0f;
                        vertex[count].rhw   = 1.0f;
                        vertex[count].color = tc;
                        count++;
                    }
                }
            }
        }
        else
        {
            vertex[count].x     = (float)(x-1+ix);
            vertex[count].y     = (float)(y-iy);
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = c;
            count++;
            vertex[count].x     = (float)(x-ix);
            vertex[count].y     = (float)(y-iy);
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = c;
            count++;
            vertex[count].x     = (float)(x-1+ix);
            vertex[count].y     = (float)(y-1+iy);
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = c;
            count++;
            vertex[count].x     = (float)(x-ix);
            vertex[count].y     = (float)(y-1+iy);
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = c;
            count++;
            if (ix != iy)
            {
                vertex[count].x     = (float)(x-1+iy);
                vertex[count].y     = (float)(y-ix);
                vertex[count].z     = 0.0f;
                vertex[count].rhw   = 1.0f;
                vertex[count].color = c;
                count++;
                vertex[count].x     = (float)(x-iy);
                vertex[count].y     = (float)(y-ix);
                vertex[count].z     = 0.0f;
                vertex[count].rhw   = 1.0f;
                vertex[count].color = c;
                count++;
                vertex[count].x     = (float)(x-1+iy);
                vertex[count].y     = (float)(y-1+ix);
                vertex[count].z     = 0.0f;
                vertex[count].rhw   = 1.0f;
                vertex[count].color = c;
                count++;
                vertex[count].x     = (float)(x-iy);
                vertex[count].y     = (float)(y-1+ix);
                vertex[count].z     = 0.0f;
                vertex[count].rhw   = 1.0f;
                vertex[count].color = c;
                count++;
            }
        }
        ix++;
        if (abs(ix*ix + iy*iy - rr) > abs(ix*ix + (iy-1)*(iy-1) - rr)) iy--;
    }
    s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, count, vertex, sizeof(*vertex));
    delete [] vertex;
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DrawFilledCircle(int x, int y, int r, RGBA color, int antialias)
{
    if (color.alpha == 0)
    {
        return;
    }

    D3DCOLOR tc;
    COLOR_VERTEX* vertex = new COLOR_VERTEX[(r*2)*(r*2)];
    int count = 0, ix = 1, iy = r, dist, n, rr = r*r, rr_m1 = (r-1)*(r-1), ca = color.alpha;
    float fr = (float)(r), fca = (float)(ca);

    while (ix <= iy)
    {
        n = iy + 1;
        while (--n >= ix)
        {
            if (antialias)
            {
                dist = ix*ix + n*n;
                if (dist > rr) dist = rr;
                if (dist > rr_m1) {color.alpha = (byte)(fca * (fr - sqrt(float(dist))));}
                else {color.alpha = ca;};
            }
            else {color.alpha = ca;};
            tc = D3DCOLOR_RGBA(color.red,
                               color.green,
                               color.blue,
                               color.alpha);
            vertex[count].x     = (float)(x-1+ix);
            vertex[count].y     = (float)(y-n);
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = tc;
            count++;
            vertex[count].x     = (float)(x-ix);
            vertex[count].y     = (float)(y-n);
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = tc;
            count++;
            vertex[count].x     = (float)(x-1+ix);
            vertex[count].y     = (float)(y-1+n);
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = tc;
            count++;
            vertex[count].x     = (float)(x-ix);
            vertex[count].y     = (float)(y-1+n);
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = tc;
            count++;
            if (ix != n)
            {
                vertex[count].x     = (float)(x-1+n);
                vertex[count].y     = (float)(y-ix);
                vertex[count].z     = 0.0f;
                vertex[count].rhw   = 1.0f;
                vertex[count].color = tc;
                count++;
                vertex[count].x     = (float)(x-n);
                vertex[count].y     = (float)(y-ix);
                vertex[count].z     = 0.0f;
                vertex[count].rhw   = 1.0f;
                vertex[count].color = tc;
                count++;
                vertex[count].x     = (float)(x-1+n);
                vertex[count].y     = (float)(y-1+ix);
                vertex[count].z     = 0.0f;
                vertex[count].rhw   = 1.0f;
                vertex[count].color = tc;
                count++;
                vertex[count].x     = (float)(x-n);
                vertex[count].y     = (float)(y-1+ix);
                vertex[count].z     = 0.0f;
                vertex[count].rhw   = 1.0f;
                vertex[count].color = tc;
                count++;
            }
        }
        ix++;
        if (abs(ix*ix + iy*iy - rr) > abs(ix*ix + (iy-1)*(iy-1) - rr)) iy--;
    }
    s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, count, vertex, sizeof(*vertex));
    delete [] vertex;
}
////////////////////////////////////////////////////////////////////////////////
void EXPORT DrawGradientCircle(int x, int y, int r, RGBA colors[2], int antialias)
{
    if (colors[0].alpha == 0 && colors[1].alpha == 0)
    {
        return;
    }

    D3DCOLOR tc;
    COLOR_VERTEX* vertex = new COLOR_VERTEX[(r*2)*(r*2)];
    int count = 0, ix = 1, iy = r, n;
    float fdr = (float)(colors[1].red - colors[0].red);
    float fdg = (float)(colors[1].green - colors[0].green);
    float fdb = (float)(colors[1].blue - colors[0].blue);
    float fda = (float)(colors[1].alpha - colors[0].alpha);
    float dist, factor, fr = (float)(r);
    const float PI_H = (float)(3.1415927 / 2.0), RR = (float)(r*r);

    while (ix <= iy)
    {
        n = iy + 1;
        while (--n >= ix)
        {
            dist = sqrt((float)(ix*ix + n*n));
            if (dist > r) dist = fr;
            factor = sin((float(1) - dist / fr) * PI_H);
            colors[0].red   = (byte)(colors[1].red - fdr * factor);
            colors[0].green = (byte)(colors[1].green - fdg * factor);
            colors[0].blue  = (byte)(colors[1].blue - fdb * factor);
            colors[0].alpha = (byte)(colors[1].alpha - fda * factor);

            if (antialias)
            {
                if (dist > r - 1)
                {
                    colors[0].alpha = (byte)((float)(colors[0].alpha) * (fr - dist));
                }
            }
            tc = D3DCOLOR_RGBA(colors[0].red,
                               colors[0].green,
                               colors[0].blue,
                               colors[0].alpha);

            vertex[count].x     = (float)(x-1+ix);
            vertex[count].y     = (float)(y-n);
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = tc;
            count++;
            vertex[count].x     = (float)(x-ix);
            vertex[count].y     = (float)(y-n);
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = tc;
            count++;
            vertex[count].x     = (float)(x-1+ix);
            vertex[count].y     = (float)(y-1+n);
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = tc;
            count++;
            vertex[count].x     = (float)(x-ix);
            vertex[count].y     = (float)(y-1+n);
            vertex[count].z     = 0.0f;
            vertex[count].rhw   = 1.0f;
            vertex[count].color = tc;
            count++;
            if (ix != n)
            {
                vertex[count].x     = (float)(x-1+n);
                vertex[count].y     = (float)(y-ix);
                vertex[count].z     = 0.0f;
                vertex[count].rhw   = 1.0f;
                vertex[count].color = tc;
                count++;
                vertex[count].x     = (float)(x-n);
                vertex[count].y     = (float)(y-ix);
                vertex[count].z     = 0.0f;
                vertex[count].rhw   = 1.0f;
                vertex[count].color = tc;
                count++;
                vertex[count].x     = (float)(x-1+n);
                vertex[count].y     = (float)(y-1+ix);
                vertex[count].z     = 0.0f;
                vertex[count].rhw   = 1.0f;
                vertex[count].color = tc;
                count++;
                vertex[count].x     = (float)(x-n);
                vertex[count].y     = (float)(y-1+ix);
                vertex[count].z     = 0.0f;
                vertex[count].rhw   = 1.0f;
                vertex[count].color = tc;
                count++;
            }
        }
        ix++;
        if (abs(ix*ix + iy*iy - RR) > abs(ix*ix + (iy-1)*(iy-1) - RR)) iy--;
    }
    s_Direct3DDevice->SetVertexShader(COLOR_VERTEX_FORMAT);
    s_Direct3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, count, vertex, sizeof(*vertex));
    delete [] vertex;
}
////////////////////////////////////////////////////////////////////////////////
