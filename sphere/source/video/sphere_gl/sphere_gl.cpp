#include <string>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include "../../common/rgb.hpp"
#include "../common/win32x.hpp"
#include "resource.h"


#define EXPORT(ret) extern "C" ret __stdcall
static bool fullscreen;

typedef struct tagIMAGE
{
    enum { EMPTY, TILE, NORMAL };

    int     width;
    int     height;
    RGBA*   pixels;
    GLuint  texture;
    GLfloat tex_width;
    GLfloat tex_height;
    int     special;  // optimization flags
}* IMAGE;


struct DRIVERINFO
{
    const char* name;
    const char* author;
    const char* date;
    const char* version;
    const char* description;
};


struct DRIVERCONFIG
{
    unsigned bpp;
    bool     scale;
    bool     fullscreen;
    bool     bilinear;
    bool     vsync;
};


std::string glErrorToString(GLenum error) {
    switch (error) {
        case GL_NO_ERROR:          return "No Error";
        case GL_INVALID_ENUM:      return "Invalid Enum";
        case GL_INVALID_VALUE:     return "Invalid Value";
        case GL_INVALID_OPERATION: return "Invalid Operation";
        case GL_STACK_OVERFLOW:    return "Stack Overflow";
        case GL_STACK_UNDERFLOW:   return "Stack Underflow";
        case GL_OUT_OF_MEMORY:     return "Out of Memory";
        default:                   return "Unknown Error";
    }
}


// forward declaration of DirectGrab implementation
EXPORT(void) DirectGrab(int x, int y, int w, int h, RGBA* pixels);

BOOL (APIENTRY *wglSwapIntervalEXT)(int interval);
int (APIENTRY *wglGetSwapIntervalEXT)();


static BOOL CALLBACK ConfigureDriverDialogProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam);
static void UpdateButtonStates(HWND dialog);


static DRIVERCONFIG DriverConfig;
#define SCALE()  (DriverConfig.scale ? 2  : 1)
#define SCALEF() (DriverConfig.scale ? 2.0f : 1.0f)
#define FILTER() (SCALE() == 1 ? GL_NEAREST : (DriverConfig.bilinear ? GL_LINEAR : GL_NEAREST))


static HINSTANCE DriverInstance;

static int ScreenWidth;
static int ScreenHeight;

static GLint MaxTexSize; // width or height

static HWND  SphereWindow;
static DWORD WindowStyle;   // } only use in fullscreen
static DWORD WindowStyleEx; // }
static HDC   MainDC;
static HGLRC MainRC;


////////////////////////////////////////////////////////////////////////////////

