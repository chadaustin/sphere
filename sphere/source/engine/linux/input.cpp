#include <stdio.h>
#include <string.h>
#include "types.h"
#include "input.h"
#include "internal.h"


// keyboard variables
byte CurrentKeyBuffer[1024];
static byte KeyBuffer[1024];

/*
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
*/


////////////////////////////////////////////////////////////////////////////////

bool InitInput(void)
{
  RefreshInput();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool CloseInput(void)
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool RefreshInput(void)
{
  UpdateSystem();
  memcpy(KeyBuffer, CurrentKeyBuffer, sizeof(KeyBuffer));
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool AnyKeyPressed(void)
{
  return KeyPressed(KEY_UP) ||
         KeyPressed(KEY_RIGHT) ||
         KeyPressed(KEY_DOWN) ||
         KeyPressed(KEY_LEFT) ||
         KeyPressed(KEY_PRIMARY) ||
         KeyPressed(KEY_SECONDARY) ||
         KeyPressed(KEY_TERTIARY) ||
         KeyPressed(KEY_QUATERNARY);
}

////////////////////////////////////////////////////////////////////////////////

bool KeyPressed(int key)
{
  switch (key)
  {
    case KEY_UP:         return KeyBuffer[98];
    case KEY_RIGHT:      return KeyBuffer[102];
    case KEY_DOWN:       return KeyBuffer[104];
    case KEY_LEFT:       return KeyBuffer[100];
    case KEY_PRIMARY:    return KeyBuffer[37];
    case KEY_SECONDARY:  return KeyBuffer[65];
    case KEY_TERTIARY:   return KeyBuffer[36];
    case KEY_QUATERNARY: return KeyBuffer[9];
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

void WhileKeyPressed(void)
{
  RefreshInput();
  while (AnyKeyPressed())
    RefreshInput();
}

////////////////////////////////////////////////////////////////////////////////

void WhileNotKeyPressed(void)
{
  RefreshInput();
  while (!AnyKeyPressed())
    RefreshInput();
}

////////////////////////////////////////////////////////////////////////////////
