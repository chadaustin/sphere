#if   defined(SPHERE_SDL)
#include "sdl/sdl_network.hpp"
#if   defined(MAC)
#include "mac/mac_network.hpp"
#elif defined(WIN32)
#include "win32/win32_network.hpp"
#elif defined(unix)
#include "unix/unix_network.h"
#else
#error unsupported platform
#endif