extern "C" BOOL WINAPI DllMain(HINSTANCE inst, DWORD, LPVOID)
{
    DriverInstance = inst;
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void GetConfigFile(char configfile[MAX_PATH])
{
    GetModuleFileName(DriverInstance, configfile, 512);
    if (strrchr(configfile, '\\')) {
        *strrchr(configfile, '\\') = 0;
        strcat(configfile, "\\");
    } else {
        configfile[0] = 0;
    }
    strcat(configfile, "sphere_gl.cfg");
}

////////////////////////////////////////////////////////////////////////////////

void LoadDriverConfig()
{
    char configfile[520];

    GetConfigFile(configfile);

    DriverConfig.bpp        = GetPrivateProfileInt("sphere_gl", "bpp", 16, configfile);
    DriverConfig.scale      = (0 != GetPrivateProfileInt("sphere_gl", "scale",      0, configfile));
    DriverConfig.bilinear   = (0 != GetPrivateProfileInt("sphere_gl", "bilinear",   1, configfile));
    DriverConfig.fullscreen = (0 != GetPrivateProfileInt("sphere_gl", "fullscreen", 0, configfile));
    DriverConfig.vsync      = (0 != GetPrivateProfileInt("sphere_gl", "vsync",      1, configfile));
}

////////////////////////////////////////////////////////////////////////////////

void SaveDriverConfig()
{
    char configfile[MAX_PATH];
    GetConfigFile(configfile);

    WritePrivateProfileInt("sphere_gl", "bpp",        DriverConfig.bpp,        configfile);
    WritePrivateProfileInt("sphere_gl", "scale",      DriverConfig.scale,      configfile);
    WritePrivateProfileInt("sphere_gl", "fullscreen", DriverConfig.fullscreen, configfile);
    WritePrivateProfileInt("sphere_gl", "bilinear",   DriverConfig.bilinear,   configfile);
    WritePrivateProfileInt("sphere_gl", "vsync",      DriverConfig.vsync,      configfile);
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) GetDriverInfo(DRIVERINFO* driverinfo)
{
    driverinfo->name        = "OpenGL";
    driverinfo->author      = "Jamie Gennis, Kisai, Chad Austin";
    driverinfo->date        = __DATE__;
    driverinfo->version     = "0.60";
    driverinfo->description = "OpenGL Sphere Video Driver";
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) ConfigureDriver(HWND parent)
{
    LoadDriverConfig();

    DialogBox(
        DriverInstance,
        MAKEINTRESOURCE(IDD_CONFIGURE),
        parent,
        ConfigureDriverDialogProc);
}

////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK ConfigureDriverDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  switch (message)
  {
    case WM_INITDIALOG:
    {
      switch (DriverConfig.bpp)
      {
        case 24:
          CheckDlgButton(window, IDC_BPP_24, BST_CHECKED);
        break;

        case 32:
          CheckDlgButton(window, IDC_BPP_32, BST_CHECKED);
        break;
                
        default:
          CheckDlgButton(window, IDC_BPP_16, BST_CHECKED);
        break;
      }

      if (DriverConfig.scale) {
        CheckDlgButton(window, IDC_SCALE, BST_CHECKED);
      }
      if (DriverConfig.bilinear) {
        CheckDlgButton(window, IDC_BILINEAR, BST_CHECKED);
      }
      if (DriverConfig.fullscreen) {
        CheckDlgButton(window, IDC_FULLSCREEN, BST_CHECKED);
      }
      if (DriverConfig.vsync) {
        CheckDlgButton(window, IDC_VSYNC, BST_CHECKED);
      }

      UpdateButtonStates(window);
            
      return TRUE;
    }

    case WM_COMMAND:
    {
      if (HIWORD(wparam) == BN_CLICKED) {
        UpdateButtonStates(window);
      }

      switch (LOWORD(wparam))
      {
        case IDOK:
          if (IsDlgButtonChecked(window, IDC_BPP_24) == BST_CHECKED) {
            DriverConfig.bpp = 24;
          } else if (IsDlgButtonChecked(window, IDC_BPP_32) == BST_CHECKED) {
            DriverConfig.bpp = 32;
          } else {
            DriverConfig.bpp = 16;
          }

          DriverConfig.scale      = (IsDlgButtonChecked(window, IDC_SCALE)      == BST_CHECKED);
          DriverConfig.bilinear   = (IsDlgButtonChecked(window, IDC_BILINEAR)   == BST_CHECKED);
          DriverConfig.fullscreen = (IsDlgButtonChecked(window, IDC_FULLSCREEN) == BST_CHECKED);
          DriverConfig.vsync      = (IsDlgButtonChecked(window, IDC_VSYNC)      == BST_CHECKED);

          SaveDriverConfig();
          EndDialog(window, 0);
          return TRUE;
        break;

        case IDCANCEL:
          EndDialog(window, 0);
          return TRUE;
        break;
      }
    }

    default:
      return FALSE;
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////

void UpdateButtonStates(HWND dialog)
{
  EnableWindow(GetDlgItem(dialog, IDC_VSYNC),    IsDlgButtonChecked(dialog, IDC_FULLSCREEN) == BST_CHECKED);
  EnableWindow(GetDlgItem(dialog, IDC_BILINEAR), IsDlgButtonChecked(dialog, IDC_SCALE)      == BST_CHECKED);
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) CloseVideoDriver();

EXPORT(bool) InitVideoDriver(HWND window, int screen_width, int screen_height)
{
  const char* error_msg = NULL;
  static bool firstcall = true;

  ScreenWidth = screen_width;
  ScreenHeight = screen_height;

  LoadDriverConfig();

  SphereWindow = window;

  int format;
  PIXELFORMATDESCRIPTOR pfd =
  { 
    sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
    1,                              // version number
    PFD_DRAW_TO_WINDOW |            // support window
    PFD_SUPPORT_OPENGL |            // support OpenGL
    PFD_DOUBLEBUFFER,               // double buffered
    PFD_TYPE_RGBA,                  // RGBA type
    DriverConfig.bpp,               // color depth
    0, 0, 0, 0, 0, 0,               // color bits
    0,                              // alpha buffer
    0,                              // shift bit
    0,                              // accumulation buffer
    0, 0, 0, 0,                     // accum bits
    0,                              // z-buffer
    0,                              // stencil buffer
    0,                              // auxiliary buffer
    PFD_MAIN_PLANE,                 // main layer
    0,                              // reserved
    0, 0, 0                         // layer masks ignored
  };


  if (firstcall) {
    fullscreen = DriverConfig.fullscreen;
    firstcall = false;
  }
    
  if (!fullscreen) {
    CenterWindow(SphereWindow, ScreenWidth * SCALE(), ScreenHeight * SCALE());
  } else {
    // set fullscreen mode
    DEVMODE dm;
    memset(&dm, 0, sizeof(dm));

    //EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);

    dm.dmSize       = sizeof(dm);
    dm.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
    dm.dmBitsPerPel = DriverConfig.bpp;
    dm.dmPelsWidth  = ScreenWidth  * SCALE();
    dm.dmPelsHeight = ScreenHeight * SCALE();

    WindowStyle   = GetWindowLong(SphereWindow, GWL_STYLE);
    WindowStyleEx = GetWindowLong(SphereWindow, GWL_EXSTYLE);

    SetWindowLong(SphereWindow, GWL_STYLE, WS_POPUP | WS_CLIPSIBLINGS);
    SetWindowLong(SphereWindow, GWL_EXSTYLE, 0);

    if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
      error_msg = "Unable to set display mode";
      goto error;
    }

    if (1) {
      DEVMODE dm;
      memset(&dm, 0, sizeof(dm));
      EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
      if (dm.dmBitsPerPel != DriverConfig.bpp) {
        error_msg = "Unable to set bbp, try a different setting";
        goto error;
      }
    }


    // Set up window
    SetWindowPos(SphereWindow, HWND_TOPMOST, 0, 0, ScreenWidth * SCALE(), ScreenHeight * SCALE(), SWP_SHOWWINDOW);
  }
    
  // Get the DC of the window
  MainDC = GetDC(SphereWindow);
  if (!MainDC) {
    MessageBox(SphereWindow, "Error getting window DC.", "Video Error", MB_ICONERROR);
    return false;
  }

  // Set the pfd   
  format = ChoosePixelFormat(MainDC, &pfd);
  if (!SetPixelFormat(MainDC, format, &pfd)) {
    error_msg = "Error setting pfd";
    goto error;
  }

  // Create Render Context
  MainRC = wglCreateContext(MainDC);
  if (!MainRC) {
    error_msg = "Error creating render context";
    goto error;
  }

  // Make context current
  if (!wglMakeCurrent(MainDC, MainRC)) {
    error_msg = "Unable to make render context current";
    goto error;
  }

  // try to get the swap control extension
  if (strstr((const char*)glGetString(GL_EXTENSIONS), "WGL_EXT_swap_control")) {
    wglSwapIntervalEXT    = (BOOL (__stdcall*)(int))wglGetProcAddress("wglSwapIntervalEXT");
    wglGetSwapIntervalEXT = (int (__stdcall*)())wglGetProcAddress("wglGetSwapIntervalEXT");
    if (fullscreen && DriverConfig.vsync) {
      wglSwapIntervalEXT(1);
    } else {
      wglSwapIntervalEXT(0);
    }
  }

  // view initialization
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0f, ScreenWidth, ScreenHeight, 0.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  //glTranslatef(0.375, 0.375, 0.0);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClearDepth(1.0);

  // render initialization
  glEnable(GL_SCISSOR_TEST);
  glScissor(0, 0, ScreenWidth, ScreenHeight);
  //    glEnable(GL_TEXTURE_2D);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glPointSize(FILTER() == GL_LINEAR ? 2.5f : (SCALE() == 1 ? 1.0f : 2.0f));
  glLineWidth(FILTER() == GL_LINEAR ? 2.5f : (SCALE() == 1 ? 1.0f : 2.0f));
  if (FILTER() == GL_LINEAR) {
    glEnable(GL_POINT_SMOOTH);
  }

  // get max texture size
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &MaxTexSize);

  extern void __stdcall SetClippingRectangle(int, int, int, int);
  SetClippingRectangle(0, 0, ScreenWidth, ScreenHeight);

  return true;

error:
  if (fullscreen) {
    DEVMODE dm;
    EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &dm);
    dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFLAGS | DM_DISPLAYFREQUENCY;
    ChangeDisplaySettings(&dm, 0);

    SetWindowLong(SphereWindow, GWL_STYLE, WindowStyle);
    SetWindowLong(SphereWindow, GWL_EXSTYLE, WindowStyleEx);
  }

  MessageBox(SphereWindow, error_msg, "Video Error", MB_ICONERROR);
  return false;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) CloseVideoDriver()
{
  // good bye, OpenGL...
  wglMakeCurrent(NULL, NULL);
  ReleaseDC(SphereWindow, MainDC);
  wglDeleteContext(MainRC);
   
  // reset screen resolution
  if (fullscreen) {
    DEVMODE dm;
    EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &dm);
    dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFLAGS | DM_DISPLAYFREQUENCY;
    ChangeDisplaySettings(&dm, 0);

    SetWindowLong(SphereWindow, GWL_STYLE, WindowStyle);
    SetWindowLong(SphereWindow, GWL_EXSTYLE, WindowStyleEx);
  }
}

