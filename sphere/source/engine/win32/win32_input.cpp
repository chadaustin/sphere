#include <windows.h>
#include <mmsystem.h>
#include <queue>
#include <stdlib.h>
#include "win32_input.hpp"
#include "win32_internal.hpp"


static HWND          SphereWindow = NULL;
static SPHERECONFIG* Config = NULL;


// mapping from Sphere key codes to Windows key codes
int SphereToWindows[MAX_KEY] =
{
  0,
  VK_ESCAPE,
  VK_F1,
  VK_F2,
  VK_F3,
  VK_F4,
  VK_F5,
  VK_F6,
  VK_F7,
  VK_F8,
  VK_F9,
  VK_F10,
  VK_F11,
  VK_F12,
  0x00C0,  // Windows 2000: VK_OEM_3
  0x0030,
  0x0031,
  0x0032,
  0x0033,
  0x0034,
  0x0035,
  0x0036,
  0x0037,
  0x0038,
  0x0039,
  0x00BD,  // Windows 2000: VK_OEM_MINUS,
  0x00BB,  // Windows 2000: VK_EQUALS,
  VK_BACK,
  VK_TAB,
  0x0041,  // a
  0x0042,  // b
  0x0043,  // c
  0x0044,  // d
  0x0045,  // e
  0x0046,  // f
  0x0047,  // g
  0x0048,  // h
  0x0049,  // i
  0x004A,  // j
  0x004B,  // k
  0x004C,  // l
  0x004D,  // m
  0x004E,  // n
  0x004F,  // o
  0x0050,  // p
  0x0051,  // q
  0x0052,  // r
  0x0053,  // s
  0x0054,  // t
  0x0055,  // u
  0x0056,  // v
  0x0057,  // w
  0x0058,  // x
  0x0059,  // y
  0x005A,  // z
  VK_SHIFT,
  VK_CONTROL,
  VK_MENU,
  VK_SPACE,
  0x00DB,  // Windows 2000: VK_OEM_4
  0x00DD,  // Windows 2000: VK_OEM_6
  186,     // colon
  0x00DE,  // Windows 2000: VK_OEM_7
  0x00BC,  // Windows 2000: VK_OEM_COMMA
  0x00BE,  // Windows 2000: VK_OEM_PERIOD
  0x00BF,  // Windows 2000: VK_OEM_2
  0x00DC,  // Windows 2000: VK_OEM_5
  VK_RETURN,
  VK_INSERT,
  VK_DELETE,
  VK_HOME,
  VK_END,
  VK_PRIOR,
  VK_NEXT,
  VK_UP,
  VK_RIGHT,
  VK_DOWN,
  VK_LEFT,
};

int WindowsToSphere[MAX_KEY]; // build dynamically


// keyboard state tables (accessed with virtual keys)
static byte CurrentKeyBuffer[MAX_KEY];
static byte KeyBuffer[MAX_KEY];

// keyboard key queue (virtual keys also)
static std::queue<int> KeyQueue;

// joystick variables
static UINT JoystickID;

static int JoyMinX;
static int JoyMaxX;
static int JoyMinY;
static int JoyMaxY;

static int JoyX;
static int JoyY;

static bool JoyButton1Pressed;
static bool JoyButton2Pressed;
static bool JoyButton3Pressed;
static bool JoyButton4Pressed;

// mouse
static int MouseX;
static int MouseY;
static bool MouseState[3];



// These functions rely on the window handler to modify the keybuffer values and mouse state

////////////////////////////////////////////////////////////////////////////////

