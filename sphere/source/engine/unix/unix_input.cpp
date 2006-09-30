#include "unix_input.h"
#include "unix_video.h"
#include "unix_internal.h"
#include <SDL.h>
#include <deque>
#include <iostream>

static std::deque<Uint8> key_queue;

// Tung: Use SDLKey, Brian.
const int total_keys = 88;
//static Uint8 KeyMapping[total_keys] = {
static SDLKey KeyMapping[total_keys] = {
  (SDLKey) 0,
  SDLK_ESCAPE,
  SDLK_F1,
  SDLK_F2,
  SDLK_F3,
  SDLK_F4,
  SDLK_F5,
  SDLK_F6,
  SDLK_F7,
  SDLK_F8,
  SDLK_F9,
  SDLK_F10,
  SDLK_F11,
  SDLK_F12,
  SDLK_BACKQUOTE,
  SDLK_0,
  SDLK_1,
  SDLK_2,
  SDLK_3,
  SDLK_4,
  SDLK_5,
  SDLK_6,
  SDLK_7,
  SDLK_8,
  SDLK_9,
  SDLK_MINUS,
  SDLK_EQUALS,
  SDLK_BACKSPACE,
  SDLK_TAB,
  SDLK_a,
  SDLK_b,
  SDLK_c,
  SDLK_d,
  SDLK_e,
  SDLK_f,
  SDLK_g,
  SDLK_h,
  SDLK_i,
  SDLK_j,
  SDLK_k,
  SDLK_l,
  SDLK_m,
  SDLK_n,
  SDLK_o,
  SDLK_p,
  SDLK_q,
  SDLK_r,
  SDLK_s,
  SDLK_t,
  SDLK_u,
  SDLK_v,
  SDLK_w,
  SDLK_x,
  SDLK_y,
  SDLK_z,
  SDLK_LSHIFT,
  SDLK_LCTRL,
  SDLK_LALT,
  SDLK_SPACE,
  SDLK_LEFTBRACKET,
  SDLK_RIGHTBRACKET,
  SDLK_SEMICOLON,
  SDLK_QUOTE,
  SDLK_COMMA,
  SDLK_PERIOD,
  SDLK_SLASH,
  SDLK_BACKSLASH,
  SDLK_RETURN,
  SDLK_INSERT,
  SDLK_DELETE,
  SDLK_HOME,
  SDLK_END,
  SDLK_PAGEUP,
  SDLK_PAGEDOWN,
  SDLK_UP,
  SDLK_RIGHT,
  SDLK_DOWN,
  SDLK_LEFT,
  SDLK_KP0,
  SDLK_KP1,
  SDLK_KP2,
  SDLK_KP3,
  SDLK_KP4,
  SDLK_KP5,
  SDLK_KP6,
  SDLK_KP7,
  SDLK_KP8,
  SDLK_KP9
};

static bool KeyBuffer[MAX_KEY];
static bool CurrentKeyBuffer[MAX_KEY];

///////////////////////////////////////////////////////////

void InitializeInput() {
  memset(KeyBuffer, false, sizeof(bool) * MAX_KEY);
  memset(CurrentKeyBuffer, false, sizeof(bool) * MAX_KEY);
}

///////////////////////////////////////////////////////////

bool RefreshInput ()
{
  // update currently pressed keys
  memcpy(KeyBuffer, CurrentKeyBuffer, sizeof(bool) * MAX_KEY);
}

///////////////////////////////////////////////////////////

bool ResetInput ()
{
  memset(KeyBuffer, false, sizeof(bool) * MAX_KEY);
  memset(CurrentKeyBuffer, false, sizeof(bool) * MAX_KEY);
  return true;
}

///////////////////////////////////////////////////////////

void UpdateSystem()
{
  SDL_Event event;
  
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT)
      exit(0);
    else if ((event.type == SDL_KEYDOWN) || (event.type == SDL_KEYUP))
    {
      // Tung: Use SDLKey, Brian.
      //Uint8 pressed = event.key.keysym.sym;
      SDLKey pressed = event.key.keysym.sym;
      int key = 0;

      switch (pressed) {
        case SDLK_SPACE:  key = KEY_SPACE;  break;
        case SDLK_ESCAPE: key = KEY_ESCAPE; break;
        case SDLK_LSHIFT:	// Tung: Redundant?
        case SDLK_RSHIFT: key = KEY_SHIFT;  break;
        case SDLK_LALT:
        case SDLK_RALT:   key = KEY_ALT;    break;
        case SDLK_LCTRL:
        case SDLK_RCTRL:  key = KEY_CTRL;   break;
        case SDLK_F1:     key = KEY_F1;     break;
        case SDLK_F2:     key = KEY_F2;     break;
        case SDLK_F3:     key = KEY_F3;     break;
        case SDLK_F4:     key = KEY_F4;     break;
        case SDLK_F5:     key = KEY_F5;     break;
        case SDLK_F6:     key = KEY_F6;     break;
        case SDLK_F7:     key = KEY_F7;     break;
        case SDLK_F8:     key = KEY_F8;     break;
        case SDLK_F9:     key = KEY_F9;     break;
        case SDLK_F10:    key = KEY_F10;    break;
        case SDLK_F11:    key = KEY_F11;    break;
        case SDLK_F12:    key = KEY_F12;    break;
                                                                                
        default:
          //std::cerr << "code: " << (SDLKey)pressed << std::endl;
          for (int lcv = 1; lcv < total_keys && key == 0; lcv++) {
            if (pressed == KeyMapping[lcv]) {
              key = lcv;
              //std::cerr << "key: "<< (int)key << "|" << (SDLKey)SDLK_LCTRL << std::endl;
              break;
            }
          }
      };
      
      if (key != 0) {
        if (event.type == SDL_KEYDOWN) {
          OnKeyDown(key);
        } else {
          OnKeyUp(key);
        }
      }
    }
  }

}

