#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "win32_filesystem.hpp"
#include "win32_internal.hpp"
#include "win32_input.hpp"
#include "../sphere.hpp"
#include "../PlayerConfig.hpp"
#include "../../common/sphere_version.h" 
#include "resource.h"

LRESULT CALLBACK SphereWindowProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam);


static HWND SphereWindow = NULL;

////////////////////////////////////////////////////////////////////////////////

static void LoadSphereConfiguration(SPHERECONFIG* config)
{
  // Loads configuration settings
  LoadSphereConfig(config, (GetSphereDirectory() + "\\engine.ini").c_str());
}

////////////////////////////////////////////////////////////////////////////////

int KeyStringToKeyCode(char* key_string)
{
  if (strlen(key_string) >= strlen("KEY_X")) {
    if (strlen(key_string) == strlen("KEY_X")) {
      switch (key_string[strlen("KEY_")]) {
        case 'A': return KEY_A;
        case 'B': return KEY_B;
        case 'C': return KEY_C;
        case 'D': return KEY_D;
        case 'E': return KEY_E;
        case 'F': return KEY_F;
        case 'G': return KEY_G;
        case 'H': return KEY_H;
        case 'I': return KEY_I;
        case 'J': return KEY_J;
        case 'K': return KEY_K;
        case 'L': return KEY_L;
        case 'M': return KEY_M;
        case 'N': return KEY_N;
        case 'O': return KEY_O;
        case 'P': return KEY_P;
        case 'Q': return KEY_Q;
        case 'R': return KEY_R;
        case 'S': return KEY_S;
        case 'T': return KEY_T;
        case 'U': return KEY_U;
        case 'V': return KEY_V;
        case 'W': return KEY_W;
        case 'X': return KEY_X;
        case 'Y': return KEY_Y;
        case 'Z': return KEY_Z;
        case '0': return KEY_0;
        case '1': return KEY_1;
        case '2': return KEY_2;
        case '3': return KEY_3;
        case '4': return KEY_4;
        case '5': return KEY_5;
        case '6': return KEY_6;
        case '7': return KEY_7;
        case '8': return KEY_8;
        case '9': return KEY_9;
      }

      return -1;
    }

    if (strcmp(key_string, "KEY_UP")  == 0) return KEY_UP;
    if (strcmp(key_string, "KEY_DOWN")  == 0) return KEY_DOWN;
    if (strcmp(key_string, "KEY_LEFT")  == 0) return KEY_LEFT;
    if (strcmp(key_string, "KEY_RIGHT")  == 0) return KEY_RIGHT;

    if (strcmp(key_string, "KEY_TILDE")  == 0) return KEY_TILDE;
    if (strcmp(key_string, "KEY_MINUS")  == 0) return KEY_MINUS;
    if (strcmp(key_string, "KEY_EQUALS") == 0) return KEY_EQUALS;
    if (strcmp(key_string, "KEY_SPACE")  == 0) return KEY_SPACE;
    if (strcmp(key_string, "KEY_OPENBRACE")  == 0) return KEY_OPENBRACE;
    if (strcmp(key_string, "KEY_CLOSEBRACE") == 0) return KEY_CLOSEBRACE;
    if (strcmp(key_string, "KEY_SEMICOLON")  == 0) return KEY_SEMICOLON;
    if (strcmp(key_string, "KEY_COMMA")  == 0) return KEY_COMMA;
    if (strcmp(key_string, "KEY_APOSTROPHE") == 0) return KEY_APOSTROPHE;
    if (strcmp(key_string, "KEY_PERIOD") == 0) return KEY_PERIOD;
    if (strcmp(key_string, "KEY_SLASH")  == 0) return KEY_SLASH;
    if (strcmp(key_string, "KEY_BACKSLASH") == 0) return KEY_BACKSLASH;

  }

  return -1;
}

int __cdecl main(int argc, const char** argv)
{
  // seed the random number generator
  srand(time(NULL));

  //printf ("SphereDirectory=%s\n", GetSphereDirectory().c_str());

  // set current directory to be Sphere directory
  SetCurrentDirectory(GetSphereDirectory().c_str());

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

  for (int i = 0; i < 4; i++) {
    SetPlayerConfig(i,
      KeyStringToKeyCode(Config.player_configurations[i].key_up_str),
      KeyStringToKeyCode(Config.player_configurations[i].key_down_str),
      KeyStringToKeyCode(Config.player_configurations[i].key_left_str),
      KeyStringToKeyCode(Config.player_configurations[i].key_right_str),
      Config.player_configurations[i].keyboard_input_allowed,
      Config.player_configurations[i].joypad_input_allowed);
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
  SphereWindow = CreateWindow(
    "SphereWindowClass", "Sphere " SPHERE_VERSION,
    WS_CAPTION | WS_MINIMIZEBOX | WS_POPUP | WS_VISIBLE | WS_SYSMENU,
    CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
    NULL,
    NULL,//LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_FILE)),
    GetModuleHandle(NULL), NULL);
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
  if (Config.allow_networking && InitNetworkSystem() == false) {
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

void Shutdown(HWND window) {
  CloseNetworkSystem();
  CloseInput();
  CloseAudio();
  CloseVideo();
  DestroyWindow(window);
  exit(0);
}

////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK SphereWindowProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case WM_CLOSE: {
      Shutdown(window);
    }

    case WM_SYSKEYDOWN: {
      if (wparam == 115) { // if user hit ALT+F4
        Shutdown(window);
      }
    }

      // fall through

    case WM_KEYDOWN: {
      if (wparam == VK_F10) {
        if (ToggleFullScreen() == false) {
          Shutdown(window);
        }
      } else if (wparam == VK_F11) {
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

      int x_scale = 1;
      int y_scale = 1;

      //RECT rect;
      //GetWindowRect(window, &rect);
      //x_scale = (rect.right - rect.left) / GetScreenWidth();
      //y_scale = (rect.bottom - rect.top) / GetScreenHeight();

      int x = ((int)(short)LOWORD(lparam)); // / x_scale;
      if (x < 0) {
        x = 0;
      } else if (x > GetScreenWidth() - 1) {
        x = GetScreenWidth() - 1;
      }

      int y = ((int)(short)HIWORD(lparam)); // / y_scale;
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