////////////////////////////////////////////////////////////////////////////////

/**
 * Switches from fullscreen to windowed or vice-versa,
 * updates the fullscreen flag as needed
 * returns whether the engine should *not* be shutdown
 */
EXPORT(bool) ToggleFullScreen() {
  return true;
/*
  // this causes weird color problems, but vaguely works...
  int x, y, w, h;
  extern void __stdcall GetClippingRectangle(int*, int*, int*, int*);
  extern void __stdcall SetClippingRectangle(int, int, int, int);

  GetClippingRectangle(&x, &y, &w, &h);

  // if we haven't set a screen size, don't close the old driver
  if (ScreenWidth != 0 || ScreenHeight != 0) {
    CloseVideoDriver();
  }

  fullscreen = !fullscreen;

  // attempt to switch
  if (InitVideoDriver(SphereWindow, ScreenWidth, ScreenHeight)) {
    SetClippingRectangle(x, y, w, h);
    return true;
  }
  else {
    fullscreen = !fullscreen;

    // attempt to switch back since the switch failed
    if (InitVideoDriver(SphereWindow, ScreenWidth, ScreenHeight)) {
      SetClippingRectangle(x, y, w, h);
      return true;
    }
  }

  return false;
*/
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) FlipScreen()
{
  SwapBuffers(MainDC);
  //extern void __stdcall DrawRectangle(int x, int y, int w, int h, RGBA mask);
  //DrawRectangle(0, 0, 320, 240, CreateRGBA(0, 0, 0, 255));
  glClear(GL_COLOR_BUFFER_BIT);
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) SetClippingRectangle(int x, int y, int w, int h)
{
  glScissor(x * SCALE(), (ScreenHeight - y - h) * SCALE(), w * SCALE(), h * SCALE());
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) GetClippingRectangle(int* x, int* y, int* w, int* h)
{
  GLint cliprect[4];
  glGetIntegerv(GL_SCISSOR_BOX, cliprect);
  *x = (cliprect[0]) / SCALE();
  *y = (cliprect[1] - ScreenHeight * SCALE() + cliprect[3]) / SCALE();
  *w = (cliprect[2]) / SCALE();
  *h = (cliprect[3]) / SCALE();
}