///////////////////////////////////////////////////////////

void OnKeyDown(int key) {
  //std::cerr << "down: " << key << std::endl;
  key_queue.push_back(key);
  CurrentKeyBuffer[key] = true;
          
  switch(key) {

    case KEY_F10:
      ToggleFullscreen();
    break;
                                
    case KEY_F11:
      ToggleFPS();
    break;

    case KEY_F12:
      ShouldTakeScreenshot = true;
    break;
  };
}

///////////////////////////////////////////////////////////

void OnKeyUp(int key) {
  //std::cerr << "up:   " << key << std::endl;
  CurrentKeyBuffer[key] = false;
}

///////////////////////////////////////////////////////////

bool IsKeyPressed (int key) {
  /* 
  Uint8* key_state;
  SDL_PumpEvents();
  key_state = SDL_GetKeyState(NULL);
  return key_state[KeyMapping[key]];
  */
  
  if (key >= 0 && key < MAX_KEY) {
    UpdateSystem();
    return KeyBuffer[key];
  }
  
  return false;
}

///////////////////////////////////////////////////////////

void GetKeyStates (bool keys[MAX_KEY]) {

  UpdateSystem();
  for (int i = 0; i < MAX_KEY; ++i) {
    keys[i] = KeyBuffer[i];
  }

}

///////////////////////////////////////////////////////////

bool AreKeysLeft () {
  UpdateSystem();
  return !key_queue.empty();
}

///////////////////////////////////////////////////////////

int GetKey () {
  int key;

  UpdateSystem();
  while (key_queue.empty()) {
    UpdateSystem();
  }

  key = key_queue.front();
  key_queue.pop_front();
  return key;
}

// MOUSE SUPPORT

///////////////////////////////////////////////////////////


void SetMousePosition (int x, int y) {
  SDL_WarpMouse(x, y);
}

int GetMouseX () {
  int x, dummy;

  SDL_PumpEvents();
  SDL_GetMouseState(&x, &dummy);
  return x;
}

int GetMouseY () {
  int y, dummy;

  SDL_PumpEvents();
  SDL_GetMouseState(&dummy, &y);
  return y;
}

bool IsMouseButtonPressed (int button) {
  int dummy;

  SDL_PumpEvents();
  if (SDL_GetMouseState(&dummy, &dummy) & SDL_BUTTON(button + 1))
    return true;
  return false;
}


// JOYSTICK SUPPORT

///////////////////////////////////////////////////////////

int GetNumJoysticks()
{
  return SDL_NumJoysticks();
}

///////////////////////////////////////////////////////////


float GetJoystickX(int joy_index)
{
  if (joy_index < 0 || joy_index >= SDL_NumJoysticks())
    return 0;

  SDL_Joystick* joy = SDL_JoystickOpen(joy_index);
  if (joy == NULL)
    return 0;

  Sint16 value = SDL_JoystickGetAxis(joy, 0);

  // GetAxis returns between -32768 to 32768 so we map that onto -1.0 to 1.0
  return ((float) value / (float) 32768.0);
}

///////////////////////////////////////////////////////////

float GetJoystickY(int joy_index)
{
  if (joy_index < 0 || joy_index >= SDL_NumJoysticks())
    return 0;

  SDL_Joystick* joy = SDL_JoystickOpen(joy_index);
  if (joy == NULL)
    return 0;

  Sint16 value = SDL_JoystickGetAxis(joy, 1);

  // GetAxis returns between -32768 to 32768 so we map that onto -1.0 to 1.0
  return ((float) value / (float) 32768.0);
}

///////////////////////////////////////////////////////////

int GetNumJoystickButtons(int joy_index)
{
  if (joy_index < 0 || joy_index >= SDL_NumJoysticks())
    return 0;

  SDL_Joystick* joy = SDL_JoystickOpen(joy_index);
  if (joy == NULL)
    return 0;

  return SDL_JoystickNumButtons(joy);
}

///////////////////////////////////////////////////////////

bool IsJoystickButtonPressed(int joy_index, int button)
{
  if (joy_index < 0 || joy_index >= SDL_NumJoysticks())
    return false;

  SDL_Joystick* joy = SDL_JoystickOpen(joy_index);
  if (joy == NULL)
    return false;

  if (button < 0 || button >= SDL_JoystickNumButtons(joy))
    return false;

  return (SDL_JoystickGetButton(joy, button) == 1);
}

///////////////////////////////////////////////////////////