bool InitInput(HWND window, SPHERECONFIG* config)
{
  // build mapping from Windows to Sphere keys
  for (int i = 0; i < MAX_KEY; ++i) {
    int k = SphereToWindows[i];
    if (k >= 0 && k < MAX_KEY) {
      WindowsToSphere[k] = i;
    }
  }

  SphereWindow = window;
  Config = config;

  if (Config->joystick)
  {
    JOYINFO ji;
    JOYCAPS jc;
    int NumJoysticks = joyGetNumDevs();

    // Does joystick driver support any joysticks?
    if (NumJoysticks == 0)
    {
      MessageBox(SphereWindow, "Joystick driver does not support any joysticks.  Disable joystick support in setup.exe", "InitInput()", MB_OK);
      return false;
    }

    // Is a joystick plugged in?
    if (joyGetPos(JOYSTICKID1, &ji) == JOYERR_NOERROR)
      JoystickID = JOYSTICKID1;
    else if (joyGetPos(JOYSTICKID2, &ji) == JOYERR_NOERROR)
      JoystickID = JOYSTICKID2;
    else
    {
      MessageBox(SphereWindow, "No joysticks detected.  Disable joystick support in setup.exe", "InitInput()", MB_OK);
      return false;
    }

    // get joystick capabilities
    joyGetDevCaps(JoystickID, &jc, sizeof(jc));
    JoyMinX = jc.wXmin;
    JoyMaxX = jc.wXmax;
    JoyMinY = jc.wYmin;
    JoyMaxY = jc.wYmax;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool CloseInput(void)
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void OnKeyDown(int virtual_key)
{
  if (virtual_key >= 0 && virtual_key < MAX_KEY) {
    int key = WindowsToSphere[virtual_key];
    CurrentKeyBuffer[key] = 1;
    KeyQueue.push(key);
  }
}

////////////////////////////////////////////////////////////////////////////////

void OnKeyUp(int virtual_key)
{
  if (virtual_key >= 0 && virtual_key < MAX_KEY) {
    int key = WindowsToSphere[virtual_key];
    CurrentKeyBuffer[key] = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////

void OnMouseMove(int x, int y)
{
  MouseX = x;
  MouseY = y;
}

////////////////////////////////////////////////////////////////////////////////

void OnMouseDown(int button)
{
  MouseState[button] = true;
}

////////////////////////////////////////////////////////////////////////////////

void OnMouseUp(int button)
{
  MouseState[button] = false;
}

////////////////////////////////////////////////////////////////////////////////

bool RefreshInput()
{
  UpdateSystem();

  // update currently pressed keys
  memcpy(KeyBuffer, CurrentKeyBuffer, MAX_KEY);

  if (Config->joystick)
  {
    // get joystick position information
    JOYINFO ji;
    joyGetPos(JoystickID, &ji);
    JoyButton1Pressed = (ji.wButtons & JOY_BUTTON1) != 0;
    JoyButton2Pressed = (ji.wButtons & JOY_BUTTON2) != 0;
    JoyButton3Pressed = (ji.wButtons & JOY_BUTTON3) != 0;
    JoyButton4Pressed = (ji.wButtons & JOY_BUTTON4) != 0;

    int joymidx = (JoyMinX + JoyMaxX) / 2;
    int joymidy = (JoyMinY + JoyMaxY) / 2;

    if      ((int)ji.wXpos < (JoyMinX + 3 * joymidx) / 4) JoyX = -1;
    else if ((int)ji.wXpos > (JoyMaxX + 3 * joymidx) / 4) JoyX = 1;
    else JoyX = 0;

    if      ((int)ji.wYpos < (JoyMinY + 3 * joymidy) / 4) JoyY = -1;
    else if ((int)ji.wYpos > (JoyMaxY + 3 * joymidy) / 4) JoyY = 1;
    else JoyY = 0;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool IsKeyPressed(int key)
{
  UpdateSystem();

  if (Config->joystick)
  {
    switch (key)
    {
      case KEY_UP:         if (JoyY < 0) return true; break;
      case KEY_RIGHT:      if (JoyX > 0) return true; break;
      case KEY_DOWN:       if (JoyY > 0) return true; break;
      case KEY_LEFT:       if (JoyX < 0) return true; break;
    }
  }

  // find the Sphere key code that corresponds with the virtual key
  return (KeyBuffer[key] != 0);
}

////////////////////////////////////////////////////////////////////////////////

void GetKeyStates(bool keys[MAX_KEY])
{
  UpdateSystem();
  for (int i = 0; i < MAX_KEY; ++i) {
    keys[i] = (KeyBuffer[i] != 0);
  }
}

////////////////////////////////////////////////////////////////////////////////

bool AreKeysLeft()
{
  UpdateSystem();
  return (!KeyQueue.empty());
}

////////////////////////////////////////////////////////////////////////////////

int GetKey()
{
  UpdateSystem();
  while (KeyQueue.empty() == true)
    UpdateSystem();

  int key = KeyQueue.front();
  KeyQueue.pop();
  return key;
}

////////////////////////////////////////////////////////////////////////////////

void SetMousePosition(int x, int y)
{
  POINT p = { x, y };
  ClientToScreen(SphereWindow, &p);
  SetCursorPos(p.x, p.y);  // will send WM_MOUSEMOVE
}

////////////////////////////////////////////////////////////////////////////////

int GetMouseX()
{
  return MouseX;
}

////////////////////////////////////////////////////////////////////////////////

int GetMouseY()
{
  return MouseY;
}

////////////////////////////////////////////////////////////////////////////////

bool IsMouseButtonPressed(int button)
{
  return MouseState[button];
}

////////////////////////////////////////////////////////////////////////////////
