#include <iostream>
#include <stdlib.h>
#include <SDL.h>

#include "mac_input.h"

void QuitMessage (const char* message)
{
    std::cerr << message << std::endl;
    exit(1);
}
