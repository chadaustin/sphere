#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "win32_internal.hpp"
#include "win32_input.hpp"
#include "../sphere.hpp"


LRESULT CALLBACK SphereWindowProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam);


static HWND SphereWindow;


////////////////////////////////////////////////////////////////////////////////

static void GetSphereDirectory(char sphere_directory[MAX_PATH])
{
#ifdef NDEBUG
  GetModuleFileName(GetModuleHandle(NULL), sphere_directory, MAX_PATH);
  *strrchr(sphere_directory, '\\') = 0;
#else
  GetCurrentDirectory(MAX_PATH, sphere_directory);
#endif
}

////////////////////////////////////////////////////////////////////////////////

static void LoadSphereConfiguration(SPHERECONFIG* config)
{
  char ConfigPath[MAX_PATH];
  GetSphereDirectory(ConfigPath);
  strcat(ConfigPath, "\\engine.ini");

  // Loads configuration settings
  LoadSphereConfig(config, ConfigPath);
}

////////////////////////////////////////////////////////////////////////////////

int __cdecl main(int argc, const char** argv)
{
  // seed the random number generator
  srand(time(NULL));

  // set current directory to be Sphere directory
  char sphere_directory[MAX_PATH];
  GetSphereDirectory(sphere_directory);
//  SetCurrentDirectory(sphere_directory);   // THE STUB WILL TAKE CARE OF THIS NOW

  // initialize screenshot directory
  char screenshot_directory[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, screenshot_directory);
  strcat(screenshot_directory, "\\screenshots");
  SetScreenshotDirectory(screenshot_directory);

  // load it from a file
  SPHERECONFIG Config;
  LoadSphereConfiguration(&Config);
  if (Config.videodriver.length() == 0) {

    // tell user
    MessageBox(NULL, "Sphere configuration not found.\n"
                     "Sphere will now run config.exe", "Sphere", MB_OK);

    // execute setup.exe
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi;
    if (CreateProcess("config.exe", "", NULL, NULL, TRUE,
          NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi) == FALSE) {
      MessageBox(NULL, "Could not run config.exe", "Sphere", MB_OK);
      return 0;
    }

    while (WaitForSingleObject(pi.hProcess, 100) != WAIT_OBJECT_0) {
      ;
    }

    LoadSphereConfiguration(&Config);
    if (Config.videodriver.length() == 0) {
      MessageBox(NULL, "Could not load configuration", "Sphere", MB_OK);
      return 0;
    }
  }

  // register the window class
  WNDCLASS wc;
  memset(&wc, 0, sizeof(wc));
  wc.lpfnWndProc   = SphereWindowProc;
  wc.hInstance     = GetModuleHandle(NULL);
  wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor       = NULL;
  wc.hbrBackground = NULL;
  wc.lpszClassName = "SphereWindowClass";
  if (RegisterClass(&wc) == 0) {
    MessageBox(NULL, "Error: Could not register the window class", "Sphere", MB_OK);
    return 0;
  }

  // create the sphere window
  SphereWindow = CreateWindow(,
    "SphereWindowClass", "Sphere",
    WS_CAPTION | WS_MINIMIZEBOX | WS_POPUP | WS_VISIBLE,
    CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
    NULL, NULL, GetModuleHandle(NULL), NULL);
  if (SphereWindow == NULL) {
    MessageBox(NULL, "Error: Could not create the window", "Sphere", MB_OK);
    return 0;
  }

  // initialize video subsystem
  if (InitVideo(SphereWindow, &Config) == false) {
    DestroyWindow(SphereWindow);
    MessageBox(NULL, "Error: Could not initialize video subsystem", "Sphere", MB_OK);
    return 0;
  }

  // initialize audio subsystem
  if (InitAudio(SphereWindow, &Config) == false) {
    CloseVideo();
    DestroyWindow(SphereWindow);
    MessageBox(NULL, "Error: Could not initialize audio subsystem", "Sphere", MB_OK);
    return 0;
  }

  // initialize input subsystem
  if (InitInput(SphereWindow, &Config) == false) {
    CloseAudio();
    CloseVideo();
    DestroyWindow(SphereWindow);
    MessageBox(NULL, "Error: Could not initialize input subsystem", "Sphere", MB_OK);
    return 0;
  }

  // initialize network subsystem
  if (InitNetworkSystem() == false) {
    CloseInput();
    CloseAudio();
    CloseVideo();
    DestroyWindow(SphereWindow);
    MessageBox(NULL, "Error: Could not initialize network subsystem", "Sphere", MB_OK);
    return 0;
  }

  UpdateSystem();
  RunSphere(argc, argv);
  UpdateSystem();

  // Clean up
  CloseNetworkSystem();
  CloseInput();
  CloseAudio();
  CloseVideo();
  DestroyWindow(SphereWindow);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK SphereWindowProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case WM_SYSKEYDOWN: {
      if (wparam == 115) { // if user hit ALT+F4
        CloseNetworkSystem();
        CloseInput();
        CloseAudio();
        CloseVideo();
        DestroyWindow(window);
        exit(0);
      }
    }

      // fall through

    case WM_KEYDOWN: {
      if (wparam == VK_F11) {
        ToggleFPS();
      } else if (wparam == VK_F12) {
        ShouldTakeScreenshot = true;
      } else {
        OnKeyDown(wparam);
      }
      return 0;
    }

    ////////////////////////////////////////////////////////////////////////////

    case WM_SYSKEYUP:
    case WM_KEYUP: {
      if (wparam != 115 && wparam != VK_F11 && wparam != VK_F12) {
        OnKeyUp(wparam);
      }
      return 0;
    }

    ////////////////////////////////////////////////////////////////////////////

    case WM_MOUSEMOVE: {
      SetCursor(NULL);

      int x = LOWORD(lparam);
      if (x < 0) {
        x = 0;
      } else if (x > GetScreenWidth() - 1) {
        x = GetScreenWidth() - 1;
      }

      int y = HIWORD(lparam);
      if (y < 0) {
        y = 0;
      } else if (y > GetScreenHeight() - 1) {
        y = GetScreenHeight() - 1;
      }      

      OnMouseMove(x, y);
      return 0;
    }

    case WM_LBUTTONDOWN: {
      SetCapture(window);
      OnMouseDown(MOUSE_LEFT);
      return 0;
    }

    case WM_LBUTTONUP: {
      OnMouseUp(MOUSE_LEFT);
      ReleaseCapture();
      return 0;
    }

    case WM_MBUTTONDOWN: {
      SetCapture(window);
      OnMouseDown(MOUSE_MIDDLE);
      return 0;
    }

    case WM_MBUTTONUP: {
      OnMouseUp(MOUSE_MIDDLE);
      ReleaseCapture();
      return 0;
    }

    case WM_RBUTTONDOWN: {
      SetCapture(window);
      OnMouseDown(MOUSE_RIGHT);
      return 0;
    }

    case WM_RBUTTONUP: {
      OnMouseUp(MOUSE_RIGHT);
      ReleaseCapture();
      return 0;
    }

    ////////////////////////////////////////////////////////////////////////////

    case WM_PAINT: {  // handle the paint message, just don't do anything
      PAINTSTRUCT ps;
      BeginPaint(window, &ps);
      EndPaint(window, &ps);
      return 0;
    }

    ////////////////////////////////////////////////////////////////////////////

    default: {
      return DefWindowProc(window, msg, wparam, lparam);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void QuitMessage(const char* message)
{
  CloseNetworkSystem();
  CloseInput();
  CloseAudio();
  CloseVideo();
  DestroyWindow(SphereWindow);
  UpdateSystem();
  MessageBox(NULL, message, "Sphere", MB_OK);
  exit(1);
}

////////////////////////////////////////////////////////////////////////////////

void UpdateSystem()
{
  MSG msg;
  int count = 0;
  while (count++ < 4 && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
  {
    DispatchMessage(&msg);
    if (msg.message == WM_QUIT)
      DestroyWindow(SphereWindow);
  }
}

////////////////////////////////////////////////////////////////////////////////
