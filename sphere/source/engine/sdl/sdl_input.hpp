#ifndef SDL_INPUT_HPP
#define SDL_INPUT_HPP


#include "../../common/types.h"

#define MAX_KEY 256

// key defines
#define KEY_ESCAPE     1
#define KEY_F1         2
#define KEY_F2         3
#define KEY_F3         4
#define KEY_F4         5
#define KEY_F5         6
#define KEY_F6         7
#define KEY_F7         8
#define KEY_F8         9
#define KEY_F9         10
#define KEY_F10        11
#define KEY_F11        12
#define KEY_F12        13
#define KEY_TILDE      14
#define KEY_0          15
#define KEY_1          16
#define KEY_2          17
#define KEY_3          18
#define KEY_4          19
#define KEY_5          20
#define KEY_6          21
#define KEY_7          22
#define KEY_8          23
#define KEY_9          24
#define KEY_MINUS      25
#define KEY_EQUALS     26
#define KEY_BACKSPACE  27
#define KEY_TAB        28
#define KEY_A          29
#define KEY_B          30
#define KEY_C          31
#define KEY_D          32
#define KEY_E          33
#define KEY_F          34
#define KEY_G          35
#define KEY_H          36
#define KEY_I          37
#define KEY_J          38
#define KEY_K          39
#define KEY_L          40
#define KEY_M          41
#define KEY_N          42
#define KEY_O          43
#define KEY_P          44
#define KEY_Q          45
#define KEY_R          46
#define KEY_S          47
#define KEY_T          48
#define KEY_U          49
#define KEY_V          50
#define KEY_W          51
#define KEY_X          52
#define KEY_Y          53
#define KEY_Z          54
#define KEY_SHIFT      55
#define KEY_CTRL       56
#define KEY_ALT        57
#define KEY_SPACE      58
#define KEY_OPENBRACE  59
#define KEY_CLOSEBRACE 60
#define KEY_SEMICOLON  61
#define KEY_APOSTROPHE 62
#define KEY_COMMA      63
#define KEY_PERIOD     64
#define KEY_SLASH      65
#define KEY_BACKSLASH  66
#define KEY_ENTER      67
#define KEY_INSERT     68
#define KEY_DELETE     69
#define KEY_HOME       70
#define KEY_END        71
#define KEY_PAGEUP     72
#define KEY_PAGEDOWN   73
#define KEY_UP         74
#define KEY_RIGHT      75
#define KEY_DOWN       76
#define KEY_LEFT       77

#define MOUSE_LEFT     0
#define MOUSE_MIDDLE   1
#define MOUSE_RIGHT    2


// general  
extern bool RefreshInput();

// keyboard
extern bool IsKeyPressed(int key);
extern void GetKeyStates(bool keys[MAX_KEY]);

// keyboard key queue
extern bool AreKeysLeft();
extern int  GetKey();

// mouse
extern int  GetMouseX();
extern int  GetMouseY();
extern bool IsMouseButtonPressed(int button);
extern void SetMousePosition(int x, int y);

// joystick
extern int GetNumJoysticks();
extern float GetJoystickX(int joy); // returns value in range [-1, 1]
extern float GetJoystickY(int joy); // returns value in range [-1, 1]
extern int GetNumJoystickButtons(int joy);
extern bool IsJoystickButtonPressed(int joy, int button);

#endif
