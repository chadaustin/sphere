#include <windows.h>
#include <windowsx.h>
#include "types.h"
#include "splash.h"


static LRESULT CALLBACK SplashWindowProc(HWND, UINT, WPARAM, LPARAM);


static HDC     PaintDC;
static HBITMAP PaintBitmap;
static int     BitmapWidth;
static int     BitmapHeight;
static int     Delay;


////////////////////////////////////////////////////////////////////////////////

bool SplashWindow(HINSTANCE inst, HWND parent, const char* bitmap_id, int delay)
{
  static bool WindowClassRegistered = false;

  HWND window;
  char windowname[520];
  MSG msg;

  BITMAP bitmap;
  RECT rect;

  Delay = delay;

  // Register window class on the first time
  if (!WindowClassRegistered)
  {
    WNDCLASS wc;

    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = SplashWindowProc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "SplashWindowClass";
    if (RegisterClass(&wc) == 0)
      return false;

    WindowClassRegistered = true;
  }

  // Load bitmap
  PaintBitmap = LoadBitmap(inst, bitmap_id);
  if (PaintBitmap == NULL)
    return false;
  PaintDC = CreateCompatibleDC(NULL);
  if (PaintDC == NULL)
    return false;
  SelectObject(PaintDC, PaintBitmap);

  // Get bitmap width and height
  GetObject(PaintBitmap, sizeof(bitmap), &bitmap);
  BitmapWidth = bitmap.bmWidth;
  BitmapHeight = bitmap.bmHeight;

  if (parent)
    GetWindowText(parent, windowname, 512);
  else
    windowname[0] = 0;

  // Make a window rectangle
  rect.left   = (GetSystemMetrics(SM_CXSCREEN) - BitmapWidth) / 2;
  rect.top    = (GetSystemMetrics(SM_CYSCREEN) - BitmapHeight) / 2;
  rect.right  = (GetSystemMetrics(SM_CXSCREEN) + BitmapWidth) / 2;
  rect.bottom = (GetSystemMetrics(SM_CYSCREEN) + BitmapHeight) / 2;
  AdjustWindowRect(&rect, WS_DLGFRAME | WS_POPUP, FALSE);

  // Create splash window
  window = CreateWindow("SplashWindowClass", windowname,
    WS_DLGFRAME | WS_POPUP,
    rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
    parent, NULL, inst, NULL);
  if (window == NULL)
    return false;
  ShowWindow(window, SW_SHOW);
  UpdateWindow(window);

  // Handle window messages until a WM_TIMER is received
  ZeroMemory(&msg, sizeof(msg));
  while (msg.message != WM_TIMER)
  {
    GetMessage(&msg, NULL, 0, 0);
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // Destroy window
  DestroyWindow(window);

  DeleteDC(PaintDC);
  DeleteObject(PaintBitmap);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK SplashWindowProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case WM_CREATE:
      SetTimer(window, 91827, Delay, NULL);
      return 0;

    case WM_DESTROY:
      KillTimer(window, 91827);
      return 0;

    case WM_TIMER:
      return 0;

    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      BeginPaint(window, &ps);
      BitBlt(ps.hdc, 0, 0, BitmapWidth, BitmapHeight, PaintDC, 0, 0, SRCCOPY);
      EndPaint(window, &ps);
      return 0;
    }

    default:
      return DefWindowProc(window, msg, wparam, lparam);
  }
}

////////////////////////////////////////////////////////////////////////////////