////////////////////////////////////////////////////////////////////////////////

static bool CreateTexture(IMAGE image)
{
  // figure out if image needs to be scaled
  double log2_width  = log10(image->width)  / log10(2);
  double log2_height = log10(image->height) / log10(2);
  int new_width = image->width;
  int new_height = image->height;

  // if they're not integers, calculate a good texture width
  if (log2_width != floor(log2_width)) {
    new_width = 1 << (int)ceil(log2_width);
  }

  // if they're not integers, calculate a good texture height
  if (log2_height != floor(log2_height)) {
    new_height = 1 << (int)ceil(log2_height);
  }
    
  RGBA* new_pixels = image->pixels;
  if (new_width != image->width || new_height != image->height) {

    // copy the old pixels into the new buffer
    new_pixels = new RGBA[new_width * new_height];
    if (!new_pixels)
      return false;

    for (int i = 0; i < image->height; i++) {
      memcpy(new_pixels + i * new_width,
             image->pixels + i * image->width,
             image->width * sizeof(RGBA));
    }
  }

  // now make sure texture is, at max, MaxTexSize by MaxTexSize

  while (new_width > MaxTexSize) {
    new_width /= 2;

    // allocate a new texture buffer
    RGBA* old_pixels = new_pixels;
    new_pixels = new RGBA[new_width * new_height];
    if (!new_pixels)
      return false;

    RGBA* p = new_pixels;
    RGBA* o = old_pixels;
    for (int iy = 0; iy < new_height; iy++) {
      for (int ix = 0; ix < new_width; ix++) {
        p[ix].red   = (o[ix * 2].red   + o[ix * 2 + 1].red)   / 2;
        p[ix].green = (o[ix * 2].green + o[ix * 2 + 1].green) / 2;
        p[ix].blue  = (o[ix * 2].blue  + o[ix * 2 + 1].blue)  / 2;
        p[ix].alpha = (o[ix * 2].alpha + o[ix * 2 + 1].alpha) / 2;
      }
      p += new_width;
      o += new_width * 2;
    }

    if (old_pixels != image->pixels) {
      delete[] old_pixels;
    }
  }

  while (new_height > MaxTexSize) {
    new_height /= 2;

    // allocate a new texture buffer
    RGBA* old_pixels = new_pixels;
    new_pixels = new RGBA[new_width * new_height];
    if (!new_pixels)
      return false;

    RGBA* p = new_pixels;
    RGBA* o = old_pixels;
    for (int ix = 0; ix < new_width; ix++) {
      for (int iy = 0; iy < new_height; iy++) {
        p[iy * new_width].red   = (o[(iy * 2) * new_width].red   + o[(iy * 2 + 1) * new_width].red)   / 2;
        p[iy * new_width].green = (o[(iy * 2) * new_width].green + o[(iy * 2 + 1) * new_width].green) / 2;
        p[iy * new_width].blue  = (o[(iy * 2) * new_width].blue  + o[(iy * 2 + 1) * new_width].blue)  / 2;
        p[iy * new_width].alpha = (o[(iy * 2) * new_width].alpha + o[(iy * 2 + 1) * new_width].alpha) / 2;
      }
      p++;
      o++;
    }
    

    if (old_pixels != image->pixels) {
      delete[] old_pixels;
    }

  }

  // minor correction factor
  float correction_x = (DriverConfig.scale && DriverConfig.bilinear ? 0.5f / (float)new_width  : 0.0f);
  float correction_y = (DriverConfig.scale && DriverConfig.bilinear ? 0.5f / (float)new_height : 0.0f);
  image->tex_width  = (GLfloat)image->width  / new_width  - correction_x;
  image->tex_height = (GLfloat)image->height / new_height - correction_y;

  glGenTextures(1, &image->texture);
  glBindTexture(GL_TEXTURE_2D, image->texture);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (float)FILTER());
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (float)FILTER());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, new_width, new_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, new_pixels);

  if (new_pixels != image->pixels) {
     delete[] new_pixels;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

static int AnalyzePixels(int width, int height, RGBA* pixels)
{
  bool is_empty = true;
  for (int i = 0; i < width * height; i++) {
    if (pixels[i].alpha) {
      is_empty = false;
      break;
    }
  }
  if (is_empty) {
    return tagIMAGE::EMPTY;
  }

  // no alpha data (tile)
  bool is_tile = true;
  for (int i = 0; i < width * height; i++) {
    if (pixels[i].alpha < 255) {
      is_tile = false;
      break;
    }
  }
  if (is_tile) {
    return tagIMAGE::TILE;
  }

  // normal image
  return tagIMAGE::NORMAL;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(IMAGE) CreateImage(int width, int height, RGBA* pixels)
{
    // put default values in image object
  IMAGE image = new tagIMAGE;
  if (!image)
    return NULL;

  image->width = width;
  image->height = height;
  image->pixels = new RGBA[width * height];
  if (!pixels) {
    delete image;
    return NULL;
  }

  memcpy(image->pixels, pixels, width * height * sizeof(RGBA));

  if (!CreateTexture(image)) {
    delete image;
    delete[] image->pixels;
    return NULL;
  }

  image->special = AnalyzePixels(width, height, pixels);
  return image;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(IMAGE) GrabImage(int x, int y, int width, int height)
{
  RGBA* pixels = new RGBA[width * height];
  if (!pixels) return NULL;
  DirectGrab(x, y, width, height, pixels);
  IMAGE result = CreateImage(width, height, pixels);
  delete[] pixels;
  return result;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DestroyImage(IMAGE image)
{
  glDeleteTextures(1, &image->texture);
  image->texture = NULL;
  image->tex_width = 0;
  image->tex_height = 0;
  delete[] image->pixels;
  image->pixels = NULL;
  delete image;
  image = NULL;
}

////////////////////////////////////////////////////////////////////////////////

static inline float sphere_x_to_opengl_x(int x) {
  return (float) x + 0.5f;
}

static inline float sphere_y_to_opengl_y(int y) {
  return (float) y + 0.5f;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) BlitImage(IMAGE image, int x, int y)
{
  extern void __stdcall BlitImageMask(IMAGE image, int x, int y, RGBA mask);
  BlitImageMask(image, x, y, CreateRGBA(255, 255, 255, 255));
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) BlitImageMask(IMAGE image, int x, int y, RGBA mask)
{
  if (image->special == tagIMAGE::EMPTY) {
    return;
  }

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, image->texture);

  glColor4ub(mask.red, mask.green, mask.blue, mask.alpha);
  glBegin(GL_QUADS);

    glTexCoord2f(0, 0);
    glVertex2i(x, y);
 
    glTexCoord2f(image->tex_width, 0); 
    glVertex2i(x + image->width, y);

    glTexCoord2f(image->tex_width, image->tex_height);
    glVertex2i(x + image->width, y + image->height);

    glTexCoord2f(0, image->tex_height);
    glVertex2i(x, y + image->height);

  glEnd();
  glDisable(GL_TEXTURE_2D);
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) TransformBlitImage(IMAGE image, int x[4], int y[4])
{
  if (image->special == tagIMAGE::EMPTY) {
    return;
  }

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, image->texture);

  int cx = (x[0] + x[1] + x[2] + x[3]) / 4;
  int cy = (y[0] + y[1] + y[2] + y[3]) / 4;
    
  glColor4f(1, 1, 1, 1);
  glBegin(GL_TRIANGLE_FAN);

      // center
    glTexCoord2f(image->tex_width / 2, image->tex_height / 2);
    glVertex2i(cx, cy);
 
    glTexCoord2f(0, 0);
    glVertex2i(x[0], y[0]);
 
    glTexCoord2f(image->tex_width, 0);
    glVertex2i(x[1], y[1]);
 
    glTexCoord2f(image->tex_width, image->tex_height);
    glVertex2i(x[2], y[2]);
 
    glTexCoord2f(0, image->tex_height);
    glVertex2i(x[3], y[3]);

    glTexCoord2f(0, 0);
    glVertex2i(x[0], y[0]);
  
  glEnd(); 
  
  glDisable(GL_TEXTURE_2D);
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) TransformBlitImageMask(IMAGE image, int x[4], int y[4], RGBA mask)
{
  if (image->special == tagIMAGE::EMPTY) {
    return;
  }

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, image->texture);

  int cx = (x[0] + x[1] + x[2] + x[3]) / 4;
  int cy = (y[0] + y[1] + y[2] + y[3]) / 4;
   
  glColor4ub(mask.red, mask.green, mask.blue, mask.alpha);
  glBegin(GL_TRIANGLE_FAN);

    // center
    glTexCoord2f(image->tex_width / 2, image->tex_height / 2);
    glVertex2i(cx, cy);
 
    glTexCoord2f(0, 0);
    glVertex2i(x[0], y[0]);
 
    glTexCoord2f(image->tex_width, 0);
    glVertex2i(x[1], y[1]);
 
    glTexCoord2f(image->tex_width, image->tex_height);
    glVertex2i(x[2], y[2]);
 
    glTexCoord2f(0, image->tex_height);
    glVertex2i(x[3], y[3]);

    glTexCoord2f(0, 0);
    glVertex2i(x[0], y[0]);

  glEnd();
    
  glDisable(GL_TEXTURE_2D);
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
  return image->pixels;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) UnlockImage(IMAGE image, bool pixels_changed)
{
  if (pixels_changed) {
    glDeleteTextures(1, &image->texture);
    CreateTexture(image);
    image->special = AnalyzePixels(image->width, image->height, image->pixels);
  }
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DirectBlit(int x, int y, int w, int h, RGBA* pixels)
{
  IMAGE i = CreateImage(w, h, pixels);
  if (i) {
    BlitImage(i, x, y);
    DestroyImage(i);
  }
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DirectTransformBlit(int x[4], int y[4], int w, int h, RGBA* pixels)
{
  IMAGE i = CreateImage(w, h, pixels);
  if (i) {
    TransformBlitImage(i, x, y);
    DestroyImage(i);
  }
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DirectGrab(int x, int y, int w, int h, RGBA* pixels)
{
  if (x < 0 || y < 0 || x + w > ScreenWidth || y + h > ScreenHeight) {
    return;
  }

  if (SCALE() == 2) {
      
    // manually scale the framebuffer down
    RGBA* new_pixels = new RGBA[4 * w * h];
    if (!new_pixels)
      return;

    glReadPixels(x * 2, (ScreenHeight - y - h) * 2, w * 2, h * 2, GL_RGBA, GL_UNSIGNED_BYTE, new_pixels);

    for (int i = 0; i < w; i++) {
      for (int j = 0; j < h; j++) {
        pixels[j * w + i] = new_pixels[(j * 2) * (w * 2) + i * 2];
      }
    }

    delete[] new_pixels;

  } else {
    glReadPixels(x, ScreenHeight - y - h, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    // puts(glErrorToString(error).c_str());
  }

  // now invert the rows
  RGBA* row = new RGBA[w];
  if (!row)
    return;

  RGBA* top = pixels;
  RGBA* bot = pixels + w * (h - 1);
  while (top < bot) {
    memcpy(row, top, w * sizeof(RGBA));
    memcpy(top, bot, w * sizeof(RGBA));
    memcpy(bot, row, w * sizeof(RGBA));

    top += w;
    bot -= w;
  }

  delete[] row;
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DrawPoint(int x, int y, RGBA color)
{
  glBegin(GL_POINTS);
  glColor4ubv((GLubyte*)&color);
  glVertex2f(sphere_x_to_opengl_x(x), sphere_y_to_opengl_y(y));
  glEnd();
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DrawLine(int x[2], int y[2], RGBA color)
{
  glBegin(GL_LINES);
    //glColor4ubv((GLubyte*)&color);
    glColor4ub(color.red, color.green, color.blue, color.alpha);
    glVertex2f(sphere_x_to_opengl_x(x[0]), sphere_y_to_opengl_y(y[0]));
    glVertex2f(sphere_x_to_opengl_x(x[1]), sphere_y_to_opengl_y(y[1]));
  glEnd();
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DrawGradientLine(int x[2], int y[2], RGBA colors[2])
{
  glBegin(GL_LINES);
    //glColor4ubv((GLubyte*)(colors + 0));
    glColor4ub(colors[0].red, colors[0].green, colors[0].blue, colors[0].alpha);
    glVertex2f(sphere_x_to_opengl_x(x[0]), sphere_y_to_opengl_y(y[0]));
    //glColor4ubv((GLubyte*)(colors + 1));
    glColor4ub(colors[1].red, colors[1].green, colors[1].blue, colors[1].alpha);
    glVertex2f(sphere_x_to_opengl_x(x[1]), sphere_y_to_opengl_y(y[1]));
  glEnd();
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DrawTriangle(int x[3], int y[3], RGBA color)
{
  glBegin(GL_TRIANGLES);
  glColor4ubv((GLubyte*)&color);
  glVertex2f(sphere_x_to_opengl_x(x[0]), sphere_y_to_opengl_y(y[0]));
  glVertex2f(sphere_x_to_opengl_x(x[1]), sphere_y_to_opengl_y(y[1]));
  glVertex2f(sphere_x_to_opengl_x(x[2]), sphere_y_to_opengl_y(y[2]));
  glEnd();
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DrawGradientTriangle(int x[3], int y[3], RGBA colors[3])
{
  glBegin(GL_TRIANGLES);
  glColor4ubv((GLubyte*)(colors + 0));
  glVertex2f(sphere_x_to_opengl_x(x[0]), sphere_y_to_opengl_y(y[0]));
  glColor4ubv((GLubyte*)(colors + 1));
  glVertex2f(sphere_x_to_opengl_x(x[1]), sphere_y_to_opengl_y(y[1]));
  glColor4ubv((GLubyte*)(colors + 2));
  glVertex2f(sphere_x_to_opengl_x(x[2]), sphere_y_to_opengl_y(y[2]));
  glEnd();
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DrawRectangle(int x, int y, int w, int h, RGBA color)
{
  if (color.alpha == 0) {
    return;
  }

  glTranslatef(-0.5f, -0.5f, 0.0f);
  glBegin(GL_QUADS);
  glColor4ubv((GLubyte*)&color);
  glVertex2f(sphere_x_to_opengl_x(x),     sphere_y_to_opengl_y(y));
  glVertex2f(sphere_x_to_opengl_x(x + w), sphere_y_to_opengl_y(y));
  glVertex2f(sphere_x_to_opengl_x(x + w), sphere_y_to_opengl_y(y + h));
  glVertex2f(sphere_x_to_opengl_x(x),     sphere_y_to_opengl_y(y + h));
  glEnd();
  glTranslatef(0.5f, 0.5f, 0.0f);
}

////////////////////////////////////////////////////////////////////////////////

EXPORT(void) DrawGradientRectangle(int x, int y, int w, int h, RGBA colors[4])
{
  glTranslatef(-0.5f, -0.5f, 0.0f);
  glBegin(GL_QUADS);
  glColor4ubv((GLubyte*)(colors + 0)); glVertex2f(sphere_x_to_opengl_x(x),     sphere_y_to_opengl_y(y));
  glColor4ubv((GLubyte*)(colors + 1)); glVertex2f(sphere_x_to_opengl_x(x + w), sphere_y_to_opengl_y(y));
  glColor4ubv((GLubyte*)(colors + 2)); glVertex2f(sphere_x_to_opengl_x(x + w), sphere_y_to_opengl_y(y + h));
  glColor4ubv((GLubyte*)(colors + 3)); glVertex2f(sphere_x_to_opengl_x(x),     sphere_y_to_opengl_y(y + h));
  glEnd();
  glTranslatef(0.5f, 0.5f, 0.0f);
}

////////////////////////////////////////////////////////////////////////////////
