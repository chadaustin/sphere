#include <windows.h>
#include <mmsystem.h>
#include <queue>
#include <stdlib.h>
#include "win32_input.hpp"
#include "win32_internal.hpp"
#include "../../common/minmax.hpp"

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

        VK_NUMPAD0,
        VK_NUMPAD1,
        VK_NUMPAD2,
        VK_NUMPAD3,
        VK_NUMPAD4,
        VK_NUMPAD5,
        VK_NUMPAD6,
        VK_NUMPAD7,
        VK_NUMPAD8,
        VK_NUMPAD9
    };
int WindowsToSphere[MAX_KEY]; // build dynamically
// keyboard state tables (accessed with virtual keys)
static byte CurrentKeyBuffer[MAX_KEY];
static byte KeyBuffer[MAX_KEY];

// keyboard key queue (virtual keys also)
static std::queue<int> KeyQueue;

// mouse
static int MouseX;
static int MouseY;
static bool MouseState[3];

// joystick
struct Joystick
{
    UINT id;
    UINT minX;
    UINT maxX;
    UINT minY;
    UINT maxY;
    UINT numButtons;

    float x;
    float y;
    UINT buttons;
};
std::vector<Joystick> Joysticks;

// These functions rely on the window handler to modify the keybuffer values
// and mouse state

////////////////////////////////////////////////////////////////////////////////
void TryJoystick(UINT id)
{
    JOYINFO ji;
    if (joyGetPos(id, &ji) != JOYERR_NOERROR)
    {
        return;
    }

    JOYCAPS jc;
    if (joyGetDevCaps(id, &jc, sizeof(jc)) != JOYERR_NOERROR)
    {
        return;
    }

    Joystick j;
    j.id = id;
    j.minX = jc.wXmin;
    j.maxX = jc.wXmax;
    j.minY = jc.wYmin;
    j.maxY = jc.wYmax;
    j.x = 0;
    j.y = 0;
    j.buttons = 0;
    j.numButtons = jc.wNumButtons;
    Joysticks.push_back(j);
}

bool InitInput(HWND window, SPHERECONFIG* config)
{
    unsigned int i;

    // build mapping from Windows to Sphere keys
    for (i = 0; i < MAX_KEY; ++i)
    {
        int k = SphereToWindows[i];
        if (k >= 0 && k < MAX_KEY)
        {
            WindowsToSphere[k] = i;
        }
    }

    SphereWindow = window;
    Config = config;

    // try to initialize joysticks
    UINT num = std::min(joyGetNumDevs(), 2U);
    UINT ids[2] = { JOYSTICKID1, JOYSTICKID2 };
    for (i = 0; i < num; ++i)
    {
        TryJoystick(ids[i]);
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool ResetInput()
{
    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CloseInput(void)
{
    return true;
}

////////////////////////////////////////////////////////////////////////////////
/*
const char* GetKeyName(const int virtual_key)
{
  struct KEY {
    const char* name;
    int virtual_key;
  } keys[] = {
    "(key) escape", 27,
    "(key) numlock", 144,
    "(numpad) 0", 96,
    "(numpad) 1", 97,
    "(numpad) 2", 98,
    "(numpad) 3", 99,
    "(numpad) 4", 100,
    "(numpad) 5", 101,
    "(numpad) 6", 102,
    "(numpad) 7", 103,
    "(numpad) 8", 104,
    "(numpad) 9", 105,
    "(arrow) left", 37,
    "(arrow) up", 38,
    "(arrow) right", 39,
    "(arrow) down", 40,
    "(number) 0", 48,
    "(number) 1", 49,
    "(number) 2", 50,
    "(number) 3", 51,
    "(number) 4", 52,
    "(number) 5", 53,
    "(number) 6", 54,
    "(number) 7", 55,
    "(number) 8", 56,
    "(number) 9", 57,
  };
  const int num_keys = sizeof(keys) / sizeof(*keys);
  const char* key_name = "unknown";
  for (int i = 0; i < num_keys; i++) {
    if (virtual_key == keys[i].virtual_key) {
      key_name = keys[i].name;
      break;
    }
  }
  return key_name;
}
*/
////////////////////////////////////////////////////////////////////////////////
void OnKeyDown(int virtual_key)
{

    if (virtual_key >= 0 && virtual_key < MAX_KEY)
    {
        int key = WindowsToSphere[virtual_key];
        CurrentKeyBuffer[key] = 1;
        KeyQueue.push(key);
        //printf ("%s [%d][%d] pressed\n", GetKeyName(virtual_key), virtual_key, key);

    }
}
////////////////////////////////////////////////////////////////////////////////
void OnKeyUp(int virtual_key)
{
    if (virtual_key >= 0 && virtual_key < MAX_KEY)
    {
        int key = WindowsToSphere[virtual_key];
        CurrentKeyBuffer[key] = 0;
        //printf ("%s [%d][%d] released\n\n", GetKeyName(virtual_key), virtual_key, key);

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

    for (unsigned int i = 0; i < Joysticks.size(); ++i)
    {
        Joystick& j = Joysticks[i];
        JOYINFO ji;
        if (joyGetPos(j.id, &ji) == JOYERR_NOERROR)
        {
            j.x = float(ji.wXpos - j.minX) / (j.maxX - j.minX) * 2 - 1;
            j.y = float(ji.wYpos - j.minY) / (j.maxY - j.minY) * 2 - 1;
            j.buttons = ji.wButtons;
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool IsKeyPressed(int key)
{
    UpdateSystem();

    // find the Sphere key code that corresponds with the virtual key
    return (KeyBuffer[key] != 0);
}

////////////////////////////////////////////////////////////////////////////////
void GetKeyStates(bool keys[MAX_KEY])
{
    UpdateSystem();
    for (int i = 0; i < MAX_KEY; ++i)
    {
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
int GetNumJoysticks()
{
    return int(Joysticks.size());
}

////////////////////////////////////////////////////////////////////////////////
float GetJoystickX(int joy)
{
    if (joy >= 0 && joy < GetNumJoysticks())
    {
        return Joysticks[joy].x;
    }
    else
    {
        return 0;
    }
}

////////////////////////////////////////////////////////////////////////////////
float GetJoystickY(int joy)
{
    if (joy >= 0 && joy < GetNumJoysticks())
    {
        return Joysticks[joy].y;
    }
    else
    {
        return 0;
    }
}

////////////////////////////////////////////////////////////////////////////////
int GetNumJoystickButtons(int joy)
{
    if (joy >= 0 && joy < GetNumJoysticks())
    {
        return Joysticks[joy].numButtons;
    }
    else
    {
        return 0;
    }
}

////////////////////////////////////////////////////////////////////////////////
bool IsJoystickButtonPressed(int joy, int button)
{
    if (joy >= 0 && joy < GetNumJoysticks())
    {
        return (Joysticks[joy].buttons & (1 << button)) != 0;
    }
    else
    {
        return 0;
    }
}

////////////////////////////////////////////////////////////////////////////////
