// identifier too long
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include <queue>
#include <map>
#include <SDL.h>
#include "sdl_internal.hpp"
#include "sdl_input.hpp"

std::map<int, int> VirtualKeys;

// keyboard state tables (accessed with virtual keys)
static byte CurrentKeyBuffer[256];
static byte KeyBuffer[256];

// keyboard key queue (virtual keys also)
std::queue<int> KeyQueue;

int MouseX, MouseY;
bool MouseButton[3];

////////////////////////////////////////////////////////////////////////////////

bool InitInput()
{
  // possibly trap the mouse into the window only?
  // SDL_WM_GrabInput(SDL_GRAB_ON);

  // set some key repeat on to simulate multiple
  // key down presses like the win32 port :)
  // todo: make this *act* more like how win32 handles them!
  // todo: figure out why event messages are losing themselves O_o
  // SDL_EnableKeyRepeat(1, 100);

  // map the many different SDL (ugh) keys into std::map
  VirtualKeys[SDLK_ESCAPE]       = KEY_ESCAPE;
  VirtualKeys[SDLK_F1]           = KEY_F1;
  VirtualKeys[SDLK_F2]           = KEY_F2;
  VirtualKeys[SDLK_F3]           = KEY_F3;
  VirtualKeys[SDLK_F4]           = KEY_F4;
  VirtualKeys[SDLK_F5]           = KEY_F5;
  VirtualKeys[SDLK_F6]           = KEY_F6;
  VirtualKeys[SDLK_F7]           = KEY_F7;
  VirtualKeys[SDLK_F8]           = KEY_F8;
  VirtualKeys[SDLK_F9]           = KEY_F9;
  VirtualKeys[SDLK_F10]          = KEY_F10;
  VirtualKeys[SDLK_F11]          = KEY_F11;
  VirtualKeys[SDLK_F12]          = KEY_F12;
  VirtualKeys[SDLK_BACKQUOTE]    = KEY_TILDE;
  VirtualKeys[SDLK_0]            = KEY_0;
  VirtualKeys[SDLK_1]            = KEY_1;
  VirtualKeys[SDLK_2]            = KEY_2;
  VirtualKeys[SDLK_3]            = KEY_3;
  VirtualKeys[SDLK_4]            = KEY_4;
  VirtualKeys[SDLK_5]            = KEY_5;
  VirtualKeys[SDLK_6]            = KEY_6;
  VirtualKeys[SDLK_7]            = KEY_7;
  VirtualKeys[SDLK_8]            = KEY_8;
  VirtualKeys[SDLK_9]            = KEY_9;
  VirtualKeys[SDLK_MINUS]        = KEY_MINUS;
  VirtualKeys[SDLK_KP_MINUS]     = KEY_MINUS;
  VirtualKeys[SDLK_EQUALS]       = KEY_EQUALS;
  VirtualKeys[SDLK_KP_EQUALS]    = KEY_EQUALS;
  VirtualKeys[SDLK_BACKSPACE]    = KEY_BACKSPACE;
  VirtualKeys[SDLK_TAB]          = KEY_TAB;
  VirtualKeys[SDLK_a]            = KEY_A;
  VirtualKeys[SDLK_b]            = KEY_B;
  VirtualKeys[SDLK_c]            = KEY_C;
  VirtualKeys[SDLK_d]            = KEY_D;
  VirtualKeys[SDLK_e]            = KEY_E;
  VirtualKeys[SDLK_f]            = KEY_F;
  VirtualKeys[SDLK_g]            = KEY_G;
  VirtualKeys[SDLK_h]            = KEY_H;
  VirtualKeys[SDLK_i]            = KEY_I;
  VirtualKeys[SDLK_j]            = KEY_J;
  VirtualKeys[SDLK_k]            = KEY_K;
  VirtualKeys[SDLK_l]            = KEY_L;
  VirtualKeys[SDLK_m]            = KEY_M;
  VirtualKeys[SDLK_n]            = KEY_N;
  VirtualKeys[SDLK_o]            = KEY_O;
  VirtualKeys[SDLK_p]            = KEY_P;
  VirtualKeys[SDLK_q]            = KEY_Q;
  VirtualKeys[SDLK_r]            = KEY_R;
  VirtualKeys[SDLK_s]            = KEY_S;
  VirtualKeys[SDLK_t]            = KEY_T;
  VirtualKeys[SDLK_u]            = KEY_U;
  VirtualKeys[SDLK_v]            = KEY_V;
  VirtualKeys[SDLK_w]            = KEY_W;
  VirtualKeys[SDLK_x]            = KEY_X;
  VirtualKeys[SDLK_y]            = KEY_Y;
  VirtualKeys[SDLK_z]            = KEY_Z;
  VirtualKeys[SDLK_LSHIFT]       = KEY_SHIFT;
  VirtualKeys[SDLK_RSHIFT]       = KEY_SHIFT;
  VirtualKeys[SDLK_LCTRL]        = KEY_CTRL;
  VirtualKeys[SDLK_RCTRL]        = KEY_CTRL;
  VirtualKeys[SDLK_LALT]         = KEY_ALT;
  VirtualKeys[SDLK_RALT]         = KEY_ALT;
  VirtualKeys[SDLK_SPACE]        = KEY_SPACE;
  VirtualKeys[SDLK_LEFTBRACKET]  = KEY_OPENBRACE;
  VirtualKeys[SDLK_RIGHTBRACKET] = KEY_CLOSEBRACE;
  VirtualKeys[SDLK_SEMICOLON]    = KEY_SEMICOLON;
  VirtualKeys[SDLK_QUOTE]        = KEY_APOSTROPHE;
  VirtualKeys[SDLK_COMMA]        = KEY_COMMA;
  VirtualKeys[SDLK_PERIOD]       = KEY_PERIOD;
  VirtualKeys[SDLK_SLASH]        = KEY_SLASH;
  VirtualKeys[SDLK_KP_DIVIDE]    = KEY_SLASH;
  VirtualKeys[SDLK_BACKSLASH]    = KEY_BACKSLASH;
  VirtualKeys[SDLK_RETURN]       = KEY_ENTER;
  VirtualKeys[SDLK_KP_ENTER]     = KEY_ENTER;
  VirtualKeys[SDLK_INSERT]       = KEY_INSERT;
  VirtualKeys[SDLK_DELETE]       = KEY_DELETE;
  VirtualKeys[SDLK_HOME]         = KEY_HOME;
  VirtualKeys[SDLK_KP7]          = KEY_HOME;
  VirtualKeys[SDLK_END]          = KEY_END;
  VirtualKeys[SDLK_KP1]          = KEY_END;
  VirtualKeys[SDLK_PAGEUP]       = KEY_PAGEUP;
  VirtualKeys[SDLK_KP9]          = KEY_PAGEUP;
  VirtualKeys[SDLK_PAGEDOWN]     = KEY_PAGEDOWN;
  VirtualKeys[SDLK_KP3]          = KEY_PAGEDOWN;
  VirtualKeys[SDLK_UP]           = KEY_UP;
  VirtualKeys[SDLK_KP8]          = KEY_UP;
  VirtualKeys[SDLK_RIGHT]        = KEY_RIGHT;
  VirtualKeys[SDLK_KP6]          = KEY_RIGHT;
  VirtualKeys[SDLK_DOWN]         = KEY_DOWN;
  VirtualKeys[SDLK_KP2]          = KEY_DOWN;
  VirtualKeys[SDLK_LEFT]         = KEY_LEFT;
  VirtualKeys[SDLK_KP4]          = KEY_LEFT;

  MouseButton[0] = MouseButton[1] = MouseButton[2] = false;
  SDL_ShowCursor(0);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool ResetInput() {
  return true;
}

bool CloseInput()
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void OnKeyDown(int virtual_key)
{
  int key = VirtualKeys[virtual_key];
  CurrentKeyBuffer[key] = 1;

  // add the key to the key queue
  KeyQueue.push(key);
}

////////////////////////////////////////////////////////////////////////////////

void OnKeyUp(int virtual_key)
{
  int key = VirtualKeys[virtual_key];
  CurrentKeyBuffer[key] = 0;
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
  switch(button)
  {
    case SDL_BUTTON_LEFT:   MouseButton[MOUSE_LEFT] = true; break;
    case SDL_BUTTON_MIDDLE: MouseButton[MOUSE_MIDDLE] = true; break;
    case SDL_BUTTON_RIGHT:  MouseButton[MOUSE_RIGHT] = true; break;
  }
}

////////////////////////////////////////////////////////////////////////////////

void OnMouseUp(int button)
{
  switch(button)
  {
    case SDL_BUTTON_LEFT:   MouseButton[MOUSE_LEFT] = false; break;
    case SDL_BUTTON_MIDDLE: MouseButton[MOUSE_MIDDLE] = false; break;
    case SDL_BUTTON_RIGHT:  MouseButton[MOUSE_RIGHT] = false; break;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool RefreshInput()
{
  //UpdateSystem();

  memcpy(KeyBuffer, CurrentKeyBuffer, 256);
  return true;
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

bool IsKeyPressed(int key)
{
  UpdateSystem();

  return (KeyBuffer[key] != 0);
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
  //puts("Popped key.");

  return key;
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
  return MouseButton[button];
}

////////////////////////////////////////////////////////////////////////////////

void SetMousePosition(int x, int y)
{
  SDL_WarpMouse(x, y);
}

////////////////////////////////////////////////////////////////////////////////

int GetNumJoysticks() {
  return 0;
}

float GetJoystickX(int joy) { // returns value in range [-1, 1]
  return 0;
}

float GetJoystickY(int joy) { // returns value in range [-1, 1]
  return 0;
}

int GetNumJoystickButtons(int joy) {
  return 0;
}

bool IsJoystickButtonPressed(int joy, int button) {
  return false;
}

////////////////////////////////////////////////////////////////////////////////

