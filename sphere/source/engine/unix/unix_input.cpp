#include "unix_input.h"
#include "unix_video.h"
#include "unix_internal.h"
#include <SDL.h>
#include <deque>
#include <iostream>

static std::deque<Uint8> key_queue;

const int total_keys = 78;
static Uint8 KeyMapping[total_keys] = {
  0,
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
  SDLK_LEFT
};

static bool key_buffer[total_keys];

void InitializeInput() {
  memset(key_buffer, 0, sizeof(bool) * total_keys);
}

bool RefreshInput () {
  SDL_Event event;
  int result;
  Uint8 key = 0;

  //SDL_EnableKeyRepeat(1, 100);
  //SDL_ShowCursor(0);

  while (result = SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT)
      exit(0);
    else if ((event.type == SDL_KEYDOWN) || (event.type == SDL_KEYUP))
    {
      Uint8 pressed = event.key.keysym.sym;

      switch (pressed) {
        case SDLK_SPACE:  key = KEY_SPACE;  break;
        case SDLK_ESCAPE: key = KEY_ESCAPE; break;
        case SDLK_RSHIFT: key = KEY_SHIFT;  break;
        case SDLK_RALT:   key = KEY_ALT;    break;
        case SDLK_RCTRL:  key = KEY_CTRL;   break;
        case SDLK_F1:     key = KEY_F1;     break;
        case SDLK_F11:    key = KEY_F11;    break;
        case SDLK_F12:    key = KEY_F12;    break;
        
        default:
          for (int lcv = 0; lcv < total_keys; lcv++) {
            if (pressed == KeyMapping[lcv]) {
              key = lcv;
              break;
            }
          }
      };
      
      if (key != 0) {
        if (event.type == SDL_KEYDOWN) {
          //std::cerr << "down: " << (int)key << std::endl;
          key_queue.push_back(key);
          key_buffer[key] = true;
          
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
          
        } else {
          //std::cerr << "up:   " << (int)key << std::endl;
          key_buffer[key] = false;
        }
      }
    }
  }
  return true; /* not sure what to do, but I guess we succeeded! */
}

bool IsKeyPressed (int key) {
  /* 
  Uint8* key_state;
  SDL_PumpEvents();
  key_state = SDL_GetKeyState(NULL);
  return key_state[KeyMapping[key]];
  */
  return key_buffer[key];
}

void GetKeyStates (bool keys[MAX_KEY]) {
  for (int i = 0; i < MAX_KEY; ++i) {
    keys[i] = key_buffer[i];
  }
}

bool AreKeysLeft () {
  RefreshInput();
  return !key_queue.empty();
}

int GetKey () {
  int key;

  while (key_queue.empty()) {
    RefreshInput();
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
  SDL_Joystick* joy = SDL_JoystickOpen(joy_index);
  if (joy == NULL)
    return 0;
  return SDL_JoystickNumButtons(joy);
}

///////////////////////////////////////////////////////////

bool IsJoystickButtonPressed(int joy_index, int button)
{
  SDL_Joystick* joy = SDL_JoystickOpen(joy_index);
  if (joy == NULL)
    return false;

  return (SDL_JoystickGetButton(joy, button) == 1);
}

///////////////////////////////////////////////////////////
