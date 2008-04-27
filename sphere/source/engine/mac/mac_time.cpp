#include <SDL.h>

#include "mac_time.h"

Uint32 GetTime ()
{
    return SDL_GetTicks();
}
