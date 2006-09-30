#ifdef SPHERE_SDL
#include "sdl/sdl_network.hpp"
#elif defined(WIN32)
#include "win32/win32_network.hpp"
#elif defined(unix)
#include "unix/unix_network.h"
#elif defined(__linux__) || defined(__UNIX__)
#include "linux/win32_network.hpp"
#else
#error unsupported platform
#endif
